/*
 * xbee.h
 *
 * Created: 12/31/2021 12:53:13 PM
 *  Author: waynej
 */ 


#ifndef XBEE_H_
#define XBEE_H_


int xbeeFSMInit(int baud, int txQueueSize, int rxQueueSize, void(*data)(), void(*error)(int code));
void networkDiscovery();
void xbeeWait();
void xbeeTx(char *msg, int length, zbAddr controllerAddress, zbNetAddr controllerNAD);




#endif /* XBEE_H_ */