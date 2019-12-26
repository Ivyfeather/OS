#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_
#include "queue.h"
#include "cond.h"
typedef enum mailbox_status{
	CLOSED,
	OPEN
}mailbox_status_t;

typedef struct mailbox
{
	char *name;				//name of mailbox
	SqQueue message;		//message queue
	mutex_lock_t lock;		//actually not used, since a lock is required for global mboxs
	condition_t notEmpty;	//condition variable for customer
	condition_t notFull;	//condition variable for producer
	int num;				//num of accesses to mailbox, if 0 then close mailbox
	mailbox_status_t status;//CLOSED/OPEN
} mailbox_t;

void mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif