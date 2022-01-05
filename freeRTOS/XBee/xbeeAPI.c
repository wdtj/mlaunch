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

#include "xbeeAPI.h"
#include "uart.h"
#include <string.h>
#include "zb.h"

#define XB_TIMEOUT 10000

void(*dataCallback)(char *data, int length);
void(*errorCallback)(int code, int state);

TimerHandle_t timeoutTimer;

/*
 *  FSM State
 */
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
} apiState = NodeIdentity;

/* xbee statistics */
static unsigned int channel;
static unsigned int signalStrength;
static char name[20];
static char *nodeID = "testNode";

/* Semaphore to guard FSM integrity.
 * This is taken when the FSM starts a sequence and given when complete.
 */
SemaphoreHandle_t xbeeBusy = NULL;

/*
 *  Start up FSM for initial sequence.
 */
void xbeeInit(char *ni)
{
    apiState = NodeIdentity;
    zb_ni(1, ni);
    //xTimerStart(timeoutTimer, 0);
}

/*
 * Timeout waiting for a response.
 *
 * Call error callback with code and state, then reset link
 */

void timeout(TimerHandle_t xTimer)
{
    (*errorCallback)(XBEE_TIMEOUT, apiState);
    xbeeInit(nodeID);
}

/* Handle an AT Response */
void handleATResp(struct zbATResponse *resp, int length)
{
    switch(apiState) {
    // We have sent the ni packet, now we have a response
    case NodeIdentity: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_jv(2, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = ChannelVerify;
            }
            break;
        }

    // We have sent the Join Verify packet, now we have a response
    case ChannelVerify: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_jn(3, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = JoinNotifications;
            }
            break;
        }

    // We have sent the Join Notifications packet, now we have a response
    case JoinNotifications: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_nw(4, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = NetworkWatchdog;
            }
            break;
        }

    // We have sent the Network Watchdog Timeout packet, now we have a response
    case NetworkWatchdog: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_ch(5);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = GetChannel;
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case GetChannel: {
            if(resp->status == ZB_AT_STATUS_OK) {
                channel = (resp->data[1] << 8) | (resp->data[0]);
                zb_db(6);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = GetReceivedSignalStrength;
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case GetReceivedSignalStrength: {
            if(resp->status == ZB_AT_STATUS_OK) {
                signalStrength = (resp->data[1] << 8) | (resp->data[0]);
                xTimerDelete(timeoutTimer, pdMS_TO_TICKS(XB_TIMEOUT));
                apiState = Idle;
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

    // We have received a packet we are not prepared for.
    default: {
            assert(0);

            break;
        }
    }
}

/*
 * Handle Modem Status message.
 */
void handleModemStatus(struct zbModemStatus *resp, int length)
{
    assert(0);      // TODO
}

/*
 * Handle Transmit Status message.
 *
 * This indicates that a transmission is complete.
 */
void handleTransmitStatus(struct zbTransmitStatus *resp, int length)
{
    apiState = Idle;
}

/*
 * Handle Receive message.
 *
 * This indicates that data has been received.  Pass to user.
 */
void handleReceivePacket(struct zbRx *resp, int length)
{
    int headerLength = ((char *)resp->data) - ((char *)resp);
    (*dataCallback)((char *)resp->data, length - headerLength);
}

/*
 * Handle Explicit Receive message.
 *
 * This indicates that data has been received.  Pass to user.
 */
void handleExplicitRx(struct zbExpRx *resp, int length)
{
    int headerLength = ((char *)resp->data) - ((char *)resp);
    (*dataCallback)((char *)resp->data, length - headerLength);
}

void handleNodeID(struct zbNID *resp, int length)
{
    assert(0);      // TODO
}

void handleRouteRecord(struct zbRR *resp, int length)
{
    assert(0);      // TODO
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

void networkDiscovery()      // TODO
{
    while(!xSemaphoreTake(xbeeBusy, 0));
    zb_nd(10);
    apiState = NodeDiscovery;
    vTaskDelay(10000);
    xSemaphoreGive(xbeeBusy);
}

/* API call to wait for the FSM guard */
void xbeeWait()
{
    while(!xSemaphoreTake(xbeeBusy, 0));
    xSemaphoreGive(xbeeBusy);
}

/* API call to Transmit data */
void xbeeTx(char *msg, int length, zbAddr controllerAddress,
            zbNetAddr controllerNAD)
{
    while(!xSemaphoreTake(xbeeBusy, 0)) {
        taskYIELD();
    }
    apiState = txInProgress;
    zb_tx(11, controllerAddress, controllerNAD, 0, 0, msg, length);
    while(apiState == txInProgress) {
        taskYIELD();
    }
    xSemaphoreGive(xbeeBusy);
}

/*
 * Main xbee task
 */
void xbeeTask(void *parameter)
{
    timeoutTimer = xTimerCreate("Timeout",
                                (XB_TIMEOUT / portTICK_RATE_MS),
                                pdFALSE,
                                NULL,
                                timeout);
    assert(timeoutTimer != NULL)  /* failure! */

    xbeeInit(nodeID);

    while(1) {
        while(uart_rxReady()) {
            zbReceive(uart_rxc());
        }
    }
}

//#define XBEE_TRACE_WRITE
#ifdef XBEE_TRACE_WRITE
char writeTrace[80];
char end[]="yy";
char *writeTracePtr = writeTrace;


void xbee_write(char *buff, int size)
{
    char *ptr;
    int count;

    for(ptr = buff, count = size; count > 0; --count) {
        *(writeTracePtr++) = *(ptr++);
        if(writeTracePtr > writeTrace + sizeof writeTrace) {
            writeTracePtr = writeTrace;
        }
    }
    uart_txBuff(buff, size);
}
#endif

/* API call to Initialize the xbee modem */
int xbeeFSMInit(
    int baud,           // Baud rate (Bits per second)
    int txQueueSize,    // Transmit queue size
    int rxQueueSize,    // Receive queue size
    void(*data)(),      // Data received callback
    void(*error)(int, int))  // Error received callback
{
    dataCallback = data;
    errorCallback = error;

    xbeeBusy = xSemaphoreCreateBinary();
    assert(xbeeBusy != NULL);
    xSemaphoreGive(xbeeBusy);

    xSemaphoreTake(xbeeBusy, 0);

    uart_init(baud, txQueueSize, rxQueueSize);

#ifdef XBEE_TRACE_WRITE
    zbInit(&xbee_write, &xbeeReceivePacket);
#else
    zbInit(&uart_txBuff, &xbeeReceivePacket);
#endif

    int rc = xTaskCreate(
                 xbeeTask,      // Function to be called
                 "xbeeTask",   // Name of task
                 150, // Stack size
                 NULL,           // Parameter to pass
                 1,              // Task priority
                 NULL);          // Created Task

    assert(rc == pdPASS) /* failure! */

    return rc;
}

