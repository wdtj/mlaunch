/*
* ledTest.c
*
* Created: 11/21/2014 3:49:31 PM
*  Author: waynej
*/

#include "config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

#include "../pad-config.h"

void init()
{
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
           | _BV(Yellow2); // PB0-2, 4-6 are output
    PORTB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
            | _BV(Yellow2);
}


void togglePadLED(void *parameters)
{
    while(1) {
        setRed1();
        setRed2();
        vTaskDelay(1000);
        resetRed1();
        resetRed2();

        setGreen1();
        setGreen2();
        vTaskDelay(1000);
        resetGreen1();
        resetGreen2();

        setYellow1();
        setYellow2();
        vTaskDelay(1000);
        resetYellow1();
        resetYellow2();

        vTaskDelay(1000);
    }
}

int main(void)
{
    init();

    // Start blink task
    xTaskCreate(
        togglePadLED,      // Function to be called
        "Toggle Pad LED",   // Name of task
        1024, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task

    vTaskStartScheduler();
}
