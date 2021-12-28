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

#include "../pad-config.h"

void init()
{
    PORTB = _BV(ContSW1) | _BV(ContSW2);			// Set pullup on switches
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
    | _BV(Yellow2);	// PB0-2, 4-6 are output
    PORTB |= _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
    | _BV(Yellow2); // Turn off leds
}


void switchTest(void *parameters)
{
    while (1)
    {
        if (!(PINB & _BV(ContSW1)))
        {
            setRed1();
        } else {
            resetRed1();
        }

        if (!(PINB & _BV(ContSW2)))
        {
            setRed2();
        } else {
            resetRed2();
        }
    }
}

int main(void)
{
    init();

    // Start blink task
    xTaskCreate(
        switchTest,      // Function to be called
        "Toggle Pad LED",   // Name of task
        1024, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task

    vTaskStartScheduler();
}
