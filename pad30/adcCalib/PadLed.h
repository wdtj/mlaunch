/*
 * Led.h
 *
 * Created: 2/26/2015 8:59:24 PM
 *  Author: waynej
 */ 

#include <avr/io.h>

#ifndef PADLED_H_
#define PADLED_H_

#define OFF 0
#define RED 1
#define GREEN 2
#define YELLOW 3

void PadLedInit(void);
void PadLed(int color, int number);

#endif /* PADLED_H_ */