/*
 * Timer1.h
 *
 * Created: 3/7/2014 8:47:28 PM
 *  Author: waynej
 */ 


#ifndef TIMER1_H_
#define TIMER1_H_

void timer1_init_normal(int ps);
void timer1_init_ctc(int ps, void (*ptr)(void));

void timer1_set_normal();
void timer1_set_ctc(unsigned int tc);

#endif /* TIMER1_H_ */