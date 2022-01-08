#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"

volatile int uart_toe = 0;
volatile int uart_fe = 0;           // Framing error
volatile int uart_doe = 0;          // Data overrun error
volatile int uart_pe = 0;           // Parity error
volatile int uart_roe = 0;          // Buffer overrun error

volatile QueueHandle_t uartTxQueue;
volatile QueueHandle_t uartRxQueue;

/*
 * UART initialization.
 */
void uart_init(long baud, int txQueueSize, int rxQueueSize)
{
    // Use freeRTOS queues for sending and receiving
    uartTxQueue = xQueueCreate(txQueueSize, sizeof(char));
    uartRxQueue = xQueueCreate(rxQueueSize, sizeof(char));

#if F_CPU < 2000000UL && defined(U2X0)
    UCSR0A = _BV(U2X0); /* improve baud rate error by using 2x clk */
    UBRR0L = (F_CPU / 8UL / baud - 1);
    UBRR0H = (F_CPU / 8UL / baud - 1) >> 8;
#else
    UBRR0L = (F_CPU / 16UL / baud - 1);
    UBRR0H = (F_CPU / 16UL / baud - 1) >> 8;
#endif
    UCSR0B = _BV(TXEN0) | _BV(RXEN0); /* tx/rx enable */

    UCSR0B |= _BV(RXCIE0);

    sei();                              // enable interrupts
}

/*
 * Transmitter interrupt vector.
 *
 * This is called when the transmitter is empty.
 */
ISR(USART0_UDRE_vect)
{
    unsigned char ch;
    BaseType_t xTaskWokenBySend = pdFALSE;

    // Get a character from our queue and send it
    if(xQueueReceiveFromISR(uartTxQueue, (void *) &ch, &xTaskWokenBySend)) {
        UDR0 = ch;
    }

    if(xTaskWokenBySend != (char) pdFALSE) {
        taskYIELD();
    }
}

/*
 * Receiver interrupt vector.
 *
 * This is called when the receiver has a character.
 */
ISR(USART0_RX_vect)
{
    volatile unsigned char ucsra = UCSR0A;
    BaseType_t xTaskWokenByReceive = pdFALSE;

    if(bit_is_set(ucsra, FE0)) {      // Receiver has detected a framing error
        uart_fe = 1;
    }
    if(bit_is_set(ucsra, DOR0)) {     // Receiver has detected a n overrun
        uart_doe = 1;
    }
    if(bit_is_set(ucsra, UPE0)) {     // Receiver has detected a parity error
        uart_pe = 1;
    }

    // Send it on it's way
    unsigned char ch = UDR0;

    volatile BaseType_t rc = xQueueSendToBackFromISR(uartRxQueue, &ch,
                             &xTaskWokenByReceive);
    if(rc != pdPASS) {              // The receiver queue is full
        uart_roe = 1;
    }

    if(xTaskWokenByReceive != (char) pdFALSE) {
        taskYIELD();
    }
}

/*
 * Transmit a character.
 *
 * We put the character on queue and tickle the transmit interrupt.
 */
void uart_txc(unsigned char ch)
{
    BaseType_t rc = xQueueSendToBack(uartTxQueue, (void *) &ch, 0);
    if(rc != pdPASS) {
        uart_toe = 1;
        return;
    }

    // Now enable the transmitter empty interrupt to get it to send what's been queued.
    UCSR0B |= _BV(UDRIE0);
}

/*
 * Put a buffer full of characters out the transmitter.
 */
void uart_txBuff(char *buff, int size)
{
    while(size > 0) {
        uart_txc(*buff);
        buff++;
        size--;
    }
}


/*
 * Returns number of available characters in the buffer
 */
int uart_rxReady()
{
    return uxQueueMessagesWaiting(uartRxQueue);
}

/*
 * Receive a single character
 */

int uart_rxc(int timeout)
{
    int ch;
    xQueueReceive(uartRxQueue, &ch, timeout);
    return ch;
}

/*
 * Read characters into a buffer.
 */
int uart_rxBuff(unsigned char *buff, int size, int *count, int timeout)
{
    *count = 0;
    while(xQueueReceive(uartRxQueue, buff++, timeout)) {
        ++(*count);
        if(--size == 0) {
            return size;
        }
    }
    return size;
}