/*
 * relayTest.c
 *
 * Created: 11/21/2014 4:35:34 PM
 *  Author: waynej
 */

#include "../../common/config.h"

#include <avr/io.h>
#include <util/delay.h>

#define Launch1 PORTA4
#define Enable1 PORTA5
#define Launch2 PORTA6
#define Enable2 PORTA7

#define setLaunch1()   set(PORTA, Launch1)
#define resetLaunch1() reset(PORTA, Launch1)
#define setEnable1()   set(PORTA, Enable1)
#define resetEnable1() reset(PORTA, Enable1)
#define setLaunch2()   set(PORTA, Launch2)
#define resetLaunch2() reset(PORTA, Launch2)
#define setEnable2()   set(PORTA, Enable2)
#define resetEnable2() reset(PORTA, Enable2)

int main(void)
{
    DDRA = _BV(Launch1) | _BV(Enable1) | _BV(Launch2) | _BV(Enable2);// PA4-7 output

    while (1)
    {
        setEnable1()
        ;
        _delay_ms(1000);
        setLaunch1()
        ;
        _delay_ms(1000);
        resetLaunch1()
        ;
        _delay_ms(1000);
        resetEnable1()
        ;
        _delay_ms(1000);

        setEnable2()
        ;
        _delay_ms(1000);
        setLaunch2()
        ;
        _delay_ms(1000);
        resetLaunch2()
        ;
        _delay_ms(1000);
        resetEnable2()
        ;
        _delay_ms(1000);
    }
}
