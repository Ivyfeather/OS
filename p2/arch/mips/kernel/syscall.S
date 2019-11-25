#include "asm.h"

LEAF(invoke_syscall)
	# TODO syscall entry
	# set CP0_CAUSE ExcCode syscall

	###### we assume that the 4 parameters for invoke_syscall
	#	   has been loaded into a0, a1, a2, a3
	#	   before jal invoke_syscall
	
	addiu	v0, a0, 0
	addiu	a0, a1, 0
	addiu	a1, a2, 0
	addiu	a2, a3, 0
	syscall
	nop

	jr		ra
END(invoke_syscall)
