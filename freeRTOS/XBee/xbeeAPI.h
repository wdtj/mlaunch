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

typedef struct xbeeNode {
    struct xbeeNode *next;
    zbAddr addr;
    zbNetAddr netAddr;
    char *name;
    unsigned char type;
    unsigned char hopCount;
    unsigned int *hops;
} xbeeNode;

typedef struct xbeeEvent {
    void(*data)(char* data, int length);
    void(*error)(int code, int state);
    void(*reset)(int code);
    void(*config)();
} xbeeEvent;

int xbeeFSMInit(int baud, int txQueueSize, int rxQueueSize, struct xbeeEvent *event);
int networkDiscovery();
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