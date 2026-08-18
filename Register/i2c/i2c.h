/**
 * @file    i2c.h
 * @brief   I2C 总线驱动（软件模拟版）——主机模式，约 100kHz
 * @note    ★软件模拟 I2C 的特点：
 *          任意普通引脚都行（不占 I2C 专用脚）、时序完全可控、
 *          更能理解协议本身（起始/停止/数据/应答四动作）。
 *          需要 400kHz 快速模式或 DMA 大块搬运时，用硬件版（i2c_hw/）。
 *
 * 协议速成（I2C 四件事）：
 *   起始：SCL 高时 SDA 由高→低      （宣布"我要说话了"）
 *   停止：SCL 高时 SDA 由低→高      （"我说完了"）
 *   发字节：MSB 先发，8 bit，每个 bit 在 SCL 高电平时保持稳定
 *   应答：第 9 个时钟，从机拉低 SDA = 收到（ACK）
 */

#ifndef I2C_H
#define I2C_H

#include "stm32f10x.h"

/* I2C 引脚配置（软件模拟，任意普通引脚都行） */
#define I2C_SDA_PIN 7
#define I2C_SCL_PIN 6
#define I2C_SDA_PORT GPIOB
#define I2C_SCL_PORT GPIOB

void I2c_Init(void);
void I2c_Start(void);
void I2c_Stop(void);
void I2c_SendByte(uint8_t data);

uint8_t I2c_WaitAck(void);


#endif
