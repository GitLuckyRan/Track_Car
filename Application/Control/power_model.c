#include "stm32f1xx_hal.h"
#include "main.h"
#include "tim.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_motor.h"
void Standby_Mode(void)
{
    //开启电源时钟，操作PWR寄存器 SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN)
    __HAL_RCC_PWR_CLK_ENABLE();
    
    //设置wakeup引脚
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    
    //清除之前唤醒标志
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    
    //清楚待机标志
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    
    //进入待机模式
    HAL_PWR_EnterSTANDBYMode();
    
}

void Stop_Mode(void)
{
    
    // 1.1 刹车：将 PWM 占空比设为 0，防止意外转动
    Car_SetSpeed(0, 0); 
    
    // 1.2 停止 PWM 输出 (最安全做法，彻底切断定时器)
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3); // 左电机
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4); // 右电机
    
    // 2. 【关键修正】挂起滴答定时器，防止 SysTick 自动唤醒
    HAL_SuspendTick();
    
    HAL_TIM_IC_Stop_IT(&htim3, TIM_CHANNEL_2);
    //清除中断标志
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_8);
    
    //进入停止模式
    HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    //醒来恢复时钟
    SystemClock_Config();
    // 3.2 重启电机 PWM (此时占空比是 0，电机不会动，是安全的)
    HAL_ResumeTick();
    
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_Base_Start_IT(&htim3);
    Car_SetSpeed(800, 800); 
    
//    osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
//    MX_FREERTOS_Init();
//    xTaskCreate(vStartRun,"StartRun",600,NULL,2,NULL);

//  /* Start scheduler */
//    osKernelStart();
//    
}
