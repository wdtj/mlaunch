/*
 * mCommon.h
 *
 * Created: 1/25/2014 3:03:19 PM
 *  Author: waynej
 */ 


#ifndef MCOMMON_H_
#define MCOMMON_H_

extern void dispatcherInit(int size);
extern void dispatcher(void);
extern void spawn(void (*ptr)(void));

#endif /* MCOMMON_H_ */