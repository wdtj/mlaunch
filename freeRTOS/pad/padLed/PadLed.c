/*
 * Led.c
 *
 * Created: 2/26/2015 8:41:30 PM
 *  Author: waynej
 */

#include "FreeRTOS.h"

#include "timers.h"

#include "PadLed.h"

#define Red1 _BV(PORTB0)
#define Green1 _BV(PORTB1)
#define Yellow1 _BV(PORTB2)
#define Red2 _BV(PORTB4)
#define Green2 _BV(PORTB5)
#define Yellow2 _BV(PORTB6)

static int red[] =
{ Red1, Red2 };
static int green[] =
{ Green1, Green2 };
static int yellow[] =
{ Yellow1, Yellow2 };

static enum LedsState ledStates[2] = {PAD_LED_OFF, PAD_LED_OFF};

static void init()
{
    PORTB |= Red1 | Green1 | Yellow1 | Red2 | Green2 | Yellow2;
    DDRB |= Red1 | Green1 | Yellow1 | Red2 | Green2 |
            Yellow2;// PB0-2, 4-6 are output
}

/* Timer to scan the switches */
static TimerHandle_t  ledTimer = NULL;

static void ledTimerTask(TimerHandle_t xTimer)
{
    int i;

    for(i = 0; i < (sizeof ledStates) / (sizeof(LedState)); ++i) {
        switch(ledStates[i]) {
        case PAD_LED_GREEN:
            PORTB &= ~(green[i]);
            PORTB |= red[i] | yellow[i];
            break;
        case PAD_LED_YELLOW:
            PORTB &= ~(yellow[i]);
            PORTB |= red[i] | green[i];
            break;
        case PAD_LED_RED:
            PORTB &= ~(red[i]);
            PORTB |= yellow[i] | green[i];
            break;
        case PAD_LED_BLINK_GREEN:
            PORTB ^= green[i];
            PORTB |= red[i] | yellow[i];
            break;
        case PAD_LED_BLINK_YELLOW:
            PORTB ^= yellow[i];
            PORTB |= red[i] | green[i];
            break;
        case PAD_LED_BLINK_RED:
            PORTB ^= red[i];
            PORTB |= yellow[i] | green[i];
            break;
        case PAD_LED_OFF:
            PORTB |= yellow[i] | green[i] | red[i];
            break;
        }
    }
}

void padLed(enum LedsState color, int number)
{
    ledStates[number]=color;
    ledTimerTask(NULL);         // Force the color
}

/* Initialize the led timer */
int padLedInit()
{
    init();

    ledTimer = xTimerCreate(
                   "led time",                  // pcTimerName
                   (100 / portTICK_RATE_MS),    // xTimerPeriod
                   pdTRUE,                      // uxAutoReload
                   0,                           // pvTimerID
                   ledTimerTask                 // Function to be called
               );

    if(ledTimer == NULL) {
        return -1;     /* failure! */
    }

    return xTimerStart(ledTimer, 0);
}

