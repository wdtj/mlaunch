/*
 * controller30.c
 *
 * Created: 2/26/2014 7:34:55 PM
 *  Author: waynej
 */ 


#include "controller30.h"
#include "../../mCommon/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "../../mCommon/led.h"
#include "../../mCommon/OLED.h"
#include "../../mCommon/adc.h"
#include "../../mCommon/uart.h"
#include "../../mCommon/zb.h"
#include "../../mCommon/Timer0.h"
#include "linkFSM.h"
#include "switchFSM.h"

void initNewPads(void);
void init(void);
void rxc(void);
void zbWrite(void *buff, unsigned int count);
void timer0(void);
void enable(int sw);
void disable(int sw);
void padFSMTimer(void);
void sendEnable( int sw );
void sendDisable( int sw );
void sendLaunch( int sw );
void sendIdent( zbAddr addr, zbNetAddr netAddr );
void sendAssign( int sw );

FILE xbee = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

char rxBuffer[80]="";
char txBuffer[80]="";

unsigned int greenLeds=0;
unsigned int redLeds=0;
unsigned int yellowLeds=0;

struct padInfo pads[]=
{
    {
	    IDLE,
		'1'
	},
    {
	    IDLE,
		'2'
	},
    {
	    IDLE,
		'3'
	},
    {
	    IDLE,
		'4'
	},
    {
	    IDLE,
		'5'
	},
    {
	    IDLE,
		'6'
	},
    {
	    IDLE,
		'7'
	},
    {
	    IDLE,
		'8'
	}
};

struct NewPad newPad[8];

bool notInitialized=false;

void initNewPads()
{
	for(int i=0; i<8; ++i)
	{
		bool wait=false;
		if(newPad[i].used)
		{
			sendIdent(newPad[i].addr, newPad[i].netAddr); 
			
			OLED_XYprintf(0, 0, "Unassigned Device:");
			OLED_XYprintf(0, 1, "%02x%02x %02x%02x %02x%02x %02x%02x", 
			newPad[i].addr.addr64[0],
			newPad[i].addr.addr64[1],
			newPad[i].addr.addr64[2],
			newPad[i].addr.addr64[3],
			newPad[i].addr.addr64[4],
			newPad[i].addr.addr64[5],
			newPad[i].addr.addr64[6],
			newPad[i].addr.addr64[7]);
			OLED_XYprintf(0, 3, "Select Pad");
			
			wait=true;
			while(wait)
			{
				for(int sw=0; sw<8; ++sw)
				{
					if(isClosed(sw))
					{
						int pair=sw/2*2;
						pads[pair].addr=newPad[i].addr;
						pads[pair].netAddr=newPad[i].netAddr;
						pads[pair].discovered=true;
						pads[pair+1].addr=newPad[i].addr;
						pads[pair+1].netAddr=newPad[i].netAddr;
						pads[pair+1].discovered=true;
						sendAssign(pair);
						greenLeds|=_BV(pair);
						greenLeds|=_BV(pair+1);
						LED_output(redLeds, greenLeds, yellowLeds);
						wait=false;
						while(isClosed(sw)) {}
						break;
					}
				}
			}
		}
	}
}

