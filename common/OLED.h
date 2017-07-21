/*
 * OLED.h
 *
 * Created: 12/19/2013 5:48:22 PM
 *  Author: waynej
 */

#ifndef OLED_H_
#define OLED_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

//#define OLED_RW    IOPORT_CREATE_PIN(PORTB, 0)
//#define OLED_ENABLE  IOPORT_CREATE_PIN(PORTB, 1)
//#define OLED_RS    IOPORT_CREATE_PIN(PORTB, 2)

void OLED_init(void);
unsigned int OLED_readFlags(void);
void OLED_wait(void);
void OLED_command(unsigned char cmd);
void OLED_clearDisplay(void);
void OLED_returnHome(void);
void OLED_entryModeSet(bool inc, bool shift);
void OLED_display(bool display, bool cursor, bool blink);
void OLED_cursorDisplayShift(bool display, bool direction);
void OLED_functionSet(bool dataLength, int font);
void OLED_setCGRAM(int address);
void OLED_setDDRAM(int address);
void OLED_data(unsigned char ch);
void OLED_setAddr(int col, int line);

int OLED_printf(char *format, ...);
int OLED_XYprintf(int col, int line, char *format, ...);
void OLED_clearEOL();
void OLED_clearLine(int line);

#endif /* OLED_H_ */
