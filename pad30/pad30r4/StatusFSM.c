/*
 * StatusFSM.c
 *
 * Created: 2/25/2014 7:33:56 PM
 *  Author: waynej
 */ 

#include "pad30.h"
#include <avr/io.h>
#include <avr/eeprom.h>

#include "../../common/zb.h"
#include "../../common/adc.h"
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
	long batt;
	extern unsigned long vBatt, v2;
	extern unsigned char channel;
	extern unsigned char signalStrength;

	extern zbAddr controllerAddress;
	extern zbNetAddr controllerNAD;
	
	if (!pads[padNum].contValid)
	{
		return;
	}
	
	int launchState=pads[padNum].launchState==PAD_LAUNCH?1:0;
	int contState=(pads[padNum].contResistance < 400)?1:0;
	
	int len=sprintf(msg, "S%c e%d l%d Cr%ld Vb%ld.%02ld Ch%02d RSSI%02d",
		pads[padNum].padAssign, contState, launchState, pads[padNum].contResistance, vBatt/100, vBatt%100, channel, signalStrength);
	
	zb_tx(0, controllerAddress, controllerNAD, 0, 0, msg, len);
}

