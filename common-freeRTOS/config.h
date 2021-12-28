/*
 * config.h
 *
 * Created: 2/10/2014 5:18:14 PM
 *  Author: waynej
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "stdbool.h"

#define F_CPU 16000000UL

#define set(port, bit) {port|=_BV(bit);}
#define reset(port, bit) {port&=~_BV(bit);}

//#define SWAPPED_LED true

#if false
#define	MARK_COMM_ON() PORTD|=_BV(7)
#define	MARK_COMM_OFF() PORTD&=~_BV(7)
#else
#define	MARK_COMM_ON() 
#define	MARK_COMM_OFF() 
#endif

#if false
#define	MARK_ADC_ON() PORTD|=_BV(7)
#define	MARK_ADC_OFF() PORTD&=~_BV(7)
#else
#define	MARK_ADC_ON()
#define	MARK_ADC_OFF()
#endif

#if false
#define	MARK_TIMER0_ON() PORTD|=_BV(7)
#define	MARK_TIMER0_OFF() PORTD&=~_BV(7)
#else
#define	MARK_TIMER0_ON()
#define	MARK_TIMER0_OFF()
#endif

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define HIGH_WATER(max, value) ((max)=MAX((max), (value)))

#endif /* CONFIG_H_ */
