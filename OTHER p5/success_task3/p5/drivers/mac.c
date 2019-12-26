#include "mac.h"
#include "irq.h"
#include "queue.h"
#include "sched.h"
#include "syscall.h"

#define NUM_DMA_DESC 48
extern queue_t recv_block_queue;
extern desc_t *send_desc;
extern desc_t *receive_desc;
uint32_t recv_flag[PNUM] = {0};
uint32_t ch_flag;
uint32_t mac_cnt = 0;
uint32_t reg_read_32(uint32_t addr)
{
    return *((uint32_t *)addr);
}
uint32_t read_register(uint32_t base, uint32_t offset)
{
    uint32_t addr = base + offset;
    uint32_t data;

    data = *(volatile uint32_t *)addr;
    return data;
}

void reg_write_32(uint32_t addr, uint32_t data)
{
    *((uint32_t *)addr) = data;
}

static void gmac_get_mac_addr(uint8_t *mac_addr)
{
    uint32_t addr;

    addr = read_register(GMAC_BASE_ADDR, GmacAddr0Low);
    mac_addr[0] = (addr >> 0) & 0x000000FF;
    mac_addr[1] = (addr >> 8) & 0x000000FF;
    mac_addr[2] = (addr >> 16) & 0x000000FF;
    mac_addr[3] = (addr >> 24) & 0x000000FF;

    addr = read_register(GMAC_BASE_ADDR, GmacAddr0High);
    mac_addr[4] = (addr >> 0) & 0x000000FF;
    mac_addr[5] = (addr >> 8) & 0x000000FF;
}

void print_tx_dscrb(mac_t *mac)
{
    uint32_t i;
    printf("send buffer mac->saddr=0x%x ", mac->saddr);
    printf("mac->saddr_phy=0x%x ", mac->saddr_phy);
    printf("send discrb mac->td_phy=0x%x\n", mac->td_phy);
#if 0
    desc_t *send=mac->td;
    for(i=0;i<mac->pnum;i++)
    {
        printf("send[%d].tdes0=0x%x ",i,send[i].tdes0);
        printf("send[%d].tdes1=0x%x ",i,send[i].tdes1);
        printf("send[%d].tdes2=0x%x ",i,send[i].tdes2);
        printf("send[%d].tdes3=0x%x ",i,send[i].tdes3);
    }
#endif
}

void print_rx_dscrb(mac_t *mac)
{
    uint32_t i;
    printf("recieve buffer add mac->daddr=0x%x ", mac->daddr);
    printf("mac->daddr_phy=0x%x ", mac->daddr_phy);
    printf("recieve discrb add mac->rd_phy=0x%x\n", mac->rd_phy);
    desc_t *recieve = (desc_t *)mac->rd;
#if 0
    for(i=0;i<mac->pnum;i++)
    {
        printf("recieve[%d].tdes0=0x%x ",i,recieve[i].tdes0);
        printf("recieve[%d].tdes1=0x%x ",i,recieve[i].tdes1);
        printf("recieve[%d].tdes2=0x%x ",i,recieve[i].tdes2);
        printf("recieve[%d].tdes3=0x%x\n",i,recieve[i].tdes3);
    }
#endif
}

static uint32_t printf_recv_buffer(uint32_t recv_buffer)
{
	int i = 0;
	int *addr = (int *)recv_buffer;
	for (;i < 19;i++) {
		printf("%x ", addr[i]);
	}
	return 1;
}

//in your design,you have the choice to choose use register_irq_handler or not.
typedef void* irq_handler_t;

void register_irq_handler(int IRQn, irq_handler_t func)
{
    // register the func ,
}

