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
  #define QUEUE_GENERIC_GET_CONTAINER_SIZE(itemSize, itemCount) ((itemSize) * (itemCount))

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
	*             itemCount-> Capacity of the queue.
  *
  * @Return     None.
  */
  static inline void QueueGeneric_InitBuffer(QueueGeneric_Buffer_t *buff, void *container,
                                             uint16_t itemSize, uint16_t itemCount)
  {
    Queue_InitBuffer(&buff->queue, container, itemSize * itemCount);
    buff->itemSize = itemSize;
  }

  /***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  */
  static inline void QueueGeneric_ClearBuffer(QueueGeneric_Buffer_t *buff)
  {
    Queue_ClearBuffer(&buff->queue);
  }

  /***
  * @Brief      Enqueues object pointer to the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *             		obj-> Object pointer to be enqueued.
  *
  * @Return     None.
  */
  static inline void QueueGeneric_Enqueue(QueueGeneric_Buffer_t *buff, void *obj)
  {
    Queue_EnqueueArr(&buff->queue, obj, buff->itemSize);
  }

  /***            
  * @Brief      Dequeues object pointer from the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *
  * @Return     Object pointer.
  */
  static inline void QueueGeneric_Dequeue(QueueGeneric_Buffer_t *buff, void *obj)
  {
    Queue_DequeueArr(&buff->queue, obj, buff->itemSize);
  }

  static inline void QueueGeneric_Peek(QueueGeneric_Buffer_t *buff, uint16_t idx, void *obj)
  {
    Queue_PeekArr(&buff->queue, buff->itemSize * idx, obj, buff->itemSize);
  }

  static inline void QueueGeneric_Write(QueueGeneric_Buffer_t *buff, uint16_t idx, void *obj)
  {
    Queue_WriteArr(&buff->queue, buff->itemSize * idx, obj, buff->itemSize);
  }

  static inline void QueueGeneric_PeekLast(QueueGeneric_Buffer_t *buff, void *obj)
  {
    Queue_PeekArr(&buff->queue, (uint16_t)Queue_GetElementCount(&buff->queue) - buff->itemSize,
                  obj, buff->itemSize);
  }

  static inline void *QueueGeneric_GetPtr(QueueGeneric_Buffer_t *buff, uint16_t idx)
  {
    return Queue_GetPtr(&buff->queue, buff->itemSize * idx);
  }

  static inline void QueueGeneric_Remove(QueueGeneric_Buffer_t *buff, uint16_t idx)
  {
    Queue_Remove(&buff->queue, buff->itemSize * idx);
  }

  /***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     buff-> Buffer pointer.
  *
  * @Return     Available space.
  */
  static inline uint16_t QueueGeneric_GetAvailableSpace(QueueGeneric_Buffer_t *buff)
  {
    return (Queue_GetAvailableSpace(&buff->queue) / buff->itemSize);
  }

  /***
  * @Brief      Gets the number of elements in queue..
  *
  * @Params     buff-> Pointer to buffer.
  *
  * @Return     Element count.
  */
  static inline uint16_t QueueGeneric_GetElementCount(QueueGeneric_Buffer_t *buff)
  {
    return ((uint16_t)Queue_GetElementCount(&buff->queue) / buff->itemSize);
  }

#ifdef __cplusplus
}
#endif

#endif
