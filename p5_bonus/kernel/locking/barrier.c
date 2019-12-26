#include "barrier.h"
#include "lock.h"
void do_barrier_init(barrier_t *barrier, int goal)
{
	barrier->value = 0;
	barrier->goal = goal;
	queue_init(&barrier->queue);
}

void do_barrier_wait(barrier_t *barrier)
{
	barrier->value++;
	if (barrier->value == barrier->goal) {
		do_unblock_all(&barrier->queue);
		barrier->value = 0;
	}
	else {
		do_block(&barrier->queue);
	}
}