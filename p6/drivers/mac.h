#ifndef INCLUDE_MAC_H_
#define INCLUDE_MAC_H_

#include "type.h"
#include "queue.h"
#include "string.h"
//#define TEST_REGS1
//#define TEST_REGS2
#define TEST_REGS3
#define GMAC_BASE_ADDR (0xbfe10000)
#define DMA_BASE_ADDR (0xbfe11000)

#define INT1_SR   0xbfd01058
#define INT1_EN   0xbfd0105c 
#define INT1_CLR  0xbfd01064
#define INT1_POL  0xbfd01068
#define INT1_EDGE 0xbfd0106c
#define LS1C_MAC_IRQ 0x8


#define SEND_DESC_BASE 0xa0200000
#define RECV_DESC_BASE 0xa0400000
#define RecieveBuffBase 0xa0600000

#define PSIZE (256)
#define PNUM (64)
#define DESC_NUM (64)

#define TASK_1 0
#define TASK_2 0
#define TASK_3 1
/* do not forget to change Makefile as well */

#define TO_PHY_ADDR(vaddr) ( ( (uint32_t)(vaddr) ) & 0x1fffffff)	//clear highest 3 bits
extern queue_t recv_block_queue;
extern uint32_t recv_flag[PNUM];
extern uint32_t ch_flag;
enum GmacRegisters
{
    GmacAddr0Low = 0x0044,  /* Mac frame filtering controls */
    GmacAddr0High = 0x0040, /* Mac address0 high Register  */
};
enum DmaRegisters
{
    DmaStatus = 0x0014,    /* CSR5 - Dma status Register                        */
    DmaInterrupt = 0x001C, /* CSR7 - Interrupt enable                           */
    DmaControl = 0x0018,   /* CSR6 - Dma Operation Mode Register                */
};
enum DmaControlReg
{

    DmaStoreAndForward = 0x00200000, /* (SF)Store and forward                            21      RW        0       */
    DmaRxThreshCtrl128 = 0x00000018, /* (RTC)Controls thre Threh of MTL tx Fifo 128      4:3   RW                */

    DmaTxStart = 0x00002000, /* (ST)Start/Stop transmission                      13      RW        0       */

    DmaTxSecondFrame = 0x00000004, /* (OSF)Operate on second frame                     4       RW        0       */
};
enum TxControlReg
{
	TxIC = (0x1 << 31),	/* Interrupt on Completion */
	TxLast = (0x1 << 30),	/* Last Segment */
	TxFirst = (0x1 << 29),	/* First Segment */

	TxEnd = (0x1 << 25),	/* Transit End of Ring */
	TxChain = (0x1 << 24),	/* Second Address Chained */

};
enum RxControlReg
{
	RxDIC = (0x1 << 31),	/* Disable Interrupt in Completion */

	RxEnd = (0x1 << 25),	/* Receive End of Ring */
	RxChain = (0x1 << 24),	/* Second Address Chained */

};

enum InitialRegisters
{
    DmaIntDisable = 0,
};
/*
typedef struct desc
{
	uint32_t tdes0;
	uint32_t tdes1;
	uint32_t tdes2;
	uint32_t tdes3;
} desc_t;
*/
typedef struct desc
{
	uint32_t desc0;
	uint32_t desc1;
	uint32_t desc2;
	uint32_t desc3;
} desc_t;
typedef struct mac
{
    uint32_t psize; // backpack size
    uint32_t pnum;
    uint32_t mac_addr; // MAC base address
    uint32_t dma_addr; // DMA base address

    uint32_t saddr; // send address
    uint32_t daddr; // receive address

    uint32_t saddr_phy; // send phy address
    uint32_t daddr_phy; // receive phy address

    uint32_t td; // DMA send desc
    uint32_t rd; // DMA receive desc

    uint32_t td_phy;
    uint32_t rd_phy;

} mac_t;

uint32_t read_register(uint32_t base, uint32_t offset);
void reg_write_32(uint32_t addr, uint32_t data);
uint32_t reg_read_32(uint32_t addr);
void clear_interrupt();
void irq_enable(int IRQn);
void print_rx_dscrb(mac_t *mac);
void print_tx_dscrb(mac_t *mac);
uint32_t do_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr);
void do_net_send(uint32_t td, uint32_t td_phy);
void do_init_mac(void);
void do_wait_recv_package(void);
void mac_irq_handle(void);
void mac_recv_handle(mac_t *test_mac);
#endif
