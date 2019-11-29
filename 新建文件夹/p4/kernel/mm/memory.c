#include "mm.h"

//TODO:Finish memory management functions here refer to mm.h and add any functions you need.

//directory: pte pointer array, each item points to a page table
pte_t *dir[3][DIR_NUM];	////// PID-2 as first index

pte_t test_page[PAGETABLE_NUM];

////// where do we allocate the new page table in physical address?
// one idea: choose an address that will not overlap with OS kernel
// e.g., 0x00f0 0000 - 0x0100 0000 or 0x0000 0180+XXX - 0x0080 0000

////// about the question TA asked
// VP of different processes with the same i will map to the same PP
// one idea: set segment for different processes
// LOW BOUND + i % SEGSIZE

void init_page_table() 
{
	int i = 0;
	memset(dir, 0, sizeof(dir));
	//task 1
	memset(test_page, 0, sizeof(test_page));

	for (i = 0;i < PAGETABLE_NUM;i++) {
		test_page[i] = (PADDR_BASE + (i % PPAGE_NUM) * PAGE_SIZE) + 0x17;
	}

}

void init_TLB()
{
	int i = 0;
	for (;i < TLB_ENTRY_NUMBER;i++) {
		pte_t entrylo0 = ((test_page[2 * i] & 0xfffff000) >> 6) + (test_page[2 * i] & 0x7f);
		pte_t entrylo1 = ((test_page[2 * i + 1] & 0xfffff000) >> 6) + (test_page[2 * i + 1] & 0x7f);
		uint32_t entryhi = i << 14;
		write_TLB(entrylo0, entrylo1, i, entryhi);
	}
	//write_TLB(0x00040017, 0x00040057, 0, 0x0); //at 16M
}

void do_TLB_Refill()
{
	pid_t pid = do_getpid();
	//int pcb_index = find_pcb(pid);
	//pcb_t *p = &pcb[pcb_index]; ////// -1, might have problem
	pcb_t *p = &pcb[2];
	uint32_t vaddr = p->user_context.cp0_badvaddr;

	pte_t *page = find_page(vaddr);
	pte_t entrylo0 = *page;
	pte_t entrylo1 = *(page + 1);
	refill_TLB(entrylo0, entrylo1);

	printf_in_kernel("refill");
}

void do_TLB_Invalid() {
	printf_in_kernel("invalid");

}

void do_page_fault() 
{

}

void init_swap()
{

}

/* find pcb with certain pid, returns pcb index */
int find_pcb(pid_t pid)
{
	int i = 0;
	for (;i < NUM_MAX_TASK;i++)	{
		if (pcb[i].pid == pid)
			return i;
	}
	return -1;
}

#define VDirNum(vaddr) ((uint32_t)(vaddr) >> 22) //take 10 bits dirnum
#define VPageNum(vaddr) (((uint32_t)(vaddr) >> 12)| 0x3ff) //take 10 bits pagenum
/* use vaddr to find even-vpage_num pte, return pte_t* */
pte_t* find_page(uint32_t vaddr)
{
	int vpage_num = VPageNum(vaddr);

	if (vpage_num % 2 == 1)
		return &test_page[vpage_num - 1];
	else
		return &test_page[vpage_num];
}



int PAGE_ALLOC_BASE = 0xa0f00000;
#define PAGE_ALLOC_TOP 0xa1000000
#define ALLOCSIZE 0x1000 //4K
void* alloc_page_table() {
	if (PAGE_ALLOC_BASE == PAGE_ALLOC_TOP) return 0;

	memset((void*)PAGE_ALLOC_BASE, 0, ALLOCSIZE);
	PAGE_ALLOC_BASE += ALLOCSIZE;
	return (void *)(PAGE_ALLOC_BASE - ALLOCSIZE);
}
