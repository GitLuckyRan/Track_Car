#include "stm32f1xx_hal.h"
#include "bsp_pwm.h"
#include "bsp_motor.h"
#include "bsp_ultraSound.h"
#include "car_pid.h"
#include "bsp_redCheck.h"
#include "tim.h"
int16_t BiasValue = 0;
int16_t BiasDirection = 0;   //方向设置
int16_t LastDirection = 0;   //上一次方向
volatile uint8_t LineValue;
uint16_t Count = 0;

void Task_Init(void)
{
    StartPWM();
    Car_Forward(500);
//    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_Base_Start_IT(&htim4);
}

void getBias(int16_t right, int16_t left)
{
    if (left > right)
    {
        Count = 0;
        for (int16_t i = 0; i<4; i++)
        {
            if (((LineValue >> (4+i))&0x01) == 0)
            {
                BiasValue = 1<<i;
                BiasDirection = 1;
                break;
            }
        }      
    }
    else if (left < right)
    { 
       Count = 0;
         for (int16_t i = 0; i<4; i++)
        {
            if (((LineValue >> (3-i))&0x01) == 0)
            {
                BiasValue = 1<<i;
                BiasDirection = 2;
                break;
            }
        }   
    }
    else
    {
        if (LineValue == 0xFF )    //检测是否越界,Count防止断连
        {
            Count += 1 ;
            if (Count > 10)
            {
                BiasDirection = 0;
            }
            else{
                BiasDirection = 3;
            }
            
        }
        else{
            Count = 0; 
            BiasDirection = 3;
        }
        BiasValue = 0;
    }
    
    if (Length < 10.0)
    {
        BiasDirection = 0;
    }
 
      
}

void SelectDirection(void)
{ 
    int16_t right = 0;
    int16_t left = 0;
    right = (LineValue >> 4) & 0x0F;
    uint8_t temp = 0x00; 
    temp |= (LineValue & 0x01)<<3;
    temp |= (LineValue & 0x02)<<1;
    temp |= (LineValue & 0x04)>>1;
    temp |= (LineValue & 0x08)>>3;
    left = temp & 0x0F;
    getBias(right,left);  
}
int16_t BASE_SPEED = 500;

//控制轮子转速
void Control_Direction(void)
{
    int16_t speed = PID(BiasValue);
    
//    int16_t left_motor = BASE_SPEED + speed;
//    int16_t right_motor =  BASE_SPEED - speed;
   
    if (BiasDirection == 1) 
    {
        Car_SetSpeed(BASE_SPEED + speed ,BASE_SPEED - speed);           
    }
    else if (BiasDirection == 2)
    {
        Car_SetSpeed(BASE_SPEED - speed ,BASE_SPEED + speed);
    }
    else if (BiasDirection == 0 )
    {
        Car_Stop();    
    }
    else
    {
        if (LastDirection == 0)
        {
           Car_Forward(1000);  //暂停后第一次启动，超过静摩擦力
        }else
        {
           Car_Forward(BASE_SPEED);
        }     
    }
    LastDirection = BiasDirection;

}

void Task_Run(void)
{
    //获取红外传感器值
    LineValue = GetRedSensorData(); 
    
    //获取红外传感器值
    Length = GetLength();
    
    //设置BiaDirection方向值
    SelectDirection();
    
    //控制转轮转速
    Control_Direction();   
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    
////  if (htim->Instance == TIM3)
////  {
////      Task_Run();
////  }
//    
//  if (htim->Instance == TIM4)
//  {
//       msCount++;
//  }
//  
//}

