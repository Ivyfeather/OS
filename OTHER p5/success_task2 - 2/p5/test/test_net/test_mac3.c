#include "mac.h"
#include "irq.h"
#include "type.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "test4.h"
#include "queue.h"

queue_t recv_block_queue;
desc_t *send_desc = (desc_t *)SEND_DESC_BASE;
desc_t *receive_desc = (desc_t *)RECV_DESC_BASE;

uint32_t cnt = 1; //record the time of iqr_mac
//uint32_t buffer[PSIZE] = {0x00040045, 0x00000100, 0x5d911120, 0x0101a8c0, 0xfb0000e0, 0xe914e914, 0x00000801,0x45000400, 0x00010000, 0x2011915d, 0xc0a80101, 0xe00000fb, 0x14e914e9, 0x01080000};
uint32_t buffer[PSIZE] = {0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};

static void mac_send_desc_init(mac_t *mac)
{
	//init send desc
	//clear space for send_desc
	memset((void *)send_desc, 0, sizeof(desc_t)*DESC_NUM);

	mac->saddr = (uint32_t)buffer;
	mac->saddr_phy = TO_PHY_ADDR(buffer);

	mac->td = (uint32_t)send_desc;
	mac->td_phy = TO_PHY_ADDR(send_desc);

	int i = 0;
	for (;i < PNUM;i++) {
		send_desc[i].desc0 = 0x00000000;
		send_desc[i].desc1 = TxLast | TxFirst | TxChain | (PSIZE * 4);
		send_desc[i].desc2 = TO_PHY_ADDR(buffer);
		send_desc[i].desc3 = TO_PHY_ADDR(send_desc + i + 1);
	}
	// something special for the last tdesc
	send_desc[PNUM - 1].desc1 = TxLast | TxFirst | TxEnd | TxChain | (PSIZE * 4);
	send_desc[PNUM - 1].desc3 = TO_PHY_ADDR(send_desc);
}
static void mac_recv_desc_init(mac_t *mac)
{
	//init recv desc
	//clear space for recv_desc
	memset((void *)receive_desc, 0, sizeof(desc_t)*DESC_NUM);
	//clear space for recv_buff
	memset((void *)RecieveBuffBase, 0, PSIZE * 4 * PNUM);

	queue_init(&recv_block_queue);

	mac->daddr = RecieveBuffBase;
	mac->daddr_phy = TO_PHY_ADDR(RecieveBuffBase);

	mac->rd = (uint32_t)receive_desc;
	mac->rd_phy = TO_PHY_ADDR(receive_desc);

	int i = 0;
	for (;i < PNUM;i++) {
		receive_desc[i].desc0 = 0x00000000;
		receive_desc[i].desc1 = RxDIC | RxChain | (PSIZE * 4);
		receive_desc[i].desc2 = TO_PHY_ADDR(RecieveBuffBase + PSIZE * 4 * i);
		receive_desc[i].desc3 = TO_PHY_ADDR(receive_desc + i + 1);
	}

	receive_desc[PNUM - 1].desc1 = RxEnd | RxChain | (PSIZE * 4);
	receive_desc[PNUM - 1].desc3 = TO_PHY_ADDR(receive_desc);
}

static void mii_dul_force(mac_t *mac)
{
    reg_write_32(mac->dma_addr, 0x80); //?s
                                       //   reg_write_32(mac->dma_addr, 0x400);
    uint32_t conf = 0xc800;            //0x0080cc00;

    // loopback, 100M
    reg_write_32(mac->mac_addr, reg_read_32(mac->mac_addr) | (conf) | (1 << 8));
    //enable recieve all
    reg_write_32(mac->mac_addr + 0x4, reg_read_32(mac->mac_addr + 0x4) | 0x80000001);
}

void dma_control_init(mac_t *mac, uint32_t init_value)
{
    reg_write_32(mac->dma_addr + DmaControl, init_value);
    return;
}

void mac_send_task()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t print_location = 2;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    mac_send_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt(&test_mac);

    mii_dul_force(&test_mac);

    register_irq_handler(LS1C_MAC_IRQ, mac_irq_handle);

    irq_enable(LS1C_MAC_IRQ);
    sys_move_cursor(1, print_location);
    printf("> [MAC SEND TASK] start send package.               \n");

    uint32_t cnt = 0;
    i = 4;
    while (i > 0)
    {
        sys_net_send(test_mac.td, test_mac.td_phy);
        cnt += PNUM;
        sys_move_cursor(1, print_location);
        printf("> [MAC SEND TASK] totally send package %d !        \n", cnt);
        i--;
    }
    sys_exit();
}

void mac_recv_task()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t ret;
    uint32_t print_location = 1;

    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum
    mac_recv_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt(&test_mac);

    mii_dul_force(&test_mac);

    queue_init(&recv_block_queue);
    sys_move_cursor(1, print_location);
    printf("[MAC RECV TASK] start recv:                    ");
    ret = sys_net_recv(test_mac.rd, test_mac.rd_phy, test_mac.daddr);

    ch_flag = 0;
    for (i = 0; i < PNUM; i++)
    {
        recv_flag[i] = 0;
    }

    uint32_t cnt = 0;
    uint32_t *Recv_desc;
    Recv_desc = (uint32_t *)(test_mac.rd + (PNUM - 1) * 16);
    //printf("(test_mac.rd 0x%x ,Recv_desc=0x%x,REDS0 0X%x\n", test_mac.rd, Recv_desc, *(Recv_desc));
    if (((*Recv_desc) & 0x80000000) == 0x80000000)
    {
        sys_move_cursor(1, print_location);
        printf("> [MAC RECV TASK] waiting receive package.\n");
        sys_wait_recv_package();
    }
    mac_recv_handle(&test_mac);

    sys_exit();
}

void mac_init_task()
{
    uint32_t print_location = 1;
    sys_move_cursor(1, print_location);
    printf("> [MAC INIT TASK] Waiting for MAC initialization .\n");
    sys_init_mac();
    sys_move_cursor(1, print_location);
    printf("> [MAC INIT TASK] MAC initialization succeeded.           \n");
    sys_exit();
}
