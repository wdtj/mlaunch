/*
 * switchFSM.c
 *
 * Created: 2/22/2014 2:03:04 PM
 *  Author: waynej
 */ 

#include "pad30.h"
#include "../../mCommon/adc.h"

#include "switchFSM.h"

enum SwitchState {
	SW_OPEN,          /* Switch is open */
	SW_PRESSED,       /* The switch was pressed, waiting for debounce */
	SW_CLOSED,        /* Switch is closed */
	SW_RELEASED		  /* The switch was released, waiting for debounce */
};

enum SwitchState switchState;
	
struct Switch {
	enum SwitchState switchState;
	unsigned int bit;
	unsigned int timer;
};

struct Switch switchInfo[2]=
{
	{
		SW_OPEN,
		ContSW1
	},
	{
		SW_OPEN,
		ContSW2
	}
};

void switchFSM(int sw);

void switchFSMtimer(void)
{
	switchFSM(0);
	switchFSM(1);
}

void switchFSM(int sw)
{
	struct Switch *swPtr=&switchInfo[sw];
	
	switch (swPtr->switchState)
	{
		case SW_OPEN:
		if ((PINB&_BV(swPtr->bit))==0)	
		{
			// switch was pressed
			swPtr->timer=SWITCH_TIMER;
			swPtr->switchState=SW_PRESSED;
		}
		break;
		
		case SW_PRESSED:
		swPtr->timer--;
		if (swPtr->timer==0)
		{
			if ((PINB&_BV(swPtr->bit))==0)	
			{
				swPtr->switchState=SW_CLOSED;
			}
			else
			{
				swPtr->switchState=SW_OPEN;
			}
		}
		break;
		
		case SW_CLOSED:
		if ((PINB&_BV(swPtr->bit))!=0)
		{
			// switch was released
			swPtr->timer=SWITCH_TIMER;
			swPtr->switchState=SW_RELEASED;
		}
		break;
		
		case SW_RELEASED:
		swPtr->timer--;
		if (swPtr->timer==0)
		{
			if ((PINB&_BV(swPtr->bit))!=0)	
			{
				swPtr->switchState=SW_OPEN;
			}
			else
			{
				swPtr->switchState=SW_CLOSED;
			}
		}
		break;
	}
}

bool isClosed(int sw)
{
	struct Switch *swPtr=&switchInfo[sw];
	
	switch (swPtr->switchState)
	{
		case SW_OPEN:
		case SW_PRESSED:
			return false;
		case SW_CLOSED:
		case SW_RELEASED:
			return true;
	}

	return false;
}