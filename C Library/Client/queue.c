#include "queue.h"
#include "debug.h"

/* Exported functions**********************************************************/
void Queue_InitBuffer(Queue_Buffer_t *buff, uint8_t *container, Queue_Capacity_t capacity)
{
	/* Check if the pointer is valid. */
	assert_param(container && buff);

	/* Initialize buffer. */
	buff->head = 0;
	buff->tail = 0;
	buff->pContainer = container;
	buff->mask = (uint16_t)(1 << capacity) - 1;
}

void Queue_ClearBuffer(Queue_Buffer_t *buff)
{
	assert_param(buff);

	buff->head = 0;
	buff->tail = 0;
}

void Queue_Enqueue(Queue_Buffer_t *buff, uint8_t byte)
{
	assert_param(buff);

	// Set new tail element and update the tail value.
	buff->pContainer[buff->tail++] = byte;
	buff->tail &= buff->mask;
}

void Queue_EnqueueArr(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength)
{
	assert_param(pBuff && pData);

	for (uint16_t i = 0; i < dataLength; i++)
	{
		buff->pContainer[buff->tail++] = data[i];
		buff->tail &= buff->mask;
	}
}

uint8_t Queue_Dequeue(Queue_Buffer_t *buff)
{
	assert_param(buff);

	// Parse head element.
	uint8_t element = buff->pContainer[buff->head++];
	buff->head &= buff->mask;

	return element;
}

void Queue_DequeueArr(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength)
{
	assert_param(buff && data);

	for (uint16_t i = 0; i < dataLength; i++)
	{
		data[i] = buff->pContainer[buff->head++];
		buff->head &= buff->mask;
	}
}

void Queue_Remove(Queue_Buffer_t *buff, uint16_t elementIndex)
{
	assert_param(buff);

	/* Just change head index. */
    if (elementIndex <= Queue_GetElementCount(buff))
	{
		buff->head += elementIndex;
		buff->head &= buff->mask;
	}
}

uint16_t Queue_Search(Queue_Buffer_t *buff, uint8_t element)
{
	assert_param(buff);

	uint16_t num_of_elements;
	num_of_elements = Queue_GetElementCount(buff);

	// Search buffer.
	for (uint16_t i = 0; i < num_of_elements; i++)
	{
		if (Queue_Peek(buff, i) == element)
		{
			return i;
		}
	}

	return 0xFFFF;
}

uint8_t Queue_Peek(Queue_Buffer_t *buff, uint16_t elementIndex)
{
	assert_param(pBuff);

	uint16_t element_position = buff->head + elementIndex;
	element_position &= buff->mask;

	return (buff->pContainer[element_position]);
}

uint16_t Queue_GetAvailableSpace(Queue_Buffer_t *buff)
{
	assert_param(pBuff);

	uint16_t elements = (buff->tail - buff->head) & buff->mask;

    return (buff->mask - elements);
}

uint16_t Queue_GetElementCount(Queue_Buffer_t *buff)
{
	assert_param(pBuff);

	uint16_t elements = (buff->tail - buff->head) & buff->mask;

	return elements;
}

Bool_t Queue_IsEmpty(Queue_Buffer_t *buff)
{
	assert_param(buff);

	return ((buff->tail == buff->head) ? TRUE : FALSE);
}

Bool_t Queue_IsFull(Queue_Buffer_t *buff)
{
	assert_param(buff);

	uint16_t next_tail = (buff->tail + 1) & buff->mask;

	return ((next_tail == buff->head) ? TRUE : FALSE);
}

uint16_t Queue_GetCapacity(Queue_Buffer_t *buff)
{
	assert_param(buff);

    return (buff->mask);
}
