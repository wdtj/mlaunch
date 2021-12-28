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

#include "uart.h"
#include <string.h> 

void init()
{
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
    | _BV(Yellow2);	// PB0-2, 4-6 are output
    PORTB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
    | _BV(Yellow2);

    uart_init(UART_BAUD, 80, 80);

    //zbInit(&zbWrite, &linkPkt);

    sei();								// enable interrupts
}

void sendXbee(void *parameter)
{
    unsigned char testIn[20];
    memset(testIn, 0, sizeof testIn);


    //zb_ni(1, "XbeeTest");
    unsigned char test[]={0x7E, 0x00, 0x04, 0x08, 0x01, 0x4e, 0x49, 0x5f};
    int len;

    while(1)
    {
        uart_txBuff(test, sizeof test);
        uart_rxBuff(testIn, sizeof testIn, &len, 1000);
        if (len == 10)
        {
            setGreen1();
            setGreen2();
            vTaskDelay(1000);
            resetGreen1();
            resetGreen2();
        }
        else
        {
            setRed1();
            setRed2();
            vTaskDelay(1000);
            resetRed1();
            resetRed2();
        }
    }
}


int main(void)
{
    init();
    
    xTaskCreate(
    sendXbee,      // Function to be called
    "SendXbee",   // Name of task
    1024, // Stack size
    NULL,           // Parameter to pass
    1,              // Task priority
    NULL);          // Created Task

    vTaskStartScheduler();
}

