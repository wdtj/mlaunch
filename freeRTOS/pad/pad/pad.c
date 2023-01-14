/*
 * pad.c
 *
 * Created: 12/23/2021 12:51:33 PM
 * Author : waynej
 */

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <string.h>

#include "config.h"
#include "pad-config.h"
#include "PadLed.h"
#include "zb.h"
#include "xbeeAPI.h"

// addresses of the network controller
const zbAddr controllerAddress = {
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

const zbNetAddr controllerNAD = {
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

/* Main pad code */
void padTask(void *parameter)
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
    padLed(PAD_LED_YELLOW, 1);

    if((remaining = uxTaskGetStackHighWaterMark(NULL)) < 20) {
        assert(0);
    }

    // Phone Home
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
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    assert(0);
}

int main(void)
{
    init();

    int rc = xTaskCreate(
                 padTask,      // Function to be called
                 "pad",   // Name of task
                 350, // Stack size
                 NULL,           // Parameter to pass
                 1,              // Task priority
                 NULL);          // Created Task
    assert(rc == pdPASS); /* failure! */

    vTaskStartScheduler();

    while(0);   // should never hit here
}

