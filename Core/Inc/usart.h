/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
/* 发送两列整数：速度、PWM指令，供SerialPlot绘图 */
HAL_StatusTypeDef USART_SendPlot(int32_t speed, int16_t pwm);
/* 绘图回传间隔。200ms=最多约5行/秒；与PID控制周期无关。 */
#define USART_PLOT_PERIOD_MS 200U

/* 六列CSV：左目标、左实际、左PWM、右目标、右实际、右PWM。
 * 速度单位计数/秒，PWM为-100~100的占空比指令。
 * 未到回传间隔时返回HAL_BUSY（跳过本次发送），不阻塞等待。 */
HAL_StatusTypeDef USART_SendDualPlot(int32_t left_target, int32_t left_speed,
                                   int16_t left_pwm, int32_t right_target,
                                   int32_t right_speed, int16_t right_pwm);

/* 每行含结束符共8字节，可容纳-30000a；发送追加CR或LF。 */
#define USART_RX_LINE_SIZE 8U
HAL_StatusTypeDef USART_SendText(const char *text);

/* 启动串口中断接收 */
void USART_StartReceive(void);

/* 按接收顺序取一行；line至少USART_RX_LINE_SIZE字节。有数据返回1。 */
uint8_t USART_ReadLine(char *line);


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

