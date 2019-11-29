#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"
#include "string.h"
#include "sched.h"

#define TLB_ENTRY_NUMBER 32

/*
	Beware: +, - has higher prior than >>, <<, &, |
*/

/* --------------------- Page Table Entry ---------------------- *
 *	     31                      12	 11	  6  5    3  2   1   0
 * -----+--------------------------+-------+-------+---+---+---+
 * NAME |    Physical Page Number  |   0   |   C   | D | V | G |
 * -------------------------------------------------------------
 * BITS |			20             |   6   |   3   | 1 | 1 | 1 |
 * ------------------------------------------------------------- 
 * INIT |           PFN            |   0   |   2   | 1 | 1 | 1 |
 * ------------------------------------------------------------- 
 */

 /* -------------- Virtual Address ---------------- *
  *	     31         22  21        12  11	      0
  * -----+------------+-------------+--------------+
  * NAME |     DIR    |  PageTable  |    Offset    |
  * ------------------------------------------------
  * BITS |	   10     |     10      |      12      |
  * ------------------------------------------------
  */

 /*
  * Virtual Memory,   2G, 0x0000_0000 to 0x8000_0000
  * Physical Memory, 32M,  0x000_0000 to  0x200_0000
  *
  * Available Physical Memory(For User Mapping), 16M, 0x100_0000 to 0x200_0000
  *
  */

#define DIR_NUM 0x400		//2^10 = 1024 directory_entries
#define PAGETABLE_NUM 0x400 //2^10 = 1024 page_table_entries

////// for task2, just let 2processes be in the same Pspace
#define PADDR_BASE 0x01000000 
#define VADDR_BASE 0x00000000

#define PAGE_SIZE  0x1000	// 4K
#define PPAGE_NUM  0x1000	// 16M/4K = 4K

void do_TLB_Refill();
void do_TLB_Invalid();
void do_page_fault();

void init_page_table();
void init_TLB();
void init_swap();
//extern pte_t *dir[DIR_NUM];
int find_pcb(pid_t pid);
pte_t* find_page(uint32_t vaddr);
#endif
