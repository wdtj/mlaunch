/*
* switchTest.c
*
* Created: 11/21/2014 3:49:31 PM
*  Author: waynej
*/

#include "config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "../pad-config.h"

void init()
{
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
           | _BV(Yellow2);  // PB0-2, 4-6 are output
    PORTB |= _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
             | _BV(Yellow2); // Turn off leds
}

void timeout(TimerHandle_t xTimer)
{
    setGreen1();
    setGreen2();
    vTaskDelay(500);
    resetGreen1();
    resetGreen2();
}

void timerTest(void *parameters)
{
    TimerHandle_t timeoutTimer = xTimerCreate("Timeout",
                                 (1000 / portTICK_RATE_MS),
                                 pdTRUE,
                                 NULL,
                                 timeout);
    if(timeoutTimer == NULL) {
        for(;;); /* failure! */
    }

    xTimerStart(timeoutTimer, 0);

    vTaskDelete(NULL);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}

int main(void)
{
    init();

    // Start blink task
    xTaskCreate(
        timerTest,      // Function to be called
        "blinky",   // Name of task
        configMINIMAL_STACK_SIZE, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task

    vTaskStartScheduler();
}
