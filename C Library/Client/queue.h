/***
  * @author     Onur Efe
  */
#ifndef __QUEUE_H
#define __QUEUE_H

#include "generic.h"

#ifdef __cplusplus
extern "C"
{
#endif
  enum
  {
    QUEUE_CAPACITY_8 = 3,
    QUEUE_CAPACITY_16,
    QUEUE_CAPACITY_32,
    QUEUE_CAPACITY_64,
    QUEUE_CAPACITY_128,
    QUEUE_CAPACITY_256,
    QUEUE_CAPACITY_512,
    QUEUE_CAPACITY_1K,
    QUEUE_CAPACITY_2K,
    QUEUE_CAPACITY_4K,
    QUEUE_CAPACITY_8K,
    QUEUE_CAPACITY_16K,
    QUEUE_CAPACITY_32K
  };
  typedef uint8_t Queue_Capacity_t;

/* Includes ------------------------------------------------------------------*/
#include "generic.h"

/* Exported macros ---------------------------------------------------------*/
#define QUEUE_CONTAINER_SIZE(c) (1 << (c))

  /* Typedefs ------------------------------------------------------------------*/
  typedef struct
  {
    uint16_t tail;
    uint16_t head;
    uint16_t mask;
    uint8_t *pContainer;
  } Queue_Buffer_t;

  /* Exported functions --------------------------------------------------------*/
  /***
  * @Brief      Creates a buffer, allocates it's memory and returns the pointer 
  *             of it.           
  * @Params     buff-> Pointer to buffer.
  *             container-> Pointer of the data container.
  *             size-> Size of the data container.
  *
  * @Return     None.
  */
  extern void Queue_InitBuffer(Queue_Buffer_t *buff, uint8_t *container, Queue_Capacity_t capacity);

  /***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  */
  extern void Queue_ClearBuffer(Queue_Buffer_t *buff);

  /***
  * @Brief      Enqueues byte to the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *             byte-> Byte to be enqueued.
  */
  extern void Queue_Enqueue(Queue_Buffer_t *buff, uint8_t byte);

  /***
  * @Brief      Enqueues byte array to the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *             data-> Pointer to byte array to be enqueued.
  */
  extern void Queue_EnqueueArr(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength);

  /***            
  * @Brief      Dequeues byte from the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *
  * @Return     Byte.
  */
  extern uint8_t Queue_Dequeue(Queue_Buffer_t *buff);

  /***            
  * @Brief      Dequeues array from the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *						
  * @Return     None.
  */
  extern void Queue_DequeueArr(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength);

  /***
  * @Brief      Removes elements until the indexed element(not including).
  *
  * @Params     buff-> Pointer to the buffer.
  *             		elementIndex-> Index of the element.
  *
  * @Return     None.
  */
  extern void Queue_Remove(Queue_Buffer_t *buff, uint16_t elementIndex);

  /***
  * @Brief      Searches an element in the buffer. Returns element index if the element
  *             exists. Returns 0xFFFF otherwise.
  *
  * @Params     buff-> Buffer to be searched.
  *             	element-> Element value.
  *
  * @Return     Element index or 0xFFFF.
  */
  extern uint16_t Queue_Search(Queue_Buffer_t *buff, uint8_t element);

  /***
  * @Brief      Peeks element in the buffer. Indexing starts at the first element.
  *                       
  * @Params     buff-> Pointer to the buffer.
  *             		elementIndex-> Index of the element.
  *
  * @Return     Element value.
  */
  extern uint8_t Queue_Peek(Queue_Buffer_t *buff, uint16_t elementIndex);

  /***
  * @Brief      Sets element in the buffer. Indexing starts at the first element.
  *                       
  * @Params     buff-> Pointer to the buffer.
  *             elementIndex-> Index of the element.
  *             value-> Value of the element to be set.
  *  
  * @Return     None.
  */
  extern void Queue_Set(Queue_Buffer_t *buff, uint16_t elementIndex, uint8_t value);

  /***
  * @Brief      Gets index of next write take place. 
  *                       
  * @Params      buff-> Pointer to the buffer.
  *  
  * @Return     Index.
  */
  extern uint16_t Queue_GetCurrentIdx(Queue_Buffer_t *buff);

  /***
  * @Brief      Returns number of elements the buffer contains.
  *
  * @Params     buff-> Buffer pointer.
  *
  * @Return     Element count.
  */
  extern uint16_t Queue_GetElementCount(Queue_Buffer_t *buff);

  /***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     buff-> Buffer pointer.
  *
  * @Return     Available space.
  */
  extern uint16_t Queue_GetAvailableSpace(Queue_Buffer_t *buff);

  /***
  * @Brief      Checks if the buffer is empty.
  *
  * @Params     buff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
  extern Bool_t Queue_IsEmpty(Queue_Buffer_t *buff);

  /***
  * @Brief      Checks if the buffer is full.
  *
  * @Params     buff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
  extern Bool_t Queue_IsFull(Queue_Buffer_t *buff);

  /***
 * @Brief	Returns capacity of the buffer.
 * 
 * @Params	buff-> Pointer to buffer.
 * 
 * @Return Capacity.
 */
  extern uint16_t Queue_GetCapacity(Queue_Buffer_t *buff);

#ifdef __cplusplus
}
#endif

#endif
