/*
 * led.c
 *
 * Created: 2/2/2014 10:44:30 AM
 *  Author: waynej
 */ 

#include "config.h"

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>

#define LED_CLK PORTD,PORTD5
#define LED_SDI PORTD,PORTD7
#define LED_LE (PORTD),(PORT6)
#define LED_SDO PORTC,PORTC0

void LED_output_byte(unsigned char output);

void LED_init(void)
{
	set(DDRD, PORT5);
	set(DDRD, PORT6);
	set(DDRD, PORT7);
	
	reset(DDRC, PORT0);
}

#define LED_DELAY() _NOP()

void LED_output(unsigned char r, unsigned char g, unsigned char y)
{
	LED_output_byte(y);
	LED_output_byte(g);
	LED_output_byte(r);

	/* Pulse LE to latch values to display */
	set(PORTD, PORT6);
	LED_DELAY();
	reset(PORTD, PORT6);
	LED_DELAY();
}

/* Shift out the byte on SDI and pulse CLK to shift it in */
void LED_output_byte(unsigned char output)
{
	for (int count=0; count < 8; ++count)
	{
		// We output the MSB first, then shift left
		if(((output & 0x80)==0x80))
		{
			set(PORTD, PORT7);
		}
		else
		{
			reset(PORTD, PORT7);
		}
		
		output=output<<1;
		LED_DELAY();

		set(PORTD, PORT5);
		LED_DELAY();
		
		reset(PORTD, PORT5);
		LED_DELAY();
	}
}

