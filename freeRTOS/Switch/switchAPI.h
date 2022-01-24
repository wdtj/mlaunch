/*
 * switch.h
 *
 * Created: 1/16/2022 4:51:30 PM
 *  Author: waynej
 */


#ifndef SWITCH_H_
#define SWITCH_H_

enum SwitchState {
    SW_OPEN, /* Switch is open */
    SW_PRESSED, /* The switch was pressed, waiting for debounce */
    SW_CLOSED, /* Switch is closed */
    SW_RELEASED /* The switch was released, waiting for debounce */
};


typedef struct {
    char id;
    enum SwitchState switchState;
} SwitchMsg;

int switchInit(size_t queueSize, size_t maxSwitches);

struct Switch * switchAdd(
    char id,
    unsigned int bit,
    volatile uint8_t *port);

QueueHandle_t switchGetQueue();



#endif /* SWITCH_H_ */