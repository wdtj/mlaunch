#include <avr/io.h>
#include <stdio.h>

void uart_init(long baud, void (*handler)());
void uart_rxBuff(void *ptr, unsigned int size);
void uart_txBuff(void *ptr, unsigned int size);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
int uart_rxReady();
int uart_rxc(void);
void uart_txc(unsigned char ch);

extern int uart_fe;
extern int uart_doe;
extern int uart_pe;
