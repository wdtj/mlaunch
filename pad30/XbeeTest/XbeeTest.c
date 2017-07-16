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

#define UART_BAUD  9600UL

FILE xbee = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

char rxBuffer[80]="";
char txBuffer[80]="";

void rxc(void);
int zbWrite(void *buff, unsigned int count);
void linkPkt(unsigned char *data, unsigned int length);

int main(void)
{
	DDRB=_BV(Red1)|_BV(Green1)|_BV(Yellow1)|_BV(Red2)|_BV(Green2)|_BV(Yellow2);	// PB0-2, 4-6 are output
	PORTB=_BV(Red1)|_BV(Green1)|_BV(Yellow1)|_BV(Red2)|_BV(Green2)|_BV(Yellow2);

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

int zbWrite(void *buff, unsigned int count)
{
	return fwrite(buff, count, 1, &xbee);
}

void linkPkt(unsigned char *data, unsigned int length)
{
	while(1)
    {
		zbPkt *pkt=(zbPkt *)data;
		switch(pkt->frameType)
		{
			case ZB_AT_COMMAND_RESPONSE:
			{
				setYellow2();
				struct zbATResponse *cr=&pkt->zbATResponse;

				switch(cr->status)
				{
					case 0:
					resetYellow2();
					setGreen2();
					break;
				}

				break;
			}
		}

    }
}