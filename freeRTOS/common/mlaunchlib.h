/*
 * stdlib.h
 *
 * Created: 1/10/2022 4:58:30 PM
 *  Author: waynej
 */ 


#ifndef STDLIB_H_
#define STDLIB_H_


void *malloc(int size);
void free(void *ptr);
char *strdup(const char *string);

#endif /* STDLIB_H_ */