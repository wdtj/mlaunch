/*
 * fifo.c
 *
 * Created: 1/27/2014 1:42:22 PM
 *  Author: waynej
 */

#include "fifo.h"

void fifo(FIFO *fifo)
{
    fifo->head = 0;
    fifo->tail = 0;
}

NODE* pop(FIFO *fifo)
{
    // check if queue is empty
    if (fifo->head == 0)
        return 0;

    NODE *node = fifo->head;
    fifo->head = node->next;

    if (fifo->head == 0)
    {
        fifo->tail = 0;
    }

    return node;
}

void push(FIFO *fifo, NODE *node)
{
    // check if queue is empty
    if (fifo->head == 0)
    {
        fifo->head = node;
        fifo->tail = node;
    } else
    {
        fifo->tail->next = node;
        fifo->tail = node;
    }
}

