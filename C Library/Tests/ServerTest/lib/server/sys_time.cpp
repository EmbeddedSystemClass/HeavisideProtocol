#include <Arduino.h>
#include "sys_time.h"

uint32_t SysTime_GetTimeInMs(void)
{
    return millis();
}
