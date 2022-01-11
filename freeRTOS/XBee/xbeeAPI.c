/*
 * XBee.c
 *
 * Created: 12/31/2021 12:36:08 PM
 * Author : waynej
 */

#include "config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include <string.h>

#include "xbeeAPI.h"
#include "uart.h"
#include "zb.h"
#include "mlaunchlib.h"

#define XB_TIMEOUT 10000

void(*dataCallback)(char *data, int length);
void(*errorCallback)(int code, int state);

TimerHandle_t timeoutTimer;

#define XBEE_TRACE
#ifdef XBEE_TRACE
char trace[256];
char *tracePtr = trace;


/*
 *  FSM State
 */
volatile static enum {
    Idle = 0,
    NodeIdentity = 1,
    ChannelVerify = 2,
    JoinNotifications = 3,
    NetworkWatchdog = 4,
    GetChannel = 5,
    GetReceivedSignalStrength = 6,
    NodeDiscovery = 7,
    txInProgress = 8
} apiState = NodeIdentity;

/* xbee statistics */
static unsigned int channel = 0;
static unsigned int signalStrength = 0;

static char *nodeID = "testNode2";

static zbAddr controllerAddr = {{0, 0, 0, 0, 0, 0, 0, 0}};
static zbNetAddr controllerAddrNad = {{0, 0}};

SemaphoreHandle_t nodeListGuard = NULL;
static xbeeNode *nodeList = NULL;

/* Semaphore to guard FSM integrity.
 * This is taken when the FSM starts a sequence and given when complete.
 */
SemaphoreHandle_t xbeeBusy = NULL;

/*
 *  Start up FSM for initial sequence.
 */
void xbeeInit(char *ni)
{
    apiState = NodeIdentity;
    zb_ni(1, ni);
}

/*
 * Timeout waiting for a response.
 *
 * Call error callback with code and state, then reset link
 */

void timeout(TimerHandle_t xTimer)
{
    (*errorCallback)(XBEE_TIMEOUT, apiState);
    xbeeInit(nodeID);
}

