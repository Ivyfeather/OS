#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"

#define RW_TIMES 3

int rand()
{	
	int current_time = get_timer();
	return current_time % 100000;
}

/*static void disable_interrupt()
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
*/

void rw_task1(unsigned long a[6])
{
	int mem1, mem2 = 0;
	int curs = 0;
	int memory[RW_TIMES];
	int i = 0;
	a[0] = a[3] = 0x100;
	a[1] = a[4] = 0x1200;
	a[2] = a[5] = 0x10000;
	for(i = 0; i < RW_TIMES; i++)
	{
		screen_move_cursor(1, curs+i);
		mem1 = a[i];
		screen_move_cursor(1, curs+i);
		memory[i] = mem2 = rand();
		*(int *)mem1 = mem2;
		printf("Write: 0x%x, %d;", mem1, mem2);
	}
	curs = RW_TIMES;
	for(i = 0; i < RW_TIMES; i++)
	{
		screen_move_cursor(1, curs+i);
		mem1 = a[RW_TIMES+i];
		screen_move_cursor(1, curs+i);
		memory[i+RW_TIMES] = *(int *)mem1;
		if(memory[i+RW_TIMES] == memory[i])
			printf("Read succeed: %d;", memory[i+RW_TIMES]);
		else
			printf("Read error: %d;", memory[i+RW_TIMES]);
	}
	while(1);
	//Input address from argv.
}
