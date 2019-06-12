/**
  * @author     Onur Efe
  */

#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdint.h>

#define MAX_INT8 0x7f
#define MIN_INT8 (-MAX_INT8 - 1)
#define MAX_UINT12 4095
#define MAX_UINT16 65535
#define MAX_INT16 32767
#define MAX_INT32 2147483647
#define MAX_UINT32 4294967295
#define FALSE (0)
#define TRUE (1)

/* Exported macros ---------------------------------------------------------*/
#define SET_BIT(variable, bit) ((variable) |= (1 << (bit)))
#define CLEAR_BIT(variable, bit) ((variable) &= ~(1 << (bit)))

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
typedef int32_t qint16_15_t;
typedef int32_t qint19_12_t;

#endif
