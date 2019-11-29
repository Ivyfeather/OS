#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156
.equ	TIMESLICE, 80000
.equ    PAGE_MASK, 0x00000000

.macro STI
.endm

.macro CLI
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS    #move $0 content to CP0_STATUS in CP0 register
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG      #CPO_CONFIG to $t0
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG

    jr  ra
END(asm_start)

# asm functions for C code
LEAF(reset_CP0_status)
    li      k0, 0x3000ff03
    mtc0    k0, CP0_STATUS
	jr		ra
END(reset_CP0_status)

LEAF(main_enable_interrupt)
	mtc0	zero, CP0_COUNT
	li		k1, TIMESLICE
	mtc0	k1, CP0_COMPARE

	la		k0, test_shell
	mtc0	k0, CP0_EPC
	eret
END(main_enable_interrupt)

LEAF(get_cp0_status)
	mfc0	v0, CP0_STATUS
	jr		ra
END(get_cp0_status)

LEAF(set_cp0_status)
	mtc0	a0, CP0_STATUS
	jr		ra
END(set_cp0_status)
#.macro SET_COMPARE_SLICE timeslice
#    li      t1, \timeslice
#    mtc0    t1, CP0_COMPARE
#.endm
#LEAF(load_CP0_TIME)  #used for test, to check CP0 regs
#	mfc0	k0, CP0_COUNT
#	mfc0	k1, CP0_COMPARE
#	jr		ra
#END(load_CP0_TIME)

LEAF(set_COMPARE_slice)
    mtc0	zero, CP0_COUNT
	li      k1, TIMESLICE
    mtc0    k1, CP0_COMPARE
	jr		ra
END(set_COMPARE_slice)
# =====
LEAF(open_int)
	mfc0	t1, CP0_STATUS
	and		t1, ~0x2
	mtc0	t1, CP0_STATUS
	jr		ra
END(open_int)

LEAF(close_int)
	mfc0	t0, CP0_STATUS
	ori		t0, 0x2
	mtc0	t0, CP0_STATUS
	jr		ra
END(close_int)

.macro SAVE_CONTEXT offset
.set noat
    # TODO save context
    lw      k0,current_running
    addiu   k0,k0,\offset

    #$0 zero not saved
    sw      AT,OFFSET_REG1(k0)  #1
    sw      v0,OFFSET_REG2(k0)  #2
    sw      v1,OFFSET_REG3(k0)  #3
    sw      a0,OFFSET_REG4(k0)  #4
    sw      a1,OFFSET_REG5(k0)  #5
    sw      a2,OFFSET_REG6(k0)  #6
    sw      a3,OFFSET_REG7(k0)  #7

    sw      t0,OFFSET_REG8(k0)  #8
    sw      t1,OFFSET_REG9(k0)  #9
    sw      t2,OFFSET_REG10(k0)  #10
    sw      t3,OFFSET_REG11(k0)  #11
    sw      t4,OFFSET_REG12(k0)  #12
    sw      t5,OFFSET_REG13(k0)  #13
    sw      t6,OFFSET_REG14(k0)  #14
    sw      t7,OFFSET_REG15(k0)  #15

    sw      s0,OFFSET_REG16(k0)  #16
    sw      s1,OFFSET_REG17(k0)  #17
    sw      s2,OFFSET_REG18(k0)  #18
    sw      s3,OFFSET_REG19(k0)  #19
    sw      s4,OFFSET_REG20(k0)  #20
    sw      s5,OFFSET_REG21(k0)  #21
    sw      s6,OFFSET_REG22(k0)  #22
    sw      s7,OFFSET_REG23(k0)  #23

    sw      t8,OFFSET_REG24(k0)  #24
    sw      t9,OFFSET_REG25(k0)  #25
    #$26 (k0) not saved
	#$27 (k1) used to jump to PC before interrupt (CP0_EPC)
	mfc0	k1,CP0_EPC
	sw		k1,OFFSET_EPC(k0)	#27
	
    sw      gp,OFFSET_REG28(k0) #28
    sw      sp,OFFSET_REG29(k0) #29
    sw      fp,OFFSET_REG30(k0) #30
	sw		ra,OFFSET_REG31(k0) #31

	mfc0	k1,CP0_BADVADDR
	sw		k1,OFFSET_BADVADDR(k0)
    #to save special registers
    #mfhi    k1
    #sw      k1,OFFSET_HI(k0)    #HI
    #mflo    k1
    #sw      k1,OFFSET_LO(k0)    #LO
	
	#mfc0	k1,CP0_CAUSE
	#sw		k1,OFFSET_CAUSE(k0)
	#mfc0	k1,CP0_STATUS
	#sw		k1,OFFSET_STATUS(k0)

.endm

