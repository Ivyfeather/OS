/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"
#include "mm.h"

#define KERNELBASE 0xa0f00000
#define USERBASE  0x20000000
#define STACKSIZE 0x8000

static void init_memory()
{
	init_page_table(); 
	//In task1&2, page table is initialized completely with address mapping, but only virtual pages in task3.
	//init_TLB();		//only used in P4 task1
	init_swap();		//only used in P4 bonus: Page swap mechanism
}

static void init_pcb()
{
	int kernel_sp = KERNELBASE;
	int user_sp = USERBASE;
	queue_init(&ready_queue);
	queue_init(&sleep_queue);
	int i;
	for (i = 0;i < NUM_MAX_TASK;i++) {
		memset(&pcb[i], 0, sizeof(pcb_t));
		pcb[i].pid = -1;
		pcb[i].status = TASK_EXITED;
		//set kernel context
		// sp
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
	}
	//alloc page table for test_shell
	pcb[1].page_dir = (pte_t **)alloc_page_table();

	for (i = 1;i < NUM_MAX_TASK;i++) {
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top = user_sp;
		user_sp -= STACKSIZE;
	}
	//entry_point
	pcb[process_id].user_context.cp0_epc    = (uint32_t)test_shell;

	pcb[process_id].pid = process_id;
	pcb[process_id].type = USER_PROCESS;
	pcb[process_id].status = TASK_RUNNING;
	pcb[process_id].d_prior = pcb[process_id].s_prior = 1;
	pcb[process_id].name = "shell";

	pcb[process_id].waited = 0;
	queue_init(&pcb[process_id].wait_queue);
	//lock array has been set to 0 

	queue_push(&ready_queue,&pcb[process_id]);
	pcb[process_id].queue = &ready_queue;

	
	pcb[0].pid = 0;
	pcb[0].status = TASK_EXITED;
    current_running = &pcb[0];
	pcb[0].name = "main";
}

static void init_exception_handler()
{
	//Add TLB_exception_handler in project_4.
}

static void init_exception()
{
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
    reset_CP0_status();

	// 3. Copy the level 2 exception handling code to 0x80000180
	memcpy((uint8_t*)0x80000000, (uint8_t*)TLBexception_handler_begin, (uint32_t)(TLBexception_handler_end - TLBexception_handler_begin));
	memcpy((uint8_t*)0x80000180, (uint8_t*)exception_handler_begin, (uint32_t)(exception_handler_end - exception_handler_begin));
	// 4. reset CP0_COMPARE & CP0_COUNT register
	// this work can be saved for later
	// in main(), when enabling interrupt
}

static void init_syscall(void)
{
	// init system call table.
	//use (*syscall[NUM_SYSCALLS])() to init 
	syscall[SYSCALL_SLEEP] = (int (*)())do_sleep;
	syscall[SYSCALL_BLOCK] = (int(*)())do_block;
	syscall[SYSCALL_UNBLOCK_ONE] = (int(*)())do_unblock_one;
	syscall[SYSCALL_UNBLOCK_ALL] = (int(*)())do_unblock_all;

	syscall[SYSCALL_WRITE] = (int(*)())screen_write;
	//syscall[SYSCALL_READ]			= FUNC_NOT_DEFINED
	syscall[SYSCALL_CURSOR] = (int(*)())screen_move_cursor;
	syscall[SYSCALL_REFLUSH] = (int(*)())screen_reflush;

	syscall[SYSCALL_MUTEX_LOCK_INIT] = (int(*)())do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (int(*)())do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (int(*)())do_mutex_lock_release;

	syscall[SYSCALL_EXIT] = (int(*)())do_exit;
	syscall[SYSCALL_SPAWN] = (int(*)())do_spawn;
	syscall[SYSCALL_KILL] = (int(*)())do_kill;
	syscall[SYSCALL_WAITPID] = (int(*)())do_waitpid;

	syscall[SYSCALL_PS] = (int(*)())do_ps;
	syscall[SYSCALL_CLEAR] = (int(*)())do_clear;
	syscall[SYSCALL_GETPID] = (int(*)())do_getpid;

	syscall[SYSCALL_BARRIER_INIT] = (int(*)())do_barrier_init;
	syscall[SYSCALL_BARRIER_WAIT] = (int(*)())do_barrier_wait;

	syscall[SYSCALL_CONDITION_INIT] = (int(*)())do_condition_init;
	syscall[SYSCALL_CONDITION_BROADCAST] = (int(*)())do_condition_broadcast;
	syscall[SYSCALL_CONDITION_WAIT] = (int(*)())do_condition_wait;
	syscall[SYSCALL_CONDITION_SIGNAL] = (int(*)())do_condition_signal;

	syscall[SYSCALL_SEMAPHORE_INIT] = (int(*)())do_semaphore_init;
	syscall[SYSCALL_SEMAPHORE_UP] = (int(*)())do_semaphore_up;
	syscall[SYSCALL_SEMAPHORE_DOWN] = (int(*)())do_semaphore_down;
}
extern mutex_lock_t mutex_lock;

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	char *str = "Hello OS!\n";
	void (*printstr)(char*) = (void *)0x80011100;
	(*printstr)(str);
	// Close the cache, no longer refresh the cache
	// when making the exception vector entry copy
	asm_start();

	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");
	
	// init virtual memory
	init_memory();
	printk("> [INIT] Virtual memory initialization succeeded.\n");
	
	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	printk("> [INIT] SCREEN initialization succeeded.\n");

	// TODO Enable interrupt
	main_enable_interrupt();
    
	//set_COMPARE_slice();
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		//do_scheduler();
	}
	return;
}