int main(void)
{
	bool launchPressed=false;
	
	init();

	LED_output(0x0, 0x0, 0x0);			/* Turn off LEDs */

	OLED_display(true, false, false);	/* Display on, Cursor off, Blink off */
	OLED_functionSet(true, 0);			/* 8 bit, English/Japanese char set */
	OLED_entryModeSet(true, false);		/* Increment, no shift */
	OLED_cursorDisplayShift(false, true); /* Shift cursor, Right */

	OLED_clearDisplay();				/* Blank display */

	OLED_XYprintf(0, 0, "mLaunch 3.0");
	OLED_XYprintf(0, 2, "Copyright 2014 by");
	OLED_XYprintf(0, 3, "Wayne Johnson");

	OLED_clearLine(2);
	OLED_clearLine(3);
	
	if (PINB!=0xFF)
	{
		notInitialized=true;
		OLED_XYprintf(0, 3, "Reinitializing");
	}
//	notInitialized=true;

	OLED_XYprintf(0, 1, "Comm Init");
	OLED_clearEOL();

	/* Flash the pad lights to be pretty */
	for(int i=0; i<10; ++i)
	{
		unsigned int bit=1<<i;
		LED_output(bit, bit>>1, bit>>2);
		_delay_ms(100);
		LED_output(0x0, 0x0, 0x0);
	}

	linkStart(notInitialized);

	while(!isLinkReady()) {
		for (int i=0; i<8; i+=2)
		if (pads[i].discovered && !pads[i].discoveredAck)
		{
			OLED_XYprintf(0, 3, "Discovered %c&%c", pads[i].padId, pads[i+1].padId);
			OLED_clearEOL();
			
			pads[i].discoveredAck=true;
			greenLeds|=_BV(i);
			pads[i+1].discoveredAck=true;
			greenLeds|=_BV(i+1);
		}

		LED_output(redLeds, greenLeds, yellowLeds);
	}
	
	OLED_clearDisplay();
	
	initNewPads();
	
	padReady();

	/* main loop */
    while(1)
    {
		bool allOpen=true;
			
		for(int sw=0; sw<8; ++sw)
		{
			if(!pads[sw].discovered)
			{
				continue;
			}
			
			if (isClosed(sw))
			{
				if (pads[sw].padState == IDLE)
				{
					enable(sw);
				} 
				else if (pads[sw].padState == ENABLED)
				{
					disable(sw);
				}
				allOpen=false;
			}
			else
			{
				if (pads[sw].padState == PRESSED2ENABLE)
				{
					pads[sw].padState=ENABLED;
				} 
				else if (pads[sw].padState == PRESSED2DISABLE)
				{
					pads[sw].padState=IDLE;
				}
			}
			
			if (pads[sw].newStatus)
			{
				if (pads[sw].padState == PRESSED2ENABLE)
				{
					OLED_clearDisplay();
					OLED_XYprintf(0,0, "Pad %d", sw+1);
					OLED_XYprintf(0,1, "ContVolt=%d.%d", pads[sw].cont/100, pads[sw].cont%100);
					OLED_XYprintf(0,2, "BattVolt=%d.%d", pads[sw].batt/100, pads[sw].batt%100);
					pads[sw].newStatus=false;
				}
			}
			
			if (pads[sw].padState == ENABLED ||
				pads[sw].padState == PRESSED2ENABLE)
			{
				if (pads[sw].statusValid)
				{
					if (pads[sw].contState==0)
					{
						redLeds|=_BV(sw);
						greenLeds&=~_BV(sw);
						yellowLeds&=~_BV(sw);
					}
					else
					{
						redLeds&=~_BV(sw);
						greenLeds|=_BV(sw);
						yellowLeds&=~_BV(sw);
					}
				}
				else
				{
					redLeds&=~_BV(sw);
					greenLeds&=~_BV(sw);
					yellowLeds|=_BV(sw);
				}
			}

			if (pads[sw].padState == IDLE)
			{
				redLeds&=~_BV(sw);
				greenLeds&=~_BV(sw);
				yellowLeds&=~_BV(sw);
			}
		}
		LED_output(redLeds, greenLeds, yellowLeds);
		
		if (allOpen == true)
		{
			OLED_clearDisplay();
		}
		
		if (isClosed(8) && !launchPressed)
		{
			launchPressed=true;
			for (int pad=0; pad<8; ++pad)
			{
				if (pads[pad].padState==ENABLED)
				{
					pads[pad].padState=PAD_LAUNCH;
					pads[pad].enableTimer=0;
				}
			}
		}
		else if (!isClosed(8) && launchPressed)
		{
			launchPressed=false;
			for (int pad=0; pad<8; ++pad)
			{
				if (pads[pad].padState==PAD_LAUNCH)
				{
					sendDisable(pad);
					pads[pad].padState=IDLE;
				}
			}
		}
    }
}

void init(void)
{
	DDRB=0x0;		// Set switches as input
	PORTB=0xff;

	DDRC&=~0x7f;	// Set Launch Switch
	PORTC|=0x80;
	
	DDRC|=_BV(1);
	DDRC|=_BV(6);

	LED_init();
	OLED_init();
	
	timer0_init(CS_1024, timer0);					// Timer uses system clock/1024
	timer0_set(F_CPU/1024/1000*TIMER0_PERIOD);
	
	uart_txBuff(txBuffer, sizeof txBuffer);
	uart_rxBuff(rxBuffer, sizeof rxBuffer);

	uart_init(UART_BAUD, rxc);
	
	zbInit(&zbWrite, &linkPkt);

	sei();								// enable interrupts
}

void rxc(void)
{
	signal2(true);

	unsigned char ch=uart_rxc();
	zbReceivedChar(ch);

	signal2(false);
}

void zbWrite(void *buff, unsigned int count)
{
	fwrite(buff, count, 1, &xbee);
}

#define xUnitTest
#ifdef UnitTest
static int utTimer=0;
#endif

void timer0(void)
{
	
#ifdef UnitTest
	utTimer++;
	if (utTimer==1000/TIMER0_PERIOD)
	{
		PORTC&=~_BV(1);
	}
	else if (utTimer==3000/TIMER0_PERIOD)
	{
		PORTC|=_BV(1);
		utTimer=0;
	}
#endif
	
	switchFSMtimer();
	padFSMTimer();
	linkTimer();

	timer0_set(F_CPU/1024/1000*TIMER0_PERIOD);
}

