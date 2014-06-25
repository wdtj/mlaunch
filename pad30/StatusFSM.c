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
	cont=pads[padNum].contVolt;
	if (cont < 0)
	{
		cont=0l;
	}
	
	int launchState=pads[padNum].launchState==PAD_LAUNCH?1:0;
	int contState=((v1>v2 && cont < 300) || (v1<v2 && cont > 900))?1:0;
	
	long contDelta=batt>cont?batt-cont:0;

	int len=sprintf(msg, "S%d e%d l%d Cv%ld.%02ld Bv%ld.%02ld Dv%ld.%02ld",
		padNum+1, contState, launchState, cont/100, cont%100, batt/100, batt%100, contDelta/100, contDelta%100);
	
	zb_tx(0, controllerAddress, controllerNAD, 0, 0, msg, len);
}

