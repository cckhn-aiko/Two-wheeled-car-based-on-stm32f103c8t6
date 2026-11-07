#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void OLED_Init(void);
void OLED_Clear(void);
/* 左右实际速度显示；刷新周期封装在OLED模块内。 */
void OLED_MotorSpeedInit(void);
void OLED_MotorSpeedUpdate(int32_t left_target, int32_t left_speed,
                           int32_t right_target, int32_t right_speed);

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String);

void OLED_ShowNum(uint8_t Line, uint8_t Column,
                  uint32_t Number, uint8_t Length);

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column,
                        int32_t Number, uint8_t Length);

void OLED_ShowHexNum(uint8_t Line, uint8_t Column,
                     uint32_t Number, uint8_t Length);

void OLED_ShowBinNum(uint8_t Line, uint8_t Column,
                     uint32_t Number, uint8_t Length);

/* 底层接口 */
void OLED_I2C_Init(void);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_SendByte(uint8_t Byte);

void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t Data);
void OLED_SetCursor(uint8_t Y, uint8_t X);

uint32_t OLED_Pow(uint32_t X, uint32_t Y);

#ifdef __cplusplus
}
#endif

#endif
