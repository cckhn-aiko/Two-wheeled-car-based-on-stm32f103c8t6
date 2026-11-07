#include "motor_command.h"
#include "motor.h"
#include "usart.h"

uint8_t MotorCommand_Parse(const char *line, uint8_t *motor, int16_t *target)
{
    uint32_t value = 0;
    int16_t sign = 1;
    uint8_t selected = MOTOR_BOTH;

    if (line == 0 || motor == 0 || target == 0) return 0;
    /* 允许正负号，例如-30a；这里不是把ASCII字符直接当数字。 */
    if (*line == '-' || *line == '+')
    {
        if (*line == '-') sign = -1;
        line++;
    }
    if (*line < '0' || *line > '9') return 0;

    while (*line >= '0' && *line <= '9')
    {
        value = value * 10U + (uint32_t)(*line - '0');
        if (value > 30000U) return 0; /* 立即拒绝越界值，防止累计溢出。 */
        line++;
    }

    /* 无后缀控制两台；a左、b右。拒绝多余字符，例如50ab、50x。 */
    if (*line == 'a')
    {
        selected = MOTOR_LEFT;
        line++;
    }
    else if (*line == 'b')
    {
        selected = MOTOR_RIGHT;
        line++;
    }
    if (*line != '\0') return 0;

    *motor = selected;
    *target = (int16_t)(sign * (int16_t)value);
    return 1;
}

/* 目标持续保持到下一条指令；计数/秒，不是PWM百分比。 */
void MotorCommand_Process(DualSpeedControl_TypeDef *control)
{
    char line[USART_RX_LINE_SIZE];
    uint8_t motor;
    int16_t target;
    if (control == 0 || !USART_ReadLine(line)) return;
    if (!MotorCommand_Parse(line, &motor, &target)) return;

    /* 主循环修改目标，下一轮PID根据新目标和实际速度调整PWM。 */
    if (motor == MOTOR_LEFT || motor == MOTOR_BOTH)
        SpeedControl_SetTarget(&control->left, (float)target);
    if (motor == MOTOR_RIGHT || motor == MOTOR_BOTH)
        SpeedControl_SetTarget(&control->right, (float)target);
}
