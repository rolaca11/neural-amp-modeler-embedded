/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "fir_filter.h"
#include "uart_io.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FIR_NUM_TAPS      5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static osThreadId_t buttonTaskHandle;
static osThreadId_t firTaskHandle;

extern UART_HandleTypeDef hlpuart1;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void ButtonTask(void *argument);
static void FirUartTask(void *argument);
/* USER CODE END FunctionPrototypes */

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

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  const osThreadAttr_t buttonTask_attributes = {
    .name = "buttonTask",
    .priority = (osPriority_t) osPriorityAboveNormal,
    .stack_size = 1024 * 4
  };
  buttonTaskHandle = osThreadNew(ButtonTask, NULL, &buttonTask_attributes);

  const osThreadAttr_t firTask_attributes = {
    .name = "firTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 2048 * 4
  };
  firTaskHandle = osThreadNew(FirUartTask, NULL, &firTask_attributes);
  printf("[NS] RTOS tasks created (defaultTask, buttonTask, firTask)\r\n");
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END defaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void ButtonTask(void *argument)
{
  (void)argument;
  printf("[NS] ButtonTask running\r\n");
  for (;;)
  {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
    HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    printf("[NS] Button pressed, LED toggled\r\n");
  }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BUTTON_Pin)
  {
    osThreadFlagsSet(buttonTaskHandle, 0x01);
  }
}

static void FirUartTask(void *argument)
{
  (void)argument;

  static const float32_t firCoeffs[FIR_NUM_TAPS] = {
    0.2f, 0.2f, 0.2f, 0.2f, 0.2f
  };

  static FIR_HandleTypeDef hfir;
  static float32_t inBuf[UART_IO_MAX_SAMPLES];
  static float32_t outBuf[UART_IO_MAX_SAMPLES];

  if (FIR_Init(&hfir, FIR_NUM_TAPS, firCoeffs, UART_IO_MAX_SAMPLES) != FIR_OK)
  {
    printf("[FIR] ERROR: Filter init failed\r\n");
    return;
  }

  UartIo_Init(&hlpuart1);
  printf("[FIR] Ready, send comma-separated numbers over LPUART1\r\n");

  for (;;)
  {
    uint16_t count = UartIo_Receive(inBuf, UART_IO_MAX_SAMPLES);
    if (count == 0)
      continue;

    if (FIR_Process(&hfir, inBuf, outBuf, count) != FIR_OK)
    {
      printf("[FIR] ERROR: Process failed\r\n");
      continue;
    }

    UartIo_SendSamples("[FIR] In ", inBuf, count);
    UartIo_SendSamples("[FIR] Out", outBuf, count);
  }
}
/* USER CODE END Application */

