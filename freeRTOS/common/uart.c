#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"
//#include "timer1.h"

/*
 * $0C ($2C) UDR   USART I/O Data Register
 * $0B ($2B) UCSRA RXC   TXC   UDRE  FE   DOR  PE    U2X  MPCM
 * $0A ($2A) UCSRB RXCIE TXCIE UDRIE RXEN TXEN UCSZ2 RXB8 TXB8
 * $09 ($29) UBRRL USART Baud Rate Register Low Byte
 * $20 ($40) UBRRH URSEL –     –    –         UBRR[11:8]
 *           UCSRC URSEL UMSEL UPM1 UPM0 USBS UCSZ1 UCSZ0 UCPOL
 *
 * UDR   USART I/O Data Register
 * UCSRA USART Control and Status Register A
 *   RXC: USART Receive Complete
 *   TXC: USART Transmit Complete
 *   UDRE: USART Data Register Empty
 *   FE: Frame Error
 *   DOR: Data OverRun
 *   PE: Parity Error
 *   U2X: Double the USART Transmission Speed
 *   MPCM: Multi-processor Communication Mode
 * UCSRB USART Control and Status Register B
 *   RXCIE: RX Complete Interrupt Enable
 *   TXCIE: TX Complete Interrupt Enable
 *   UDRIE: USART Data Register Empty Interrupt Enable
 *   RXEN: Receiver Enable
 *   TXEN: Transmitter Enable
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
 * UBRRH USART Baud Rate Registers
 * UBRRL
 * URSEL: Register Select
 * UBRR11:0: USART Baud Rate Register
 */

volatile static unsigned int maxTxTime = 0;
volatile static int uart_toe = 0;
volatile static unsigned int maxTx = 0;
volatile int uart_fe = 0;           // Framing error
volatile int uart_doe = 0;          // Data overrun error
volatile int uart_pe = 0;           // Parity error
volatile int uart_roe = 0;          // Buffer overrun error

volatile QueueHandle_t uartTxQueue;
volatile QueueHandle_t uartRxQueue;

void uart_init(long baud, int txQueueSize, int rxQueueSize)
{
    uartTxQueue = xQueueCreate(txQueueSize, 1);
    uartRxQueue = xQueueCreate(rxQueueSize, 1);

#if F_CPU < 2000000UL && defined(U2X)
    UCSRA = _BV(U2X); /* improve baud rate error by using 2x clk */
    UBRRL = (F_CPU/8UL/baud-1);
    UBRRH = (F_CPU/8UL/baud-1)>>8;
#else
    UBRRL = (F_CPU / 16UL / baud - 1);
    UBRRH = (F_CPU / 16UL / baud - 1) >> 8;
#endif
    UCSRB = _BV(TXEN) | _BV(RXEN); /* tx/rx enable */

    UCSRB |= _BV(RXCIE);

    sei();                              // enable interrupts
}

ISR( USART_UDRE_vect)
{
    unsigned char ch;
    BaseType_t xTaskWokenByReceive = pdFALSE;

    if( xQueueReceiveFromISR( uartTxQueue, ( void * ) &ch, &xTaskWokenByReceive) ) {
        UDR = ch;
    }

    if ( xTaskWokenByReceive != ( char ) pdFALSE) {
        taskYIELD ();
    }
}

ISR( USART_RXC_vect)
{
    volatile unsigned char ucsra = UCSRA;
    if (bit_is_set(ucsra, FE)) {
        uart_fe = 1;
    }
    if (bit_is_set(ucsra, DOR)) {
        uart_doe = 1;
    }
    if (bit_is_set(ucsra, UPE)) {
        uart_pe = 1;
    }

    unsigned char ch=UDR;

    volatile BaseType_t rc=xQueueSendToBackFromISR(uartRxQueue, &ch, 0);
    if (rc != pdPASS ) {
        uart_roe=1;
    }
}

void uart_txc(unsigned char ch)
{
    volatile BaseType_t rc=xQueueSendToBack( uartTxQueue, ( void * ) &ch, 0);
    if (rc != pdPASS ) {
        uart_toe=1;
        return;
    }

    UCSRB |= _BV(UDRIE);
}

void uart_txBuff(char *buff, int size)
{
    while(size > 0) {
        uart_txc(*buff);
        buff++;
        size--;
    }
}

// Returns number of available characters in the buffer
int uart_rxReady()
{
    return uxQueueMessagesWaiting(uartRxQueue);
}

int uart_rxc(void)
{
    int ch;
    xQueueReceive(uartRxQueue, &ch, 0);
    return ch;
}

int uart_rxBuff(unsigned char *buff, int size, int *count, int timeout)
{
    *count=0;
    while(xQueueReceive(uartRxQueue, buff++, timeout)) {
        ++(*count);
        if (--size == 0) {
            return size;
        }
    }
    return size;
}