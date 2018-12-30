/**
  * @author     Onur Efe
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "assert.h"

/* Exported functions --------------------------------------------------------*/
void Assert_Failed(const char *pFile, unsigned int line)
{
	printf("%s%u", pFile, line);
}
