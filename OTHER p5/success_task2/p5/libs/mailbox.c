#include "string.h"
#include "mailbox.h"
#include "sched.h"
#include "syscall.h"
#include "lock.h"

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX];
static mutex_lock_t mutex;

void mbox_init()
{
	memset(mboxs, 0, sizeof(mboxs));
	// status set to CLOSED(0)
	// num to 0
	mutex_lock_init(&mutex);
}

mailbox_t *mbox_open(char *name)
{
	mutex_lock_acquire(&mutex);
	int i = 0, found = 0;
	for (;i < MAX_NUM_BOX;i++) {
		if (mboxs[i].status == OPEN && strcmp(mboxs[i].name, name) == 0) {
			mboxs[i].num++;
			found = 1;
			//////might be interrupted?
			mutex_lock_release(&mutex);
			return &mboxs[i];
		}
	}
	if (!found) {
		for (i = 0;mboxs[i].status == OPEN && i < MAX_NUM_BOX;i++)
			;
		if (i == MAX_NUM_BOX) {
			printf("Mailbox Full[%d].\n",MAX_NUM_BOX);
			mutex_lock_release(&mutex);
			return NULL;
		}
		// memset mbox 0 when destroying it
		mboxs[i].name = name;
		mboxs[i].status = OPEN;
		//mutex_lock_init(&mboxs[i].lock);
		condition_init(&mboxs[i].notEmpty);
		condition_init(&mboxs[i].notFull);
		mboxs[i].num = 1;
		mutex_lock_release(&mutex);
		return &mboxs[i];
	}

}

void mbox_close(mailbox_t *mailbox)
{
	mailbox->num--;
	if (mailbox->num == 0) {
		memset(mailbox, 0, sizeof(mailbox_t));
		// status set to CLOSED(0)
	}
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
	mutex_lock_acquire(&mutex);
	char *_msg = (char*)msg;
	int i = 0;
	for (i = 0;i < msg_length;i++) {
		while (QueueLength(&mailbox->message) == MBOXSIZE) {
			condition_wait(&mutex, &mailbox->notFull);
		}
		EnQueue(&mailbox->message, *_msg);
		_msg++;
		if (QueueLength(&mailbox->message) == 1) {
			condition_signal(&mailbox->notEmpty);
		}
	}
	mutex_lock_release(&mutex);
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
	mutex_lock_acquire(&mutex);
	char *_msg = (char*)msg;
	int i = 0;
	for (i = 0;i < msg_length;i++) {
		while (QueueLength(&mailbox->message) == 0) {
			condition_wait(&mutex, &mailbox->notEmpty);
		}
		*_msg = DeQueue(&mailbox->message);
		_msg++;
		if (QueueLength(&mailbox->message) == MBOXSIZE-1) {
			condition_signal(&mailbox->notFull);
		}
	}
	mutex_lock_release(&mutex);
}