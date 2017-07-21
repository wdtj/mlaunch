/*
 * Timer0.c
 *
 * Created: 1/27/2014 12:52:19 PM
 *  Author: waynej
 */

#include <avr/interrupt.h>
#include "dispatcher.h"
#include "timer0.h"
#include "config.h"

void (*timer0Ptr)(void);

void timer0_init(int ps, void (*ptr)(void))
{
    timer0Ptr = ptr; /* Set interrupt handler */

    /* Set prescaler and CTC (Clear Timer on Compare Match) */
    TCCR0 = (ps << CS00) | _BV(WGM01);
}

void timer0_set(unsigned int tc)
{
    TIMSK &= ~_BV(OCIE0);
    TIFR &= ~_BV(OCF0);

    OCR0 = tc; /* We count to 0xffff so take complement */

    TIMSK |= _BV(OCIE0);
}

volatile static unsigned int maxT0Time = 0;

ISR( TIMER0_COMP_vect)
{
    unsigned int start = TCNT1;
    (*timer0Ptr)();

    maxT0Time = MAX(maxT0Time, TCNT1 - start);
}
