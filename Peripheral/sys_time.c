#include "stm32f3xx_hal.h"
#include "sys_time.h"

uint32_t SysTime_GetTimeInMs(void)
{
    return HAL_GetTick();
}
