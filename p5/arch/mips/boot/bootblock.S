.text
.global main

main:
	# use your own code in here!
	# 1) call BIOS print string "It's bootblock!
	# 2) call BIOS read kernel in SD card.
	# 3) jump to kernel.
# 4) while(1) --> stop here
        la $a0,msg
        lw $t2,printstr
        jal $t2
        li $a0,0xa0800200
        li $a1,512
        li $t1,0xa08001f7
        lb $a2,($t1)
        sll $a2,$a2,9
        lw $t2,read_sd_card
        jal $t2
        jal 0xa0800200
stop:
	j stop
.data

msg: .ascii "It's a bootloader...\n"

# 1. PMON read SD card function address
# read_sd_card();
read_sd_card: .word 0x80011000

# 2. PMON print string function address
# printstr(char *string)
printstr: .word 0x80011100

# 3. PMON print char function address
# printch(char ch)
printch: .word 0x80011140

# 4. kernel address (move kernel to here ~)
kernel : .word 0xa0800200

# 5. kernel main address (jmp here to start kernel main!)
kernel_main : .word 0xa0800200
