#ifndef MOTOR_COMMAND_H
#define MOTOR_COMMAND_H
#include <stdint.h>
#include "speed_control.h"
/* 目标速度：1000a左、1000b右、1000两台；负数反向。
 * 范围±30000计数/秒是输入限制，不代表硬件最高速度。 */
uint8_t MotorCommand_Parse(const char *line, uint8_t *motor, int16_t *target);
/* main调用：接收完整行并设置持续目标，不直接覆盖PWM。 */
void MotorCommand_Process(DualSpeedControl_TypeDef *control);
#endif
