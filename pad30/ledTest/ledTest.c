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
#define Red2 PORTB4
#define Green2 PORTB5

#define setRed1()     set(PORTB, Red1)
#define resetRed1()   reset(PORTB, Red1)
#define setGreen1()   set(PORTB, Green1)
#define resetGreen1() reset(PORTB, Green1)
#define setYellow1()   set(PORTB, Green1); set(PORTB, Red1)
#define resetYellow1() reset(PORTB, Green1); reset(PORTB, Red1)

#define setRed2()     set(PORTB, Red2)
#define resetRed2()   reset(PORTB, Red2)
#define setGreen2()   set(PORTB, Green2)
#define resetGreen2() reset(PORTB, Green2)
#define setYellow2()   set(PORTB, Green2); set(PORTB, Red2)
#define resetYellow2() reset(PORTB, Green2); reset(PORTB, Red2)

#define Siren PORTB3

int main(void)
{
	DDRB=_BV(Red1)|_BV(Green1)|_BV(Siren)|_BV(Red2)|_BV(Green2);	// PB0-1, 3-5 are output

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