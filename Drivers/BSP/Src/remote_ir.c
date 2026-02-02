#include "remote_ir.h"
#include "tim.h"
#include <string.h>
uint16_t ir_buff[1024] = {0};
uint16_t ir_count = 0;
uint8_t ir_flag = 0;
#define STARTFLAG1 0x00
#define STARTFLAG2 0xFF
#include "bsp_ultraSound.h"
volatile uint8_t IR_code = 0xFF;

uint8_t IR_key[RCKeyNum] ="123456789*0#+LOR-";
//按键及其对应的波形  (引导码) 地址码 地址码取反  数据 数据取反
uint8_t RemoteControl_Table[RCKeyNum][RCIRLength] = {
	{0x00,0xFF,0x45,0xBA},
	{0x00,0xFF,0x46,0xB9},
	{0x00,0xFF,0x47,0xB8},	
	{0x00,0xFF,0x44,0xBB},	
	{0x00,0xFF,0x40,0xBF},	
	{0x00,0xFF,0x43,0xBC},	
	{0x00,0xFF,0x07,0xF8},	
	{0x00,0xFF,0x15,0xEA},	
	{0x00,0xFF,0x09,0xF6},	
	{0x00,0xFF,0x16,0xE9},	
	{0x00,0xFF,0x19,0xE6},	
	{0x00,0xFF,0x0D,0xF2},		
	{0x00,0xFF,0x18,0xE7},		
	{0x00,0xFF,0x08,0xF7},	
	{0x00,0xFF,0x1C,0xE3},	
	{0x00,0xFF,0x5A,0xA5},		
	{0x00,0xFF,0x52,0xAD},		
};

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4)
  {
       msCount++;
  }
    
  if (htim->Instance == TIM3)
  {
      ir_flag = 1;            //标志为1表示开始新一轮接收开始
      Ir_RecvAnalysis();  
  }
  
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        if(ir_flag == 0)
        {
            if (ir_count < 1024)
            {
                ir_buff[ir_count++] = IR_TIM->CCR2;    //获取该次标志持续时长 
            }
            else{
                ir_count = 0;
            
            }
            __HAL_TIM_SET_COUNTER(&htim3,0);
            IR_TIM->CCER ^= (1<<5);             //通道二 极性翻转
        
        }
    
    }

}
//逻辑“0”是由0.56ms的38KHZ载波和0.560ms的无载波间隔组成；
//逻辑“1”是由0.56ms的38KHZ载波和1.68ms的无载波间隔组成；
//结束位是0.56ms的38K载波
//判断时长是否异常有误差，是否在合理范围内
uint8_t Time_Range(uint16_t time1, uint16_t time2, uint16_t range1, uint16_t range2)
{
	if((time1 > (time2-range1)) && (time1 < (time2+range2)))
		return 1;
	else 
		return 0;
}

//获取遥控按键数值
//正确 返回按键对应的行号， -1 起始地址错误，不是指定遥控器  -2地址正确 没有这个按键 
int Check_RemoteKey(uint8_t *buff)
{
    uint8_t i = 0;
    int res;
    if ((buff[0] != STARTFLAG1) || (buff[1] != STARTFLAG2))
    {
        res = -1;
    }
    for (i = 0; i < RCKeyNum; i++)
    {
        if((buff[2]==RemoteControl_Table[i][2]) && (buff[3]==RemoteControl_Table[i][3]))
        {
            res = i;
            break;
        }
    
    }
    if(i >= RCKeyNum)
    {
        res = -2;      
    }
    return res;
    
}

IR_DATA_TypeDef ir = {0};
//uint8_t IR_code = 0XFF;

//返回值 
//0 解析成功 1 没有接收完成 2 引导码错误 3 前半段数据错误 4 后半段数据错误
uint8_t Ir_RecvAnalysis(void)
{
    uint16_t i = 0;
    uint8_t err;
    if (ir_flag == 0)
    {
        return 1;
    }
    //引导码间隔是9 ms + 4.5 ms
    if (Time_Range(ir_buff[1], 9000, 1000, 1000) == 0)
    {
        err = 2;
        goto error;
    }
    if (Time_Range(ir_buff[2],4500, 500, 500) == 0)
    {
        err = 2;
        goto error;
    }
    for(i=3; i<ir_count-1; i++)
    {
       
        if(Time_Range(ir_buff[i], 560, 200, 200))
        {
            i++;
            if(Time_Range(ir_buff[i], 560, 200, 200))
            {
                ir.Ir_Data[ir.Ir_Length/8] &= ~(1<<ir.Ir_Length%8);
                ir.Ir_Length++; 
            }
            else if(Time_Range(ir_buff[i], 1690, 200, 200))
            {
                ir.Ir_Data[ir.Ir_Length/8] |= (1<<ir.Ir_Length%8);
                ir.Ir_Length++;       
            }
            else
            {
                err = 4;
                goto error;
            }
            
        }
        else
        {
            err = 3;
            goto error;
        }
 
    }
    if(Check_RemoteKey(ir.Ir_Data) < 17 && Check_RemoteKey(ir.Ir_Data)>-1)
    {
        IR_code = IR_key[Check_RemoteKey(ir.Ir_Data)];
        memset(ir_buff, 0, sizeof(ir_buff));
        memset(ir.Ir_Data, 0, sizeof(ir.Ir_Data));
        ir.Ir_Length = 0;
        ir_count  = 0;
        ir_flag = 0;
        return 0;
    }
error:
	memset(ir_buff,0,sizeof(ir_buff));
	memset(ir.Ir_Data,0,sizeof(ir.Ir_Data));
	ir.Ir_Length = 0;
	ir_count = 0;
	ir_flag = 0;
	HAL_TIM_Base_Start_IT(&htim3);
	if(err)	return err;
	else	return 0;
}

