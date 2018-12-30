/***
  * @author  Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "utils.h"

/* Exported functions --------------------------------------------------------*/
void Utils_MemoryCopy(uint8_t *source, uint8_t *destination, uint32_t length)
{
	if (source && destination)
	{
		for (uint32_t i = 0; i < length; i++)
		{
			destination[i] = source[i];
		}
	}
}
