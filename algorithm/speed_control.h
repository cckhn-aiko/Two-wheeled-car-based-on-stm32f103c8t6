#ifndef SPEED_CONTROL_H
#define SPEED_CONTROL_H
#include "encoder.h"
#include "pid.h"

/* 初始调试参数；速度单位统一为编码器计数/秒。 */
#define SPEED_CONTROL_PERIOD_MS 20U//控制周期，单位：毫秒
#define SPEED_CONTROL_KP 0.02f
#define SPEED_CONTROL_KI 0.02f
#define SPEED_CONTROL_KD 0.0f

typedef struct
{
    Encoder_SpeedTypeDef encoder;
    PID_TypeDef pid;
    float target_speed;
    int16_t pwm;
    int16_t pending_pwm;       // 电脑发来的待执行PWM
    uint8_t pwm_pending;      // 1：有一条尚未执行的指令
} SpeedControl_TypeDef;

/* 两台电机分别保存PID积分、上次误差、目标和待执行PWM，互不串扰。 */
typedef struct
{
    SpeedControl_TypeDef left;
    SpeedControl_TypeDef right;
} DualSpeedControl_TypeDef;

void SpeedControl_InitMotor(SpeedControl_TypeDef *control, uint8_t motor);
void DualSpeedControl_Init(DualSpeedControl_TypeDef *control);
/* 返回位标志：bit0左已更新，bit1右已更新；0代表尚未到采样周期。 */
uint8_t DualSpeedControl_Update(DualSpeedControl_TypeDef *control);

/* 兼容旧接口：不带motor参数时，只初始化右电机。 */
void SpeedControl_Init(SpeedControl_TypeDef *control);
void SpeedControl_SetTarget(SpeedControl_TypeDef *control, float counts_per_second);
/* 主循环持续调用：有新反馈时计算PID并更新PWM，返回1。 */
uint8_t SpeedControl_Update(SpeedControl_TypeDef *control);
void SpeedControl_RequestPWM(SpeedControl_TypeDef *control,int16_t pwm);
#endif
