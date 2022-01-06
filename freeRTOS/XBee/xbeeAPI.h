/*
 * xbee.h
 *
 * Created: 12/31/2021 12:53:13 PM
 *  Author: waynej
 */


#ifndef XBEE_H_
#define XBEE_H_

#include "zb.h"

#define XBEE_TIMEOUT -1

int xbeeFSMInit(int baud, int txQueueSize, int rxQueueSize, void(*data)(),
                void(*error)(int code, int state));
void networkDiscovery();
void xbeeWait();
void xbeeTx(char *msg, int length,
            zbAddr controllerAddress,
            zbNetAddr controllerNAD);
void xbeeExpTx(char *msg, int length,
               zbAddr controllerAddress,
               zbNetAddr controllerNAD,
               char src, char dest,
               unsigned short clust,
               unsigned short prof,
               char radius,
               char opt);





#endif /* XBEE_H_ */