#include "stm32f1xx_hal.h"
#include "bsp_pwm.h"
#include "bsp_motor.h"
#include "bsp_ultraSound.h"
#include "car_pid.h"
#include "bsp_redCheck.h"
#include "tim.h"

int16_t BiasValue = 0;
int16_t BiasDirection = 0;   //0-停止 1-右转 2-左转 3-前进
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
        
        if (LineValue == 0xFF )    //没有检测到黑线，防止断连接触不良等原因，继续运行0.5s后，停止运行,
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
int16_t BASE_SPEED = 600;
//0-停止 1-右转 2-左转 3-前进
void Control_Direction(void)
{
    int16_t speed = PID(BiasValue);
    
//    int16_t left_motor = BASE_SPEED + speed;
//    int16_t right_motor =  BASE_SPEED - speed;
   
    if (BiasDirection == 1) 
    {
        Car_SetSpeed(BASE_SPEED + speed ,BASE_SPEED - 100);           
    }
    else if (BiasDirection == 2)
    {
        Car_SetSpeed(BASE_SPEED - 100 ,BASE_SPEED + speed);
    }
    else if (BiasDirection == 0 )
    {
        Car_Stop();    
    }
    else
    {
        Car_Forward(BASE_SPEED);
    }

}

void Task_Run(void)
{
    LineValue = GetRedSensorData(); 
    //获取偏转方向及大小
    SelectDirection();
    //检测前方是否由物体
    Length = GetLength();
    //控制其运行 
    Control_Direction();   
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    
//  if (htim->Instance == TIM3)
//  {
//      Task_Run();
//  }
  if (htim->Instance == TIM4)
  {
       msCount++;
  }
  
}

