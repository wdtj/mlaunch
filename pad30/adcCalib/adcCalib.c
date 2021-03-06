/*
 * adcCalib.c
 *
 * Created: 1/15/2015 7:20:30 PM
 *  Author: waynej
 */

#include "../../common/config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "../../common/adc.h"
#include <limits.h>
#include "PadLed.h"

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

unsigned int cd1values[105];
unsigned int cd2values[105];
unsigned int batt1values[105];
unsigned int batt2values[105];

int main(void)
{
    PadLedInit();

    DDRA = _BV(Launch1) | _BV(Enable1) | _BV(Launch2) | _BV(Enable2);// PA4-7 output

    setEnable1()
    ;
    setEnable2()
    ;

    adc_init(0, ADMUX_REF_INT | ADMUX_AD0);
    adc_init(1, ADMUX_REF_INT | ADMUX_AD1);
    adc_init(2, ADMUX_REF_INT | ADMUX_AD2);
    adc_init(3, ADMUX_REF_INT | ADMUX_AD3);
    adc_init(4, ADMUX_REF_INT | ADMUX_122);
    adc_init(5, ADMUX_REF_INT | ADMUX_GND);

    adc_run();

    sei();								// enable interrupts

    // Wait till we've completed the initial voltage checks.
    while ((!adc_valid(0)) || (!adc_valid(1)) || (!adc_valid(2))
            || (!adc_valid(3)) || (!adc_valid(4)) || (!adc_valid(5)))
    {
    }

    unsigned int cd1min = UINT_MAX;
    unsigned int cd1max = 0;
    unsigned long cd1sum = 0;

    unsigned int cd2min = UINT_MAX;
    unsigned int cd2max = 0;
    unsigned long cd2sum = 0;

    unsigned int batt1min = UINT_MAX;
    unsigned int batt1max = 0;
    unsigned long batt1sum = 0;

    unsigned int batt2min = UINT_MAX;
    unsigned int matt2max = 0;
    unsigned long batt2sum = 0;

    unsigned count = 0;

    while (adc_count(0) <= 100)
    {
    }

    while (adc_count(0) <= 200)
    {
        int value;

        for (int last = adc_count(0); last == adc_count(0);)
        {
        }

        value = adc_value(0);
        cd1values[count] = value;
        cd1min = MIN(cd1min, value);
        cd1max = MAX(cd1max, value);
        cd1sum += value;

        value = adc_value(1);
        cd2values[count] = value;
        cd2min = MIN(cd2min, value);
        cd2max = MAX(cd2max, value);
        cd2sum += value;

        value = adc_value(2);
        batt1values[count] = value;
        batt1min = MIN(batt1min, value);
        batt1max = MAX(batt1max, value);
        batt1sum += value;

        value = adc_value(3);
        batt2values[count] = value;
        batt2min = MIN(batt2min, value);
        matt2max = MAX(matt2max, value);
        batt2sum += value;

        count++;
    }

    int cd1ave = cd1sum / count;
    int cd2ave = cd2sum / count;
    int batt1ave = batt1sum / count;
    int batt2ave = batt2sum / count;

    long v1 = (((long) batt1ave) - 20) * 100000 / 62428;
    long v2 = (((long) batt2ave) - 20) * 100000 / 62428;

    long r1cd;
    long r1batt;
    long r1;
    if (batt1ave < batt2ave)
    {
        r1cd = ((long) cd1ave) * 139;
        r1batt = ((long) batt2ave) * 133;
        r1 = (r1batt - r1cd + 5659) / 100;
    } else
    {
        r1cd = ((long) cd1ave) * 148;
        r1batt = ((long) batt1ave) * 7;
        r1 = (r1cd - r1batt + 3798) / 100;
    }

    long r2cd;
    long r2batt;
    long r2;

    if (batt1ave < batt2ave)
    {
        r2cd = ((long) cd2ave) * 139;
        r2batt = ((long) batt2ave) * 133;
        r2 = (r2batt - r2cd + 5659) / 100;
    } else
    {
        r2cd = ((long) cd2ave) * 148;
        r2batt = ((long) batt1ave) * 7;
        r2 = (r2cd - r2batt + 3798) / 100;
    }

    while (true)
    {
    };
}
