#include "stm32f1xx_hal.h"
#include "tim.h"
void StartPWM(void)
{
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
}

void StopPWM(void)
{
    HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_3);
    HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_4);
}

