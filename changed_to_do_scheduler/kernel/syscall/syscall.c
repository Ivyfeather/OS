#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "syscall.h"

void system_call_helper(int fn, int arg1, int arg2, int arg3)
{
    // syscall[fn](arg1, arg2, arg3)
	switch(fn){
		case SYSCALL_SLEEP:			do_sleep(arg1); break;
		case SYSCALL_BLOCK:			do_block((queue_t*)arg1); break;
		case SYSCALL_UNBLOCK_ONE:	do_unblock_one((queue_t*)arg1); break;
		case SYSCALL_UNBLOCK_ALL:	do_unblock_all((queue_t*)arg1); break;
		//////
		case SYSCALL_WRITE:			screen_write((char*)arg1); screen_reflush(); break;
		case SYSCALL_READ:			break;
		case SYSCALL_CURSOR:		screen_move_cursor(arg1,arg2); break;
		case SYSCALL_REFLUSH:		screen_reflush(); break;
		case SYSCALL_MUTEX_LOCK_INIT:	do_mutex_lock_init((mutex_lock_t*)arg1); break;
		case SYSCALL_MUTEX_LOCK_ACQUIRE:do_mutex_lock_acquire((mutex_lock_t*)arg1); break;
		case SYSCALL_MUTEX_LOCK_RELEASE:do_mutex_lock_release((mutex_lock_t*)arg1); break;
		default:	;
	}
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

void sys_block(queue_t *queue)
{
    invoke_syscall(SYSCALL_BLOCK, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_one(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ONE, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_all(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ALL, (int)queue, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (int)buff, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void mutex_lock_init(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_INIT, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_acquire(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_ACQUIRE, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_release(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_RELEASE, (int)lock, IGNORE, IGNORE);
}
