/*
 * commFSM.h
 *
 * Created: 2/27/2014 7:38:37 PM
 *  Author: waynej
 */ 


#ifndef COMMFSM_H_
#define COMMFSM_H_

void linkInit(bool notInitialized);
void linkFSM();
void linkPkt(unsigned char *data, unsigned int length);
void linkData( zbRx* rxPkt, unsigned int length );
void linkTimer(void);
bool isLinkReady(void);

#endif /* COMMFSM_H_ */