/*
 * linkFSM.h
 *
 * Created: 5/25/2014 3:17:18 PM
 *  Author: waynej
 */

#ifndef LINKFSM_H_
#define LINKFSM_H_

#include "../common/zb.h"

void linkFSMinit(char * nodeName);
void linkFSMtimer(void);
void linkFSMpkt(zbPkt *pkt);
void linkFSMToDo(void);
enum LINK_STATUS linkFSMStatus(void);

#endif /* LINKFSM_H_ */
