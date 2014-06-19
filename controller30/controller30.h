/*
 * controller30.h
 *
 * Created: 2/26/2014 7:40:38 PM
 *  Author: waynej
 */ 


#ifndef CONTROLLER30_H_
#define CONTROLLER30_H_

#include "../../mCommon/config.h"
#include "../../mCommon/zb.h"
#include <avr/io.h>

#define UART_BAUD  9600UL

#define SW1 PORTB0
#define SW2 PORTB1
#define SW3 PORTB2
#define SW4 PORTB3
#define SW5 PORTB4
#define SW6 PORTB5
#define SW7 PORTB6
#define SW8 PORTB7

#define signal1(bit) if(bit) {PORTC|=_BV(1);}else{PORTC&=~_BV(1);}
#define signal2(bit) if(bit) {PORTC|=_BV(6);}else{PORTC&=~_BV(6);}

enum PadState {
	IDLE,					/* We're idle and nothing is happening */
	PRESSED2ENABLE,			/*  */
	ENABLED,				/*  */
	PRESSED2DISABLE,		/*  */
	PAD_LAUNCH        /* Launch command received, launch relay is enabled */
};


struct padInfo
{
	enum PadState padState;
	unsigned char padId;
	zbAddr addr;
	zbNetAddr netAddr; 
	int statusTimer;
	int enableTimer;
	bool statusValid;
	bool newStatus;
	bool discovered;
	bool discoveredAck;
	int contState;
	int launchState;
	int cont;
	int batt;
};

struct NewPad
{
	bool used;
	zbAddr addr;
	zbNetAddr netAddr; 
};

#define TIMER0_PERIOD 20

void statusMessage(char * data, int length);
void padDiscovered( zbAddr addr, zbNetAddr netAddr, unsigned char *ni );
void padReady(void);

#endif /* CONTROLLER30_H_ */