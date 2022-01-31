/*
* ledTest.c
*
* Created: 11/21/2014 3:49:31 PM
*  Author: waynej
*/

#include "config.h"

#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

#include "padLed.h"

void togglePadLED(void *parameters)
{
    padLedInit();

    while(1) {
        padLed(PAD_LED_GREEN, 0);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 0);
        padLed(PAD_LED_GREEN, 1);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 1);
        padLed(PAD_LED_YELLOW, 0);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 0);
        padLed(PAD_LED_YELLOW, 1);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 1);
        padLed(PAD_LED_RED, 0);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 0);
        padLed(PAD_LED_RED, 1);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 1);

        vTaskDelay(1000);

        padLed(PAD_LED_BLINK_GREEN, 0);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 0);
        padLed(PAD_LED_BLINK_GREEN, 1);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 1);
        padLed(PAD_LED_BLINK_YELLOW, 0);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 0);
        padLed(PAD_LED_BLINK_YELLOW, 1);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 1);
        padLed(PAD_LED_BLINK_RED, 0);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 0);
        padLed(PAD_LED_BLINK_RED, 1);
        vTaskDelay(1000);
        padLed(PAD_LED_OFF, 1);

        vTaskDelay(1000);
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}

int main(void)
{
    // Start blink task
    xTaskCreate(
        togglePadLED,      // Function to be called
        "Toggle Pad LED",   // Name of task
        1024, // Stack size
        NULL,           // Parameter to pass
        1,              // Task priority
        NULL);          // Created Task

    vTaskStartScheduler();
}
