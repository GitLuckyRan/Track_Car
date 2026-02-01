#include "stm32f1xx_hal.h"
#include "tim.h"
uint16_t msCount;
float Length;
static void OpenTimerForHc()
{
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    msCount = 0;
    HAL_TIM_Base_Start_IT(&htim4);

}

static void CloseTimerForHc(void)
{
    HAL_TIM_Base_Stop_IT(&htim4);
}

uint32_t GetEchoTimer(void)
{
    uint32_t t=0;
    t = msCount * 1000;
    t += __HAL_TIM_GET_COUNTER(&htim4);     
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    return t;
}


void HAL_Delay_us(uint32_t us)
{
    uint32_t ticks = 0;
    uint32_t told = 0, tnow = 0, tcnt = 0;
    uint32_t reload = SysTick->LOAD; 
    ticks = us * (SystemCoreClock / 1000000);  
    told = SysTick->VAL;  
    while(1)
    {
        tnow = SysTick->VAL;
        if(tnow != told)
        {
            if(tnow < told) tcnt += told - tnow;
            else tcnt += reload - tnow + told;
            told = tnow;
            if(tcnt >= ticks) break;
        }
    }
}

float GetLength(void)
{
    uint32_t t =0;
    float length = 0;
    float sum = 0;
    float max_val=0,min_val=100;
    uint16_t timeout = 0;
    for(int i = 0; i<1; i++)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
        HAL_Delay_us(20);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
        while (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == 0)
        {
            timeout +=1;
            if(timeout > 20000)
            {
                break;
            }
        
        };
        OpenTimerForHc(); 
        timeout = 0;
        while (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == 1)
        {
             timeout +=1;
            if(timeout > 20000)
            {
                break;
            }
        
        }
                 
        CloseTimerForHc();
        t = GetEchoTimer();
        length = ((float)t/58.0);
        if(length>max_val) max_val=length;
        if(length<min_val) min_val=length;
        sum += length;
    }
    return sum;
//    return (sum-max_val-min_val)/3.0;
}


