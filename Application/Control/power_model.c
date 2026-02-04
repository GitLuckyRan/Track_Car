#include "stm32f1xx_hal.h"
#include "main.h"
#include "tim.h"
#include "task.h"
#include "bsp_motor.h"
#include "remote_ir.h"
#include "bsp_pwm.h"
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


//停止模式会维持原值不变！！！

void Stop_Mode(void)
{
    
    // 1.1 刹车：将 PWM 占空比设为 0，防止意外转动
    Car_SetSpeed(0, 0); 
    
    // 1.2 停止 PWM 输出 
    StopPWM();
    
    // 2.挂起滴答定时器，防止SysTick自动唤醒
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
    
    //恢复IR_code 防止一唤醒就进入睡眠模式，巨坑~忽略了这个卡了半天！！！
    IR_code = 0xFF;
    
    //恢复PWM
    StartPWM();
  
    // 恢复红外遥控定时器中断
    StartRemote_IR();

}
