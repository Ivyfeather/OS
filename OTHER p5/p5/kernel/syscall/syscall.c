#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "syscall.h"

void system_call_helper(int fn, int arg1, int arg2, int arg3)
{
	//return value in v0
	current_running->user_context.cp0_epc += 4;
	////// what if current running has beem changed in *syscall[fn]?
	current_running->user_context.regs[2] = (*syscall[fn])(arg1, arg2, arg3);
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
//==================================
void sys_ps(void) 
{
	invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE);
}

void sys_clear(void)
{
	invoke_syscall(SYSCALL_CLEAR, IGNORE, IGNORE, IGNORE);
}

pid_t sys_getpid(void)
{
	invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);
}
//==================================
void sys_spawn(task_info_t *task, int para_num)
{
	invoke_syscall(SYSCALL_SPAWN, (int)task, para_num, IGNORE);
}
void sys_kill(pid_t pid) 
{
	invoke_syscall(SYSCALL_KILL, (int)pid, IGNORE, IGNORE);
}
void sys_exit(void)
{
	invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}
void sys_waitpid(pid_t pid)
{
	invoke_syscall(SYSCALL_WAITPID, (int)pid, IGNORE, IGNORE);
}
//==================================
void semaphore_init(semaphore_t *sem, int val)
{
	invoke_syscall(SYSCALL_SEMAPHORE_INIT, (int)sem, val, IGNORE);
}
void semaphore_up(semaphore_t *sem)
{
	invoke_syscall(SYSCALL_SEMAPHORE_UP, (int)sem, IGNORE, IGNORE);
}
void semaphore_down(semaphore_t *sem)
{
	invoke_syscall(SYSCALL_SEMAPHORE_DOWN, (int)sem, IGNORE, IGNORE);
}
//==================================
void barrier_init(barrier_t *barrier, int goal)
{
	invoke_syscall(SYSCALL_BARRIER_INIT, (int)barrier, goal, IGNORE);
}
void barrier_wait(barrier_t *barrier)
{
	invoke_syscall(SYSCALL_BARRIER_WAIT, (int)barrier, IGNORE, IGNORE);
}
//==================================
void condition_init(condition_t *cond)
{
	invoke_syscall(SYSCALL_CONDITION_INIT, (int)cond, IGNORE, IGNORE);
}
void condition_signal(condition_t *cond)
{
	invoke_syscall(SYSCALL_CONDITION_SIGNAL, (int)cond, IGNORE, IGNORE);
}
void condition_broadcast(condition_t *cond) 
{
	invoke_syscall(SYSCALL_CONDITION_BROADCAST, (int)cond, IGNORE, IGNORE);
}
void condition_wait(mutex_lock_t *lock, condition_t *cond)
{
	invoke_syscall(SYSCALL_CONDITION_WAIT, (int)lock, (int)cond, IGNORE);
}
//==================================
void sys_init_mac(void)
{
	invoke_syscall(SYSCALL_INIT_MAC, IGNORE, IGNORE, IGNORE);
}
uint32_t sys_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{
	invoke_syscall(SYSCALL_NET_RECV, (int)rd, (int)rd_phy, (int)daddr);
}
void sys_net_send(uint32_t td, uint32_t td_phy)
{
	invoke_syscall(SYSCALL_NET_SEND, (int)td, (int)td_phy, IGNORE);
}
void sys_wait_recv_package(void)
{
	invoke_syscall(SYSCALL_WAIT_RECV_PACKAGE, IGNORE, IGNORE, IGNORE);
}
