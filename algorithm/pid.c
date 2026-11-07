#include "pid.h"


/*如果实际速度接近目标：
- P 项通常变小。
- I 项保留维持转速所需的输出。
-   如果启用了 D 项，它会根据误差的变化作出响应。
    即使误差变成零，PWM 也不一定变成零。 
    因为积分项可以保留克服摩擦、负载所需的输出。
*/
//PID 的目标是稳定速度，不是强制 PWM 每次只变化一点。

/*
        P 太大，PWM 可能大幅变化。
        I 太大，可能超调、来回振荡。
        D 会放大编码器测速中的跳动，参数不合适时反而使 PWM 更抖。
        改变目标速度时，当前“对误差求微分”的 D 项也可能出现瞬间较大输出。
*/

/*PID 调整 PWM 的方式，就是周期性地给 Motor_SetSpeed() 一个新的数值（PWM 值），进而更新 CCR2。*/
float PID_Update(PID_TypeDef *pid, float target, float actual, float dt)
{
    float error;/* 误差 = 目标 - 实际 */
    float output;/* PID 输出 = P + I + D */

    if (pid == 0 || dt <= 0.0f) return 0.0f;

    error = target - actual;/* 计算误差，单位为编码器计数/秒。 */

    /* Integral contribution, bounded to avoid unlimited accumulation. */
    pid->integral_output += pid->ki * error * dt;//积分项
    if (pid->integral_output > 100.0f) pid->integral_output = 100.0f;
    if (pid->integral_output < -100.0f) pid->integral_output = -100.0f;

    output = pid->kp * error
           + pid->integral_output
           + pid->kd * (error - pid->previous_error) / dt;/* PID 输出 = P + I + D */
    pid->previous_error = error;/* 保存当前误差，以便下次计算微分项。 */

    if (output > 100.0f) output = 100.0f;
    if (output < -100.0f) output = -100.0f;
    return output;
}
