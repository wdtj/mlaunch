/*
 * config.h
 *
 * Created: 2/12/2014 9:00:34 PM
 *  Author: waynej
 */ 

#include "../../common/config.h"
#include <avr/io.h>

#ifndef PAD30_CONFIG_H_
#define PAD30_CONFIG_H_

#define Launch1 PORTA4
#define Enable1 PORTA5
#define Launch2 PORTA6
#define Enable2 PORTA7

#define ContSW1 PORTB3
#define ContSW2 PORTB7

#define setLaunch1()   set(PORTA, Launch1)
#define resetLaunch1() reset(PORTA, Launch1)
#define setEnable1()   set(PORTA, Enable1)
#define resetEnable1() reset(PORTA, Enable1)
#define setLaunch2()   set(PORTA, Launch2)
#define resetLaunch2() reset(PORTA, Launch2)
#define setEnable2()   set(PORTA, Enable2)
#define resetEnable2() reset(PORTA, Enable2)

#define Siren PORTC7

#define BattADC1 MUX2
#define BattADC2 MUX3

#define ContADC1 MUX0
#define ContADC2 MUX1

#define dac2volt(b) ((((float)b/1024)*3.3)*(1000000+330000)/(330000))

#define sw(s) ((PINB&_BV(s))==0)

#define UART_BAUD  9600UL

enum LaunchState {
	IDLE,             /* We're idle and nothing is happening */
	SW_ENABLE,       /* The switch was pressed and we're waiting for the relay to engage */
	SW_ENABLED,       /* Relay is engaged, update LED */
	PAD_ENABLE,       /* An Enable message was received and we're waiting for the relay to engage */
	PAD_ENABLED,      /* Relay is engaged, update LED */
	PAD_LAUNCH        /* Launch command received, launch relay is enabled */
};

struct padStruct
{
	enum LaunchState launchState;
	unsigned int enableBit;
	unsigned int launchBit;
	unsigned char padAssign;
	unsigned int flashTimer;
	unsigned int relayTimer;
	unsigned int statusTimer;
	unsigned int timeout;
	unsigned long contResistance;
	bool contValid;
};

#define PAD_COUNT 2

struct padStruct pads[PAD_COUNT];

#define TIMER0_PERIOD 10

#define SWITCH_TIMER (50/TIMER0_PERIOD)
#define STATUS_TIMER (1000/TIMER0_PERIOD)
#define FLASH_TIMER50 (50/TIMER0_PERIOD)
#define FLASH_TIMER (100/TIMER0_PERIOD)
#define DIS_TIMER50 (20/TIMER0_PERIOD)
#define DIS_TIMER (200/TIMER0_PERIOD)
#define RESET_TIMER (1000*10/TIMER0_PERIOD)
#define RELAY_TIMER (50/TIMER0_PERIOD)

volatile bool modemDown;
int histptr;

struct epromStruct
{
	unsigned char init[4];
	unsigned char padAssign[PAD_COUNT];
};

extern struct epromStruct eprom;

enum LINK_STATUS
{
	MODEM_INIT,
	MODEM_RESET,
	MODEM_READY
};

#ifdef DEBUG
#define ASSERT(x) if (x) while(true) {};
#else
#define ASSERT(x)
#endif

#endif /* PAD32_CONFIG_H_ */