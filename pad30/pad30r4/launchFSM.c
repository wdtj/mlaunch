#include "pad30.h"
#include "../../common/adc.h"
#include "../../common/zb.h"

#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "launchFSM.h"
#include "linkFSM.h"
#include "PadLed.h"

void sendStatus(int padNum, float contVolt, float battVolt);

/**
 * Pad Finite State timer
 */
void padFSMtimer(int padNum)
{
	struct padStruct *pad=&pads[padNum];

    /* After 10 seconds of no contact from the controller, reset to the IDLE state. */
	pad->timeout--;
	if (pad->timeout==0)
	{
		padReset(padNum);
		return;
	}
	
    switch (pad->launchState)
    {
        case IDLE:
		switch(linkFSMStatus())
		{
		case MODEM_RESET:
			if (pad->flashTimer <DIS_TIMER50)
			{
				PadLed(YELLOW, padNum);
			}
			else
			{
				PadLed(OFF, padNum);
			}
			pad->flashTimer++;
			pad->flashTimer%=DIS_TIMER;
			break;
		case MODEM_INIT:
			if (pad->flashTimer <DIS_TIMER50)
			{
				PadLed(RED, padNum);
			}
			else
			{
				PadLed(OFF, padNum);
			}
			pad->flashTimer++;
			pad->flashTimer%=DIS_TIMER;
			break;
		case MODEM_READY:
			PadLed(OFF, padNum);
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
		if (pads[padNum].contValid==true)
		{
			if (pads[padNum].contResistance < 400)
			{
				PadLed(GREEN, padNum);
			}
			else
			{
				PadLed(RED, padNum);
			}
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

		if (pads[padNum].contResistance < 400)
		{
			PadLed(GREEN, padNum);
		}
		else
		{
			PadLed(RED, padNum);
		}
			
        break;

        case PAD_LAUNCH:
		if (pad->flashTimer <FLASH_TIMER50)
		{
			PadLed(RED, padNum);

		}
		else
		{
			PadLed(OFF, padNum);
		}
		pad->flashTimer++;
		pad->flashTimer%=FLASH_TIMER;
        break;
    }
}

/**
 * Handle switch enable
 *
 * User has pressed the continuity switch.  This engages the continuity relay
 * take resistance measurements, and displays the continuity status on the LED.
 */
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

/**
 * Handle switch disable
 *
 * User has released the continuity switch.  This disgages the continuity relay
 * and turns off the LED.
 */
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

	PadLed(OFF, sw);
}

/**
 * Received Enable command.
 * 
 * Handle an enable command for the specified pad.  This causes the pad to
 * engage the continuity relay, take resistance measurements, and starts 
 * responding with a Status command every second.
 *
 * If the pad does not hear a Enable or Launch command within 10 seconds, the
 * pad->timeout counter decrements to 0 and the pad resets to IDLE state.
 */
void padEnable(int sw)
{
    struct padStruct *pad=&pads[sw];
	
    reset(PORTA, pad->launchBit);
    set(PORTA, pad->enableBit);
    set(PORTC, Siren);
	
    pad->relayTimer=RELAY_TIMER;   /* Relay latch time is 10ms + margin */
	pad->timeout=RESET_TIMER;
    pad->launchState=PAD_ENABLE;
}

/**
 * Handle Reset command.
 * 
 * Handles a reset command for the specified pad.  This causes the pad to
 * disengage all relays and reset to it's IDLE state.
 */
void padReset(int sw)
{
    struct padStruct *pad=&pads[sw];

    pad->launchState=IDLE;

    reset(PORTA, pad->enableBit);
    reset(PORTA, pad->launchBit);
	
	PadLed(OFF, sw);
	
    reset(PORTC, Siren);

	pad->timeout=0;
	pad->flashTimer=0;
	pad->relayTimer=0;
	pad->statusTimer=0;
}

/**
 * Handle Launch command.
 * 
 * Handle a launch command for the specified pad.  This causes the pad to
 * engage it's launch relay allowing full current to flow through the igniter.
 *
 * If the pad does not hear a Enable or Launch command within 10 seconds, the
 * pad->timeout counter decrements to 0 and the pad resets to IDLE state.
 */
void padLaunch(int sw)
{
    struct padStruct *pad=&pads[sw];
	
    /* If we're not already enabled, ignore this launch command */
    if(pad->launchState!=PAD_ENABLED && pad->launchState!=PAD_LAUNCH)
    {
        return;
    }
	
    set(PORTA, pad->launchBit);
	
	pad->flashTimer=0;
    pad->launchState=PAD_LAUNCH;
	pad->timeout=RESET_TIMER;
}

/**
 * Handle Unlaunch command.
 * 
 * Handle an unlaunch command for the specified pad.  This causes the pad to
 * disengage the launch relay and regress to it's Enabled state.
 */
void padUnlaunch(int sw)
{
    struct padStruct *pad=&pads[sw];
    reset(PORTA, pad->launchBit);
    pad->timeout=RESET_TIMER;
	pad->flashTimer=0;
	pad->launchState=PAD_ENABLED;
}
