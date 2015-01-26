#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "config.h"

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

unsigned int adc_read(int channel)
{
  unsigned int adcValue;

  ADMUX=_BV(REFS0) | channel;  /* AVcc as Aref, select ADC4 */
  
  ADCSRA=_BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
							/* Enable ADC and set prescaler to x128 */

  ADCSRA|=_BV(ADSC);		/* Start conversion */

  loop_until_bit_is_set(ADCSRA, ADIF);
							/* Wait for conversion */

  adcValue=ADCL+(ADCH<<8);	/* Read Low and High data */
  
  ADCSRA|=_BV(ADIF);		/* Reenable ADC */

  return adcValue;
}

struct AdcChannel 
{
	bool enabled;
	bool valid;
	unsigned int value;
	unsigned int count;
	unsigned int admux;
};

volatile struct AdcChannel adcChannel[8];

int channelEnabled=0;

#define ADREF _BV(REFS0) /*|_BV(REFS1)*/ | 0

void adc_init(int channel, unsigned int admux)
{
	adcChannel[channel].enabled=true;
	adcChannel[channel].admux=admux;
	adcChannel[channel].count=0;
	adcChannel[channel].valid=false;
}

void adc_run()
{
	/* Initialize and enable ADC */
	ADCSRA=_BV(ADEN) | _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
							/* Enable ADC, ADC interrupt, and set prescaler to x128 */

	/* Initiate first conversion */
	for (channelEnabled=0; channelEnabled<8; ++channelEnabled)
	{
		if (adcChannel[channelEnabled].enabled)
		{
			ADMUX = adcChannel[channelEnabled].admux;  
			ADCSRA |= _BV(ADSC);		/* Start conversion */
			break;
		}
	}
}

ISR(ADC_vect)
{
	int value;
	
	value=ADCL+(ADCH<<8);	/* Read Low and High data */
	
	adcChannel[channelEnabled].value=value;
	adcChannel[channelEnabled].valid=true;
	adcChannel[channelEnabled].count++;

	for(int x=0; x<8; ++x)
	{
		channelEnabled++;
		channelEnabled%=8;
		if (adcChannel[channelEnabled].enabled)
		{
			ADMUX = adcChannel[channelEnabled].admux;
			ADCSRA |= _BV(ADSC)|_BV(ADIF);		/* Start next conversion */
			break;
		}
	}
}

unsigned int adc_value(int channel)
{
	return adcChannel[channel].value;
}

unsigned int adc_valid(int channel)
{
	return adcChannel[channel].valid;
}

unsigned int adc_count(int channel)
{
	return adcChannel[channel].count;
}
