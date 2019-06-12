#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "generic.h"

	/* Exported functions --------------------------------------------------------*/
	/***
  * @brief      Calculates CRC16 code of the given byte array.
  *
  * @params     seed-> Initial crc value.
  * 			buff-> Pointer to buffer.
  *             size-> Size of the buffer.
  *
  * @retval     CRC code.
  */
	extern uint16_t CRC_Calculate16(uint16_t seed, uint8_t *buff, uint16_t size);

	/***
  * @brief      Calculates CRC8 code of the given byte array.
  *
  * @params     seed-> Initial crc value.
  * 			buff-> Pointer to buffer.
  *             size-> Size of the buffer.
  *
  * @retval     CRC code.
  */
	extern uint8_t CRC_Calculate8(uint8_t seed, uint8_t *buff, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
