/*
 * SLQueue.h
 *
 * Created: 1/27/2014 1:44:56 PM
 *  Author: waynej
 */

#ifndef FIFO_H_
#define FIFO_H_

typedef struct fifo_t
{
    struct node_t *head;
    struct node_t *tail;
} FIFO;

typedef struct node_t
{
    struct node_t *next;
    void *data;
} NODE;

void fifo(FIFO *fifo);
NODE* pop(FIFO *fifo);
void push(FIFO *fifo, NODE *node);

#endif /* FIFO_H_ */
