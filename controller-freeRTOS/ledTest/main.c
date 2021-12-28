/*
* LedTest.c
*
* Created: 11/18/2014 6:32:03 PM
*  Author: waynej
*/


#include "config.h"

#include <avr/io.h>
#include <util/delay.h>

#include "FreeRTOS.h"
#include "task.h"

#include "led.h"

void toggleLED(void *parameters)
{
  while(1)
  {
    LED_output(0xff, 0x00, 0x00);			/* Turn on all red LEDs */
    _delay_ms(1000);
    LED_output(0x00, 0xff, 0x00);			/* Turn on all green LEDs */
    _delay_ms(1000);
    LED_output(0x00, 0x00, 0xff);			/* Turn on all yellow LEDs */
    _delay_ms(1000);
    LED_output(0x00, 0x00, 0x00);			/* Turn on all LEDs off */
    _delay_ms(1000);
    /* Flash the pad lights to be pretty */
    for(int i=0; i<10; ++i)
    {
      unsigned int bit=1<<i;
      LED_output(bit, bit>>1, bit>>2);
      _delay_ms(250);
      LED_output(0x0, 0x0, 0x0);
    }
  }


}
int main(void)
{
  LED_init();
  
  // Start blink task
  xTaskCreate(
      toggleLED,      // Function to be called
      "Toggle LED",   // Name of task
      configMINIMAL_STACK_SIZE, // Stack size
      NULL,           // Parameter to pass
      1,              // Task priority
      NULL);          // Created Task

}