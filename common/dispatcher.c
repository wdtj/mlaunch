/*
 * mCommon.c
 *
 * Created: 1/25/2014 2:47:03 PM
 *  Author: waynej
 */ 

#include <avr/io.h>
#include <stdlib.h>

#include "dispatcher.h"
#include "fifo.h"

FIFO activeThread;
FIFO freeThread;

void dispatcherInit(int size)
{
	fifo(&activeThread);
	fifo(&freeThread);

	void *ptr=malloc(sizeof (NODE)*size);
	
	for(int count=0; count<size; ++count)
	{
		push(&freeThread, ptr);
		ptr+=sizeof(NODE);
	}
}

void dispatcher(void)
{
    while(1)
	{
		NODE *thread=pop(&activeThread);
		void(*callback)()=thread->data;
		(callback)();
	}
}

void spawn(void (*ptr)(void))
{
	NODE *thread=pop(&freeThread);
	thread->data=ptr;
	push(&activeThread, thread);
}