/* Handle an AT Response */
void handleATResp(struct zbATResponse *resp, int length)
{
    switch(apiState) {
    // We have sent the ni packet, now we have a response
    case NodeIdentity: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_jv(2, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = ChannelVerify;
            }
            break;
        }

    // We have sent the Join Verify packet, now we have a response
    case ChannelVerify: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_jn(3, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = JoinNotifications;
            }
            break;
        }

    // We have sent the Join Notifications packet, now we have a response
    case JoinNotifications: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_nw(4, 1);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = NetworkWatchdog;
            }
            break;
        }

    // We have sent the Network Watchdog Timeout packet, now we have a response
    case NetworkWatchdog: {
            if(resp->status == ZB_AT_STATUS_OK) {
                zb_ch(5);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = GetChannel;
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case GetChannel: {
            if(resp->status == ZB_AT_STATUS_OK) {
                channel = resp->data[0];
                zb_db(6);
                xTimerStart(timeoutTimer, (XB_TIMEOUT / portTICK_RATE_MS));
                apiState = GetReceivedSignalStrength;
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case GetReceivedSignalStrength: {
            if(resp->status == ZB_AT_STATUS_OK) {
                signalStrength = resp->data[0];
                xTimerDelete(timeoutTimer, pdMS_TO_TICKS(XB_TIMEOUT));
                apiState = Idle;
                xSemaphoreGive(xbeeBusy);
            }
            break;
        }

    // We have sent the Received Signal Strength packet, now we have a response
    case NodeDiscovery: {
            if(resp->status == ZB_AT_STATUS_OK) {
                xSemaphoreTake(nodeListGuard, 0);

                struct ATNDData *nid = (struct ATNDData *) resp->data;
                xbeeNode *node = nodeList;
                while(node != NULL) {
                    if(zbAddrCmp(nid->addr, node->addr)) {
                        break;
                    }
                    node = node->next;
                }

                if(node == NULL) {
                    node = malloc(sizeof(struct xbeeNode));
                    assert(node != 0);
                    node->next = nodeList;
                    memcpy(&node->addr, &nid->addr, sizeof(node->addr));
                    memcpy(&node->netAddr, &nid->netAddr, sizeof(node->netAddr));
                    node->name = strdup(nid->ni);
                    nid = (struct ATNDData *) resp->data + strlen(node->name);
                    node->type = nid->type;
                    nodeList = node;

                }
                xSemaphoreGive(nodeListGuard);
            }
        }
        break;

    // We have received a packet we are not prepared for.
    default: {
            assert(0);

            break;
        }
    }
}

/*
 * Handle Modem Status message.
 */
void handleModemStatus(struct zbModemStatus *resp, int length)
{
    enum status status = resp->status;
    switch(status) {
    case zb_mdm_hwrst:
    case zb_mdm_wdrst:
    case zb_mdm_assoc:
    case zb_mdm_start:
        // TODO: Do something here
        break;
    case zb_mdm_disassoc: {
            // We lost the network, trash the list
            xSemaphoreTake(nodeListGuard, 0);

            xbeeNode *node = nodeList;
            while(node != NULL) {
                node = node->next;
                free(node);
            }
            nodeList = NULL;
            xSemaphoreGive(nodeListGuard);
        }
    }
}

/*
 * Handle Transmit Status message.
 *
 * This indicates that a transmission is complete.
 */
void handleTransmitStatus(struct zbTransmitStatus *resp, int length)
{
    apiState = Idle;
}

/*
 * Handle Receive message.
 *
 * This indicates that data has been received.  Pass to user.
 */
void handleReceivePacket(struct zbRx *resp, int length)
{
    int headerLength = ((char *)resp->data) - ((char *)resp);
    (*dataCallback)((char *)resp->data, length - headerLength);
}

/*
 * Handle Explicit Receive message.
 *
 * This indicates that data has been received.  Pass to user.
 */
void handleExplicitRx(struct zbExpRx *resp, int length)
{
    int headerLength = ((char *)resp->data) - ((char *)resp);
    (*dataCallback)((char *)resp->data, length - headerLength);
}

void handleNodeID(struct zbNID *resp, int length)
{
    xSemaphoreTake(nodeListGuard, 0);

    xbeeNode *node = nodeList;
    while(node != NULL) {
        if(zbAddrCmp(resp->addr, node->addr)) {
            break;
        }
        node = node->next;
    }

    if(node == NULL) {
        node = malloc(sizeof(struct xbeeNode));
        assert(node != 0);
        node->next = nodeList;
        memcpy(&node->addr, &resp->addr, sizeof(node->addr));
        memcpy(&node->netAddr, &resp->netAddr, sizeof(node->netAddr));
        node->name = strdup(resp->ni);
        assert(node->name != 0);
        node->type = resp->type;
        nodeList = node;
    }
    xSemaphoreGive(nodeListGuard);
}

void handleRouteRecord(struct zbRR *resp, int length)
{
    assert(0);      // TODO
}

void handleMany2OneRouteRecord(struct zbManyToOneRouteRequestIndicator *resp,
                               int length)
{
    controllerAddr = resp->dest;
    controllerAddrNad = resp->destNad;
}

/*
  Identify the received packet and dispatch to the proper handler.
  */
void xbeeReceivePacket(unsigned char *pkt, unsigned int length)
{
    if(pkt[0] == ZB_AT_COMMAND_RESPONSE) {
        handleATResp((struct zbATResponse *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_MODEM_STATUS) {
        handleModemStatus((struct zbModemStatus *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_TRANSMIT_STATUS) {
        handleTransmitStatus((struct zbTransmitStatus *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_RECEIVE_PACKET) {
        handleReceivePacket((struct zbRx *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_EXPLICIT_RX_INDICATOR) {
        handleExplicitRx((struct zbExpRx *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_NODE_IDENTIFICATION) {
        handleNodeID((struct zbNID *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_ROUTE_RECORD) {
        handleRouteRecord((struct zbRR *)(pkt + 1), length - 1);
    } else if(pkt[0] == ZB_MANY_TO_ONE_ROUTE_REQUEST_INDICATOR) {
        handleMany2OneRouteRecord((struct zbManyToOneRouteRequestIndicator *)(pkt + 1),
                                  length - 1);
    }
}

int networkDiscovery()      // TODO
{
    xSemaphoreTake(xbeeBusy, portMAX_DELAY);

    zb_nd(10);
    apiState = NodeDiscovery;
    vTaskDelay(0x3c * 100);

    xSemaphoreGive(xbeeBusy);

    int count = 0;
    xbeeNode *node = nodeList;
    while(node) {
        count++;
        node = node->next;
    }

    return count;
}

/* API call to wait for the FSM guard */
void xbeeWait()
{
    xSemaphoreTake(xbeeBusy, portMAX_DELAY);
    xSemaphoreGive(xbeeBusy);
}

/* API call to Transmit data */
void xbeeTx(char *msg, int length,
            zbAddr controllerAddress,
            zbNetAddr controllerNAD)
{
    while(!xSemaphoreTake(xbeeBusy, 0)) {
        taskYIELD();
    }
    apiState = txInProgress;
    zb_tx(11, controllerAddress, controllerNAD, 0, 0, msg, length);
    while(apiState == txInProgress) {
        taskYIELD();
    }
    xSemaphoreGive(xbeeBusy);
}

/* API call to Explicit Transmit data */
void xbeeExpTx(char *msg, int length,
               zbAddr controllerAddress,
               zbNetAddr controllerNAD,
               char src, char dest,
               unsigned short clust,
               unsigned short prof,
               char radius,
               char opt)
{

    while(!xSemaphoreTake(xbeeBusy, 0)) {
        taskYIELD();
    }
    apiState = txInProgress;
    zb_tx_ex(12,
             controllerAddress, // dest addr
             controllerNAD,     // dest nad
             src,               // src endpoint
             dest,              // dst endpoint
             clust,             // cluster
             prof,              // profile
             0,                 // radius
             0,                 // options
             msg, length);
    while(apiState == txInProgress) {
        taskYIELD();
    }
    xSemaphoreGive(xbeeBusy);
}

/*
 * Main xbee task
 */
void xbeeTask(void *parameter)
{
    //int depth;

    //depth=uxTaskGetStackHighWaterMark(NULL);

    timeoutTimer = xTimerCreate("Timeout",
                                (XB_TIMEOUT / portTICK_RATE_MS),
                                pdFALSE,
                                NULL,
                                timeout);
    assert(timeoutTimer != NULL)  /* failure! */

    xbeeInit(nodeID);

    while(1) {
        while(uart_rxReady()) {
            char ch = uart_rxc(0);
            *(tracePtr++) = ch;
            if(tracePtr > trace + sizeof trace) {
                tracePtr = trace;
            }
            zbReceive(ch);
        }
        //depth=uxTaskGetStackHighWaterMark(NULL);
    }
}

void xbee_write(char *buff, int size)
{
    char *ptr;
    int count;

    for(ptr = buff, count = size; count > 0; --count) {
        *(tracePtr++) = *(ptr++);
        if(tracePtr > trace + sizeof trace) {
            tracePtr = trace;
        }
    }
    uart_txBuff(buff, size);
}
#endif

/* API call to Initialize the xbee modem */
int xbeeFSMInit(
    int baud,           // Baud rate (Bits per second)
    int txQueueSize,    // Transmit queue size
    int rxQueueSize,    // Receive queue size
    void(*data)(),      // Data received callback
    void(*error)(int, int))  // Error received callback
{
    dataCallback = data;
    errorCallback = error;

    // Mutex to prevent multiple threads from using xbee
    xbeeBusy = xSemaphoreCreateBinary();
    assert(xbeeBusy != NULL);
    xSemaphoreGive(xbeeBusy);

    // Mutex to prevent multiple threads from updating the nodeList during Network Discovery
    nodeListGuard = xSemaphoreCreateBinary();
    assert(nodeListGuard != NULL);
    xSemaphoreGive(nodeListGuard);

    xSemaphoreTake(xbeeBusy, 0);

    uart_init(baud, txQueueSize, rxQueueSize);

    zbInit(&xbee_write, &xbeeReceivePacket);

    int rc = xTaskCreate(
                 xbeeTask,      // Function to be called
                 "xbeeTask",   // Name of task
                 256, // Stack size
                 NULL,           // Parameter to pass
                 1,              // Task priority
                 NULL);          // Created Task

    if(rc != pdPASS) {
        assert(rc == pdPASS)     /* failure! */
    }
    return rc;
}

