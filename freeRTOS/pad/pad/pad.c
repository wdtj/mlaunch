/*
 * pad.c
 *
 * Created: 12/23/2021 12:51:33 PM
 * Author : waynej
 */

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "config.h"
#include "pad-config.h"
#include "PadLed.h"


void init()
{
    DDRA = _BV(Launch1) | _BV(Enable1) | _BV(Launch2) | _BV(Enable2);// PA4-7 output
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Siren) | _BV(Red2) | _BV(
               Green2);// PB0-1, 3-5 are output
    PORTB = _BV(ContSW1) | _BV(ContSW2);            // Set pullup on switches

    DDRD |= _BV(7);
}

void padTask(void *parameter)
{
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    assert(0);
}

int main(void)
{
    init();

    int rc = xTaskCreate(
                 padTask,      // Function to be called
                 "pad",   // Name of task
                 350, // Stack size
                 NULL,           // Parameter to pass
                 1,              // Task priority
                 NULL);          // Created Task
    assert(rc == pdPASS); /* failure! */

    vTaskStartScheduler();

    while(0);   // should never hit here
}

