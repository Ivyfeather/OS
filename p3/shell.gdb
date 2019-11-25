set arch mips
target remote localhost:50010
symbol-file main
layout regs
define print_queue
	set var $n = (pcb_t *)(($arg0).head)
	while $n != ((pcb_t *)(($arg0).tail))
		p/x *($n)
		set var $n = (pcb_t *)($n->next)
	end
	p/x *($n)
end
