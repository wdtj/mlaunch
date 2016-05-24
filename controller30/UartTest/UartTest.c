/*
 * UartTest.c
 *
 * Created: 5/13/2016 10:52:36 PM
 * Author : wayne
 */ 

#include "../../common/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../../common/uart.h"

#define UART_BAUD  9600UL

void rxc(void);

int main(void)
{
	uart_init(UART_BAUD, rxc);

	sei();								// enable interrupts

    /* Replace with your application code */
    while (1) 
    {
		uart_txc('\x01');
    }
}

static volatile unsigned char received=0;

void rxc(void)
{
	unsigned char ch=uart_rxc();
	received=ch;
}

