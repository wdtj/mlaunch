/*
* software.c
*
* Created: 2/16/2014 10:44:30 AM
*  Author: waynej
*/

#include "pad30.h"
#include "../../mCommon/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h> 

#include "../../mCommon/adc.h"
#include "../../mCommon/uart.h"
#include "../../mCommon/zb.h"
#include "../../mCommon/Timer0.h"

#include "launchFSM.h"
#include "switchFSM.h"
#include "statusFSM.h"
#include "linkFSM.h"

FILE xbee = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

char txBuffer[80]=
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

zbAddr controllerAddress={{0,0,0,0,0,0,0xff,0xff}};
zbNetAddr controllerNAD={{0xff, 0xfe}};

struct padStruct pads[2]=
{
	{
		IDLE,
		Enable1,
		Launch1,
		Green1,
		Red1
	},
	{
		IDLE,
		Enable2,
		Launch2,
		Green2,
		Red2
	}
};

unsigned int adc0hist[10];
unsigned int adc1hist[10];
unsigned int adc2hist[10];
unsigned int adc3hist[10];

volatile unsigned int histTimer=1000/TIMER0_PERIOD;

volatile unsigned long v1, v2, r1, r2;

volatile enum LINK_STATE linkState=MODEM_INIT;

bool ident=false;
int identTimer;
int histptr=0;

struct epromStruct EEMEM eprom;

#define noTRACE
#if defined(TRACE)
unsigned char trace_buffer[256];
unsigned char *trace_buffer_ptr=trace_buffer;
unsigned char *trace_buffer_end=trace_buffer+sizeof trace_buffer-1;
#endif



void init();
void rxc(void);
void rxData( zbRx* rxPkt );
void rxPkt(unsigned char *data, unsigned int length);
void zbWrite(void *buff, unsigned int count);
void timer0(void);
void initFlash();
void handleEnable( zbRx* rxPkt, zbAddr addr, zbNetAddr nad );
void handleReset( zbRx* rxPkt );
void handleLaunch( zbRx* rxPkt );
void handleUnlaunch( zbRx* rxPkt );
void handleAssign( zbRx* rxPkt );
void handleIdent( zbRx* rxPkt );

int main(void)
{
	bool sw1pressed=false, sw2pressed=false;
	long adc0, adc1, adc2, adc3;

	init();
	
	setRed1();
	setRed2();

	char nodeName[]="PADxx";
	nodeName[3]=pads[0].padAssign;
	nodeName[4]=pads[1].padAssign;
	
	zb_ni(1, nodeName);

	while(linkState!=MODEM_READY) 
	{
	}
		
	resetRed1();
	resetRed2();

	while(1)
	{
		/*	
		 *  Converting from the ADC value to a voltage is pretty complicated.  There 
		 *  is a diode bridge that biases ground by about .8 volts and then 
		 *  the level shift created by the resistor matrix.  To solve this I took 
		 *  some empirical readings and constructed a graph.  The trendline for the 
		 *  graph of these values came out with a slope of 55.583 V/bit with an x 
		 *  intercept of 14.44.  The following reconstructs the voltage (in 
		 *  hundredths of a volt so we don't need floating point).
		 */

		adc2=(long)adc_value(2);
		if (adc2 > 12)
		{
			v1=(((adc2-12)*10000l)/5766l);
		}
		else
		{
			v1=0;
		}
		
		adc3=(long)adc_value(3);
		if (adc3 > 12)
		{
			v2=(((adc3-12)*10000l)/5766l);
		}
		else
		{
			v2=0;
		}
		
		if (v1>v2)
		{
			adc0=(long)adc_value(0);
			pads[0].contVolt=((adc0-12)*10000l/5766l);
			r1=(((long)adc_value(0)-13)*100l/65l);
		}
		pads[0].contValid=true;
		pads[1].contVolt=(((long)adc_value(1)-16)*10000l/5558l);
		r2=(((long)adc_value(1)-13)*100l/65l);
		pads[1].contValid=true;

		if (!sw1pressed && isClosed(0))
		{	// sw1 has transitioned from open to closed
			SWEnable(0);
			sw1pressed=true;
		}
		
		if (sw1pressed && !isClosed(0))
		{	// sw1 has transitioned from closed to open
			SWReset(0);
			sw1pressed=false;
		}
		
		if (!sw2pressed && isClosed(1))
		{	// sw2 has transitioned from open to closed
			SWEnable(1);
			sw2pressed=true;
		}

		if (sw2pressed && !isClosed(1))
		{	// sw2 has transitioned from closed to open
			SWReset(1);
			sw2pressed=false;
		}
		
		if (histTimer==0)
		{
			memmove(&adc0hist[1], &adc0hist[0], sizeof(adc0hist)-sizeof(adc0hist[0]));
			adc0hist[0]=adc_value(0);

			memmove(&adc1hist[1], &adc1hist[0], sizeof(adc1hist)-sizeof(adc1hist[0]));
			adc1hist[0]=adc_value(1);

			memmove(&adc2hist[1], &adc2hist[0], sizeof(adc2hist)-sizeof(adc2hist[0]));
			adc2hist[0]=adc_value(2);

			memmove(&adc3hist[1], &adc3hist[0], sizeof(adc3hist)-sizeof(adc3hist[0]));
			adc3hist[0]=adc_value(3);

			histTimer=1000/TIMER0_PERIOD;
		}
	}
}

