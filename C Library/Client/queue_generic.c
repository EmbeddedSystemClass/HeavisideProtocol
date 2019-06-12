#include "queue_generic.h"

/* Exported functions**********************************************************/
void QueueGeneric_InitBuffer(QueueGeneric_Buffer_t *buff, void *container,
                             uint16_t itemSize, Queue_Capacity_t capacity)
{
	Queue_InitBuffer(&buff->queue, container, capacity);
	buff->itemSize = itemSize;
}

void QueueGeneric_ClearBuffer(QueueGeneric_Buffer_t *buff)
{
	Queue_ClearBuffer(&buff->queue);
}

void QueueGeneric_Enqueue(QueueGeneric_Buffer_t *buff, void *obj)
{
	Queue_EnqueueArr(&buff->queue, obj, buff->itemSize);
}

void QueueGeneric_Dequeue(QueueGeneric_Buffer_t *buff, void *obj)
{
	Queue_DequeueArr(&buff->queue, obj, buff->itemSize);
}

uint16_t QueueGeneric_GetAvailableSpace(QueueGeneric_Buffer_t *buff)
{
	return (Queue_GetAvailableSpace(&buff->queue) / buff->itemSize);
}

uint16_t QueueGeneric_GetElementCount(QueueGeneric_Buffer_t *buff)
{
	return ((uint16_t)Queue_GetElementCount(&buff->queue) / buff->itemSize);
}
