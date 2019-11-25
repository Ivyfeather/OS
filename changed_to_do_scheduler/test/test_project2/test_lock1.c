#include "test2.h"
#include "lock.h"
#include "stdio.h"
#include "syscall.h"
/* test */
#include "queue.h"
#include "sched.h"
/* test */
int is_init = FALSE;
static char blank[] = {"                                             "};

/* if you want to use spin lock, you need define SPIN_LOCK */
//  #define SPIN_LOCK
spin_lock_t spin_lock;

/* if you want to use mutex lock, you need define MUTEX_LOCK */
#define MUTEX_LOCK
mutex_lock_t mutex_lock;

void lock_task1(void)
{
        int print_location = 3;
        while (1)
        {
                int i;
                if (!is_init)
                {

#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        do_mutex_lock_init(&mutex_lock);
#endif
                        is_init = TRUE;
                }

                vt100_move_cursor(1, print_location);
                printk("%s", blank);

                vt100_move_cursor(1, print_location);
                printk("> [TASK] Applying for a lock.\n");

                do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                do_mutex_lock_acquire(&mutex_lock);
#endif

                for (i = 0; i < 20; i++)
                {
                        vt100_move_cursor(1, print_location);
					#ifdef TEST

				        pcb_t *tmp;
						printk("ready_queue:");
						for(tmp = ready_queue.head; tmp != NULL; tmp = tmp->next)
							printk("%d ",tmp->pid);
						printk("|| block_queue:");
						for(tmp = block_queue.head; tmp != NULL; tmp = tmp->next)
							printk("%d ",tmp->pid);
						printk("|| lock status:%s",mutex_lock.status==LOCKED?"LOCKED":"UNLOCKED");
						printk("||status:%s",pcb[4].status==TASK_RUNNING?"RUNNING":"BLOCKED");
	
					#endif
																								
						printk("> [TASK] Has acquired lock and running.(%d)\n", i);
                        do_scheduler();
                }

                vt100_move_cursor(1, print_location);
                printk("%s", blank);

                vt100_move_cursor(1, print_location);
                printk("> [TASK] Has acquired lock and exited.\n");

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                do_mutex_lock_release(&mutex_lock);
#endif
                do_scheduler();
        }
}

void lock_task2(void)
{
        int print_location = 4;
        while (1)
        {
                int i;
                if (!is_init)
                {

#ifdef SPIN_LOCK
                        spin_lock_init(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                        do_mutex_lock_init(&mutex_lock);
#endif
                        is_init = TRUE;
                }

                vt100_move_cursor(1, print_location);
                printk("%s", blank);

                vt100_move_cursor(1, print_location);
                printk("> [TASK] Applying for a lock.\n");

                do_scheduler();

#ifdef SPIN_LOCK
                spin_lock_acquire(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                do_mutex_lock_acquire(&mutex_lock);
#endif

                for (i = 0; i < 20; i++)
                {
                        vt100_move_cursor(1, print_location);
					#ifdef TEST

				        pcb_t *tmp;
						printk("ready_queue:");
						for(tmp = ready_queue.head; tmp != NULL; tmp = tmp->next)
							printk("%d ",tmp->pid);
						printk("|| block_queue:");
						for(tmp = block_queue.head; tmp != NULL; tmp = tmp->next)
							printk("%d ",tmp->pid);
						printk("|| lock status:%s",mutex_lock.status==LOCKED?"LOCKED":"UNLOCKED");
						printk("||status:%s",pcb[5].status==TASK_RUNNING?"RUNNING":"BLOCKED");
	
					#endif
                        printk("> [TASK] Has acquired lock and running.(%d)\n", i);
                        do_scheduler();
                }

                vt100_move_cursor(1, print_location);
                printk("%s", blank);

                vt100_move_cursor(1, print_location);
                printk("> [TASK] Has acquired lock and exited.\n");

#ifdef SPIN_LOCK
                spin_lock_release(&spin_lock);
#endif

#ifdef MUTEX_LOCK
                do_mutex_lock_release(&mutex_lock);
#endif
                do_scheduler();
        }
}
