#include "lock.h"
#include "sched.h"
#include "syscall.h"

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
	lock->status = UNLOCKED;
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
	if (lock->status == LOCKED){
		do_block(&block_queue); 
	#ifdef TEST
		vt100_move_cursor(1,7);
		printk("lock acquire failed");
	#endif
	lock->status = LOCKED;
	}
	else{
		lock->status = LOCKED;
	#ifdef TEST
		vt100_move_cursor(1,7);
		printk("lock acquire success");
	#endif
	}
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
	if(lock->status == LOCKED){

		lock->status = UNLOCKED;

		do_unblock_one(&block_queue);
	}
}
