/*
 * switch.c
 *
 * Created: 1/16/2022 4:51:14 PM
 *  Author: waynej
 */

#include "config.h"

#include <avr/io.h>

#include "FreeRTOS.h"

#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "mlaunchlib.h"

#include "switchAPI.h"

/*
 * This module tracks the state of the switches.  Every 10 ms we scan the switches
 * and if we detect that the state has changed we update the state.  Since mechanical
 * switches bounce, we go from an open to closed we change the state to SW_PRESSED.
 * If on the next scan the switch is still closed, we change the state to SW_CLOSED.
 * We do the same on opening the switch.
*/

/* Switch information */
typedef struct Switch {
    SwitchMsg switchMsg;
    unsigned int bit;
    volatile uint8_t *port;
} Switch;

/*
 * We create an array of Switch structures for each switch.
 */
struct Switch **switchInfo;
size_t numberOfSwitches = 0;    // Number of switches defined

/*
 * When a changes occurs we notify the user with a message.
 */
QueueHandle_t switchQueue;

/* Timer to scan the switches */
TimerHandle_t  switchTimer = NULL;

/*
 * The FSM to debounce the switches
 */
void switchFSM(int sw)
{
    struct Switch *swPtr = switchInfo[sw];
    if(swPtr == NULL) {
        return;
    }

    switch(swPtr->switchMsg.switchState) {
    case SW_OPEN:
        if((*(swPtr->port) & _BV(swPtr->bit)) == 0) {
            // switch was pressed
            swPtr->switchMsg.switchState = SW_PRESSED;
        }
        break;

    case SW_PRESSED:
        if((*(swPtr->port) & _BV(swPtr->bit)) == 0) {
            swPtr->switchMsg.switchState = SW_CLOSED;
            if(switchQueue) {
                xQueueSend(switchQueue, &swPtr->switchMsg, 0);
            }
        } else {
            swPtr->switchMsg.switchState = SW_OPEN;
        }
        break;

    case SW_CLOSED:
        if((*(swPtr->port) & _BV(swPtr->bit)) != 0) {
            // switch was released
            swPtr->switchMsg.switchState = SW_RELEASED;
        }
        break;

    case SW_RELEASED:
        if((*(swPtr->port) & _BV(swPtr->bit)) != 0) {
            swPtr->switchMsg.switchState = SW_OPEN;
            if(switchQueue) {
                xQueueSend(switchQueue, &swPtr->switchMsg, 0);
            }
        } else {
            swPtr->switchMsg.switchState = SW_CLOSED;
        }
        break;
    }
}


/* Timer task to scan the switches */
void switchTask(TimerHandle_t xTimer)
{
    int i;
    for(i = 0; i < numberOfSwitches; ++i) {
        switchFSM(i);
    }

    taskYIELD();
}

/* The user needs to tell us about the switches we have */
struct Switch * switchAdd(
    char id,                // Id to pass back to the user
    unsigned int bit,       // Bit mask for the IO pin of the switch
    volatile uint8_t *port)          // IO port/address for the switch
{
    /* We create a Switch structure and add to our list */
    struct Switch *switchPtr = (struct Switch *)malloc(sizeof(struct Switch));
    if(switchPtr == NULL) {
        return NULL;
    }

    switchPtr->bit = bit;
    switchPtr->port = port;
    switchPtr->switchMsg.switchState = SW_OPEN;
    switchPtr->switchMsg.id = id;

    switchInfo[numberOfSwitches] = switchPtr;
    numberOfSwitches++;

    return switchPtr;
}

/* Initialize the switches */
int switchInit(
    size_t queueSize,   // Queue size to notify the user of switch changes
    size_t maxSwitches) // Maximum so we don't waste array space.
{
    switchQueue = xQueueCreate(queueSize, sizeof(SwitchMsg*));

    /* Create an array of pointers to Switch structures */
    switchInfo = malloc(maxSwitches * (sizeof(struct Switch*)));
    if(switchInfo == NULL) {
        return -1;     /* failure! */
    }

    switchTimer = xTimerCreate(
                      "switch time",            // pcTimerName
                      (10 / portTICK_RATE_MS),  // xTimerPeriod
                      pdTRUE,                   // uxAutoReload
                      0,                        // pvTimerID
                      switchTask                // Function to be called
                  );

    if(switchTimer == NULL) {
        return -1;     /* failure! */
    }

    xTimerStart(switchTimer, 0);

    return 0;
}

/* Queue for the user to watch */
QueueHandle_t switchGetQueue()
{
    return switchQueue;
}

