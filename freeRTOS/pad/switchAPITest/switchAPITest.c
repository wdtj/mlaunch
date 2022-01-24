/*
* xbeeTest.c
*
* Created: 11/21/2014 3:49:31 PM
*  Author: waynej
*/

#include "config.h"
#include "../pad-config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "uart.h"
#include <string.h>
#include "zb.h"
#include "switchAPI.h"
#include "PadLed.h"

/* Unit test to validate the switch API */

void init()
{
    PORTB = _BV(ContSW1) | _BV(ContSW2);            // Set pullup on switches

    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
           | _BV(Yellow2);  // PB0-2, 4-6 are output

    PORTB |= _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
             | _BV(Yellow2); // Turn off leds

}

/* Initialize and set up for Pad configuration */
void testSwitchTask(void *parameter)
{
    /* 2 message queue for 2 switches */
    int rc = switchInit(2, 2);
    if(rc != 0) {
        assert(rc == 0);
    }
    switchAdd('1', ContSW1, &PINB);
    switchAdd('2', ContSW2, &PINB);

    while(1) {
        SwitchMsg switchMsg;
        xQueueReceive(switchGetQueue(), &switchMsg, portMAX_DELAY);

        switch(switchMsg.id) {
        case '1':
            switch(switchMsg.switchState) {
            case SW_OPEN:
                resetGreen1();
                break;
            case SW_CLOSED:
                setGreen1();
                break;
            default:
                break;
            }
            break;
        case '2':
            switch(switchMsg.switchState) {
            case SW_OPEN:
                resetGreen2();
                break;
            case SW_CLOSED:
                setGreen2();
                break;
            default:
            break;
            }
            break;
        }
        taskYIELD();
    }
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    assert(0);
}

int main(void)
{
    init();

    int rc = xTaskCreate(
                 testSwitchTask,      // Function to be called
                 "testSwitch",   // Name of task
                 250, // Stack size
                 NULL,           // Parameter to pass
                 1,              // Task priority
                 NULL);          // Created Task
    assert(rc == pdPASS) /* failure! */

    vTaskStartScheduler();

    while(0);   // should never hit here
}

