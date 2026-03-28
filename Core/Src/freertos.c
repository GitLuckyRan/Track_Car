/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_redCheck.h"
#include "task_tracking.h"
#include "remote_ir.h"
#include "bsp_motor.h"
#include "power_model.h"
#include "iwdg.h"
#include "bsp_ultraSound.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */  

TaskHandle_t trackTaskHandle;
TaskHandle_t remoteTaskHandle;
TaskHandle_t aVoidTaskHandle;
QueueHandle_t xLengthQueue;
uint8_t TrackMode = 0;
uint8_t RemoteMode = 0;
uint8_t AvoidMode = 0;
uint8_t SavePowerMode = 0;
/* USER CODE END Variables */
/* Definitions for defaultTask */
//osThreadId_t defaultTaskHandle;
//const osThreadAttr_t defaultTask_attributes = {
//  .name = "defaultTask",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void DeleteTask(TaskHandle_t* xTaskToDelete)
{
     if (xTaskToDelete != NULL && *xTaskToDelete != NULL )
     {
         vTaskDelete(*xTaskToDelete);
         *xTaskToDelete = NULL;
     }      
    
}


void vAvoidMode(void *argument)
{
    uint32_t tick = osKernelGetTickCount(); // 获取系统内部时间戳
    if (xLengthQueue == NULL) xLengthQueue = xQueueCreate(1, sizeof(float)); //第一次调用，创建队列，接收距离值
    if (xLengthQueue == NULL) vTaskDelay(NULL);
    float Length = 100.0;
    for(;;)
    {
       tick += 50;
       Length = GetLength();
       xQueueOverwrite(xLengthQueue, &Length);
       osDelayUntil(tick);   
    }
}


void vTrackMode(void *argument)
{
    uint32_t tick = osKernelGetTickCount(); // 获取系统内部时间戳
    for(;;)
    { 
        tick += 50;
        if (AvoidMode)
        {
           if (xQueueReceive(xLengthQueue, &Length,0) == pdPASS)
           {
               if (Length < 10.0f)
               {
                   Car_Stop();
                   continue;
               }
           }
        }
        Task_Run();
        osDelayUntil(tick);
    }
    
}


void vRemoteMode(void *argument)
{
    uint32_t tick = osKernelGetTickCount(); // 获取系统内部时间戳
    float Length = 100.0f;
    for(;;)
    {
        tick += 50;
        if (AvoidMode)
        {
           if (xQueueReceive(xLengthQueue, &Length,0) == pdPASS)
           {
               if (Length < 10.0f)
               {
                   Car_Stop();
                   continue;
               }
           }
        }
        if (IR_code == 'L')       //  左转
        {
            Car_SetSpeed(0,900);
        }
        else if(IR_code == 'R')   //  右转
        {
            Car_SetSpeed(900,0);
        }
        else if(IR_code == '+')    // 前进
        {
            Car_Forward(800);
        }
        else if(IR_code == '-')  // 后退
        {
            Car_Back(800);
        }
        else if(IR_code == '8')    // 自转
        {
            Car_SetSpeed(900,-900);
        }
        else if(IR_code == '7')    // 自转
        {
            Car_SetSpeed(-900,900);
        }
        else                      //其他按键异常，就停止
        {
//            Car_Stop();            
        }
        osDelayUntil(tick);
    }
    
}



void vModeSelect(void *argument)
{
    uint32_t tick = osKernelGetTickCount(); // 获取系统内部时间戳
    for(;;)
    {
        tick += 20;  
        //循迹模式--*  遥控模式--#   避障模式--0  省电模式--O
        if(IR_code == '*')
        {
            IR_code = 0xFF;
            TrackMode = TrackMode ^ 1;
            RemoteMode = 0;
            if (TrackMode)
            {
                DeleteTask(&remoteTaskHandle);
                if (trackTaskHandle == NULL)
                {
                    xTaskCreate(vTrackMode, "vTrackMode", 400, NULL, 5, &trackTaskHandle);
                }
            }
            else
            {
                DeleteTask(&trackTaskHandle);
                Car_Stop();   
            }
        }
        else if (IR_code == '#')
        {
            IR_code = 0xFF;
            RemoteMode = RemoteMode ^ 1;
            TrackMode = 0;
            if (RemoteMode)
            {
                DeleteTask(&trackTaskHandle);
                if (remoteTaskHandle == NULL)
                {
                    xTaskCreate(vRemoteMode, "vRemoteMode", 100, NULL, 5, &remoteTaskHandle);
                }
            }
            else
            {
                DeleteTask(&remoteTaskHandle);
                Car_Stop();   
            }
        }
        else if(IR_code == '0')
        {
            IR_code = 0xFF;
            AvoidMode = AvoidMode ^ 1;
            if (AvoidMode)
            {
                if (aVoidTaskHandle == NULL)
                {
                    xTaskCreate(vAvoidMode, "vAvoidMode", 100, NULL, 5, &aVoidTaskHandle);
                }
            }
            else
            {
                DeleteTask(&aVoidTaskHandle);
                Car_Stop();   
            }
        }
        else if(IR_code == 'O')
        {
            SavePowerMode = SavePowerMode ^ 1;
//            vAvoidMode
        }
        else
        {
            //其他按键不处理
        }
        osDelayUntil(tick);
    }

}

void vIWDG(void *argument)
{
     uint32_t tick = osKernelGetTickCount(); // 获取系统内部时间戳
    for(;;)
    {
        tick += 50;
        HAL_IWDG_Refresh(&hiwdg);
        osDelayUntil(tick);
    }

}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
//  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
//  xTaskCreate(vGetLineValue,"GetLine",1000,NULL,3,NULL);
  xTaskCreate(vModeSelect, "vModeSelect", 400, NULL, 4, NULL);
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

