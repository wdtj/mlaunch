/*
 * Led.h
 *
 * Created: 2/26/2015 8:59:24 PM
 *  Author: waynej
 */

#include <avr/io.h>

#ifndef PADLED_H_
#define PADLED_H_

typedef
enum LedsState {
    PAD_LED_OFF,
    PAD_LED_GREEN,
    PAD_LED_YELLOW,
    PAD_LED_RED,
    PAD_LED_BLINK_GREEN,
    PAD_LED_BLINK_YELLOW,
    PAD_LED_BLINK_RED
} LedState;

int padLedInit(void);
void padLed(LedState color, int number);

#endif /* PADLED_H_ */
