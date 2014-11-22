/*
 * XbeeTest.c
 *
 * Created: 11/19/2014 8:25:27 PM
 *  Author: waynej
 */ 

#include "../../common/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../../common/uart.h"
#include "../../common/zb.h"

#define UART_BAUD  9600UL

FILE xbee = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

char rxBuffer[80]="";
char txBuffer[80]="";

void rxc(void);
void zbWrite(void *buff, unsigned int count);
void linkPkt(unsigned char *data, unsigned int length);

int main(void)
{
	uart_txBuff(txBuffer, sizeof txBuffer);
	uart_rxBuff(rxBuffer, sizeof rxBuffer);

	uart_init(UART_BAUD, rxc);

	zbInit(&zbWrite, &linkPkt);

	sei();								// enable interrupts    
	
	zb_ni(1, "XbeeTest");
	
	while(1)
    {
    }
}

void rxc(void)
{
	unsigned char ch=uart_rxc();
	zbReceivedChar(ch);
}

void zbWrite(void *buff, unsigned int count)
{
	fwrite(buff, count, 1, &xbee);
}

void linkPkt(unsigned char *data, unsigned int length)
{
	while(1)
    {
    }
}