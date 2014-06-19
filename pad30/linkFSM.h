/*
 * linkFSM.h
 *
 * Created: 5/25/2014 3:17:18 PM
 *  Author: waynej
 */ 


#ifndef LINKFSM_H_
#define LINKFSM_H_

#include "../../mCommon/zb.h"

void linkFSMinit( char * nodeName );
void linkFSMtimer( void );
bool linkFSMpkt(zbPkt *pkt);
bool linkFSMready(void);

#endif /* LINKFSM_H_ */