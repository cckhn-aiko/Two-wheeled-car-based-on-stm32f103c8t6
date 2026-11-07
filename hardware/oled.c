#include "main.h"
#include "OLED.h"
#include "OLED_Font.h"
#include <stdio.h>

/*
 * 默认接线：
 * OLED SCL -> PB8
 * OLED SDA -> PB9
 *
 * 软件 I2C，不需要配置硬件 I2C 外设。
 * main.h 应包含当前工程的 HAL 头文件。
 */

#define OLED_SCL_PORT        GPIOB
#define OLED_SCL_PIN         GPIO_PIN_8

#define OLED_SDA_PORT        GPIOB
#define OLED_SDA_PIN         GPIO_PIN_9

/* 包含写方向位的地址，7 位地址为 0x3C */
#define OLED_I2C_WRITE_ADDR  0x78U

#define OLED_W_SCL(x) \
    HAL_GPIO_WritePin(OLED_SCL_PORT, OLED_SCL_PIN, \
                      (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

#define OLED_W_SDA(x) \
    HAL_GPIO_WritePin(OLED_SDA_PORT, OLED_SDA_PIN, \
                      (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

/*
 * 软件 I2C 延时。
 * 按 SystemCoreClock 调整循环次数，并保留循环开销，
 * 使总线速度保持在较保守的范围内。
 */
static void OLED_I2C_Delay(void)
{
    volatile uint32_t Count;

    Count = SystemCoreClock / 200000U + 1U;

    while (Count != 0U)
    {
        __NOP();
        Count--;
    }
}

void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 预置为高电平，开漏输出高电平表示释放总线 */
    OLED_W_SCL(1);
    OLED_W_SDA(1);

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = OLED_SCL_PIN;
    HAL_GPIO_Init(OLED_SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    HAL_GPIO_Init(OLED_SDA_PORT, &GPIO_InitStruct);

    OLED_I2C_Delay();
}

void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_I2C_Delay();

    OLED_W_SCL(1);
    OLED_I2C_Delay();

    OLED_W_SDA(0);
    OLED_I2C_Delay();

    OLED_W_SCL(0);
    OLED_I2C_Delay();
}

void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_I2C_Delay();

    OLED_W_SCL(1);
    OLED_I2C_Delay();

    OLED_W_SDA(1);
    OLED_I2C_Delay();
}

void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;

    for (i = 0; i < 8U; i++)
    {
        OLED_W_SDA((Byte & 0x80U) != 0U);
        OLED_I2C_Delay();

        OLED_W_SCL(1);
        OLED_I2C_Delay();

        OLED_W_SCL(0);
        OLED_I2C_Delay();

        Byte <<= 1;
    }

    /* 第九个时钟：释放 SDA，供 OLED 输出应答 */
    OLED_W_SDA(1);
    OLED_I2C_Delay();

    OLED_W_SCL(1);
    OLED_I2C_Delay();

    OLED_W_SCL(0);
    OLED_I2C_Delay();
}

void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_WRITE_ADDR);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_WRITE_ADDR);
    OLED_I2C_SendByte(0x40);
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

