/*
 * SwitchTest.c
 *
 * Created: 11/18/2014 8:13:45 PM
 *  Author: waynej
 */ 

#include "../../common/config.h"

#include <avr/io.h>
#include <util/delay.h>

#include "../../common/led.h"

int main(void)
{
	LED_init();

	DDRB=0x00;		// Set switches as input
	PORTB=0xff;

	DDRC&=~0x7f;	// Set Launch Switch
	PORTC|=0x80;
	
    while(1)
    {
		int x=PINB;
		int y=PINC;
		LED_output((~y)&0x80, ~x, 0x00);			/* Turn on all red LEDs */
    }
}
