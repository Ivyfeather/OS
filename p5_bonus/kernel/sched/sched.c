#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"
#include "irq.h"
#include "mm.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;
static int time_count = 0;
static void check_sleeping()
{
	pcb_t *tmp;
	uint32_t time = get_timer();

	vt100_move_cursor(1,8);
	printk("%d",time_count++);
	
	for(tmp = sleep_queue.head; tmp!= NULL; tmp = tmp->next){
		if(tmp->time <= time && tmp->time != 0){
			queue_remove(&sleep_queue, tmp);
			//tmp->user_context.cp0_epc += 4;	//to inst after sys_sleep
			queue_push(&ready_queue, tmp);
		}
	}
}

#include "mac.h"
#if TASK_2
static int recv_64 = 1;
extern int start_to_recv;
static void check_recv()
{
	desc_t *receive_desc = (desc_t *)RECV_DESC_BASE;
	int i = 0;
	for (;i < PNUM;i++) {
		int received = ( (receive_desc[i].desc0 & 0x80000000) == 0 );
		int invalid = ( reg_read_32((receive_desc[i].desc2 | 0xa0000000)) != 0xb57b5500 );
		// have received invalid package
		if ( received && invalid && start_to_recv) {
			//vt100_move_cursor(1, 9);
			//printk("invalid:%d ",i);
			
			memset( (void *)(receive_desc[i].desc2 | 0xa0000000), 0, PSIZE*4);
			receive_desc[i].desc0 = 0x80000000;
			reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);
			recv_64 = 0;
		}
		
		if (!received) {
			recv_64 = 0;
			//vt100_move_cursor(1, 10);
			//printk("not_recv:%d ", i);
		}
	}
	if (recv_64 == 1) {
		do_unblock_all(&recv_block_queue);
	}
	recv_64 = 1;
}
////// Caution!  the last few packages will be filled in the first several places (where packages from OS lie when recv begins),
//////	but when to print buff, we start from buff base and print successively the following content
//////  in this way, the last few packages will be printed out first
//////  which might cause problem
#endif

#if TASK_3

#endif

void scheduler(void)
{
	check_sleeping();
#if TASK_2
	check_recv();
#endif
	// push current_running task into queue
	if (current_running->status == TASK_RUNNING) {
		current_running->status = TASK_READY;
		current_running->cursor_x = screen_cursor_x;
		current_running->cursor_y = screen_cursor_y;
		queue_push(&ready_queue, current_running);
	}
	// load the head task from ready_queue
	/*
	pcb_t *tmp;
	int pr = 0;
	for(tmp = ready_queue.head; tmp!=NULL; tmp = tmp->next){
		if(tmp->d_prior > pr){
			pr = tmp->d_prior;
			current_running = tmp;
		}
	}
	queue_remove(&ready_queue, current_running);
	current_running->d_prior --;
	if(current_running->d_prior == 0)
		current_running->d_prior = current_running->s_prior;

	*/
	//forget about priority sched, change to old
	current_running = queue_dequeue(&ready_queue);
	//process_id = current_running->pid;
	current_running->status = TASK_RUNNING;
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;

	change_EntryHi_ASID(current_running->pid);
}


void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
	// record wake_up time(seconds) into pcb
	current_running->time	= get_timer() + sleep_time; 
	current_running->status = TASK_BLOCKED;
	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;
	queue_push(&sleep_queue, current_running);
	////// do_scheduler?
	scheduler();
}


// there might be multiple block_queues, each for a lock
// as for ready_queue, there is only one
//  so we use ready_queue as global variable
//        call block_queue as parameter
void do_block(queue_t *queue)
{
    // block the current_running task into the queue
	current_running->status = TASK_BLOCKED;
	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;
	queue_push(queue, current_running);
	scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from block_queue into ready_queue
	if ( !queue_is_empty(queue)){
		pcb_t *tmp = queue_dequeue(queue);
		tmp->status = TASK_READY;
		queue_push(&ready_queue, tmp);
	}
}

void do_unblock_all(queue_t *queue)
{
    // unblock all tasks in block_queue
	pcb_t *tmp = NULL;
	while( !queue_is_empty(queue) ){
		tmp = queue_dequeue(queue);
		tmp->status = TASK_READY;
		queue_push(&ready_queue, tmp);
	}
}

