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

/* Includes ------------------------------------------------------------------*/
#include "generic.h"

  /* Exported macros ---------------------------------------------------------*/
  /* Typedefs ------------------------------------------------------------------*/
  typedef struct
  {
    uint16_t tail;
    uint16_t head;
    uint16_t size;
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
  static inline void Queue_InitBuffer(Queue_Buffer_t *buff, uint8_t *container, uint16_t size)
  {
    /* Initialize buffer. */
    buff->head = 0;
    buff->tail = 0;
    buff->pContainer = container;
    buff->size = size;
  }

  /***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  */
  static inline void Queue_ClearBuffer(Queue_Buffer_t *buff)
  {
    buff->head = buff->tail;
  }

  /***
  * @Brief      Enqueues byte to the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *             byte-> Byte to be enqueued.
  */
  static inline void Queue_Enqueue(Queue_Buffer_t *buff, uint8_t byte)
  {
    uint16_t __tail = buff->tail;

    // Set new tail element and update the tail value.
    buff->pContainer[__tail++] = byte;
    if (__tail == buff->size)
    {
      __tail = 0;
    }

    buff->tail = __tail;
  }

  /***
  * @Brief      Enqueues byte array to the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *             data-> Pointer to byte array to be enqueued.
  */
  static inline void Queue_EnqueueArr(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength)
  {
    uint16_t __tail = buff->tail;

    for (uint16_t i = 0; i < dataLength; i++)
    {
      buff->pContainer[__tail++] = data[i];
      if (__tail == buff->size)
      {
        __tail = 0;
      }
    }

    buff->tail = __tail;
  }

  /***            
  * @Brief      Dequeues byte from the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *
  * @Return     Byte.
  */
  static inline uint8_t Queue_Dequeue(Queue_Buffer_t *buff)
  {
    uint16_t __head = buff->head;

    // Parse head element.
    uint8_t element = buff->pContainer[__head++];
    if (__head == buff->size)
    {
      __head = 0;
    }

    buff->head = __head;

    return element;
  }

  /***            
  * @Brief      Dequeues array from the given buffer.
  *
  * @Params     buff-> Pointer to the buffer.
  *						
  * @Return     None.
  */
  static inline void Queue_DequeueArr(Queue_Buffer_t *buff, uint8_t *data, uint16_t dataLength)
  {
    uint16_t __head = buff->head;

    for (uint16_t i = 0; i < dataLength; i++)
    {
      data[i] = buff->pContainer[__head++];
      if (__head == buff->size)
      {
        __head = 0;
      }
    }

    buff->head = __head;
  }

  /***
  * @Brief      Returns number of elements the buffer contains.
  *
  * @Params     buff-> Buffer pointer.
  *
  * @Return     Element count.
  */
  static inline uint16_t Queue_GetElementCount(Queue_Buffer_t *buff)
  {
    int32_t elements = buff->tail - buff->head;
    if (elements < 0)
    {
      elements += buff->size;
    }

    return ((uint16_t)elements);
  }

  /***
  * @Brief      Removes elements until the indexed element(not including).
  *
  * @Params     buff-> Pointer to the buffer.
  *             length-> Length of removal.
  *
  * @Return     None.
  */
  static inline void Queue_Remove(Queue_Buffer_t *buff, uint16_t length)
  {
    /* Just change head index. */
    if (length <= Queue_GetElementCount(buff))
    {
      uint16_t __head = buff->head;
      __head += length;
      if (__head >= buff->size)
      {
        __head -= buff->size;
      }
      buff->head = __head;
    }
  }

  /***
  * @Brief      Peeks element in the buffer. Indexing starts at the first element.
  *                       
  * @Params     buff-> Pointer to the buffer.
  *             		elementIndex-> Index of the element.
  *
  * @Return     Element value.
  */
  static inline uint8_t Queue_Peek(Queue_Buffer_t *buff, uint16_t elementIndex)
  {
    uint16_t element_position = buff->head + elementIndex;
    if (element_position >= buff->size)
    {
      element_position -= buff->size;
    }

    return (buff->pContainer[element_position]);
  }

  /***
  * @Brief      Searches an element in the buffer. Returns element index if the element
  *             exists. Returns 0xFFFF otherwise.
  *
  * @Params     buff-> Buffer to be searched.
  *             	element-> Element value.
  *
  * @Return     Element index or 0xFFFF.
  */
  static inline uint16_t Queue_Search(Queue_Buffer_t *buff, uint8_t element)
  {
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

  /***
  * @Brief      Searches an array in the buffer. Returns array start index if the such an array
  *             exists. Returns 0xFFFF otherwise.
  *
  * @Params     buff-> Buffer to be searched.
  *             arr-> Pointer to array.
  *
  * @Return     Array start index or 0xFFFF.
  */
  static inline uint16_t Queue_SearchArr(Queue_Buffer_t *buff, uint8_t *arr, uint16_t arrLength)
  {
    uint16_t num_of_elements;
    num_of_elements = Queue_GetElementCount(buff);

    // Search buffer.
    for (uint16_t i = 0; i < num_of_elements; i++)
    {
      Bool_t matched = TRUE;
      for (uint16_t j = 0; j < arrLength; j++)
      {
        if (Queue_Peek(buff, i) != arr[j])
        {
          matched = FALSE;
          break;
        }
      }

      // If match found; return the start index.
      if (matched)
      {
        return i;
      }
    }

    return 0xFFFF;
  }

  static inline void Queue_PeekArr(Queue_Buffer_t *buff, uint16_t idx, uint8_t *data,
                                   uint16_t dataLength)
  {
    uint16_t __head = buff->head + idx;
    if (__head >= buff->size)
    {
      __head -= buff->size;
    }

    for (uint16_t i = 0; i < dataLength; i++)
    {
      data[i] = buff->pContainer[__head++];
      if (__head == buff->size)
      {
        __head = 0;
      }
    }
  }

  static inline void Queue_WriteArr(Queue_Buffer_t *buff, uint16_t idx, uint8_t *data,
                                    uint16_t dataLength)
  {
    uint16_t __head = buff->head + idx;
    if (__head >= buff->size)
    {
      __head -= buff->size;
    }

    for (uint16_t i = 0; i < dataLength; i++)
    {
      buff->pContainer[__head++] = data[i];
      if (__head == buff->size)
      {
        __head = 0;
      }
    }
  }

  static inline uint8_t *Queue_GetPtr(Queue_Buffer_t *buff, uint16_t idx)
  {
    uint16_t element_position = buff->head + idx;
    if (element_position >= buff->size)
    {
      element_position -= buff->size;
    }

    return (&buff->pContainer[element_position]);
  }

  /***
  * @Brief      Sets element in the buffer. Indexing starts at the first element.
  *                       
  * @Params     buff-> Pointer to the buffer.
  *             elementIndex-> Index of the element.
  *             value-> Value of the element to be set.
  *  
  * @Return     None.
  */
  static inline void Queue_Set(Queue_Buffer_t *buff, uint16_t elementIndex, uint8_t value)
  {
    uint16_t element_position = buff->head + elementIndex;
    if (element_position >= buff->size)
    {
      element_position -= buff->size;
    }

    buff->pContainer[element_position] = value;
  }

  /***
  * @Brief      Gets index of next write take place. 
  *                       
  * @Params      buff-> Pointer to the buffer.
  *  
  * @Return     Index.
  */
  static inline uint16_t Queue_GetCurrentIdx(Queue_Buffer_t *buff)
  {
    int32_t element_position = buff->tail - buff->head;
    if (element_position < 0)
    {
      element_position += buff->size;
    }

    return ((uint16_t)element_position);
  }

  /***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     buff-> Buffer pointer.
  *
  * @Return     Available space.
  */
  static inline uint16_t Queue_GetAvailableSpace(Queue_Buffer_t *buff)
  {
    int32_t elements = buff->tail - buff->head;
    if (elements < 0)
    {
      elements += buff->size;
    }

    return (buff->size - (elements + 1));
  }

  /***
  * @Brief      Checks if the buffer is empty.
  *
  * @Params     buff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
  static inline Bool_t Queue_IsEmpty(Queue_Buffer_t *buff)
  {
    return ((buff->tail == buff->head) ? TRUE : FALSE);
  }

  /***
  * @Brief      Checks if the buffer is full.
  *
  * @Params     buff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
  static inline Bool_t Queue_IsFull(Queue_Buffer_t *buff)
  {
    uint16_t next_tail = (buff->tail + 1);
    if (next_tail == buff->size)
    {
      next_tail = 0;
    }

    return ((next_tail == buff->head) ? TRUE : FALSE);
  }

  /***
 * @Brief	Returns capacity of the buffer.
 * 
 * @Params	buff-> Pointer to buffer.
 * 
 * @Return Capacity.
 */
  static inline uint16_t Queue_GetCapacity(Queue_Buffer_t *buff)
  {
    return (buff->size - 1);
  }

#ifdef __cplusplus
}
#endif

#endif
