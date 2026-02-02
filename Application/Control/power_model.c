#include "stm32f1xx_hal.h"

void Standby_Mode(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    
}