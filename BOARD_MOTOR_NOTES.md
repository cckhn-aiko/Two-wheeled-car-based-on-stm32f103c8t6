> 历史单电机说明。当前双电机闭环及串口协议请阅读 DUAL_MOTOR_GUIDE.md。

# 固定 PCB：右电机调试

当前工程继续使用 STM32 HAL，仅对照标准库参考工程适配硬件，不混入标准库。

| 功能 | 配置 |
|---|---|
| 右电机 PWM | PA1 / TIM2_CH2，PSC=35，ARR=99，72MHz 下 20kHz |
| 右电机方向 | PB14 / PB15，正转低/高，反转高/低 |
| 右编码器 | PB6 / PB7，TIM4 编码器 TI12，16 位计数，上拉输入 |
| 编码器极性/滤波 | CH1 Rising，CH2 Falling；两路滤波均为 15，与参考工程一致 |
| PC 串口 | USART1，PA9 TX / PA10 RX，115200，8N1，无流控 |
| 控制定时器预留 | TIM1，PSC=71，ARR=999，1ms 时基；当前不启动 |

## 当前行为

上电 speed=0。串口发送文本 `+` 加 10，`-` 减 10，限幅 -100..100。
`speed` 是有符号占空比指令，不是转速测量值。零时 PWM=0。
主循环使用阻塞串口接收；PWM 和编码器由硬件独立运行。
没有加入 PID 闭环、PID 参数、转速换算或周期测速业务代码。
没有实现左电机接口；PA0、PB12、PB13 仅作为低电平 GPIO，保持未使用侧停止。
参考工程没有软件 STBY 控制，已移除旧 PB14/STBY 逻辑，PB14 专用于右方向。

## 后续测速/PID 接口

- `Motor_SetSpeed(int16_t speed)`：右电机，-100..100。
- `Motor_Stop()`：右电机停止。
- `Encoder_GetCount()`：读取右编码器原始 16 位计数，不清零。
- `Encoder_GetDelta(uint16_t *previous)`：返回相对上次计数的有符号增量，并更新调用者变量。

使用计数增量前，在调用者中初始化 `uint16_t previous = Encoder_GetCount();`。
之后按选定采样周期调用 `Encoder_GetDelta(&previous)`。
每个采样间隔的实际增量须在 -32768..32767 内。不要由多个任务共用同一 previous。
根据实际编码器每圈计数、减速比、采样周期换算转速；这些参数未写死在驱动中。

TIM1 的 1ms 只是预留时基，不代表 PID 必须每 1ms 计算。
需要启用时，先清更新标志，再启动：

```c
__HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
HAL_TIM_Base_Start_IT(&htim1);
```

在 `HAL_TIM_PeriodElapsedCallback()` 中判断 `htim->Instance == TIM1` 后安排采样/控制；
不要在中断中使用阻塞串口或 OLED 刷屏。
如果把 PID 放在主循环，需改成有限超时或非阻塞串口接收，避免当前无限等待阻塞 PID。

## 配置与验证

`motor_pid.ioc` 已同步硬件映射。改变 PWM 周期后，驱动仍按 ARR 换算百分比。
TIM1 / TIM2 / TIM4 配置在 `Core/Src/tim.c`；方向引脚定义在 `Core/Inc/main.h`。
业务初始化与循环位于 main.c 的 USER CODE 区域，后续 CubeMX 重新生成时可保留。
尚未烧录或实测。上板先核对正转时编码器增量符号，再接入速度负反馈闭环。
