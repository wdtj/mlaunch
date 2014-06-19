/*
 * OLED.c
 *
 * Created: 12/19/2013 4:48:16 PM
 *  Author: waynej
 */ 

#include "config.h"

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <stdarg.h>

#include "OLED.h"

#define OLED_RW PD3
#define OLED_E PD4
#define OLED_RS PD2

#define OLED_DELAY() {	for(int x=0; x<5; ++x) _NOP(); }
//#define OLED_DELAY() { _NOP(); _NOP(); _NOP(); _NOP(); }
//#define OLED_DELAY() { }
	
void OLED_init()
{
	reset(PORTD, OLED_E);
	
	set(DDRD, OLED_E);		/* Enable to output */
	set(DDRD, OLED_RW);		/* R/W to output */
	set(DDRD, OLED_RS);		/* Register Select to output */
}

unsigned int OLED_readFlags()
{
	reset(PORTD, OLED_RS);	/* 0=Command */
	set(PORTD, OLED_RW);	/* 1=Read */

	DDRA=0x00;				/* DB as input */
	
	set(PORTD, OLED_E);		/* 1=Enable */
	OLED_DELAY();
	int flags=PINA;
	reset(PORTD, OLED_E);	/* 0=Disable */
	
	return flags;
}

void OLED_wait()
{
	volatile static int count;
	count=0;
	while((OLED_readFlags()&0x80)!=0x00)
	{
		count++;
	}
}

void OLED_command(unsigned char cmd)
{
	OLED_wait();

	reset(PORTD, OLED_RS);	/* 0=Command */
	reset(PORTD, OLED_RW);	/* 0=Write */
	
	DDRA=0xff;				/* DB as output */
	PORTA=cmd;				/* Output command */
	
	set(PORTD, OLED_E);		/* 1=Enable */
	OLED_DELAY();
	reset(PORTD, OLED_E);	/* 0=Disable */

	OLED_DELAY();
}

void OLED_data(unsigned char ch)
{
	OLED_wait();
	
	set(PORTD, OLED_RS);	/* 1=Data */
	reset(PORTD, OLED_RW);	/* 0=Write */
	
	DDRA=0xff;				/* DB as output */
	PORTA=ch;				/* Output command */
	
	set(PORTD, OLED_E);		/* 1=Enable */
	OLED_DELAY();
	reset(PORTD, OLED_E);	/* 0=Disable */

	OLED_DELAY();
}

void OLED_clearDisplay()
{
	OLED_command(0x1);
}

void OLED_returnHome()
{
	OLED_command(0x2);
}

void OLED_entryModeSet(bool inc, bool shift)
{
	OLED_command(0x4|(inc?0x2:0)|(shift?0x1:0));
}

void OLED_display(bool display, bool cursor, bool blink)
{
	OLED_command(0x8|(display?0x4:0)|(cursor?0x2:0)|(blink?0x1:0));
}

void OLED_cursorDisplayShift(bool display, bool direction)
{
	OLED_command(0x10|(display?0x8:0)|(direction?0x4:0));
}

void OLED_functionSet(bool dataLength, int font)
{
	OLED_command(0x28|(dataLength?0x10:0)|(font&0x3));
}

void OLED_setCGRAM(int address)
{
	OLED_command(0x40|(address&0x3f));
}

void OLED_setDDRAM(int address)
{
	OLED_command(0x80|(address&0x7f));
}

static unsigned int addrXlate[]=
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53,
	0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 
	0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27,
	0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 
	0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63,
	0x64, 0x65, 0x66, 0x67
};

void OLED_setAddr(int col, int line)
{
	OLED_setDDRAM(addrXlate[line*20+col]);
}

int OLED_printf(char *format, ...)
{
	char buffer[40];
	char *ptr=buffer;
	int length;
	va_list vl;
	va_start(vl, format);
	
	length=vsprintf(buffer, format, vl);
	
	while(*ptr)
	{
		OLED_data(*ptr);
		ptr++;
	}
	
	return length;
}

int OLED_XYprintf(int col, int line, char *format, ...)
{
	char buffer[40];
	char *ptr=buffer;
	int length;
	va_list vl;
	va_start(vl, format);
	
	OLED_setAddr(col, line);
	length=vsprintf(buffer, format, vl);
	
	while(*ptr)
	{
		OLED_data(*ptr);
		ptr++;
	}
	
	return length;
}

void OLED_clearEOL()
{
	unsigned int addr=OLED_readFlags()&0x7f;
	if (addr &0x10)
	{
		addr=addr-0x14+0x80;
	}
	addr&=0x1f;
	for (;addr<20; ++addr)
	{
		OLED_data(' ');
	}
}

void OLED_clearLine(int line)
{
	unsigned int addr=addrXlate[line*20];
	
	OLED_setDDRAM(addr);
	for(int i=0; i<20; i++)
	{
		OLED_data(' ');
	}
}