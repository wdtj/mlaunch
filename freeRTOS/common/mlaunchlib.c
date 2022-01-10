/*
 * stdlib.c
 *
 * Created: 1/10/2022 4:56:46 PM
 *  Author: waynej
 */ 

 #include "FreeRTOS.h"

 #include "string.h"
 #include "mlaunchlib.h"

 /*
  * These functions are available in avr-libc, but do not work with FreeRTOS
  */

 void *malloc(int size)
 {
     return pvPortMalloc(size);
 }

 void free(void *ptr)
 {
     vPortFree(ptr);
 }

 char *strdup(const char *string)
 {
     size_t length = strlen(string);
     char *newString = malloc(length + 1);
     memcpy(newString, string, length + 1);
     return newString;
 }

