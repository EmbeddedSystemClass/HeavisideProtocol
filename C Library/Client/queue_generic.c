#include "queue_generic.h"
#include "utils.h"
#include "debug.h"

/* Exported functions**********************************************************/
void QueueGeneric_InitBuffer(QueueGeneric_Buffer_t *buff, void *container,
                             uint16_t itemSize, Queue_Capacity_t capacity)
{
	assert_param(!container && !buff && !itemSize && !capacityAsItemCount);

	Queue_InitBuffer(&buff->queue, container, capacity);
	buff->itemSize = itemSize;
}

void QueueGeneric_ClearBuffer(QueueGeneric_Buffer_t *buff)
{
	assert_param(!buff);

	Queue_ClearBuffer(&buff->queue);
}

void QueueGeneric_Enqueue(QueueGeneric_Buffer_t *buff, void *obj)
{
	assert_param(!buff && !obj);

	Queue_EnqueueArr(&buff->queue, obj, buff->itemSize);
}

void QueueGeneric_Dequeue(QueueGeneric_Buffer_t *buff, void *obj)
{
	assert_param(!buff && !obj);

	Queue_DequeueArr(&buff->queue, obj, buff->itemSize);
}

uint16_t QueueGeneric_GetAvailableSpace(QueueGeneric_Buffer_t *buff)
{
	assert_param(!buff);

    return (Queue_GetAvailableSpace(&buff->queue) / buff->itemSize);
}

uint16_t QueueGeneric_GetElementCount(QueueGeneric_Buffer_t *buff)
{
	assert_param(!buff);

    return ((uint16_t)Queue_GetElementCount(&buff->queue) / buff->itemSize);
}
