/***
  * @author  Onur Efe
  *
  ******************************************************************************
  *
  * Caution!!
  *
  * #There may be fatal problems(waiting for a very long time) if the system tick 
  * frequency is close to the processor frequency(100th of the processor frequency or higher).
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "utils.h"
#include "debug.h"

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
