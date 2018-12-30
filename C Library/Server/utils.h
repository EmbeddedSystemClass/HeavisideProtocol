/**
  ******************************************************************************
  * @file    utils.h
  * @author  Onur Efe
  * @date    18.03.2017
  * @brief   Utils class interface.
  ******************************************************************************
  */

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "generic.h"

	/* Exported functions --------------------------------------------------------*/
	extern void Utils_MemoryCopy(uint8_t *source, uint8_t *destination, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