void do_ps(void) {
	int i = 0;
	printf_in_kernel("pid     name    status \n");
	for (i = 0;i < NUM_MAX_TASK;i++) {
		if (pcb[i].status != TASK_EXITED) {
			printf_in_kernel("[%d]    %s    ", pcb[i].pid, pcb[i].name);
			if (pcb[i].status == TASK_RUNNING) printf_in_kernel("RUNNING\n");
			else if (pcb[i].status == TASK_BLOCKED) printf_in_kernel("BLOCKED\n");
			else if (pcb[i].status == TASK_READY) printf_in_kernel("READY\n");
			else printf_in_kernel("ERROR!\n");
		}
	}
	// print ready queue, for test
	pcb_t *p = NULL;
	for (p = ready_queue.head;p != NULL;p = p->next)
		printf_in_kernel("%s %d;", p->name, p->pid);
	printf_in_kernel("\n");

}

void do_clear(void) {
	screen_clear(0, SCREEN_HEIGHT - 1);
	screen_move_cursor(0, SCREEN_HEIGHT / 2);
	printf_in_kernel("==============================  COMMAND  =====================================");
}
//return pid in parameter by using pointer
pid_t do_getpid(void) {
	return current_running->pid;
}

extern unsigned long arr[6];

void do_spawn(task_info_t *task, int para_num) {
	int i;
	for (i = 1;i < NUM_MAX_TASK && pcb[i].status!=TASK_EXITED;i++)
		;
	if (i == NUM_MAX_TASK) {
		printf_in_kernel("Error Spawning: pcb array full[16].\n");
		return;
	}
	process_id++;
	
	// entry point
	pcb[i].user_context.cp0_epc = task->entry_point;
	// parameters
	if (para_num != 0) {
		/*
		int j = 0;
		for (j = 0;j < 3;j++) {
			printf_in_kernel("Input %d addr: ", j);
			scanf_ul(&arr[j]);
			arr[j + 3] = arr[j];
		}
		*/
		pcb[i].user_context.regs[4] = (uint32_t)arr;
	}

	// init page table
	pcb[i].page_dir = (pte_t **)alloc_page_table();

	pcb[i].prev = pcb[i].next = NULL;
	pcb[i].pid = process_id;
	pcb[i].type = task->type;
	pcb[i].status = TASK_READY;
	pcb[i].d_prior = pcb[i].s_prior = 1;
	pcb[i].name = task->name;

	pcb[i].waited = 0;
	queue_init(&pcb[i].wait_queue);
	//lock array set to 0
	int j = 0;
	for (;j < NUM_LOCKS;j++)
		pcb[i].lock_array[j] = NULL;


	queue_push(&ready_queue, &pcb[i]);
	pcb[i].queue = &ready_queue;

}


void do_exit(void) {
	// release tasks waiting for it
	if (current_running->waited == 1) {
		do_unblock_all(&current_running->wait_queue);
	}
	////// clear stack
	//memset 0



	// release lock
	int i = 0;
	for (;i < NUM_LOCKS;i++)
		if (current_running->lock_array[i] != NULL)
			do_mutex_lock_release(current_running->lock_array[i]);

	current_running->status = TASK_EXITED;
	//////test
	//printf_in_kernel("exit finished\n");
	scheduler();
	//scheduler will put it back into ready_queue? no
}

void do_waitpid(pid_t pid) {
	//search for pcb with certain pid
	int i = 0;
	pcb_t *p = NULL;
	for (i = 2;i < NUM_MAX_TASK;i++) {
		if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED) {
			p = &pcb[i];
			break;
		}
	}
	if (p == NULL) return;
	p->waited = 1;
	do_block(&p->wait_queue);
}

void do_kill(pid_t pid) {
	int i = 0;
	pcb_t *p = NULL;
	for (i = 2;i < NUM_MAX_TASK;i++) {
		if (pcb[i].pid == pid && pcb[i].status != TASK_EXITED) {
			p = &pcb[i];
			break;
		}
	}
	if (p == NULL) {
		printf_in_kernel("process pid=%d not running\n", pid);
		return;
	}

	// release tasks waiting for it
	if (p->waited == 1) {
		do_unblock_all(&p->wait_queue);
	}
	// clear stack
	//memset 0



	// release lock
	for (i = 0;i < NUM_LOCKS;i++)
		if (p->lock_array[i] != NULL)
			do_mutex_lock_release(p->lock_array[i]);

	queue_remove(p->queue,p);
	p->status = TASK_EXITED;

	printf_in_kernel("kill process pid=%d\n",pid);

}


// to scan parameters for process 2
static void disable_interrupt()
{
	uint32_t cp0_status = get_cp0_status();
	cp0_status &= 0xfffffffe;
	set_cp0_status(cp0_status);
}
static void enable_interrupt()
{
	uint32_t cp0_status = get_cp0_status();
	cp0_status |= 0x01;
	set_cp0_status(cp0_status);
}
static char read_uart_ch(void)
{
	char ch = 0;
	unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
	unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

	while ((*stat_port & 0x01))
	{
		ch = *read_port;
	}
	return ch;
}

