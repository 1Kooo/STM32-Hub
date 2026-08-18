/**
 * @file    oled.h
 * @brief   OLED 显示驱动（SSD1306，128×64，I2C 接口）——应用层驱动
 * @note    ★分层思想：本驱动不碰底层总线，所有字节收发都交给 i2c 驱动
 *          （I2c_Start / I2c_SendByte / I2c_WaitAck / I2c_Stop）。
 *          所以 Register 版和 StdPeriph 版代码完全相同。
 *
 * 屏幕结构（理解这个就懂了一切）：
 *   128×64 像素 = 8 页（page）× 128 列（column）
 *   每页 8 像素高，一页里 1 字节 = 竖着 8 个点（bit0 在最上面）
 *   ★所以写屏是"竖着写的"，字模必须纵向取模
 *
 * 坐标说明：
 *   x = 列 0~127（每字符宽 8 像素）
 *   y = 页 0~7（8×16 字符占 2 页，所以 y 只能用 0~6）
 *
 * I2C 帧格式：
 *   从机地址 0x78（7 位地址 0x3C 左移 1 位，写方向）
 *   控制字节：0x00 = 命令，0x40 = 数据
 */

#ifndef OLED_H
#define OLED_H

#include "stm32f10x.h"

#define OLED_ADDR   0x78            /* SSD1306 I2C 写地址（0x3C << 1） */

void Oled_Init(void);                          /* 初始化（内部调 I2c_Init + 25 条命令序列） */
void Oled_Clear(void);                         /* 清屏（8 页 × 128 列全写 0） */
void Oled_SetPos(uint8_t x, uint8_t y);        /* 定位：x=列 0~127，y=页 0~7 */
void Oled_ShowChar(uint8_t x, uint8_t y, char ch);    /* 显示 8×16 字符（占两页） */
void Oled_ShowString(uint8_t x, uint8_t y, char *str); /* 显示字符串（自动换行） */

#endif
