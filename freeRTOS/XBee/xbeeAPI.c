/*
 * XBee.c
 *
 * Created: 12/31/2021 12:36:08 PM
 * Author : waynej
 */

#include "config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "uart.h"
#include <string.h>
#include "zb.h"

#define XB_TIMEOUT 10000

void(*dataCallback)();
void(*errorCallback)(int code);

TimerHandle_t timeoutTimer;

volatile static enum {
    Idle,
    NodeIdentity,
    ChannelVerify,
    JoinNotifications,
    NetworkWatchdog,
    GetChannel,
    GetReceivedSignalStrength,
    NodeDiscovery,
    txInProgress
} portState = NodeIdentity;

unsigned int channel;
unsigned int signalStrength;
static char name[46];

SemaphoreHandle_t xbeeBusy = NULL;

void timeout(TimerHandle_t xTimer)
{
    (*errorCallback)(portState);
    portState = NodeIdentity;
}

void handleATResp(struct zbATResponse *resp, int length)
{
    switch(portState) {
    // We have sent the ni packet, now we have a response
    case NodeIdentity: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_jv(2, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                portState = ChannelVerify;
            }
            break;
        }

    // We have sent the Join Verify packet, now we have a response
    case ChannelVerify: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_jn(3, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                portState = JoinNotifications;
                break;
            }
        }

    // We have sent the Join Notifications packet, now we have a response
    case JoinNotifications: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_nw(4, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                portState = NetworkWatchdog;
            }
            break;
        }

    // We have sent the Network Watchdog Timeout packet, now we have a response
    case NetworkWatchdog: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_ch(5);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                portState = GetChannel;
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case GetChannel: {
            if(resp->status == ZB_AT_STATUS_OK) {
                channel = (resp->data[1] << 8) | (resp->data[0]);
                zb_db(6);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                portState = GetReceivedSignalStrength;
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case GetReceivedSignalStrength: {
            if(resp->status == ZB_AT_STATUS_OK) {
                signalStrength = (resp->data[1] << 8) | (resp->data[0]);
                xTimerDelete(timeoutTimer, pdMS_TO_TICKS(XB_TIMEOUT));
                portState = Idle;
                xSemaphoreGive(xbeeBusy);
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case NodeDiscovery: {
            if(resp->status == ZB_AT_STATUS_OK) {
                struct ATNDData *nid = (struct ATNDData *) resp->data;
                strncpy(name, (char *)nid->ni, sizeof name);
            }
            xSemaphoreGive(xbeeBusy);
            break;
        }

    // Initial Sequence is complete
    default: {
            while(1);
            break;
        }
    }
}

void handleModemStatus(struct zbModemStatus *resp, int length)
{
    while(1);
}

void handleTransmitStatus(struct zbTransmitStatus *resp, int length)
{
    portState = Idle;
}

void handleReceivePacket(struct zbRx *resp, int length)
{
    int headerLength = ((char *)resp->data) - ((char *)resp);
    (*dataCallback)(resp->data, length - headerLength);
}

void handleExplicitRx(struct zbExpRx *resp, int length)
{
    while(1);
}

void handleNodeID(struct zbNID *resp, int length)
{
    while(1);
}

void handleRouteRecord(struct zbRR *resp, int length)
{
    while(1);
}


void xbeeReceivePacket(unsigned char *pkt, unsigned int length)
{
    if(pkt[0] == ZB_AT_COMMAND_RESPONSE) {
        handleATResp((struct zbATResponse *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_MODEM_STATUS) {
        handleModemStatus((struct zbModemStatus *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_TRANSMIT_STATUS) {
        handleTransmitStatus((struct zbTransmitStatus *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_RECEIVE_PACKET) {
        handleReceivePacket((struct zbRx *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_EXPLICIT_RX_INDICATOR) {
        handleExplicitRx((struct zbExpRx *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_NODE_IDENTIFICATION) {
        handleNodeID((struct zbNID *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_ROUTE_RECORD) {
        handleRouteRecord((struct zbRR *)(pkt + 1), length - 1);
    }
}

void xbeeTask(void *parameter)
{
    timeoutTimer = xTimerCreate("Timeout",
                                (XB_TIMEOUT / portTICK_RATE_MS),
                                pdFALSE,
                                NULL,
                                timeout);
    if(timeoutTimer == NULL) {
        for(;;); /* failure! */
    }

    zb_ni(1, "test02");
    xTimerStart(timeoutTimer, 0);

    while(1) {
        while(uart_rxReady()) {
            zbReceive(uart_rxc());
        }
    }
}

void networkDiscovery()
{
    while(!xSemaphoreTake(xbeeBusy, 0));
    zb_nd(10);
    portState = NodeDiscovery;
    vTaskDelay(10000);
    xSemaphoreGive(xbeeBusy);
}

void xbeeWait()
{
    while(!xSemaphoreTake(xbeeBusy, 0));
    xSemaphoreGive(xbeeBusy);
}


void xbeeTx(char *msg, int length, zbAddr controllerAddress,
            zbNetAddr controllerNAD)
{
    while(!xSemaphoreTake(xbeeBusy, 0)) {
        taskYIELD();
    }
    portState = txInProgress;
    zb_tx(11, controllerAddress, controllerNAD, 0, 0, msg, length);
    while(portState == txInProgress) {
        taskYIELD();
    }
    xSemaphoreGive(xbeeBusy);
}

int xbeeFSMInit(int baud, int txQueueSize, int rxQueueSize,
                void(*data)(),
                void(*error)(int code))
{
    dataCallback = data;
    errorCallback = error;

    xbeeBusy = xSemaphoreCreateBinary();
    if(xbeeBusy == NULL) {
        for(;;); /* failure! */
    }
    xSemaphoreGive(xbeeBusy);

    xSemaphoreTake(xbeeBusy, 0);

    uart_init(baud, txQueueSize, rxQueueSize);

    zbInit(&uart_txBuff, &xbeeReceivePacket);

    int success = xTaskCreate(
                      xbeeTask,      // Function to be called
                      "xbeeTask",   // Name of task
                      128, // Stack size
                      NULL,           // Parameter to pass
                      1,              // Task priority
                      NULL);          // Created Task

    return success;
}

