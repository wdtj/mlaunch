/*
* xbeeTest.c
*
* Created: 11/21/2014 3:49:31 PM
*  Author: waynej
*/

#include "config.h"
#include "../pad-config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "uart.h"
#include <string.h>
#include "zb.h"
#include "xbeeAPI.h"
#include "padLed.h"

// addresses of the network controller
zbAddr controllerAddress = {
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

zbNetAddr controllerNAD = {
    { 0xff, 0xfe }
};

void handleData(char *data, int length)
{
    padLed(PAD_LED_GREEN, 1);
    vTaskDelay(100);
    padLed(PAD_LED_YELLOW, 1);
}

void handleError(int code, int state)
{
    resetRed1();
    resetRed2();
}

void init()
{
    padLedInit();
}

void testXbeeTask(void *parameter)
{
    char *msg;
    int remaining;
    if((remaining = uxTaskGetStackHighWaterMark(NULL)) < 20) {
        assert(0);
    }

    padLed(PAD_LED_RED, 0);
    padLed(PAD_LED_RED, 1);

    xbeeFSMInit(UART_BAUD, 180, 180, 4);

    xbeeWait();

    padLed(PAD_LED_YELLOW, 0);
    padLed(PAD_LED_OFF, 1);

    if((remaining = uxTaskGetStackHighWaterMark(NULL)) < 20) {
        assert(0);
    }

    msg = "there";
    xbeeTx(msg, strlen(msg), controllerAddress, controllerNAD);
    padLed(PAD_LED_GREEN, 0);
    vTaskDelay(100);
    padLed(PAD_LED_YELLOW, 0);
    vTaskDelay(100);

    msg = "there2";
    xbeeExpTx(msg, strlen(msg),
              controllerAddress, // dest addr
              controllerNAD,     // dest nad
              0xE8,              // src endpoint
              0xE8,              // dst endpoint
              0x12,              // cluster
              0xC105,            // profile
              0,                 // radius
              0);                // options

    padLed(PAD_LED_GREEN, 0);
    vTaskDelay(100);
    padLed(PAD_LED_YELLOW, 0);
    vTaskDelay(100);

    XbeeEventMsg message;
    // See if we have a response
    if(xQueueReceive(xbeeGetEventQueue(), &message, portMAX_DELAY)) {
        switch(message.eventType) {
        case 'R':
            padLed(PAD_LED_GREEN, 1);
            vTaskDelay(100);
            padLed(PAD_LED_OFF, 1);
            break;
        case 'X':
            padLed(PAD_LED_GREEN, 1);
            vTaskDelay(100);
            padLed(PAD_LED_OFF, 1);
            break;
        case 'T':
            padLed(PAD_LED_RED, 1);
            vTaskDelay(100);
            padLed(PAD_LED_OFF, 1);
            break;
        }

    }

    if((remaining = uxTaskGetStackHighWaterMark(NULL)) < 20) {
        assert(0);
    }

    networkDiscovery();

    padLed(PAD_LED_GREEN, 0);
    vTaskDelay(100);
    padLed(PAD_LED_YELLOW, 0);
    vTaskDelay(100);

    vTaskDelete(NULL);
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    assert(0);
}

int main(void)
{
    init();

    int rc = xTaskCreate(
                 testXbeeTask,      // Function to be called
                 "testXbee",   // Name of task
                 350, // Stack size
                 NULL,           // Parameter to pass
                 1,              // Task priority
                 NULL);          // Created Task
    assert(rc == pdPASS) /* failure! */

    vTaskStartScheduler();

    while(0);   // should never hit here
}