void enable(int sw)
{
	pads[sw].padState=PRESSED2ENABLE;
	redLeds&=~_BV(sw);
	yellowLeds|=_BV(sw);
	greenLeds&=~_BV(sw);
	LED_output(redLeds, greenLeds, yellowLeds);	
	pads[sw].enableTimer=0;
	pads[sw].statusTimer=5000;
	pads[sw].newStatus=false;
}


void disable(int sw)
{
	pads[sw].padState=PRESSED2DISABLE;	
	redLeds&=~_BV(sw);
	yellowLeds&=~_BV(sw);
	greenLeds&=~_BV(sw);
	LED_output(redLeds, greenLeds, yellowLeds);
	pads[sw].statusValid=false;

	sendDisable(sw);
}

void sendEnable( int sw )
{
	char msg[]="En";
	msg[1]=pads[sw].padId;
	zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 2);
}

void sendDisable( int sw )
{
	char msg[]="Rn";
	msg[1]=pads[sw].padId;
	zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 2);
}

void sendLaunch( int sw )
{
	char msg[]="Ln";
	msg[1]=pads[sw].padId;
	zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 2);
}

void sendAssign( int sw )
{
	char msg[]="Ann";
	msg[1]=pads[sw].padId;
	msg[2]=pads[sw+1].padId;
	zb_tx(0, pads[sw].addr, pads[sw].netAddr, 0, 0, msg, 3);
}

void sendIdent( zbAddr addr, zbNetAddr netAddr )
{
	char msg[]="I";
	zb_tx(0, addr, netAddr, 0, 0, msg, 2);
}

void padFSMTimer(void)
{
	for(int pad=0; pad<8; ++pad)
	{
		switch (pads[pad].padState)
		{
		case PRESSED2ENABLE:
		case ENABLED:
			{
				pads[pad].enableTimer--;
				if (pads[pad].enableTimer<=0)
				{
					sendEnable(pad);
					pads[pad].enableTimer=2000/TIMER0_PERIOD;
				}
				
				pads[pad].statusTimer--;
				if (pads[pad].statusTimer<=0)
				{
					pads[pad].statusValid=false;
				}
			}
		break;
		case IDLE:
		case PRESSED2DISABLE:
		break;
		case PAD_LAUNCH:
			pads[pad].enableTimer--;
			if (pads[pad].enableTimer<=0)
			{
				sendLaunch(pad);
				pads[pad].enableTimer=1000/TIMER0_PERIOD;
			}
		break;
		}
	}
}

void statusMessage(char * data,int length)
{
	unsigned char padnum;
	int contState, launchState;
	int pad;
	int cont, cont1, cont2, batt, batt1, batt2, d1, d2;			
	
	sscanf(data, "S%c e%d l%d Cv%d.%d Bv%d.%d Dv%d.%d", 
		&padnum, &contState, &launchState, &cont1, &cont2, &batt1, &batt2, &d1, &d2);

	batt=batt1*100+batt2;
	cont=cont1*100+cont2;

	pad=padnum-'1';
	if (pad<8)
	{
		pads[pad].contState=contState;
		pads[pad].launchState=launchState;
		pads[pad].cont=cont;
		pads[pad].batt=batt;
		pads[pad].statusValid=true;
		pads[pad].newStatus=true;
		pads[pad].statusTimer=5000/TIMER0_PERIOD;
	}
}

void padDiscovered( zbAddr addr, zbNetAddr netAddr, unsigned char *ni )
{
	if (ni[0]=='P' && ni[1]=='A' && ni[2]=='D' &&
	ni[3]>'0' && ni[3]<'9' &&
	ni[4]>'0' && ni[4]<'9' )
	{
		unsigned int padId0=ni[3]-'0'-1;
		unsigned int padId1=ni[4]-'0'-1;
		
		pads[padId0].addr=addr;
		pads[padId0].netAddr=netAddr;
		
		pads[padId1].addr=addr;
		pads[padId1].netAddr=netAddr;

		pads[padId0].discovered=true;
		pads[padId1].discovered=true;
	}
	else
	{
		// Add pad to ToBeAssigned list
		
		// find a slot in newPad
		for(int i=0; i<8; ++i)
		{
			if (newPad[i].used==false)
			{
				newPad[i].addr=addr;
				newPad[i].netAddr=netAddr;
				newPad[i].used=true;
				break;
			}
		}
	}
}

void padReady()
{
	OLED_clearDisplay();
	OLED_XYprintf(0, 0, "Ready");
	
	redLeds=0;
	yellowLeds=0;
	greenLeds=0;
	LED_output(redLeds, greenLeds, yellowLeds);	
}