/* used in irq.c, check whether new recv packet is arriving */
void mac_irq_handle(void)
{
	// check whether recieved 64 valid packages
	int cnt = 0;
	int i = 0;
	for (;i < PNUM;i++) {
		int received = ((receive_desc[i].desc0 & 0x80000000) == 0);
		int valid = (reg_read_32((receive_desc[i].desc2 | 0xa0000000)) == 0xb57b5500);

		if (received) {
			if (valid) {
				cnt++;
				//vt100_move_cursor(1, 10);
				//printk("received:%d ", cnt);

			}
			else { // invalid, refresh its desc
				//vt100_move_cursor(1, 9);
				//printk("invalid:%d ",i);

				memset((void *)(receive_desc[i].desc2 | 0xa0000000), 0, PSIZE * 4);
				receive_desc[i].desc0 = 0x80000000;
				receive_desc[i].desc1 = RxChain | (PSIZE * 4);	//enable interrupt
				reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);
			}
		}
	}
	/*
	vt100_move_cursor(1, 12);
	printk("buff 0:%x ;buff 1:%x", reg_read_32((receive_desc[0].desc2 | 0xa0000000)), reg_read_32((receive_desc[1].desc2 | 0xa0000000)));

	screen_move_cursor(1, 13);
	for (i = 0;i < PNUM;i++) {
		printf_in_kernel("buff %d:%x;", i, reg_read_32((receive_desc[i].desc2 | 0xa0000000)) );
	}
	*/
	vt100_move_cursor(1, 11);
	cnt = 0;
	for (i = 0;i < PNUM;i++) {
		int received = ((receive_desc[i].desc0 & 0x80000000) == 0);
		int valid = (reg_read_32((receive_desc[i].desc2 | 0xa0000000)) == 0xb57b5500);
		if (valid && received) {
			cnt++;
		}
	}
	//printk("final:%d ", cnt);

	if (cnt == PNUM) { // received 64 valid packages
		do_unblock_all(&recv_block_queue);
	}
	clear_interrupt();
}


/*
 Caution!  the last few packages will be filled in the first several places (where packages from OS lie when recv begins),
	but when to print buff, we start from buff base and print successively the following content
    in this way, the last few packages will be printed out first
    which might cause problem
*/

/* set INT1_EN reg to enable mac interrupt */
void irq_enable(int IRQn)
{
	reg_write_32(INT1_EN, IRQn);
}
/**
 * Clears all the pending interrupts.
 * If the Dma status register is read then all the interrupts gets cleared
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 * you will use it in task3
 */
void clear_interrupt()
{
    uint32_t data;
    data = reg_read_32(0xbfe11000 + DmaStatus);
    reg_write_32(0xbfe11000 + DmaStatus, data);
}

/* use it to print buffer alone in task3, leave the check and wake-up work for mac_irq_handler */
/* user function */
void mac_recv_handle(mac_t *test_mac)
{
	//sys_wait_recv_package();
	//sys_move_cursor(1, 1);
	//printf("recv done.           \n");
	int i = 0;
	int print_location = 1;
	int valid_num = 0;
	for (i = 0; i < PNUM; i++)
	{
		uint32_t *Recv_desc = (uint32_t *)(test_mac->rd + i * 16);
		desc_t *recv = (desc_t *)Recv_desc;

		sys_move_cursor(1, print_location);
		printf("\n%d recv buffer,r_desc( 0x%x) =0x%x:          \n", i, Recv_desc, *(Recv_desc));

		uint32_t recv_buffer = recv->desc2;
		valid_num += printf_recv_buffer((recv->desc2 | 0xa0000000));
		sys_sleep(1);
		printf("\n");
	}

	print_location = 4;
	sys_move_cursor(1, print_location);
	printf("\nrecv valid %d packages!:\n                                ", valid_num);

}

static uint32_t printk_recv_buffer(uint32_t recv_buffer)
{
    //for debug ,you can use this function to print the contents of recv buffer
}

