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

#include "uart.h"
#include <string.h>
#include "zb.h"

#define XB_TIMEOUT 10000

void(*dataCallback)();
void(*errorCallback)(int code);

TimerHandle_t timeoutTimer;

static enum {
    NodeIdentity,
    ChannelVerify,
    JoinNotifications,
    GetChannel,
    GetReceivedSignalStrength,
    Idle

} portState = NodeIdentity;


void timeout(TimerHandle_t xTimer)
{
    (*errorCallback)(1);
    portState = NodeIdentity;
 }

void xbeeReceivePacket(unsigned char *pkt, unsigned int length)
{
    switch(portState) {
        // We have sent the ni packet, now we have a response
        case NodeIdentity: {
            struct zbATResponse *resp = (struct zbATResponse *)pkt;
            if(resp->status != ZB_AT_STATUS_OK) {
                zb_jv(2, 1);
                xTimerStart(timeoutTimer, 0);
                portState = ChannelVerify;
            }
            break;
        }
        case ChannelVerify: {
            struct zbATResponse *resp = (struct zbATResponse *)pkt;
            if(resp->status != ZB_AT_STATUS_OK) {
                zb_jn(3, 1);
                xTimerStart(timeoutTimer, 0);
                portState = JoinNotifications;
                break;
            }
        }
        case JoinNotifications: {
            struct zbATResponse *resp = (struct zbATResponse *)pkt;
            if(resp->status != ZB_AT_STATUS_OK) {
                zb_nw(4, 1);
                xTimerStart(timeoutTimer, 0);
                portState = GetChannel;
            }
            break;
        }
        case GetChannel: {
            struct zbATResponse *resp = (struct zbATResponse *)pkt;
            if(resp->status != ZB_AT_STATUS_OK) {
                zb_ch(5);
                xTimerStart(timeoutTimer, 0);
                portState = GetReceivedSignalStrength;
            }
            break;
        }
        case GetReceivedSignalStrength: {
            struct zbATResponse *resp = (struct zbATResponse *)pkt;
            if(resp->status != ZB_AT_STATUS_OK) {
                zb_ch(5);
                xTimerStart(timeoutTimer, 0);
                portState = Idle;
            }
            break;
        }
        case Idle: {
            xTimerDelete(timeoutTimer, 0);
            (*dataCallback)();
            break;
        }
    }
}


void sendXbee(void *parameter)
{
    timeoutTimer = xTimerCreate("Timeout",
                                pdMS_TO_TICKS(XB_TIMEOUT),
                                pdFALSE,
                                NULL,
                                timeout);
    if(timeoutTimer == NULL) {
        for(;;); /* failure! */
    }

    zb_ni(1, "test02");
    xTimerStart(timeoutTimer, pdMS_TO_TICKS(XB_TIMEOUT));

    while(1) {
        while(uart_rxReady()) {
            zbReceive(uart_rxc());
        }
    }
}

/* Replace with your library code */
int xbeeFSMInit(int baud, int txQueueSize, int rxQueueSize, void(*data)(), void(*error)(int code))
{
    dataCallback=data;
    errorCallback=error;

    uart_init(baud, txQueueSize, rxQueueSize);

    zbInit(&uart_txBuff, &xbeeReceivePacket);

    return xTaskCreate(
        sendXbee,      // Function to be called
        "SendXbee",   // Name of task
        configMINIMAL_STACK_SIZE, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task
}

