#include <avr/io.h>

#define ADMUX_REF_AREF 0						// AREF, Internal Vref turned off
#define ADMUX_REF_AVCC _BV(REFS0)				// AVCC with external capacitor at AREF pin
#define ADMUX_REF_INT _BV(REFS0) | _BV(REFS1)	// Internal 2.56V Voltage Reference with external capacitor at AREF pin

#define ADMUX_AD0       0	// ADC0
#define ADMUX_AD1       1	// ADC1
#define ADMUX_AD2       2	// ADC2
#define ADMUX_AD3       3	// ADC3
#define ADMUX_AD4       4	// ADC4
#define ADMUX_AD5       5	// ADC5
#define ADMUX_AD6       6	// ADC6
#define ADMUX_AD7       7	// ADC7

#define ADMUX_AD00X10   8	// Differential Input +ADC0 -ADC0 10x gain
#define ADMUX_AD10X10   9	// Differential Input +ADC1 -ADC0 10x gain
#define ADMUX_AD00X200 10	// Differential Input +ADC0 -ADC0 200x gain
#define ADMUX_AD10X200 11	// Differential Input +ADC1 -ADC0 200x gain
#define ADMUX_AD22X10  12	// Differential Input +ADC2 -ADC2 10x gain
#define ADMUX_AD32X10  13	// Differential Input +ADC3 -ADC2 10x gain
#define ADMUX_AD22X200 14	// Differential Input +ADC2 -ADC2 10x gain
#define ADMUX_AD32X200 15	// Differential Input +ADC3 -ADC2 10x gain

#define ADMUX_AD01X1   16	// Differential Input +ADC0 -ADC1 1x gain
#define ADMUX_AD11X1   17	// Differential Input +ADC1 -ADC1 1x gain
#define ADMUX_AD21X1   18	// Differential Input +ADC2 -ADC1 1x gain
#define ADMUX_AD31X1   19	// Differential Input +ADC3 -ADC1 1x gain
#define ADMUX_AD41X1   20	// Differential Input +ADC4 -ADC1 1x gain
#define ADMUX_AD51X1   21	// Differential Input +ADC5 -ADC1 1x gain
#define ADMUX_AD61X1   22	// Differential Input +ADC6 -ADC1 1x gain
#define ADMUX_AD71X1   23	// Differential Input +ADC7 -ADC1 1x gain

#define ADMUX_AD02X1   24	// Differential Input +ADC0 -ADC2 1x gain
#define ADMUX_AD12X1   25	// Differential Input +ADC1 -ADC2 1x gain
#define ADMUX_AD22X1   26	// Differential Input +ADC2 -ADC2 1x gain
#define ADMUX_AD32X1   27	// Differential Input +ADC3 -ADC2 1x gain
#define ADMUX_AD42X1   28	// Differential Input +ADC4 -ADC2 1x gain
#define ADMUX_AD52X1   29	// Differential Input +ADC5 -ADC2 1x gain
#define ADMUX_122      30	// 1.22v ref
#define ADMUX_GND      31	// 0v ref

unsigned int adc_read(int channel);
 
void adc_init(int channel, unsigned int admux);
void adc_run(void);

unsigned int adc_value(int channel);
unsigned int adc_valid(int channel);
unsigned int adc_count(int channel);
