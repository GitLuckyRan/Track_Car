#ifndef __BSP_MOTOR_H
#define __BSP_MOTOR_H

void Control_Right_Motor(int speed);
void Control_Left_Motor(int speed);
void Car_Forward(int speed);
void Car_Back(int speed);
void Car_Stop(void);
void Car_SetSpeed(int left_speed, int right_speed);

#endif 
