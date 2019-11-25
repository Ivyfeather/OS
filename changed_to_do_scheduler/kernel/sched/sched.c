#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;
int i=0;
static void check_sleeping()
{
	pcb_t *tmp;
	uint32_t time = get_timer();
	vt100_move_cursor(1,7);
	
	printk("%d",i++);
	
	for(tmp = sleep_queue.head; tmp!= NULL; tmp = tmp->next){
		if(tmp->time <= time && tmp->time != 0){
			queue_remove(&sleep_queue, tmp);
			tmp->user_context.cp0_epc += 4;	//to inst after sys_sleep
			queue_push(&ready_queue, tmp);
		}
	}
}

void scheduler(void)
{
	check_sleeping();

	// push current_running task into queue
	if (current_running->status == TASK_RUNNING) {
		current_running->status = TASK_READY;
		current_running->cursor_x = screen_cursor_x;
		current_running->cursor_y = screen_cursor_y;
		queue_push(&ready_queue, current_running);
	}

	// load the head task from ready_queue
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
	//current_running = queue_dequeue(&ready_queue);
	process_id = current_running->pid;
	current_running->status = TASK_RUNNING;
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;
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
	do_scheduler();
}


// there might be multiple block_queues, each for a lock
// as for ready_queue, there is only one
//  so we use ready_queue as global variable
//        call block_queue as parameter
void do_block(queue_t *queue)
{
    // block the current_running task into the queue
	current_running->status = TASK_BLOCKED;
	queue_push(queue, current_running);
	do_scheduler();

	/*
	queue_push(&block_queue, current_running);
	// load the head task from ready_queue
	current_running = queue_dequeue(&ready_queue);
	process_id = current_running->pid;
	current_running->status = TASK_RUNNING;
	*/
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
	pcb_t *tmp;
	while( !queue_is_empty(queue) ){
		tmp = queue_dequeue(queue);
		tmp->status = TASK_READY;
		queue_push(&ready_queue, tmp);
	}
}
