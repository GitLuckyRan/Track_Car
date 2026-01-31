#include "stm32f1xx_hal.h"
float kp = 60.0f;
float ki = 0;
float kd = 30.0f;
int16_t lasterror = 0;
int16_t PID(int16_t BiasValue)
{
    
    //pid = k*c_error + ki*s_errot + kd * c_error/dt; 
    float res = 0;
    int16_t cur_error = BiasValue * 10;
    res = kp*cur_error + ki*0 + kd*(cur_error-lasterror);
    lasterror = cur_error;
    return (int16_t)res;  
}
