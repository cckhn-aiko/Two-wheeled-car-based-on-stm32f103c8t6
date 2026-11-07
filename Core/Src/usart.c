/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"
/* USER CODE BEGIN 0 */
#include <stdio.h>


/* 中断写队列，main读队列：容量7条完整指令，避免50a/30b连发互相覆盖。
 * 队列满时丢弃新行，不执行被截断或只接收一部分的指令。
 * 这些状态只属于串口驱动，在main中不需要接收标志位。 */
#define USART_RX_QUEUE_SIZE 8U
static uint8_t rx_byte;
static char rx_build[USART_RX_LINE_SIZE];
static uint8_t rx_length;
static uint8_t rx_discard;
static volatile char rx_queue[USART_RX_QUEUE_SIZE][USART_RX_LINE_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;
/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_AFIO_REMAP_USART2_DISABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

HAL_StatusTypeDef USART_SendPlot(int32_t speed, int16_t pwm)/* 发送两列整数：速度、PWM指令，供SerialPlot绘图 */
{
    char buffer[48];
    int length;

    /* 两列数字，用逗号分隔，每组数据以换行结束 */
    length = snprintf(buffer,
                      sizeof(buffer),
                      "%ld,%d\r\n",/* 格式化字符串，生成要发送的ASCII数据，包含速度和PWM指令 */
                      (long)speed,/* 格式化字符串，生成要发送的ASCII数据，包含速度和PWM指令 */
                      (int)pwm);/* 格式化字符串，生成要发送的ASCII数据，包含速度和PWM指令 */

    /* 格式化失败或者缓冲区不够时，不发送 */
    if (length <= 0 || length >= (int)sizeof(buffer))//
    {
        return HAL_ERROR;
    }

    return HAL_UART_Transmit(&huart2,
                             (uint8_t *)buffer,
                             (uint16_t)length,
                             100);//通过UART发送数据，超时时间为100毫秒
}

HAL_StatusTypeDef USART_SendText(const char *text)
{
    uint16_t length = 0;
    if (text == 0) return HAL_ERROR;
    while (text[length] != '\0') length++;
    return HAL_UART_Transmit(&huart2, (uint8_t *)text, length, 100);
}

void USART_StartReceive(void)
{
    rx_length = 0;
    rx_discard = 0;
    rx_head = 0;
    rx_tail = 0;
    if (HAL_UART_Receive_IT(&huart2, &rx_byte, 1) != HAL_OK) Error_Handler();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t i, next;
    if (huart->Instance != USART2) return;

    /* CR、LF都可结束指令，CRLF的第二个字符作为空行忽略。 */
    if (rx_byte == '\r' || rx_byte == '\n')
    {
        if (rx_length > 0U && !rx_discard)
        {
            next = (uint8_t)((rx_head + 1U) % USART_RX_QUEUE_SIZE);
            if (next != rx_tail)
            {
                for (i = 0; i < rx_length; i++) rx_queue[rx_head][i] = rx_build[i];
                rx_queue[rx_head][rx_length] = '\0';
                rx_head = next; /* 完整写好后才发布给main。 */
            }
        }
        rx_length = 0;
        rx_discard = 0;
    }
    else if (!rx_discard)
    {
        if (rx_byte < 32U || rx_byte > 126U || rx_length >= USART_RX_LINE_SIZE - 1U)
            rx_discard = 1; /* 二进制控制字符或超长行整行丢弃。 */
        else
            rx_build[rx_length++] = (char)rx_byte;
    }

    /* 回调只收字节，不阻塞发送，也不在中断里操作电机。 */
    if (HAL_UART_Receive_IT(&huart2, &rx_byte, 1) != HAL_OK) Error_Handler();
}

uint8_t USART_ReadLine(char *line)
{
    uint32_t irq_state;
    uint8_t i, ready = 0;
    if (line == 0) return 0;

    /* 复制最多8字节时暂时保护队列，恢复调用前的中断状态。 */
    irq_state = __get_PRIMASK();
    __disable_irq();
    if (rx_tail != rx_head)
    {
        for (i = 0; i < USART_RX_LINE_SIZE; i++)
        {
            line[i] = rx_queue[rx_tail][i];
            if (line[i] == '\0') break;
        }
        rx_tail = (uint8_t)((rx_tail + 1U) % USART_RX_QUEUE_SIZE);
        ready = 1;
    }
    __set_PRIMASK(irq_state);
    return ready;
}

/* 发生溢出等串口错误时丢弃半条指令，重新收下一条，避免一直停止接收。 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2) return;
    HAL_UART_AbortReceive(huart);
    __HAL_UART_CLEAR_OREFLAG(huart);
    rx_length = 0;
    rx_discard = 1; /* 等到换行再恢复，避免把错误后的后半条当成新指令。 */
    if (HAL_UART_Receive_IT(huart, &rx_byte, 1) != HAL_OK) Error_Handler();
}
/* 双电机曲线回传只在main执行，不能放在串口接收中断里阻塞发送。 */
HAL_StatusTypeDef USART_SendDualPlot(int32_t left_target, int32_t left_speed,
                                   int16_t left_pwm, int32_t right_target,
                                   int32_t right_speed, int16_t right_pwm)
{
    static uint32_t last_send_tick = 0; /* 串口模块内部保存回传时间。 */
    uint32_t now;
    char buffer[80];
    int length;

    /* 只控制发送频率，不改变测速/PID周期，也不调用HAL_Delay。 */
    now = HAL_GetTick();
    if ((uint32_t)(now - last_send_tick) < USART_PLOT_PERIOD_MS)
        return HAL_BUSY;
    last_send_tick = now;
    length = snprintf(buffer, sizeof(buffer), "%ld,%ld,%d,%ld,%ld,%d\r\n",
                      (long)left_target, (long)left_speed, (int)left_pwm,
                      (long)right_target, (long)right_speed, (int)right_pwm);
    if (length <= 0 || length >= (int)sizeof(buffer)) return HAL_ERROR;
    return HAL_UART_Transmit(&huart2, (uint8_t *)buffer, (uint16_t)length, 100);
}
/* USER CODE END 1 */

