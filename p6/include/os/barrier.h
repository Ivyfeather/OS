#ifndef INCLUDE_BARRIER_H_
#define INCLUDE_BARRIER_H_

#include "queue.h"

typedef struct barrier
{
	int goal;
	int value;
	queue_t queue;
} barrier_t;

void do_barrier_init(barrier_t *, int);
void do_barrier_wait(barrier_t *);

#endif