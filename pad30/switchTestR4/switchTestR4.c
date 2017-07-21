/*
 * switchTest.c
 *
 * Created: 11/21/2014 4:06:38 PM
 *  Author: waynej
 */

#include "../../common/config.h"

#include <avr/io.h>

#define Red1 PORTB0
#define Green1 PORTB1
#define Yellow1 PORTB2
#define Red2 PORTB4
#define Green2 PORTB5
#define Yellow2 PORTB6

#define ContSW1 PORTB3
#define ContSW2 PORTB7

#define setRed1()     reset(PORTB, Red1)
#define resetRed1()   set(PORTB, Red1)
#define setGreen1()   reset(PORTB, Green1)
#define resetGreen1() set(PORTB, Green1)
#define setYellow1()   reset(PORTB, Yellow1)
#define resetYellow1() set(PORTB, Yellow1)

#define setRed2()     reset(PORTB, Red2)
#define resetRed2()   set(PORTB, Red2)
#define setGreen2()   reset(PORTB, Green2)
#define resetGreen2() set(PORTB, Green2)
#define setYellow2()   reset(PORTB, Yellow2)
#define resetYellow2() set(PORTB, Yellow2)

int main(void)
{
    PORTB = _BV(ContSW1) | _BV(ContSW2);			// Set pullup on switches
    DDRB = _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
            | _BV(Yellow2);	// PB0-2, 4-6 are output
    PORTB |= _BV(Red1) | _BV(Green1) | _BV(Yellow1) | _BV(Red2) | _BV(Green2)
            | _BV(Yellow2); // Turn off leds

    while (1)
    {
        if (!(PINB & _BV(ContSW1)))
        {
            setRed1()
            ;
        } else
        {
            resetRed1()
            ;
        }

        if (!(PINB & _BV(ContSW2)))
        {
            setRed2()
            ;
        } else
        {
            resetRed2()
            ;
        }
    }
}
