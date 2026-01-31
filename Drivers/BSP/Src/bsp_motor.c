#include "stm32f1xx_hal.h"
#include "tim.h"

int MAXSPEED = 1000;

void Control_Right_Motor(int speed)
{
    if(speed > 0)
    {
        speed = (speed > MAXSPEED) ? MAXSPEED : speed;
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4, speed);
    }
    else
    {
        speed = (speed < -MAXSPEED) ? -MAXSPEED : speed;
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4, -speed);
    }

}

void Control_Left_Motor(int speed)
{
    if(speed > 0)
    {
        speed = (speed > MAXSPEED) ? MAXSPEED : speed;
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3, speed);
    }
    else
    {
        speed = (speed < -MAXSPEED) ? -MAXSPEED : speed;
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3, -speed);
    }

}

void Car_SetSpeed(int left_speed, int right_speed)
{
    Control_Left_Motor(left_speed);
    Control_Right_Motor(right_speed);
}


void Car_Forward(int speed)
{
    Control_Left_Motor(speed);
    Control_Right_Motor(speed);
}

void Car_Back(int speed)
{
    Control_Left_Motor(-speed);
    Control_Right_Motor(-speed);
}

void Car_Stop(void)
{
    Control_Left_Motor(0);
    Control_Right_Motor(0);
}