.macro RESTORE_CONTEXT offset
.set noat
    # TODO restore context
    lw      k0,current_running
    addiu   k0,k0,\offset

    #$0 zero not restored
    lw      AT,OFFSET_REG1(k0)  #1
    lw      v0,OFFSET_REG2(k0)  #2
    lw      v1,OFFSET_REG3(k0)  #3
    lw      a0,OFFSET_REG4(k0)  #4
    lw      a1,OFFSET_REG5(k0)  #5
    lw      a2,OFFSET_REG6(k0)  #6
    lw      a3,OFFSET_REG7(k0)  #7

    lw      t0,OFFSET_REG8(k0)  #8
    lw      t1,OFFSET_REG9(k0)  #9
    lw      t2,OFFSET_REG10(k0)  #10
    lw      t3,OFFSET_REG11(k0)  #11
    lw      t4,OFFSET_REG12(k0)  #12
    lw      t5,OFFSET_REG13(k0)  #13
    lw      t6,OFFSET_REG14(k0)  #14
    lw      t7,OFFSET_REG15(k0)  #15

    lw      s0,OFFSET_REG16(k0)  #16
    lw      s1,OFFSET_REG17(k0)  #17
    lw      s2,OFFSET_REG18(k0)  #18
    lw      s3,OFFSET_REG19(k0)  #19
    lw      s4,OFFSET_REG20(k0)  #20
    lw      s5,OFFSET_REG21(k0)  #21
    lw      s6,OFFSET_REG22(k0)  #22
    lw      s7,OFFSET_REG23(k0)  #23

    lw      t8,OFFSET_REG24(k0)  #24
    lw      t9,OFFSET_REG25(k0)  #25
    #$26 (k0)  not restored
	lw		k1,OFFSET_EPC(k0)
	mtc0	k1,CP0_EPC

    lw      gp,OFFSET_REG28(k0) #28
    lw      sp,OFFSET_REG29(k0) #29
    lw      fp,OFFSET_REG30(k0) #30
    lw      ra,OFFSET_REG31(k0) #31

	lw		k1,OFFSET_BADVADDR(k0)
	mtc0	k1,CP0_BADVADDR

    #to restore special registers
    #lw      k1,OFFSET_HI(k0)    #HI
    #mthi    k1
    #lw      k1,OFFSET_LO(k0)    #LO
    #mtlo    k1

	#lw		k1,OFFSET_CAUSE(k0)
	#mtc0	k1,CP0_CAUSE
	#lw		k1,OFFSET_STATUS(k0)
	#mtc0	k1,CP0_STATUS
.endm

NESTED(do_scheduler, 0, ra) #use do_schedular
    SAVE_CONTEXT(KERNEL)    #save context
    jal     scheduler       #switch to next process
    RESTORE_CONTEXT(KERNEL) #restore context
    jr      ra
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)
exception_handler_begin:
    # TODO close interrupt
    # jmp exception_handler[i] which decided by CP0_CAUSE
    # Leve2 exception Handler.

    # ATTENTION: when exception, hardware sets EXL to 1
    #            when eret,      hardware sets EXL to 0
    #      which means opening and closing interrupt are handled by hardware itself
    # (interrput is enabled when ERL, EXL, IE == 0, 0, 1)

    # save context
    SAVE_CONTEXT(USER)

    #jump to handle_int according to CP0_CAUSE
    mfc0	k0, CP0_CAUSE		
    andi    k0, k0, 0x7c	    # select ExcCode(6th~2rd bits,0x7C), defined in "irq.h"
    srl     k0, k0, 2

    li      k1, 8               # SYS(8)
    beq     k0, k1, Syscall
    
    li      k1, 0               # INT(0)
    beq     k0, k1, Interrupt

#Other:
    j	    handle_other
Interrupt:
    j       handle_int
Syscall:
    j       handle_syscall

exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    # interrupt handler
    # Leve3 exception Handler.
    addiu   sp, sp, -8
    mfc0    a0, CP0_STATUS
    mfc0    a1, CP0_CAUSE
    jal     interrupt_helper
	nop
    addiu   sp, sp, 8

    # restore context
    RESTORE_CONTEXT(USER)
	
	mtc0	zero, CP0_COUNT
	li		k1, TIMESLICE
	mtc0	k1, CP0_COMPARE
    # jump to CP0_EPC of current running, restored in k1
	eret

END(handle_int)

NESTED(handle_syscall, 0, sp)
	# system call handler
	addiu	sp, sp, -16
	addiu	a3, a2, 0	
	addiu	a2, a1, 0
	addiu	a1, a0, 0
	addiu	a0, v0, 0
	jal		system_call_helper
	nop
	addiu	sp, sp, 16
	
	# restore context
	RESTORE_CONTEXT(USER)

	eret
END(handle_syscall)

NESTED(handle_other, 0, sp)
    # other exception handler
	eret
END(handle_other)

.global TLBexception_handler_begin
.global TLBexception_handler_end

NESTED(TLBexception_handler_entry, 0, sp)
TLBexception_handler_begin:
	# TODO: TLB exception entry
	# jmp exception_handler[i] which decided by CP0_CAUSE
TLBexception_handler_end:
END(TLBexception_handler_entry)

NESTED(write_TLB, 0, sp)		# write_TLB(pte_even, pte_odd, index)
	mtc0	a0, CP0_ENTRYLO0
	mtc0	a1, CP0_ENTRYLO1
	li		t0, PAGE_MASK
	mtc0	t0, CP0_PAGEMASK
	mtc0    a2, CP0_INDEX
	mtc0	a3, CP0_ENTRYHI		#for task 1
	tlbwi
    jr      ra
END(write_TLB)