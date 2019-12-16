#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"

#define SLICE 150000
static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    scheduler();

    set_COMPARE_slice();
    // set CP0_compare to slice
    time_elapsed += 100000;
	screen_reflush();
}

void other_exception_handler()
{
    // TODO other exception handler
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    // TODO interrupt handler.
    // Level3 exception Handler.
    // read CP0 register to analyze the type of interrupt.

    ////// ERR
    if( (status & 0x7c)!= 0x0)
        printk("ERROR in interrupt_helper");

    if( (cause & 0xff00) == 0x8000) // IP7(15)=1, IP6~IP0(14~8)=0
        irq_timer();
    else
        other_exception_handler();
}


