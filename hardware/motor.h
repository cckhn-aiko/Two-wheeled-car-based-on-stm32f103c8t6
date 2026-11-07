#ifndef MOTOR_H
#define MOTOR_H
#include <stdint.h>

/* 与标准库平衡车工程一致：左电机=1，右电机=2。 */
#define MOTOR_LEFT  1U
#define MOTOR_RIGHT 2U
#define MOTOR_BOTH  3U

/* 在MX_GPIO_Init、MX_TIM2_Init之后调用，两台电机均从0%启动。 */
void Motor_Init(void);
/* pwm为有符号占空比百分数：-100~100；0停止，负数反转。
 * MOTOR_LEFT/RIGHT只修改指定电机；MOTOR_BOTH同时修改两台。 */
void Motor_SetPWM(uint8_t motor, int16_t pwm);

/* 兼容旧的单右电机代码，保留给独立speed_control模块使用。 */
void Motor_SetSpeed(int16_t speed);
void Motor_Stop(void);
#endif
