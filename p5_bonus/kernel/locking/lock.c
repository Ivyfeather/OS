#include "lock.h"
#include "sched.h"
#include "syscall.h"
#include "queue.h"

void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
    };
    lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
	queue_init(&lock->queue);
	lock->status = UNLOCKED;
	lock->prev = NULL;
	lock->next = NULL;
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
	/*
	if (lock->status == LOCKED){
		do_block(&lock->queue); 
		lock->status = LOCKED;
	}
	else{
		lock->status = LOCKED;
	}
	*/
	/*
	while (lock->status == LOCKED)
	{
		do_block(&lock->queue);
	}
	
	lock->status = LOCKED;
	*/
	if (current_running == &pcb[1]) return;

	if (lock->status == LOCKED) {
		do_block(&lock->queue);
	}
	lock->status = LOCKED;

	// record lock in pcb
	int i = 0;
	for ( ;current_running->lock_array[i] != NULL && i < NUM_LOCKS;i++);
	if (i >= NUM_LOCKS) printf_in_kernel("lock_array_full.\n");
	else current_running->lock_array[i] = lock;

}

void do_mutex_lock_release(mutex_lock_t *lock)
{
	if(lock->status == LOCKED){

		lock->status = UNLOCKED;

		do_unblock_one(&lock->queue);

		int i = 0;
		for (; i < NUM_LOCKS;i++) {
			if (current_running->lock_array[i] == lock) {
				current_running->lock_array[i] = NULL;
				break;
			}
		}
	}
}

