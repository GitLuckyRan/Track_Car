#include "stm32f1xx_hal.h"
#include "tim.h"

uint8_t GetRedSensorData(void)
{
    uint8_t res = 0x00;
    uint16_t out_GPIOB = 0x0000;
    out_GPIOB = GPIOB->IDR;
    uint16_t out_GPIOA = 0x0000;
    out_GPIOA = GPIOA->IDR;
    for(int i = 12 ;i<16;i++)
    {
        res |= (((out_GPIOB >> i)&0x01)<<(i-12));
    }
    for(int i = 9 ;i<13;i++)
    {
        res |= (((out_GPIOA >> i)&0x01)<<(i-5));
    }
    return res;

}
