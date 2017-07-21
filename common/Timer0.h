/*
 * timer0.h
 *
 * Created: 1/27/2014 1:06:35 PM
 *  Author: waynej
 */

#ifndef TIMER0_H_
#define TIMER0_H_

#define CS_NULL 0
#define CS_1    1
#define CS_8    2
#define CS_64   3
#define CS_256  4
#define CS_1024 5
#define CS_FALL 6
#define CS_RISE 7

void timer0_init(int ps, void (*ptr)(void));
void timer0_set(unsigned int tc);

#endif /* TIMER0_H_ */
