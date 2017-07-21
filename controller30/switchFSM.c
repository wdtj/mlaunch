/*
 * switchFSM.c
 *
 * Created: 3/1/2014 3:52:35 PM
 *  Author: waynej
 */

#include "controller30.h"

#include "switchFSM.h"

enum SwitchState
{
    SW_OPEN, /* Switch is open */
    SW_PRESSED, /* The switch was pressed, waiting for debounce */
    SW_CLOSED, /* Switch is closed */
    SW_RELEASED /* The switch was released, waiting for debounce */
};

struct Switch
{
    enum SwitchState switchState;
    unsigned int bit;
    volatile uint8_t *port;
    unsigned int timer;
};

struct Switch switchInfo[] =
    {
        { SW_OPEN,
        SW1, &PINB },
        { SW_OPEN,
        SW2, &PINB },
        { SW_OPEN,
        SW3, &PINB },
        { SW_OPEN,
        SW4, &PINB },
        { SW_OPEN,
        SW5, &PINB },
        { SW_OPEN,
        SW6, &PINB },
        { SW_OPEN,
        SW7, &PINB },
        { SW_OPEN,
        SW8, &PINB },
        { SW_OPEN, PORTC7, &PINC } };

void switchFSM(int sw);

void switchFSMtimer(void)
{
    switchFSM(0);
    switchFSM(1);
    switchFSM(2);
    switchFSM(3);
    switchFSM(4);
    switchFSM(5);
    switchFSM(6);
    switchFSM(7);
    switchFSM(8);
}

void switchFSM(int sw)
{
    struct Switch *swPtr = &switchInfo[sw];

    switch (swPtr->switchState)
    {
    case SW_OPEN:
        if ((*(swPtr->port) & _BV(swPtr->bit)) == 0)
        {
            // switch was pressed
            swPtr->timer = 100 / TIMER0_PERIOD;
            swPtr->switchState = SW_PRESSED;
        }
        break;

    case SW_PRESSED:
        swPtr->timer--;
        if (swPtr->timer == 0)
        {
            if ((*(swPtr->port) & _BV(swPtr->bit)) == 0)
            {
                swPtr->switchState = SW_CLOSED;
            } else
            {
                swPtr->switchState = SW_OPEN;
            }
        }
        break;

    case SW_CLOSED:
        if ((*(swPtr->port) & _BV(swPtr->bit)) != 0)
        {
            // switch was released
            swPtr->timer = 100 / TIMER0_PERIOD;
            swPtr->switchState = SW_RELEASED;
        }
        break;

    case SW_RELEASED:
        swPtr->timer--;
        if (swPtr->timer == 0)
        {
            if ((*(swPtr->port) & _BV(swPtr->bit)) != 0)
            {
                swPtr->switchState = SW_OPEN;
            } else
            {
                swPtr->switchState = SW_CLOSED;
            }
        }
        break;
    }
}

bool isClosed(int sw)
{
    struct Switch *swPtr = &switchInfo[sw];

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
