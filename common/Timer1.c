/*
 * Timer1.c
 *
 * Created: 3/7/2014 8:47:13 PM
 *  Author: waynej
 */ 

#include <avr/interrupt.h>
#include "dispatcher.h"
#include "timer1.h"

void (*timer1Ptr)(void);

void timer1_init(int ps, void (*ptr)(void))
{
	timer1Ptr=ptr;	/* Set interrupt handler */
	
	/* Set prescaler and CTC (Clear Timer on Compare Match) */
    TCCR1A=0;
    TCCR1B=(ps<<CS10)|_BV(WGM12);
}

void timer1_set(unsigned int tc)
{
  TIMSK&=~_BV(OCIE1A);
  TIFR&=~_BV(OCF1A);

  OCR1A=tc;    /* We count to 0xffff so take complement */

  TIMSK|=_BV(OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
	(*timer1Ptr)();
}