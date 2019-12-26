#include "mm.h"

//TODO:Finish memory management functions here refer to mm.h and add any functions you need.

//directory: pte pointer array, each item points to a page table
//pte_t *dir[DIR_NUM];	
pte_t **dir = NULL;

/* for task 1 and 2, a pre-filled page table */
//pte_t test_page[PAGETABLE_NUM]; 
pte_t *test_page = NULL;

//usage of physical memory, 1: busy, 0: free
char mem[PPAGE_NUM];

////// where do we allocate the new page table in physical address?
// one idea: choose an address that will not overlap with OS kernel
// e.g., 0x00f0 0000 - 0x0100 0000 or 0x0000 0180+XXX - 0x0080 0000

////// about the question TA asked
// VP of different processes with the same i will map to the same PP
// one idea: set segment for different processes
// LOW BOUND + i % SEGSIZE

int PAGE_ALLOC_BASE = 0xa0200000;
#define PAGE_ALLOC_TOP 0xa0300000
#define ALLOCSIZE 0x1000 //4K
/* alloc space for page table in unmapped seg */		////// consider how to reuse the space of PT when process is killed
void* alloc_page_table() {		
	if (PAGE_ALLOC_BASE == PAGE_ALLOC_TOP) {
		printf_in_kernel("PageTable Space Full\n");
		return 0;
	}
	memset((void *)PAGE_ALLOC_BASE, 0, ALLOCSIZE);
	PAGE_ALLOC_BASE += ALLOCSIZE;
	return (void *)(PAGE_ALLOC_BASE - ALLOCSIZE);
}


void init_page_table() 
{
	int i = 0;
	dir = (pte_t **)alloc_page_table();
	
	//task 1
	test_page = (pte_t *)alloc_page_table();
	/*
	dir[0] = test_page;
	for (i = 0;i < PAGETABLE_NUM;i++) {
		test_page[i] = (PADDR_BASE + (i % PPAGE_NUM) * PAGE_SIZE) + PTE_FLAGS ;
	}
	*/
	//init physical memory
	memset(mem, 0, sizeof(mem));
}

#define PTE_TO_TLBLO(pte) ( (( (pte) & 0xfffff000)>> 6) + ( (pte) & 0x7f) )
/* for task one, manually fill TLB */
void init_TLB()
{
	int i = 0;
	for (;i < TLB_ENTRY_NUMBER;i++) {
		pte_t entrylo0 = PTE_TO_TLBLO(test_page[2 * i]);
		pte_t entrylo1 = PTE_TO_TLBLO(test_page[2 * i + 1]);
		uint32_t entryhi = i << 13;
		write_TLB(entrylo0, entrylo1, i, entryhi);
	}
	//write_TLB(0x00040017, 0x00040057, 0, 0x0); //at 16M
}

void do_TLB_Refill()
{
	//test
	//printf_in_kernel("refill");

	pcb_t *p = current_running;
	uint32_t vaddr = p->user_context.cp0_badvaddr;

	pte_t *page = find_page(vaddr);
	pte_t entrylo0 = PTE_TO_TLBLO(*page);
	pte_t entrylo1 = PTE_TO_TLBLO(*(page + 1));

	//test
	//printf_in_kernel("pte:%x|", *page);

	pte_t entryhi = (vaddr & 0xffffe000) + (current_running->pid & 0xff);
	//EntryHi[12] should be 0
	//ASID 0~7 bits

	refill_TLB(entrylo0, entrylo1, entryhi);


}

void do_TLB_Invalid(uint32_t index) 
{
	//test
	//printf_in_kernel("invalid ");


	pcb_t *p = current_running;
	uint32_t vaddr = p->user_context.cp0_badvaddr;
	
	//test
	//printf_in_kernel("vaddr:%x|", vaddr);

	pte_t *page = find_page(vaddr);
	//test
	//printf_in_kernel("|;%x;|",page);

	//test
	//printf_in_kernel("pte:%x|", *page);

	pte_t entrylo0 = PTE_TO_TLBLO(*page);
	pte_t entrylo1 = PTE_TO_TLBLO(*(page + 1));

	if (index & 0x80000000) { //TLB NOT found, refill
		//test
		//vt100_move_cursor(1, 7);
		//printk("refillEXL=1 ");
		pte_t entryhi = (vaddr & 0xffffe000) + (current_running->pid & 0xff);
		refill_TLB(entrylo0, entrylo1, entryhi);
		return;
	}

	invalid_TLB(entrylo0, entrylo1, index);
	
	//test
	//printf_in_kernel("index:%x|", index);


}

void do_page_fault() 
{

}

void init_swap()
{

}


/* alloc physical page for pte, return its address */
void *alloc_ppage()
{
	int i = 0;
	for (;i < PPAGE_NUM;i++) {
		if (mem[i] == 0) {
			mem[i] = 1;
			return (void *)(PADDR_BASE + PAGE_SIZE * i);
		}
	}
	printf_in_kernel("Physical Memory Full");
	return (void *)NULL;
}

/* free allocated physical page, we can get v from pte in page table of pcb */
void free(int v)
{
	mem[v] = 0;
}

/* ======== Other functions ======== */

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

#define DirNum(vaddr) ( (vaddr) >> 22) //take 10 bits dirnum
#define VPageNum(vaddr) (( (vaddr) >> 12) & 0x3ff) //take 10 bits pagenum

/* use vaddr to find even-vpage_num pte, return pte_t* */
pte_t* find_page(uint32_t vaddr)
{
	int dir_num = DirNum(vaddr);
	int vpage_num = VPageNum(vaddr);
	
	//test
	//printf_in_kernel("1;");

	pte_t *page_table = current_running->page_dir[dir_num];

	if (page_table == NULL) {	//page table not exist
		pte_t *new_page = (pte_t *)alloc_page_table();
		page_table = current_running->page_dir[dir_num] = new_page;

		// for task 1 & 2, init page table
		/*
		int i = 0;
		int j = (vaddr >> 22) << 10;
		for (;i < PAGETABLE_NUM;i++) {
			new_page[i] = (PADDR_BASE + ((i+j) % PPAGE_NUM) * PAGE_SIZE) + 0x17;
		}
		*/
	}
	//test
	//printf_in_kernel("2;");

	if ((page_table[vpage_num] & 0x2) == 0) {
		uint32_t tmp = (uint32_t)(alloc_ppage());
		page_table[vpage_num] = (tmp & 0xfffff000) + PTE_FLAGS;
	}

	if (vpage_num % 2 == 1) {
		if ((page_table[vpage_num - 1] & 0x2) == 0) {
			uint32_t tmp = (uint32_t)(alloc_ppage());
			page_table[vpage_num - 1] = (tmp & 0xfffff000) + PTE_FLAGS;
		}
		return page_table + vpage_num - 1;
	}
	else {
		return page_table + vpage_num;
	}
	/*
	if (vpage_num % 2 == 1)
		return page_table + vpage_num - 1; //page_table + vpage_num -1
	else
		return page_table + vpage_num;
	*/
}


