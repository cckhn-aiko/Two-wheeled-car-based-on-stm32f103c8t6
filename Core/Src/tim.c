#include "tim.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

TIM_HandleTypeDef htim3; /* 左编码器PA6/PA7 */
TIM_HandleTypeDef htim4; /* 右编码器PB6/PB7 */

/* 72 MHz / 72 / 1000 = 1 kHz. Reserved for future control scheduling.
 * Initialization does not start the timer or its interrupts. */
void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef clock = {0};
    TIM_MasterConfigTypeDef master = {0};
    
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 71;//
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;//
    htim1.Init.Period = 999;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;//
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) Error_Handler();
    clock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &clock) != HAL_OK) Error_Handler();
    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &master) != HAL_OK) Error_Handler();
}

/* 双电机PWM：左PA0/TIM2_CH1，右PA1/TIM2_CH2。共享20kHz，CCR独立。 */
void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef clock = {0};
    TIM_MasterConfigTypeDef master = {0};
    TIM_OC_InitTypeDef pwm = {0};

    htim2.Instance = TIM2;//
    htim2.Init.Prescaler = 35;//
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 99;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();//
    clock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &clock) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) Error_Handler();
    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &master) != HAL_OK) Error_Handler();


    pwm.OCMode = TIM_OCMODE_PWM1;//
    pwm.Pulse = 0;
    pwm.OCPolarity = TIM_OCPOLARITY_HIGH;//
    pwm.OCFastMode = TIM_OCFAST_DISABLE;
    /* 两路都从Pulse=0开始；PSC=35、ARR=99，72MHz下频率20kHz。 */
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &pwm, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &pwm, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
    HAL_TIM_MspPostInit(&htim2);

    
}

/* 左编码器：TIM3 TI12，A/B均不反相，与标准库参考一致。 */
void MX_TIM3_Init(void)
{
    TIM_Encoder_InitTypeDef encoder = {0};
    TIM_MasterConfigTypeDef master = {0};
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 65535;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    encoder.EncoderMode = TIM_ENCODERMODE_TI12;

    encoder.IC1Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC1Prescaler = TIM_ICPSC_DIV1;
    encoder.IC1Filter = 15;

    encoder.IC2Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC2Prescaler = TIM_ICPSC_DIV1;
    encoder.IC2Filter = 15;
    if (HAL_TIM_Encoder_Init(&htim3, &encoder) != HAL_OK) Error_Handler();
    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &master) != HAL_OK) Error_Handler();
}

void MX_TIM4_Init(void)
{
    TIM_Encoder_InitTypeDef encoder = {0};
    TIM_MasterConfigTypeDef master = {0};
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 65535;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    encoder.EncoderMode = TIM_ENCODERMODE_TI12;

    encoder.IC1Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC1Prescaler = TIM_ICPSC_DIV1;
    encoder.IC1Filter = 15;

    encoder.IC2Polarity = TIM_ICPOLARITY_FALLING;
    encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC2Prescaler = TIM_ICPSC_DIV1;
    encoder.IC2Filter = 15;
    if (HAL_TIM_Encoder_Init(&htim4, &encoder) != HAL_OK) Error_Handler();
    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &master) != HAL_OK) Error_Handler();
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *timer)
{
    if (timer->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM1_UP_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
    }
    else if (timer->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timer)
{
    GPIO_InitTypeDef gpio = {0};
    if (timer->Instance == TIM2)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_AFIO_REMAP_TIM2_DISABLE();
        gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *timer)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    if (timer->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_AFIO_REMAP_TIM3_DISABLE();
        HAL_GPIO_Init(GPIOA, &gpio);
    }
    else if (timer->Instance == TIM4)
    {
        __HAL_RCC_TIM4_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_AFIO_REMAP_TIM4_DISABLE();
        HAL_GPIO_Init(GPIOB, &gpio);
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *timer)
{
    if (timer->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
    }
    else if (timer->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);
    }
}

void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef *timer)
{
    if (timer->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6 | GPIO_PIN_7);
    }
    else if (timer->Instance == TIM4)
    {
        __HAL_RCC_TIM4_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
    }
}
