#include "queue.h"
#include "sched.h"
#include "lock.h"
typedef pcb_t item_t;
queue_t ready_queue;
//queue_t block_queue;
queue_t sleep_queue;

void queue_init(queue_t *queue)
{
    queue->head = queue->tail = NULL;
}

int queue_is_empty(queue_t *queue)
{
    if (queue->head == NULL)
    {
        return 1;
    }
    return 0;
}

void queue_push(queue_t *queue, void *item)
{
    item_t *_item = (item_t *)item;
    /* queue is empty */
    if (queue->head == NULL)
    {
        queue->head = item;
        queue->tail = item;
        _item->next = NULL;
        _item->prev = NULL;
    }
    /*insert to tail*/
    else
    {
        ((item_t *)(queue->tail))->next = item;
        _item->next = NULL;
        _item->prev = queue->tail;
        queue->tail = item;
    }
	((pcb_t *)item)->queue = queue;
}

/*remove the first item and return it*/
void *queue_dequeue(queue_t *queue)
{
    item_t *temp = (item_t *)queue->head;
	if (temp == NULL) { //queue empty
		return (void *)NULL;
	}

    /* this queue only has one item */
    if (temp->next == NULL)
    {
        queue->head = queue->tail = NULL;
    }
    else
    {
        queue->head = ((item_t *)(queue->head))->next;
        ((item_t *)(queue->head))->prev = NULL;
    }

    temp->prev = NULL;
    temp->next = NULL;
	((pcb_t *)temp)->queue = NULL;
    return (void *)temp;
}

/* remove this item and return next item */
void *queue_remove(queue_t *queue, void *item)
{
    item_t *_item = (item_t *)item;
    item_t *next = (item_t *)_item->next;

    if (item == queue->head && item == queue->tail)
    {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else if (item == queue->head)
    {
        queue->head = _item->next;
        ((item_t *)(queue->head))->prev = NULL;
    }
    else if (item == queue->tail)
    {
        queue->tail = _item->prev;
        ((item_t *)(queue->tail))->next = NULL;
    }
    else
    {
        ((item_t *)(_item->prev))->next = _item->next;
        ((item_t *)(_item->next))->prev = _item->prev;
    }

    _item->prev = NULL;
    _item->next = NULL;
	((pcb_t *)item)->queue = NULL;
    return (void *)next;
}


void InitQueue(SqQueue *Q) {
	Q->front = Q->rear = 0;
}

int QueueLength(SqQueue *Q) {
	return (Q->rear - Q->front + (MBOXSIZE + 1)) % (MBOXSIZE + 1);
}

int EnQueue(SqQueue *Q, QElemType e) {
	if ((Q->rear + 1) % (MBOXSIZE + 1) == Q->front) return -1;//queue full
	Q->base[Q->rear] = e;
	Q->rear = (Q->rear + 1) % (MBOXSIZE + 1);
	return 1;
}

QElemType DeQueue(SqQueue *Q) {//delete first element, return it
	if (Q->front == Q->rear) return -1;
	QElemType tmp = Q->base[Q->front];
	Q->front = (Q->front + 1) % (MBOXSIZE + 1);
	return tmp;
}