void init()
{
	unsigned char init[4];
	
	DDRA=_BV(Launch1)|_BV(Enable1)|_BV(Launch2)|_BV(Enable2);		// PA4-7 output
	DDRB=_BV(Red1)|_BV(Green1)|_BV(Siren)|_BV(Red2)|_BV(Green2);	// PB0-1, 3-5 are output
	PORTB=_BV(ContSW1)|_BV(ContSW2);								// Set pullup on switches

	DDRD|=_BV(7);
	
	if(!(PINB&_BV(ContSW1)) || !(PINB&_BV(ContSW2)))
	{
		_delay_ms(100);
		if(!(PINB&_BV(ContSW1)) || !(PINB&_BV(ContSW2)))
		{
			eeprom_write_byte(&eprom.init[0], 0);
		}
	}

	eeprom_read_block(init, &eprom.init[0], sizeof init);
	
	if (init[0] == 'I' && init[1] == 'n' && init[2] == 'i' && init[3] == 't')
	{
		// Read configured pad assignments
		pads[0].padAssign=eeprom_read_byte(&eprom.padAssign[0]);
		pads[1].padAssign=eeprom_read_byte(&eprom.padAssign[1]);
	}

	timer0_init(CS_1024, timer0);					// Timer uses system clock/1024
	timer0_set(F_CPU/1024/1000*TIMER0_PERIOD);
	
	uart_txBuff(txBuffer, sizeof txBuffer);
	
	uart_init(UART_BAUD, rxc);

	adc_init(0, ADMUX_REF_INT|ADMUX_AD0);
	adc_init(1, ADMUX_REF_INT|ADMUX_AD1);
	adc_init(2, ADMUX_REF_INT|ADMUX_AD2);
	adc_init(3, ADMUX_REF_INT|ADMUX_AD3);
	adc_init(4, ADMUX_REF_INT|ADMUX_122);
	adc_init(5, ADMUX_REF_INT|ADMUX_GND);
	
	adc_run();

	zbInit(&zbWrite, &rxPkt);
	
	sei();								// enable interrupts

	// Wait till we've completed the initial voltage checks.
	while(!adc_valid(0) && !adc_valid(1) && !adc_valid(2) && !adc_valid(3)){}
}

/* Timer 0 interrupt */
void timer0(void)
{
	timer0_set(F_CPU/1024/1000*TIMER0_PERIOD);

	if (ident)
	{
		identTimer++;
		identTimer%=100/TIMER0_PERIOD;
		if (identTimer == 0/TIMER0_PERIOD)	
		{
			setRed1();
			setGreen1();
			setRed2();
			setGreen2();
		} 
		else if (identTimer == 50/TIMER0_PERIOD) 
		{
			resetRed1();
			resetGreen1();
			resetRed2();
			resetGreen2();
		}
	}
	else
	{
		switchFSMtimer();

		padFSMtimer(0);
		padFSMtimer(1);

		statusFSMtimer();
	}
	
	if (histTimer>0) histTimer--;
}

/* Flash the lights twice at initialization */
void initFlash()
{
	setRed1();
	setGreen1();
	setRed2();
	setGreen2();
	_delay_ms(100);
	resetRed1();
	resetGreen1();
	resetRed2();
	resetGreen2();
	_delay_ms(100);
	setRed1();
	setGreen1();
	setRed2();
	setGreen2();
	_delay_ms(100);
	resetRed1();
	resetGreen1();
	resetRed2();
	resetGreen2();
}

