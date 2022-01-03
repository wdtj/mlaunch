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
#include "xbeeAPI.h"

void handleData()
{
    setGreen1();
    setGreen2();
}

void handleError(int code)
{
    resetRed1();
    resetRed2();
}

void init()
{
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
           | _BV(Yellow2); // PB0-2, 4-6 are output
    PORTB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
            | _BV(Yellow2);
}

void testXbeeTask(void *parameter)
{
    xbeeFSMInit(UART_BAUD, 80, 80, handleData, handleError);

    //networkDiscovery();

}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}

int main(void)
{
    init();

    xTaskCreate(
        testXbeeTask,      // Function to be called
        "testXbee",   // Name of task
        128, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task

    vTaskStartScheduler();

    while(1);   // should never hit here
}

