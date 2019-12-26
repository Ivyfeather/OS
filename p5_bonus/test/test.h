#ifndef INCLUDE_TEST_H
#define INCLUDE_TEST_H

//#include "test4.h"
#include "sched.h"

#include "queue.h"

extern queue_t recv_block_queue;
#define EPT_ARP 0x0608 /* type: ARP */

static void init_data(uint32_t *addr);

void mac_send_task(void);
void mac_recv_task(void);
void mac_init_task(void);

static void init_mac(void);
void test_shell();

#endif