void set_sram_ctr()
{
    *((volatile unsigned int *)0xbfd00420) = 0x8000; /* 使能GMAC0 */
}
static void s_reset(mac_t *mac) //reset mac regs
{
    uint32_t time = 1000000;
    reg_write_32(mac->dma_addr, 0x01);

    while ((reg_read_32(mac->dma_addr) & 0x01))
    {
        reg_write_32(mac->dma_addr, 0x01);
        while (time)
        {
            time--;
        }
    };
}
void disable_interrupt_all(mac_t *mac)
{
    reg_write_32(mac->dma_addr + DmaInterrupt, DmaIntDisable);
    return;
}
void set_mac_addr(mac_t *mac)
{
    uint32_t data;
    uint8_t MacAddr[6] = {0x00, 0x55, 0x7b, 0xb5, 0x7d, 0xf7};
    uint32_t MacHigh = 0x40, MacLow = 0x44;
    data = (MacAddr[5] << 8) | MacAddr[4];
    reg_write_32(mac->mac_addr + MacHigh, data);
    data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0];
    reg_write_32(mac->mac_addr + MacLow, data);
}
uint32_t do_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{

	reg_write_32(DMA_BASE_ADDR + 0xc, rd_phy);
	reg_write_32(GMAC_BASE_ADDR, reg_read_32(GMAC_BASE_ADDR) | 0x4);
	reg_write_32(DMA_BASE_ADDR + 0x18, reg_read_32(DMA_BASE_ADDR + 0x18) | 0x02200002); // start tx, rx
	reg_write_32(DMA_BASE_ADDR + 0x1c, 0x10001 | (1 << 6));
	// to do :write DMA reg 2
	int i = 0;
	desc_t *receive = (desc_t *)rd;
	for (i = 0;i < PNUM;i++)
		receive[i].desc0 = 0x80000000; //DMA own
	for (i = 0;i < PNUM;i++)
		reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);		//write 1 to DMA Reg 2

#if TASK_1
	// Polling for recv
	while (1) {
		reg_write_32(DMA_BASE_ADDR + 0x8, 0x1);
		for (i = 0;i < PNUM;i++){
			if ( (receive[64].desc0 & 0x80000000 ) == 0 ) {
				
				screen_move_cursor(3, 2);
				printf_in_kernel("desc0:0x%x\n", receive[0].desc0);
				printf_recv_buffer(daddr + i * 4 * PSIZE);
				receive[i].desc0 = 0x80000000;
				return 0;
			}
		}
	}
#endif
    return 0;
}

void do_net_send(uint32_t td, uint32_t td_phy)
{

    reg_write_32(DMA_BASE_ADDR + 0x10, td_phy);		// fill phy_addr of tdesc into DMA Reg 4 (Offset 0x10)

    // MAC rx/tx enable

    reg_write_32(GMAC_BASE_ADDR, reg_read_32(GMAC_BASE_ADDR) | 0x8);                    // enable MAC-TX
    reg_write_32(DMA_BASE_ADDR + 0x18, reg_read_32(DMA_BASE_ADDR + 0x18) | 0x02202000); //0x02202002); // start tx, rx
    reg_write_32(DMA_BASE_ADDR + 0x1c, 0x10001 | (1 << 6));
    //to do: write DMA reg 1
	int i = 0;
	
	desc_t *send = (desc_t *)td;
	for (i = 0;i < PNUM;i++)
		//	((desc_t *)(td + sizeof(desc_t)*i))->desc0 = 0x80000000; //DMA own
		send[i].desc0 = 0x80000000;
	
	for (i = 0;i < PNUM;i++)
		reg_write_32(DMA_BASE_ADDR + 0x4, 0x1);		//write 1 to DMA Reg 1

}

void do_init_mac(void)
{
    mac_t test_mac;
    uint32_t i;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    set_sram_ctr(); /* 使能GMAC0 */
    s_reset(&test_mac);
    disable_interrupt_all(&test_mac);
    set_mac_addr(&test_mac);

#if TASK_3
	reg_write_32(INT1_CLR, 0xffffffff);
	reg_write_32(INT1_POL, 0xffffffff);
	reg_write_32(INT1_EDGE, 0x0);
#endif
}

void do_wait_recv_package(void)
{
    //to do: block the recv thread
	do_block(&recv_block_queue);
}