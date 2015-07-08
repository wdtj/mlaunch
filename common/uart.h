#include <avr/io.h>
#include <stdio.h>

void uart_init(long baud, void (*handler)());
void uart_txBuff(void *ptr, unsigned int size);
int uart_putchar(char c, FILE *stream);
void uart_txc(unsigned char ch);

void uart_rxBuff(void *ptr, unsigned int size);
int uart_getchar(FILE *stream);
int uart_rxReady();
int uart_rxc(void);

volatile extern int uart_fe;
volatile extern int uart_doe;
volatile extern int uart_pe;
volatile extern int uart_roe;
