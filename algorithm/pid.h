#ifndef PID_H
#define PID_H

/* Target and actual must use the same units (here: encoder counts/second).
 * Integral and derivative include dt in seconds. */
typedef struct
{
    float kp;
    float ki;
    float kd;
    float integral_output;
    float previous_error;
} PID_TypeDef;

/* Zero-initialize PID state before first use. Output: signed duty -100..100. */
float PID_Update(PID_TypeDef *pid, float target, float actual, float dt);

#endif
