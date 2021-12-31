/*
* xbeeTest.c
*
* Created: 11/21/2014 3:49:31 PM
*  Author: waynej
*/

#include "config.h"
#include "../pad-config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "uart.h"
#include <string.h>
#include "zb.h"

#define ZB_TIMEOUT 10000

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
    portState = NodeIdentity;
    resetRed1();
    resetRed2();
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
            setGreen1();
            setGreen2();
            break;
        }
    }
}


void init()
{
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
           | _BV(Yellow2); // PB0-2, 4-6 are output
    PORTB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
            | _BV(Yellow2);

    uart_init(UART_BAUD, 80, 80);

    zbInit(&uart_txBuff, &xbeeReceivePacket);

    sei();                              // enable interrupts
}

void sendXbee(void *parameter)
{
    timeoutTimer = xTimerCreate("Timeout",
                                pdMS_TO_TICKS(ZB_TIMEOUT),
                                pdFALSE,
                                NULL,
                                timeout);
    if(timeoutTimer == NULL) {
        for(;;); /* failure! */
    }

    zb_ni(1, "test02");
    xTimerStart(timeoutTimer, pdMS_TO_TICKS(ZB_TIMEOUT));

    while(1) {
        while(uart_rxReady()) {
            zbReceive(uart_rxc());
        }
    }

}


int main(void)
{
    init();

    xTaskCreate(
        sendXbee,      // Function to be called
        "SendXbee",   // Name of task
        configMINIMAL_STACK_SIZE, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task

    vTaskStartScheduler();
}

