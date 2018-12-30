/***
  * @author     Onur Efe
  */
#ifndef QUEUE_GENERIC_H
#define QUEUE_GENERIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "generic.h"
#include "queue.h"

/* Definitions ---------------------------------------------------------------*/
#define QUEUE_GENERIC_BUFFER_SIZE(CAPACITY) \
	(1 << (CAPACITY))

	/* Typedefs ------------------------------------------------------------------*/
	typedef struct
	{
		Queue_Buffer_t queue;
        uint16_t itemSize;
	} QueueGeneric_Buffer_t;

	/* Exported functions --------------------------------------------------------*/
	/***
  * @Brief      Inits a buffer.
  * @Params     buff-> Pointer to buffer.
  *             container-> Address of the container which contains object addresses.
  *             itemSize-> Size of objects in bytes.
	*             capacity-> Capacity of the byte queue.
  *
  * @Return     None.
  */
	extern void QueueGeneric_InitBuffer(QueueGeneric_Buffer_t *buff, void *container,
                                        uint16_t itemSize, Queue_Capacity_t capacity);

	/***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  */
	extern void QueueGeneric_ClearBuffer(QueueGeneric_Buffer_t *buff);

	/***
  * @Brief      Enqueues object pointer to the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *             		obj-> Object pointer to be enqueued.
  *
  * @Return     None.
  */
	extern void QueueGeneric_Enqueue(QueueGeneric_Buffer_t *buff, void *obj);

	/***            
  * @Brief      Dequeues object pointer from the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *
  * @Return     Object pointer.
  */
	extern void QueueGeneric_Dequeue(QueueGeneric_Buffer_t *buff, void *obj);

	/***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     buff-> Buffer pointer.
  *
  * @Return     Available space.
  */
    extern uint16_t QueueGeneric_GetAvailableSpace(QueueGeneric_Buffer_t *buff);

	/***
  * @Brief      Gets the number of elements in queue..
  *
  * @Params     buff-> Pointer to buffer.
  *
  * @Return     Element count.
  */
    extern uint16_t QueueGeneric_GetElementCount(QueueGeneric_Buffer_t *buff);

#ifdef __cplusplus
}
#endif

#endif
