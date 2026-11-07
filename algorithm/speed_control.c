#include "speed_control.h"
#include "motor.h"

void SpeedControl_InitMotor(SpeedControl_TypeDef *control, uint8_t motor)
{
    if (control == 0) return;
    Encoder_SpeedInitMotor(&control->encoder, motor);
    control->pid.kp = SPEED_CONTROL_KP;//比例增益
    control->pid.ki = SPEED_CONTROL_KI;//积分增益
    control->pid.kd = SPEED_CONTROL_KD;//微分增益
    control->pid.integral_output = 0.0f;//积分项
    control->pid.previous_error = 0.0f;//上一次误差
    control->pending_pwm = 0;
    control->pwm_pending = 0;       
    control->target_speed = 0.0f;//目标速度
    control->pwm = 40;//PWM输出
    Motor_SetPWM(control->encoder.motor, control->pwm);//初始化时停止电机
}

void SpeedControl_SetTarget(SpeedControl_TypeDef *control, float counts_per_second)/* 设置目标速度，单位为编码器计数/秒。 */
{
    if (control == 0) return;/* 如果传入的指针为空，则直接返回，不进行任何操作。 */

    control->target_speed = counts_per_second;//设置目标速度
}

uint8_t SpeedControl_Update(SpeedControl_TypeDef *control)/* 主循环持续调用：有新反馈时计算PID并更新PWM，返回1。 */
{
    uint32_t previous_tick, elapsed;
    float output;

    uint8_t override_this_cycle;/* 本次循环是否覆盖PID输出，使用电脑发来的PWM指令。 */
    int16_t requested_pwm;/* 电脑发来的待执行PWM指令。*/

    if (control == 0) return 0;

    previous_tick = control->encoder.previous_tick;/* 保存上一次的时间戳，以便计算实际采样间隔。 */
    if (!Encoder_SpeedUpdate(&control->encoder, SPEED_CONTROL_PERIOD_MS)) return 0;/* 如果没有新的速度反馈，则返回0，表示没有更新。 */

    /* 本轮开始：取走之前提交的指令 */
    override_this_cycle = control->pwm_pending;
    requested_pwm = control->pending_pwm;
    control->pwm_pending = 0;

    /* PID与测速使用同一次采样的实际时间间隔，处理毫秒计数回绕。 */
    elapsed = (uint32_t)(control->encoder.previous_tick - previous_tick);

    // PID 控制器计算输出，counts_per_second传入实际速度
    output = PID_Update(&control->pid,//计算PID输出
                        control->target_speed,/* 目标速度，单位为编码器计数/秒。 */
                        (float)control->encoder.counts_per_second,/* 实际速度，单位为编码器计数/秒。 */
                        (float)elapsed / 1000.0f);/* 将毫秒转换为秒 */



    /* 有电脑指令时，只替代本轮最终输出 */
    if (override_this_cycle)
    {
        control->pwm = requested_pwm;
    }
    else
    {
        control->pwm = (int16_t)output;
    }

    Motor_SetPWM(control->encoder.motor, control->pwm);//将计算得到的PWM值传递给电机控制函数，以调整电机的速度。
    return 1;
}

void SpeedControl_RequestPWM(SpeedControl_TypeDef *control,int16_t pwm)/* 电脑发来的待执行PWM请求，限制在-100到100之间。 */
{
    if (control == 0)
    {
        return;
    }

    if (pwm > 100)  pwm = 100;
    if (pwm < -100) pwm = -100;

    control->pending_pwm = pwm;
    control->pwm_pending = 1;
}

void SpeedControl_Init(SpeedControl_TypeDef *control)
{
    SpeedControl_InitMotor(control, MOTOR_RIGHT);
}

void DualSpeedControl_Init(DualSpeedControl_TypeDef *control)
{
    if (control == 0) return;
    SpeedControl_InitMotor(&control->left, MOTOR_LEFT);
    SpeedControl_InitMotor(&control->right, MOTOR_RIGHT);
}

uint8_t DualSpeedControl_Update(DualSpeedControl_TypeDef *control)
{
    uint8_t updated = 0;
    if (control == 0) return 0;
    /* 左右独立执行一轮；每次只消耗对应电机自己的一次PWM请求。 */
    if (SpeedControl_Update(&control->left)) updated |= 1U;
    if (SpeedControl_Update(&control->right)) updated |= 2U;
    return updated;
}
