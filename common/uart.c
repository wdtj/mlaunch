#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"
#include "timer1.h"

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

volatile static void *txBuff=NULL;
volatile static void *txBuffIn=NULL;
volatile static void *txBuffOut=NULL;
volatile static unsigned int txBuffSize=0;
volatile static unsigned int txCount=0;
volatile static unsigned int maxTxTime=0;	
volatile static int uart_toe=0;
volatile static unsigned int maxTx=0;
volatile int uart_fe=0;			// Framing error
volatile int uart_doe=0;			// Data overrun error
volatile int uart_pe=0;			// Parity error
volatile int uart_roe=0;			// Buffer overrun error
volatile unsigned int maxRxTime=0;	
volatile unsigned int maxRx=0;
volatile static void *rxBuff=NULL;
volatile static unsigned rxBuffSize=0;
volatile static void *rxBuffIn=NULL;
volatile static void *rxBuffOut=NULL;
volatile static unsigned int rxCount=0;

void (*uart_handler)(void); 

void uart_init(long baud, void (*handler)(void))
{
  uart_handler=handler;
	
#if F_CPU < 2000000UL && defined(U2X)
  UCSRA = _BV(U2X);             /* improve baud rate error by using 2x clk */
  UBRRL = (F_CPU/8UL/baud-1);
  UBRRH = (F_CPU/8UL/baud-1)>>8;
#else
  UBRRL = (F_CPU/16UL/baud-1);
  UBRRH = (F_CPU/16UL/baud-1)>>8;
#endif
  UCSRB = _BV(TXEN) | _BV(RXEN);  /* tx/rx enable */

  UCSRB |= _BV(TXCIE);  // Reenable interrupt 
  UCSRB |= _BV(RXCIE);
}

void uart_txBuff(void *ptr, unsigned int size)
{
  txBuff=ptr;
  txBuffIn=ptr;
  txBuffOut=ptr;
  txBuffSize=size;
  txCount=0;
}

static void txWrite()
{
  txCount--;
  UDR = *(unsigned char *)txBuffOut;
  txBuffOut++;
  if (txBuffOut >= txBuff+txBuffSize)
  {
    txBuffOut=txBuff;
  }
}

static void txStart()
{
  if (bit_is_set(UCSRA, UDRE))
  {
    txWrite();
  }
}

ISR(USART_TXC_vect )
{
  unsigned int startTime=TCNT1;

  if (txCount != 0)
  {
    txWrite();
  }

  HIGH_WATER(maxTxTime, TCNT1-startTime);
}

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
int
uart_putchar(char c, FILE *stream)
{
  int rc=0;

  UCSRB &= ~_BV(TXCIE);
  if (txCount <= txBuffSize)
  {
    *(unsigned char *)txBuffIn=c;
    txBuffIn++;
    if (txBuffIn >= txBuff+txBuffSize)
    {
      txBuffIn=txBuff;
    }
    txCount++;
    HIGH_WATER(maxTx, txCount);
    txStart();
  }
  else
  {
    uart_toe++;
    rc=-1;
  }
  UCSRB |= _BV(TXCIE);

  return rc;
}

void uart_txc(unsigned char ch)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );

	/* Put data into buffer, sends the data */
	UDR = ch;
}

void uart_rxBuff(void *ptr, unsigned int size)
{
  rxBuff=ptr;
  rxBuffSize=size;
  rxBuffIn=ptr;
  rxBuffOut=ptr;
  rxCount=0;
}

/*
 * Receive character from the UART Rx, wait until rx holding register
 * is ready.
 */
int
uart_getchar(FILE *stream)
{
  while (rxCount ==0) {}

  UCSRB &= ~_BV(RXCIE);

  unsigned char ch=*(unsigned char *)rxBuffOut;
  rxBuffOut++;
  if (rxBuffOut >= rxBuff+rxBuffSize)
  {
    rxBuffOut=rxBuff;
  }
  rxCount--;

  UCSRB |= _BV(RXCIE);
  return ch;
}

// Returns number of available characters in the buffer
int uart_rxReady()
{
   return (rxCount>0);
}


ISR(USART_RXC_vect )
{
	unsigned int startTime=TCNT1;
	
	volatile unsigned char ucsra=UCSRA;
	if (bit_is_set(ucsra, FE))
	{
		uart_fe=1;
	}
	if (bit_is_set(ucsra, DOR))
	{
		uart_doe=1;
	}
	if (bit_is_set(ucsra, UPE))
	{
		uart_pe=1;
	}

	if (uart_handler)
	{
		(*uart_handler)();
	}
	else
	{
		if (rxCount > rxBuffSize)
		{
			uart_roe=1;
		}
		
		*(unsigned char *)(rxBuffIn++)=UDR;
		if (rxBuffIn >= rxBuff+rxBuffSize)
		{
			rxBuffIn=rxBuff;
		}

		rxCount++;
    HIGH_WATER(maxRx, rxCount);
	}

  HIGH_WATER(maxRxTime, TCNT1-startTime);
}

int uart_rxc(void)
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) );
	
	/* Get and return received data from buffer */
	return UDR;
}

