/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor_command.h"
#include "OLED.h"



/* USER CODE END Includes */
#include "motor.h"
#include "encoder.h"
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"



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

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  DualSpeedControl_TypeDef motors; /* 左右各有独立测速和PID状态。 */
  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();          /* CH1左PA0，CH2右PA1；两路20kHz。 */
  MX_USART2_UART_Init();   /* 蓝牙串口PA2/PA3，115200、8N1。 */
  MX_TIM3_Init();          /* 左编码器：PA6/PA7，TIM3。 */
  MX_TIM4_Init();          /* 右编码器：PB6/PB7，TIM4。 */

  /* USER CODE BEGIN 2 */
  Motor_Init();           /* 两台电机初始PWM都为0，然后启动两路PWM硬件。 */
  OLED_MotorSpeedInit(); /* 在零输出时完成OLED上电初始化。 */
  Encoder_Init();
  DualSpeedControl_Init(&motors); /* 初始目标、PWM、积分、待执行请求全部为0。 */

  /* 初始目标0；串口设置持续目标速度，单位计数/秒。 */
  SpeedControl_SetTarget(&motors.left, 0.0f);
  SpeedControl_SetTarget(&motors.right, 0.0f);
  USART_StartReceive();
  /* USER CODE END 2 */
  char *text = "aiko";
  HAL_UART_Transmit(&huart2, text, 4, 100);
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
    /* 1. 串口目标速度：a左、b右、无后缀两台。 */
    MotorCommand_Process(&motors);

    /* 2. 两台独立测速和PID计算，持续更新PWM。 */
    if (DualSpeedControl_Update(&motors))
    {
      /* 3. 六列数据回传，usart内部限制发送频率。 */
      USART_SendDualPlot((int32_t)motors.left.target_speed,
                         motors.left.encoder.counts_per_second, motors.left.pwm,
                         (int32_t)motors.right.target_speed,
                         motors.right.encoder.counts_per_second, motors.right.pwm);
    }
    /* 4. OLED显示左右目标和实际速度，模块内部定时刷新。 */
    OLED_MotorSpeedUpdate((int32_t)motors.left.target_speed,
                          motors.left.encoder.counts_per_second,
                          (int32_t)motors.right.target_speed,
                          motors.right.encoder.counts_per_second);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  **/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
/**HCLK  = 72 MHz
  APB1  = 36 MHz
  APB2  = 72 MHz */
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */






