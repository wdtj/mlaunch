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

typedef struct XbeeEventMsg {
    char eventType;
    char data[80];
} XbeeEventMsg;


typedef struct XbeeEvent {
    void(*data)(char* data, int length);
    void(*error)(int code, int state);
    void(*reset)(int code);
    void(*config)();
} XbeeEvent;

int xbeeFSMInit(int baud, int txQueueSize, int rxQueueSize,
                size_t eventQueueSize);
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
QueueHandle_t xbeeGetEventQueue();
#endif /* XBEE_H_ */