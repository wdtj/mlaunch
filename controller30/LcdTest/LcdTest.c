/*
 * LcdTest.c
 *
 * Created: 11/20/2014 10:12:03 AM
 *  Author: waynej
 */ 

#include "../../common/config.h"

#include <avr/io.h>

#include "../../common/OLED.h"

int main(void)
{
	OLED_init();

	OLED_display(true, false, false);	/* Display on, Cursor off, Blink off */
	OLED_functionSet(true, 0);			/* 8 bit, English/Japanese char set */
	OLED_entryModeSet(true, false);		/* Increment, no shift */
	OLED_cursorDisplayShift(false, true); /* Shift cursor, Right */

	OLED_clearDisplay();				/* Blank display */

	OLED_XYprintf(0, 0, "[ LedTest123456789 ]");
	OLED_XYprintf(0, 1, "[ abcdefghijhlmnop ]");
	OLED_XYprintf(0, 2, "[ ABCDEFGHIJKLMNOP ]");
	OLED_XYprintf(0, 3, "[ !@#$%%^&*()_+-=<> ]");

    while(1)
    {
    }
}