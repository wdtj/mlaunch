/*
 * ADCTest.c
 *
 * Created: 11/21/2014 5:24:54 PM
 *  Author: waynej
 */ 

#include "../../common/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "../../common/adc.h"

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

#define Launch1 PORTA4
#define Enable1 PORTA5
#define Launch2 PORTA6
#define Enable2 PORTA7

#define setLaunch1()   set(PORTA, Launch1)
#define resetLaunch1() reset(PORTA, Launch1)
#define setEnable1()   set(PORTA, Enable1)
#define resetEnable1() reset(PORTA, Enable1)
#define setLaunch2()   set(PORTA, Launch2)
#define resetLaunch2() reset(PORTA, Launch2)
#define setEnable2()   set(PORTA, Enable2)
#define resetEnable2() reset(PORTA, Enable2)

int values[6]={0,0,0,0,0,0};
		
int main(void)
{
	DDRB=_BV(Red1)|_BV(Green1)|_BV(Siren)|_BV(Red2)|_BV(Green2);	// PB0-1, 3-5 are output
	DDRA=_BV(Launch1)|_BV(Enable1)|_BV(Launch2)|_BV(Enable2);		// PA4-7 output

	adc_init(0, ADMUX_REF_INT|ADMUX_AD0);
	adc_init(1, ADMUX_REF_INT|ADMUX_AD1);
	adc_init(2, ADMUX_REF_INT|ADMUX_AD2);
	adc_init(3, ADMUX_REF_INT|ADMUX_AD3);
	adc_init(4, ADMUX_REF_INT|ADMUX_122);
	adc_init(5, ADMUX_REF_INT|ADMUX_GND);
	
	adc_run();

	sei();								// enable interrupts

	setEnable1();
	setEnable2();

	// Wait till we've completed the initial voltage checks.
	while((!adc_valid(0)) || (!adc_valid(1)) || (!adc_valid(2)) || (!adc_valid(3)) || (!adc_valid(4)) || (!adc_valid(5)) )
	{}

    while(1)
    {
		values[0]=adc_value(0);
		values[1]=adc_value(1);
		values[2]=adc_value(2);
		values[3]=adc_value(3);
		values[4]=adc_value(4);
		values[5]=adc_value(5);
		
        if (values[0]>0x200)
		{
			setRed1();
			resetGreen1();
		}
		else
		{
			setGreen1();
			resetRed1();
		}

        if (values[1]>0x200)
		{
			setRed2();
			resetGreen2();
		}
		else
		{
			setGreen2();
			resetRed2();
		}
    }
}