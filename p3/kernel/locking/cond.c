#include "cond.h"
#include "lock.h"
#include "sched.h"
void do_condition_init(condition_t *condition)
{
	queue_init(&condition->queue);
}

void do_condition_wait(mutex_lock_t *lock, condition_t *condition)
{
	// since this function is in kernel, it will not be interrupted by time_int
	// (lock_release and do_block) is atomic
	do_mutex_lock_release(lock);
	do_block(&condition->queue);
	do_mutex_lock_acquire(lock);
}

void do_condition_signal(condition_t *condition)
{
	do_unblock_one(&condition->queue);
}

void do_condition_broadcast(condition_t *condition)
{
	do_unblock_all(&condition->queue);
}