/*
 * launchFSM.h
 *
 * Created: 2/22/2014 1:54:01 PM
 *  Author: waynej
 */ 


#ifndef LAUNCHFSM_H_
#define LAUNCHFSM_H_

void padFSMtimer(int padNum);
void SWEnable(int sw);
void SWReset(int sw);
void padEnable(int sw);
void padReset(int sw);
void padLaunch(int sw);
void padUnlaunch(int sw);

#endif /* LAUNCHFSM_H_ */