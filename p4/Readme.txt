For some unknown reason, test on QEMU might stopped halfway while it runs successfully on CHIP. 

One way to solve this:
	1. set TIMESLICE in entry.S larger (150000, for instance)
	2. add "screen_reflush();" after "screen_write();" in screen.c

so change these terms back when to test on chip.


Q1: do we need to store/load BADVADDR during context switch?
// has been done for now