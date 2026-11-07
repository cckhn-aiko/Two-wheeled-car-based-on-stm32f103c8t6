#ifndef ENCODER_H
#define ENCODER_H
#include <stdint.h>
#include "motor.h"

/* 每台电机有自己的测速状态，不能共享previous_count/previous_tick。 */
typedef struct
{
    uint8_t motor;
    uint16_t previous_count;
    uint32_t previous_tick;
    int32_t counts_per_second;
} Encoder_SpeedTypeDef;

/* 必须先MX_TIM3_Init、MX_TIM4_Init，再启动左右编码器。 */
void Encoder_Init(void);
uint16_t Encoder_GetCountMotor(uint8_t motor);
void Encoder_SpeedInitMotor(Encoder_SpeedTypeDef *state, uint8_t motor);
/* 使用实际时间间隔计算计数/秒；采样位移绝对值须小于32768。 */
uint8_t Encoder_SpeedUpdate(Encoder_SpeedTypeDef *state, uint32_t period_ms);

/* 兼容原单右电机学习代码。 */
uint16_t Encoder_GetCount(void);
int16_t Encoder_GetDelta(uint16_t *previous);
void Encoder_SpeedInit(Encoder_SpeedTypeDef *state);
#endif
