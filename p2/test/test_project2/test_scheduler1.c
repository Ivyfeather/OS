#include "sched.h"
#include "stdio.h"
#include "test2.h"
#include "syscall.h"
/* test */
#include "queue.h"
#include "lock.h"
extern mutex_lock_t mutex_lock;
/* test */
static char blank[] = {"                   "};
static char plane1[] = {"    ___         _  "};
static char plane2[] = {"| __\\_\\______/_| "};
static char plane3[] = {"<[___\\_\\_______| "};
static char plane4[] = {"|  o'o             "};

void printk_task1(void)
{
    int i;
    int print_location = 1;

    for (i = 0;; i++)
    {
        close_int();
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
        printk("||status:%s",pcb[1].status==TASK_RUNNING?"RUNNING":"READY");
		#endif
		printk("> [TASK] This task is to test scheduler. (%d)", i);
        open_int();

		//do_scheduler();
    }
}

void printk_task2(void)
{
    int i;
    int print_location = 2;

    for (i = 0;; i++)
    {
		close_int();
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
	    printk("||status:%s",pcb[2].status==TASK_RUNNING?"RUNNING":"READY");
		#endif
        printk("> [TASK] This task is to test scheduler. (%d)", i);
        open_int();
		//do_scheduler();
    }
}

void drawing_task1(void)
{
    int i, j = 22;

    while (1)
    {
		close_int();
        for (i = 60; i > 0; i--)
        {
            /* move */
            vt100_move_cursor(i, j + 0);
            printk("%s", plane1);

            vt100_move_cursor(i, j + 1);
            printk("%s", plane2);

            vt100_move_cursor(i, j + 2);
            printk("%s", plane3);

            vt100_move_cursor(i, j + 3);
            printk("%s", plane4);
        }
        //do_scheduler();

        vt100_move_cursor(1, j + 0);
        printk("%s", blank);

        vt100_move_cursor(1, j + 1);
        printk("%s", blank);

        vt100_move_cursor(1, j + 2);
        printk("%s", blank);

        vt100_move_cursor(1, j + 3);
        printk("%s", blank);
		open_int();
	}

}
