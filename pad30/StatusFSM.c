/*
 * StatusFSM.c
 *
 * Created: 2/25/2014 7:33:56 PM
 *  Author: waynej
 */ 

#include "pad30.h"
#include <avr/io.h>
#include <avr/eeprom.h>

#include "../common/zb.h"
#include "../common/adc.h"
#include <stdio.h>

void statusFSM(int padNum);
void sendStatus(int padNum);

void statusFSMtimer(void)
{
	statusFSM(0);
	statusFSM(1);
}

void statusFSM(int padNum)
{
	struct padStruct *pad=&pads[padNum];

	if (pad->launchState==PAD_ENABLED ||
		pad->launchState==PAD_LAUNCH)
	{
		if (pad->statusTimer==0)
		{
			sendStatus(padNum);
			pad->statusTimer=STATUS_TIMER;
		}
		else
		{
			--pad->statusTimer;
		}
	}
}

void sendStatus(int padNum)
{
	char msg[80];
	long batt, cont;
	extern unsigned long v1, v2;
	extern zbAddr controllerAddress;
	extern zbNetAddr controllerNAD;
	
	if (!pads[padNum].contValid)
	{
		return;
	}
	
	batt=v1>v2?v1:v2;

	int launchState=pads[padNum].launchState==PAD_LAUNCH?1:0;
	int contState=(pads[padNum].contResistance < 400)?1:0;
	
	int len=sprintf(msg, "S%d e%d l%d Rc%ld Bv%ld.%02ld",
		padNum+1, contState, launchState, pads[padNum].contResistance, batt/100, batt%100);
	
	zb_tx(0, controllerAddress, controllerNAD, 0, 0, msg, len);
}

