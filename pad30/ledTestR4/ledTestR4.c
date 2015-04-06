/*
 * ledTest.c
 *
 * Created: 11/21/2014 3:49:31 PM
 *  Author: waynej
 */ 

#include "../../common/config.h"

#include <avr/io.h>
#include <util/delay.h>

#define Red1 PORTB0
#define Green1 PORTB1
#define Yellow1 PORTB2
#define Red2 PORTB4
#define Green2 PORTB5
#define Yellow2 PORTB6

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
	DDRB=_BV(Red1)|_BV(Green1)|_BV(Yellow1)|_BV(Red2)|_BV(Green2)|_BV(Yellow2);	// PB0-2, 4-6 are output
	PORTB=_BV(Red1)|_BV(Green1)|_BV(Yellow1)|_BV(Red2)|_BV(Green2)|_BV(Yellow2);
    while(1)
    {
		setRed1();
		setRed2();
		_delay_ms(1000);
		resetRed1();
		resetRed2();

		setGreen1();
		setGreen2();
		_delay_ms(1000);
		resetGreen1();
		resetGreen2();

		setYellow1();
		setYellow2();
		_delay_ms(1000);
		resetYellow1();
		resetYellow2();

		_delay_ms(1000);
    }
}