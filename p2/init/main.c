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

#define KERNELBASE 0xa0f00000
#define STACKSIZE 0x3000

// scheduler test
//extern task_info *sched1_tasks[16];
//extern int num_sched1_tasks;
// lock test
//extern task_info *lock_tasks[16];
//extern int num_lock_tasks;

static void init_pcb()
{
	int i = 0;
	int kernel_sp = KERNELBASE;
	int num_tasks = num_sched1_tasks;
	queue_init(&ready_queue);
	queue_init(&block_queue);
	queue_init(&sleep_queue);
//scheduler test
	for(i=0; i<num_sched1_tasks; i++,process_id++){
		memset(&pcb[process_id],0,sizeof(pcb_t));
		//set kernel context
		// sp
		pcb[process_id].kernel_context.regs[29] = pcb[process_id].kernel_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		pcb[process_id].user_context.regs[29]   = pcb[process_id].user_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		//entry_point
		pcb[process_id].user_context.cp0_epc    = sched1_tasks[i]->entry_point;

		pcb[process_id].pid = process_id;
		pcb[process_id].type = sched1_tasks[i]->type;
		pcb[process_id].status = TASK_READY;
		//queue_push(&ready_queue,&pcb[process_id]);
	}
//lock test
	num_tasks += num_lock_tasks;
	for(i=0; i<num_lock_tasks; i++,process_id++){
		memset(&pcb[process_id],0,sizeof(pcb_t));
		//set kernel context
		// sp
		pcb[process_id].kernel_context.regs[29] = pcb[process_id].kernel_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		pcb[process_id].user_context.regs[29]   = pcb[process_id].user_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		//entry_point
		pcb[process_id].user_context.cp0_epc    = lock_tasks[i]->entry_point;

		pcb[process_id].pid = process_id;
		pcb[process_id].type = lock_tasks[i]->type;
		pcb[process_id].status = TASK_READY;
		pcb[process_id].d_prior = pcb[process_id].s_prior = 1;
		queue_push(&ready_queue,&pcb[process_id]);
	}
//timer test
	num_tasks += num_timer_tasks;
	for(i=0; i<num_timer_tasks; i++,process_id++){
		memset(&pcb[process_id],0,sizeof(pcb_t));
		//set kernel context
		// sp
		pcb[process_id].kernel_context.regs[29] = pcb[process_id].kernel_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		pcb[process_id].user_context.regs[29]   = pcb[process_id].user_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		//cp0_epc
		pcb[process_id].user_context.cp0_epc    = timer_tasks[i]->entry_point;

		pcb[process_id].pid = process_id;
		pcb[process_id].type = timer_tasks[i]->type;
		pcb[process_id].status = TASK_READY;
		pcb[process_id].d_prior = pcb[process_id].s_prior = 1;
		queue_push(&ready_queue,&pcb[process_id]);
	}
//sheduler test_syscall
	num_tasks += num_sched2_tasks;
	for(i=0; i<num_sched2_tasks; i++,process_id++){
		memset(&pcb[process_id],0,sizeof(pcb_t));
		//set kernel context
		// sp
		pcb[process_id].kernel_context.regs[29] = pcb[process_id].kernel_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		pcb[process_id].user_context.regs[29]   = pcb[process_id].user_stack_top = kernel_sp;
		kernel_sp -= STACKSIZE;
		//entry_point
		pcb[process_id].user_context.cp0_epc    = sched2_tasks[i]->entry_point;

		pcb[process_id].pid = process_id;
		pcb[process_id].type = sched2_tasks[i]->type;
		pcb[process_id].status = TASK_READY;
		pcb[process_id].d_prior = pcb[process_id].s_prior = 1;
		queue_push(&ready_queue,&pcb[process_id]);
	}
	pcb[8].d_prior = pcb[8].s_prior = 2;

	memset(&pcb[0],0,sizeof(pcb_t));
	pcb[0].pid = 0;
	pcb[0].status = TASK_RUNNING;
    current_running = &pcb[0];

	//stackoverflow

	//num_tasks > 16

}

static void init_exception_handler()
{

}

static void init_exception()
{
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
    reset_CP0_status();

	// 3. Copy the level 2 exception handling code to 0x80000180
    uint8_t addr_exception_handler_begin = (uint8_t)(exception_handler_begin);
    uint8_t addr_exception_handler_end   = (uint8_t)(exception_handler_end);
    uint32_t length_exception_handler    = (uint32_t)(addr_exception_handler_end - addr_exception_handler_begin);
    //exception handler address: 0x80000180
    memcpy(0x80000180,addr_exception_handler_begin,length_exception_handler);

	// 4. reset CP0_COMPARE & CP0_COUNT register
	// this work can be saved for later
	// in main(), when enabling interrupt
}

static void init_syscall(void)
{
	// init system call table.
	//use (*syscall[NUM_SYSCALLS])() to init 
	syscall[SYSCALL_SLEEP] = do_sleep;
	//syscall[SYSCALL_EXIT]  = 
	syscall[SYSCALL_BLOCK] = do_block;
	syscall[SYSCALL_UNBLOCK_ONE] = do_unblock_one;
	syscall[SYSCALL_UNBLOCK_ALL] = do_unblock_all;
	syscall[SYSCALL_WRITE] = screen_write;
	//syscall[SYSCALL_READ]			= FUNC_NOT_DEFINED
	syscall[SYSCALL_CURSOR] = screen_move_cursor;
	syscall[SYSCALL_REFLUSH] = screen_reflush;
	syscall[SYSCALL_MUTEX_LOCK_INIT] = do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = do_mutex_lock_release;
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
	enable_interrupt();
    
	//set_COMPARE_slice();
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		//do_scheduler();
	}
	return;
}