/* Y：页地址 0~7；X：列地址 0~127 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    if (Y > 7U || X > 127U)
    {
        return;
    }

    OLED_WriteCommand((uint8_t)(0xB0U | Y));
    OLED_WriteCommand((uint8_t)(0x10U | (X >> 4)));
    OLED_WriteCommand((uint8_t)(X & 0x0FU));
}

void OLED_Clear(void)
{
    uint8_t Page;
    uint8_t Column;

    for (Page = 0; Page < 8U; Page++)
    {
        OLED_SetCursor(Page, 0);

        OLED_I2C_Start();
        OLED_I2C_SendByte(OLED_I2C_WRITE_ADDR);
        OLED_I2C_SendByte(0x40);

        for (Column = 0; Column < 128U; Column++)
        {
            OLED_I2C_SendByte(0x00);
        }

        OLED_I2C_Stop();
    }
}

/* Line：1~4；Column：1~16；字体：8×16 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    uint8_t Index;

    if (Line < 1U || Line > 4U ||
        Column < 1U || Column > 16U)
    {
        return;
    }

    if ((uint8_t)Char < 32U || (uint8_t)Char > 126U)
    {
        Char = '?';
    }

    Index = (uint8_t)Char - 32U;

    OLED_SetCursor((uint8_t)((Line - 1U) * 2U),
                   (uint8_t)((Column - 1U) * 8U));

    for (i = 0; i < 8U; i++)
    {
        OLED_WriteData(OLED_F8x16[Index][i]);
    }

    OLED_SetCursor((uint8_t)((Line - 1U) * 2U + 1U),
                   (uint8_t)((Column - 1U) * 8U));

    for (i = 0; i < 8U; i++)
    {
        OLED_WriteData(OLED_F8x16[Index][i + 8U]);
    }
}

/* 到达当前行末尾后停止显示 */
void OLED_ShowString(uint8_t Line, uint8_t Column,
                     const char *String)
{
    if (String == 0 ||
        Line < 1U || Line > 4U ||
        Column < 1U || Column > 16U)
    {
        return;
    }

    while (*String != '\0' && Column <= 16U)
    {
        OLED_ShowChar(Line, Column, *String);
        String++;
        Column++;
    }
}

uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1U;

    while (Y != 0U)
    {
        Result *= X;
        Y--;
    }

    return Result;
}

/* 无符号十进制；Length：1~10；不足位数补零 */
void OLED_ShowNum(uint8_t Line, uint8_t Column,
                  uint32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Divisor;

    if (Line < 1U || Line > 4U ||
        Column < 1U || Column > 16U ||
        Length < 1U || Length > 10U)
    {
        return;
    }

    Divisor = OLED_Pow(10U, Length - 1U);

    for (i = 0; i < Length && Column <= 16U; i++)
    {
        OLED_ShowChar(Line, Column,
                      (char)('0' + Number / Divisor % 10U));

        Divisor /= 10U;
        Column++;
    }
}

/* 带符号十进制；Length 为数字位数，不包含符号 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column,
                        int32_t Number, uint8_t Length)
{
    uint32_t Magnitude;

    if (Line < 1U || Line > 4U ||
        Column < 1U || Column > 16U ||
        Length < 1U || Length > 10U)
    {
        return;
    }

    if (Number < 0)
    {
        OLED_ShowChar(Line, Column, '-');

        /* 可正确处理 INT32_MIN */
        Magnitude = 0U - (uint32_t)Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '+');
        Magnitude = (uint32_t)Number;
    }

    if (Column < 16U)
    {
        OLED_ShowNum(Line, (uint8_t)(Column + 1U),
                     Magnitude, Length);
    }
}

/* 十六进制；Length：1~8 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column,
                     uint32_t Number, uint8_t Length)
{
    static const char Digits[] = "0123456789ABCDEF";
    uint8_t i;
    uint8_t Shift;

    if (Line < 1U || Line > 4U ||
        Column < 1U || Column > 16U ||
        Length < 1U || Length > 8U)
    {
        return;
    }

    for (i = 0; i < Length && Column <= 16U; i++)
    {
        Shift = (uint8_t)((Length - i - 1U) * 4U);

        OLED_ShowChar(Line, Column,
                      Digits[(Number >> Shift) & 0x0FU]);

        Column++;
    }
}

/* 二进制；Length：1~16 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column,
                     uint32_t Number, uint8_t Length)
{
    uint8_t i;
    uint8_t Shift;

    if (Line < 1U || Line > 4U ||
        Column < 1U || Column > 16U ||
        Length < 1U || Length > 16U)
    {
        return;
    }

    for (i = 0; i < Length && Column <= 16U; i++)
    {
        Shift = (uint8_t)(Length - i - 1U);

        OLED_ShowChar(Line, Column,
                      (char)('0' + ((Number >> Shift) & 1U)));

        Column++;
    }
}

void OLED_Init(void)
{
    OLED_I2C_Init();

    /* 需在 HAL_Init() 和系统时钟配置完成后调用 */
    HAL_Delay(100);

    OLED_WriteCommand(0xAE); /* 关闭显示 */

    OLED_WriteCommand(0xD5); /* 显示时钟 */
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8); /* 64 行 */
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3); /* 显示偏移 */
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40); /* 显示起始行 */

    OLED_WriteCommand(0x20); /* 页寻址模式 */
    OLED_WriteCommand(0x02);

    OLED_WriteCommand(0xA1); /* 左右方向 */
    OLED_WriteCommand(0xC8); /* 上下方向 */

    OLED_WriteCommand(0xDA); /* COM 引脚配置 */
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81); /* 对比度 */
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9); /* 预充电周期 */
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB); /* VCOMH */
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4); /* 显示显存内容 */
    OLED_WriteCommand(0xA6); /* 正常显示 */

    OLED_WriteCommand(0x8D); /* 开启电荷泵 */
    OLED_WriteCommand(0x14);

    OLED_Clear();
    OLED_WriteCommand(0xAF); /* 开启显示 */
}

