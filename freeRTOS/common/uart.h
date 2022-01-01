#include "FreeRTOS.h"

#include <avr/io.h>
#include <stdio.h>

#include "queue.h"

void uart_init(long baud, int txQueueSize, int rxQueueSize); 
void uart_txc(unsigned char ch);
void uart_txBuff(char *buff, int size);
int uart_rxReady();
int uart_rxc(void);
int uart_rxBuff(unsigned char *buff, int size, int *count, int timeout);

volatile extern int uart_fe;
volatile extern int uart_doe;
volatile extern int uart_pe;
volatile extern int uart_roe;
