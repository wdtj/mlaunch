#include "pad30.h"
#include "../common/adc.h"
#include "../common/zb.h"

#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "launchFSM.h"
#include "pad30.h"

void sendStatus(int padNum, float contVolt, float battVolt);

void padFSMtimer(int padNum)
{
	struct padStruct *pad=&pads[padNum];

	extern unsigned long v1, v2;

	pad->timeout--;
	if (pad->timeout==0)
	{
		padReset(padNum);
		return;
	}
	
    switch (pad->launchState)
    {
        case IDLE:
		switch(linkState)
		{
		case MODEM_DIS:
			if (pad->flashTimer <DIS_TIMER50)
			{
				set(PORTB, pad->green);
				set(PORTB, pad->red);
			}
			else
			{
				reset(PORTB, pad->green);
				reset(PORTB, pad->red);
			}
			pad->flashTimer++;
			pad->flashTimer%=DIS_TIMER;
			break;
		case MODEM_INIT:
			if (pad->flashTimer <DIS_TIMER50)
			{
				set(PORTB, pad->red);
			}
			else
			{
				reset(PORTB, pad->red);
			}
			pad->flashTimer++;
			pad->flashTimer%=DIS_TIMER;
			break;
		case MODEM_READY:
			reset(PORTB, pad->green);
			reset(PORTB, pad->red);
			break;
			
		}
		break;
		
        case SW_ENABLE:
        pad->relayTimer--;
        if (pad->relayTimer == 0)
        {
            pad->launchState=SW_ENABLED;
        }
        break;

        case SW_ENABLED:
        if (((v1>v2) && (pads[padNum].contVolt < 300)) ||
			((v1<v2) && (pads[padNum].contVolt > 900)))
        {
            set(PORTB, pad->green);
            reset(PORTB, pad->red);
        }
        else
        {
            set(PORTB, pad->red);
            reset(PORTB, pad->green);
        }
        break;

        case PAD_ENABLE:
        pad->relayTimer--;
        if (pad->relayTimer == 0)
        {
			pad->contValid=false;
            pad->launchState=PAD_ENABLED;
			pad->statusTimer=0;
        }
        break;

        case PAD_ENABLED:
        pad->relayTimer--;

		if (((v1>v2) && (pads[padNum].contVolt < 300)) ||
			((v1<v2) && (pads[padNum].contVolt > 900)))
		{
			set(PORTB, pad->green);
			reset(PORTB, pad->red);
		}
		else
		{
			set(PORTB, pad->red);
			reset(PORTB, pad->green);
		}
			
        break;

        case PAD_LAUNCH:
		reset(PORTB, pad->green);
		if (pad->flashTimer <FLASH_TIMER50)
		{
			set(PORTB, pad->red);
		}
		else
		{
			reset(PORTB, pad->red);
		}
		pad->flashTimer++;
		pad->flashTimer%=FLASH_TIMER;
        break;
    }
}

static unsigned int zeros[10];

void SWEnable(int sw)
{
    struct padStruct *pad=&pads[sw];

    if (pad->launchState!=IDLE)
    {
        return;
    }

    set(PORTA, pad->enableBit);
    pad->relayTimer=RELAY_TIMER;   /* Relay latch time is 10ms + margin */
    pad->launchState=SW_ENABLE;
}

void SWReset(int sw)
{
    struct padStruct *pad=&pads[sw];

    if (pad->launchState!=SW_ENABLE && pad->launchState!=SW_ENABLED)
    {
        return;
    }

    pad->launchState=IDLE;

    reset(PORTA, pad->enableBit);
    reset(PORTA, pad->launchBit);

	reset(PORTB, pad->green);
	reset(PORTB, pad->red);
}

void padEnable(int sw)
{
    struct padStruct *pad=&pads[sw];
	
    reset(PORTA, pad->launchBit);
    set(PORTA, pad->enableBit);
    set(PORTB, Siren);
	
    pad->relayTimer=RELAY_TIMER;   /* Relay latch time is 10ms + margin */
	pad->timeout=RESET_TIMER;
    pad->launchState=PAD_ENABLE;
}

void padReset(int sw)
{
    struct padStruct *pad=&pads[sw];

    pad->launchState=IDLE;

    reset(PORTA, pad->enableBit);
    reset(PORTA, pad->launchBit);
	
	reset(PORTB, pad->green);
	reset(PORTB, pad->red);
	
    reset(PORTB, Siren);

	pad->timeout=0;
	pad->flashTimer=0;
	pad->relayTimer=0;
	pad->statusTimer=0;
}

void padLaunch(int sw)
{
    struct padStruct *pad=&pads[sw];
	
    if(pad->launchState!=PAD_ENABLED)
    {
        return;
    }
	
	pad->timeout=RESET_TIMER;
	
    set(PORTA, pad->launchBit);
	
	pad->flashTimer=0;
    pad->launchState=PAD_LAUNCH;
}

void padUnlaunch(int sw)
{
    struct padStruct *pad=&pads[sw];
    reset(PORTA, pad->launchBit);
    pad->timeout=RESET_TIMER;
	pad->flashTimer=0;
	pad->launchState=PAD_ENABLED;
}