/* 使用控制器已经采样的速度，不重复采样或清零编码器。 */
void OLED_MotorSpeedInit(void)
{
    OLED_Init();
    /* 四行单位均为计数/秒；Set目标，Now实际速度。 */
    OLED_ShowString(1, 1, "L Set:");
    OLED_ShowString(2, 1, "L Now:");
    OLED_ShowString(3, 1, "R Set:");
    OLED_ShowString(4, 1, "R Now:");
    OLED_ShowSignedNum(1, 7, 0, 7);
    OLED_ShowSignedNum(2, 7, 0, 7);
    OLED_ShowSignedNum(3, 7, 0, 7);
    OLED_ShowSignedNum(4, 7, 0, 7);
}

/* 一次仅写一个显存字节；每块之间返回main，让PID先获得运行机会。
 * 仍是短时间同步I2C，不保证严格20ms定时；避免一次阻塞整屏/两行。 */
void OLED_MotorSpeedUpdate(int32_t left_target, int32_t left_speed,
                           int32_t right_target, int32_t right_speed)
{
    static uint32_t previous_tick = 0;
    static char text[4][9];
    static uint16_t offset = 512U;
    int32_t values[4];
    uint32_t now = HAL_GetTick();
    uint8_t row, i, page_half, column, character, font_column, x, page;

    if (offset >= 512U)
    {
        if ((uint32_t)(now - previous_tick) < 200U) return;
        /* 四个值在开始一帧时一起保存，分块刷新期间保持不变。 */
        values[0] = left_target;
        values[1] = left_speed;
        values[2] = right_target;
        values[3] = right_speed;
        for (i = 0; i < 4U; i++)
        {
            if (values[i] > 9999999) values[i] = 9999999;
            if (values[i] < -9999999) values[i] = -9999999;
            (void)snprintf(text[i], sizeof(text[i]), "%+08ld", (long)values[i]);
        }
        offset = 0;
    }

    row = (uint8_t)(offset / 128U);
    page_half = (uint8_t)((offset % 128U) / 64U);
    column = (uint8_t)(offset % 64U);
    character = (uint8_t)text[row][column / 8U] - 32U;
    font_column = (uint8_t)(column % 8U + page_half * 8U);
    x = (uint8_t)(48U + column); /* 第7字符列开始，保留六字符标签。 */
    page = (uint8_t)(row * 2U + page_half);

    /* 三个定位命令合并为一次传输，保持原来的总线延时。 */
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_I2C_WRITE_ADDR);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte((uint8_t)(0xB0U | page));
    OLED_I2C_SendByte((uint8_t)(0x10U | (x >> 4)));
    OLED_I2C_SendByte((uint8_t)(x & 0x0FU));
    OLED_I2C_Stop();
    OLED_WriteData(OLED_F8x16[character][font_column]);

    offset++;
    /* 本帧结束后再间隔200ms；实际刷新率取决于软件I2C耗时。 */
    if (offset >= 512U) previous_tick = HAL_GetTick();
}
