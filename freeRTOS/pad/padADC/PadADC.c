/*
* padADC.c
*
* Created: 02/15/2022
* Author : waynej
*/

#include "config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "timers.h"

#include "PadADC.h"

/*
* $30 ($50) SFIOR  ADTS2 ADTS1 ADTS0
* $07 ($27) ADMUX  REFS1 REFS0 ADLAR MUX4 MUX3 MUX2  MUX1  MUX0
* $06 ($26) ADCSRA ADEN  ADSC  ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
* $05 ($25) ADCH   ADC   Data Register High Byte
* $04 ($24) ADCL   ADC   Data Register Low Byte
*
* ADSC  ADC Start Conversion bit
* ADATE ADC Auto Trigger Enable bit
* ADEN  ADC Enable bit
* ADMUX ADC Multiplexer Selection Register
* ADLAR ADC Left Adjust Result
* ADIF  ADC Interrupt Flag
* ADIE  ADC Interrupt Enable
* ADPS  ADC Prescaler Select Bits
* ADCL  ADC Data Register
* ADCH  ADC Data Register
* ADTS  ADC Auto Trigger Source
*
* REFS1 REFS0 Voltage Reference Selection
* 0     0     AREF, Internal Vref turned off
* 0     1     AVCC with external capacitor at AREF pin
* 1     0     Reserved
* 1     1     Internal 2.56V Voltage Reference with external capacitor at AREF pin
*
*/

struct AdcChannel
{
    bool enabled;
    bool valid;
    unsigned int value;
    unsigned int count;
    unsigned int admux;
};

volatile struct AdcChannel adcChannel[8];

int channelEnabled = 0;

#define ADREF _BV(REFS0) /*|_BV(REFS1)*/ | 0

void adc_init(int channel, unsigned int admux)
{
    adcChannel[channel].enabled = true;
    adcChannel[channel].admux = admux;
    adcChannel[channel].count = 0;
    adcChannel[channel].valid = false;
}


/* Initialize the ADC */
int padADCInit()
{
    adc_init(0, ADMUX_REF_INT | ADMUX_AD0);
    adc_init(1, ADMUX_REF_INT | ADMUX_AD1);
    adc_init(2, ADMUX_REF_INT | ADMUX_AD2);
    adc_init(3, ADMUX_REF_INT | ADMUX_AD3);
    adc_init(4, ADMUX_REF_INT | ADMUX_122);
    adc_init(5, ADMUX_REF_INT | ADMUX_GND);

    /* Initialize and enable ADC */
    ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    /* Enable ADC, ADC interrupt, and set prescaler to x128 */

    /* Initiate first conversion */
    for (channelEnabled = 0; channelEnabled < 8; ++channelEnabled)
    {
        if (adcChannel[channelEnabled].enabled)
        {
            ADMUX = adcChannel[channelEnabled].admux;
            ADCSRA |= _BV(ADSC); /* Start conversion */
            break;
        }
    }

    return 0;
}

ISR( ADC_vect)
{
    int value;

    value = ADCL + (ADCH << 8); /* Read Low and High data */

    adcChannel[channelEnabled].value = value;
    adcChannel[channelEnabled].valid = true;
    adcChannel[channelEnabled].count++;

    for (int x = 0; x < 8; ++x)
    {
        channelEnabled++;
        channelEnabled %= 8;
        if (adcChannel[channelEnabled].enabled)
        {
            ADMUX = adcChannel[channelEnabled].admux;
            ADCSRA |= _BV(ADSC) | _BV(ADIF); /* Start next conversion */
            break;
        }
    }
}

