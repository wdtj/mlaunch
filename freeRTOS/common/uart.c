#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"

/*
 * $0C ($2C) UDR0   USART I/O Data Register
 * $0B ($2B) UCSR0A RXC   TXC   UDR0E  FE   DOR0  PE    U2X  MPCM
 * $0A ($2A) UCSR0B RXCIE0 TXCIE UDR0IE0 RXEN0 TXEN00 UCSZ2 RXB8 TXB8
 * $09 ($29) UBRR0L USART Baud Rate Register Low Byte
 * $20 ($40) UBR0H URSEL –     –    –         UBRR[11:8]
 *           UCSRC URSEL UMSEL UPM1 UPM0 USBS UCSZ1 UCSZ0 UCPOL
 *
 * UDR0   USART I/O Data Register
 * UCSR0A USART Control and Status Register A
 *   RXC: USART Receive Complete
 *   TXC: USART Transmit Complete
 *   UDR0E: USART Data Register Empty
 *   FE: Frame Error
 *   DOR0: Data OverRun
 *   PE: Parity Error
 *   U2X: Double the USART Transmission Speed
 *   MPCM: Multi-processor Communication Mode
 * UCSR0B USART Control and Status Register B
 *   RXCIE0: RX Complete Interrupt Enable
 *   TXCIE: TX Complete Interrupt Enable
 *   UDR0IE0: USART Data Register Empty Interrupt Enable
 *   RXEN0: Receiver Enable
 *   TXEN00: Transmitter Enable
 *   UCSZ2: Character Size
 *   RXB8: Receive Data Bit 8
 *   TXB8: Transmit Data Bit 8
 * UCSRC USART Control and Status Register C
 *   URSEL: Register Select
 *   UMSEL: USART Mode Select
 *   UPM1:0: Parity Mode
 *   USBS: Stop Bit Select
 *   UCSZ1:0: Character Size
 *   UCPOL: Clock Polarity
 * UBR0H USART Baud Rate Registers
 * UBRR0L
 * URSEL: Register Select
 * UBRR11:0: USART Baud Rate Register
 */

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