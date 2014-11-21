/*
 * LedTest.c
 *
 * Created: 11/18/2014 6:32:03 PM
 *  Author: waynej
 */ 


#include "../../common/config.h"

#include <avr/io.h>
#include <util/delay.h>

#include "../../common/led.h"

int main(void)
{
	LED_init();

    while(1)
    {
		LED_output(0xff, 0x00, 0x00);			/* Turn on all red LEDs */
		_delay_ms(1000);
		LED_output(0x00, 0xff, 0x00);			/* Turn on all green LEDs */
		_delay_ms(1000);
		LED_output(0x00, 0x00, 0xff);			/* Turn on all yellow LEDs */
		_delay_ms(1000);
		LED_output(0x00, 0x00, 0x00);			/* Turn on all LEDs off */
		_delay_ms(1000);
		/* Flash the pad lights to be pretty */
		for(int i=0; i<10; ++i)
		{
			unsigned int bit=1<<i;
			LED_output(bit, bit>>1, bit>>2);
			_delay_ms(250);
			LED_output(0x0, 0x0, 0x0);
		}
    }
}