/*
 * Led.c
 *
 * Created: 2/26/2015 8:41:30 PM
 *  Author: waynej
 */ 

#include "PadLed.h""

#define Red1 _BV(PORTB0)
#define Green1 _BV(PORTB1)
#define Yellow1 _BV(PORTB2)
#define Red2 _BV(PORTB4)
#define Green2 _BV(PORTB5)
#define Yellow2 _BV(PORTB6)

static int red[]={Red1, Red2};
static int green[]={Green1, Green2};
static int yellow[]={Yellow1, Yellow2};

void PadLedInit()
{
	PORTB|=Red1|Green1|Yellow1|Red2|Green2|Yellow2;
	DDRB|=Red1|Green1|Yellow1|Red2|Green2|Yellow2;	// PB0-2, 4-6 are output
}

void PadLed(int color, int number)
{
	switch(color)
	{
		case RED:
			PORTB&=~(red[number]); PORTB|=green[number]|yellow[number];
			break;
		case GREEN:
			PORTB&=~(green[number]); PORTB|=red[number]|yellow[number];
			break;
		case YELLOW:
			PORTB&=~(yellow[number]); PORTB|=green[number]|red[number];
			break;
		case OFF:
			PORTB|=yellow[number]|green[number]|red[number];
		break;
	}
}