/* Receive a single character from the UART */
void rxc(void)
{
	unsigned char ch=UDR;

	#if defined(TRACE)
	*(trace_buffer_ptr++)=ch;
	if (trace_buffer_ptr==trace_buffer_end)
	{
		trace_buffer_ptr=trace_buffer;
	}
	#endif

	zbReceivedChar(ch);
}

/* Write a buffer of data to XBee */
void zbWrite(void *buff, unsigned int count)
{
	fwrite(buff, count, 1, &xbee);
}

/* Handle received XBee packet */
void rxPkt(unsigned char *data, unsigned int length)
{
	zbPkt *pkt=(zbPkt *)data;
	switch(pkt->frameType)
	{
		case ZB_MODEM_STATUS:
		{
			struct zbModemStatus *ms=&pkt->zbModemStatus;
			if (ms->status==zb_mdm_assoc)
			{
				linkState=MODEM_READY;
			}
			else if (ms->status==zb_mdm_disassoc)
			{
				linkState=MODEM_DIS;
			}
		}
		break;

		case ZB_RECEIVE_PACKET:
		rxData(&pkt->zbRX);
		break;
		
		case ZB_TRANSMIT_STATUS:
		{
			struct zbTransmitStatus *ts=&pkt->zbTransmitStatus;
			
			zb_txBusy=false;
		}
		break;

		case ZB_AT_COMMAND_RESPONSE:
		{
			struct zbATResponse *cr=&pkt->zbATResponse;

			if (cr->cmd[0]=='N' && cr->cmd[1]=='I')
			{
				zb_jv(1, 1);
			}
			else if (cr->cmd[0]=='J' && cr->cmd[1]=='V')
			{
				zb_jn(1, 1);
			} 
			else if (cr->cmd[0]=='J' && cr->cmd[1]=='N')
			{
				zb_nr(0, 0);
			}
		}
		break;
		
		default:
#if defined(myDEBUG)
		while(false){}
#endif
		break;
	}
}

/* Handle received XBee Data */
void rxData( zbRx* rxPkt )
{
	switch (rxPkt->data[0])
	{
		case 'E':
		handleEnable(rxPkt, rxPkt->dest, rxPkt->nad);
		break;
		case 'R':
		handleReset(rxPkt);
		break;
		case 'L':
		handleLaunch(rxPkt);
		break;
		case 'U':
		handleUnlaunch(rxPkt);
		break;
		case 'A':
		handleAssign(rxPkt);
		break;
		case 'I':
		handleIdent(rxPkt);
		break;
	}
}

void handleEnable( zbRx* rxPkt, zbAddr addr, zbNetAddr nad)
{
	controllerAddress=addr;
	controllerNAD=nad;

	for(int i=0; i<PAD_COUNT; ++i)
	{
		if (rxPkt->data[1]==pads[i].padAssign)
		{
			padEnable(i);
		}
	}
}

void handleReset( zbRx* rxPkt )
{
	for(int i=0; i<PAD_COUNT; ++i)
	{
		if (rxPkt->data[1]==pads[i].padAssign)
		{
			padReset(i);
		}
	}
}

void handleLaunch( zbRx* rxPkt )
{
	for(int i=0; i<PAD_COUNT; ++i)
	{
		if (rxPkt->data[1]==pads[i].padAssign)
		{
			padLaunch(i);
		}
	}
}

void handleUnlaunch( zbRx* rxPkt )
{
	for(int i=0; i<PAD_COUNT; ++i)
	{
		if (rxPkt->data[1]==pads[i].padAssign)
		{
			padUnlaunch(i);
		}
	}
}

void handleAssign( zbRx* rxPkt )
{
	ident=false;
	
	unsigned char pad0=rxPkt->data[1];
	unsigned char pad1=rxPkt->data[2];
	
	pads[0].padAssign=pad0;
	pads[1].padAssign=pad1;

	eeprom_write_block("Init", &eprom.init, 4);
	eeprom_write_byte(&eprom.padAssign[0], pad0);
	eeprom_write_byte(&eprom.padAssign[1], pad1);
	
	char nodeName[]="PADxx";
	nodeName[3]=pad0;
	nodeName[4]=pad1;
	
	zb_ni(0, nodeName);

	resetRed1();
	resetGreen1();
	resetRed2();
	resetGreen2();
}

void handleIdent( zbRx* rxPkt )
{
	ident=true;
}
