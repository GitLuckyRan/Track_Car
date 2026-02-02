#ifndef REMOTE_IR_H
#define REMOTE_IR_H
#include "stm32f1xx_hal.h"

#define IR_TIM TIM3
#define MAX_DATALEN 5

#define RCKeyNum 17
#define RCIRLength 	4
typedef struct{
    uint8_t mode;
    uint8_t Ir_Data[MAX_DATALEN];
    uint16_t Ir_Length;
}IR_DATA_TypeDef;
extern volatile uint8_t IR_code;

//extern uint8_t IR_key[RCKeyNum];
////按键及其对应的波形  (引导码) 地址码 地址码取反  数据 数据取反
//extern uint8_t RemoteControl_Table[RCKeyNum][RCIRLength];

uint8_t Ir_RecvAnalysis(void);
#endif
