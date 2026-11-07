#include "encoder.h"
#include "tim.h"

void Encoder_Init(void)
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    if (HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL) != HAL_OK) Error_Handler();
    if (HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL) != HAL_OK) Error_Handler();
}

uint16_t Encoder_GetCountMotor(uint8_t motor)
{
    if (motor == MOTOR_LEFT) return (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
    if (motor == MOTOR_RIGHT) return (uint16_t)__HAL_TIM_GET_COUNTER(&htim4);
    return 0;
}

void Encoder_SpeedInitMotor(Encoder_SpeedTypeDef *state, uint8_t motor)
{
    if (state == 0) return;
    state->motor = motor;
    state->previous_count = Encoder_GetCountMotor(motor);
    state->previous_tick = HAL_GetTick();
    state->counts_per_second = 0;
}

uint8_t Encoder_SpeedUpdate(Encoder_SpeedTypeDef *state, uint32_t period_ms)
{
    uint32_t now, elapsed;
    uint16_t current, difference;
    int32_t delta;
    if (state == 0 || period_ms == 0U) return 0;
    if (state->motor != MOTOR_LEFT && state->motor != MOTOR_RIGHT) return 0;
    now = HAL_GetTick();
    elapsed = (uint32_t)(now - state->previous_tick);
    if (elapsed < period_ms) return 0;

    current = Encoder_GetCountMotor(state->motor);
    difference = (uint16_t)(current - state->previous_count);
    delta = difference;
    if (delta >= 32768) delta -= 65536; /* 还原跨过0/65535时的有符号增量。 */
    state->previous_count = current;
    state->previous_tick = now;
    state->counts_per_second = (int32_t)((int64_t)delta * 1000 / elapsed);
    return 1;
}

uint16_t Encoder_GetCount(void)
{
    return Encoder_GetCountMotor(MOTOR_RIGHT);
}

int16_t Encoder_GetDelta(uint16_t *previous)
{
    uint16_t current, difference;
    int32_t delta;
    if (previous == 0) return 0;
    current = Encoder_GetCount();
    difference = (uint16_t)(current - *previous);
    *previous = current;
    delta = difference;
    if (delta >= 32768) delta -= 65536;
    return (int16_t)delta;
}

void Encoder_SpeedInit(Encoder_SpeedTypeDef *state)
{
    Encoder_SpeedInitMotor(state, MOTOR_RIGHT);
}
