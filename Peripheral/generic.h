/**
  * @author     Onur Efe
  */

#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdint.h>

#define MAX_INT8 0x7f
#define MIN_INT8 (-MAX_INT8 - 1)

#define MAX_UINT12 (0x0FFF)
#define MAX_UINT16 (0xFFFF)
#define MAX_UINT32 (0xFFFFFFFF)

#define MAX_INT16 (0x7FFF)
#define MIN_INT16 (-MAX_INT16 - 1)
#define MAX_INT32 (0x7FFFFFFF)
#define MIN_INT32 (-MAX_INT32 - 1)

#define FALSE ((uint8_t)0)
#define TRUE ((uint8_t)1)

#define NEGATIVE 0
#define POSITIVE 1

/* Exported macros ---------------------------------------------------------*/
#define SET_MASKED(variable, mask) ((variable) |= mask)
#define CLEAR_MASKED(variable, mask) ((variable) &= ~mask)

/* Exported types ----------------------------------------------------------*/
enum
{
	OPERATION_RESULT_FAILURE = 0x00,
	OPERATION_RESULT_SUCCESS = 0x01
};
typedef uint8_t OperationResult_t;

typedef uint8_t Bool_t;

typedef uint64_t quint48_16_t;
typedef int64_t qint47_16_t;
typedef int64_t qint31_32_t;
typedef int32_t qint15_16_t;
typedef uint32_t quint16_16_t;
typedef uint64_t quint32_32_t;
typedef int32_t qint16_15_t;
typedef int32_t qint19_12_t;

#endif
