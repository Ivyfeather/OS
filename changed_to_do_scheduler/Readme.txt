For some unknown reason, test on QEMU might stopped halfway while it runs successfully on CHIP. 

One way to solve this:
	1. set TIMESLICE in entry.S larger (150000, for instance)
	2. add "screen_reflush();" after "screen_write();" in syscall
