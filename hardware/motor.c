#include "motor.h"
#include "main.h"
#include "tim.h"

void Motor_SetPWM(uint8_t motor, int16_t pwm)
{
    uint32_t magnitude, compare;
    GPIO_PinState left_in1, right_in1;

    /* 非法电机编号不执行；限幅后再取绝对值，避免负数越界。 */
    if (motor != MOTOR_LEFT && motor != MOTOR_RIGHT && motor != MOTOR_BOTH)
        return;
    if (pwm > 100) pwm = 100;
    if (pwm < -100) pwm = -100;
    magnitude = (uint32_t)((pwm < 0) ? -pwm : pwm);

    /* CCR决定高电平宽度。ARR=99时，pwm=40对应CCR=40，即40%。 */
    compare = magnitude * (__HAL_TIM_GET_AUTORELOAD(&htim2) + 1U) / 100U;

    if (motor == MOTOR_LEFT || motor == MOTOR_BOTH)
    {
        /* 左电机：PA0/TIM2_CH1；正转PB12高、PB13低。 */
        left_in1 = (pwm > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
        HAL_GPIO_WritePin(motor_left_in1_GPIO_Port, motor_left_in1_Pin, left_in1);
        HAL_GPIO_WritePin(motor_left_in2_GPIO_Port, motor_left_in2_Pin,
                          (pwm < 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, compare);
    }

    if (motor == MOTOR_RIGHT || motor == MOTOR_BOTH)
    {
        /* 右电机：PA1/TIM2_CH2；正转PB14低、PB15高。
         * 左右电机安装方向不同，所以相同正指令使用相反的方向电平。 */
        right_in1 = (pwm < 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        HAL_GPIO_WritePin(motor_in1_GPIO_Port, motor_in1_Pin, right_in1);
        HAL_GPIO_WritePin(motor_in2_GPIO_Port, motor_in2_Pin,
                          (pwm > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
    }
}

void Motor_Init(void)
{
    /* 先清两路CCR和方向，再启动PWM，避免上电时电机意外起转。 */
    Motor_SetPWM(MOTOR_BOTH, 0);
    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) != HAL_OK) Error_Handler();
}

/* 旧接口仍然只操作右电机，避免旧单电机PID接口语义改变。 */
void Motor_SetSpeed(int16_t speed)
{
    Motor_SetPWM(MOTOR_RIGHT, speed);
}

void Motor_Stop(void)
{
    Motor_SetPWM(MOTOR_RIGHT, 0);
}
