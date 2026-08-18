/**
 * @file    i2c.h
 * @brief   I2C 总线驱动（★硬件外设版）——主机模式，100kHz 标准速率
 * @note    与软件模拟版（i2c/ 目录）API 完全相同：
 *          I2c_Init / I2c_Start / I2c_Stop / I2c_SendByte / I2c_WaitAck
 *          → main/oled 调用代码一字不改，拷这份文件替换即可切换实现！
 *
 * 硬件版 vs 软件版（同一接口，不同实现）：
 *   软件版：GPIO 手动翻电平，代码就是时序
 *   硬件版：I2C 外设内部状态机自动出时序，代码只做两件事——
 *           1) 写"启动位/停止位/数据"（CR1/DR）
 *           2) 等状态标志（SR1：SB/ADDR/BTF/TXE/AF）
 *
 * ★引脚是硬件固定的（不能像软件版那样任意选）：
 *   I2C1 → PB6(SCL)/PB7(SDA)，I2C2 → PB10(SCL)/PB11(SDA)
 *   换实例 = 改下面 4 个宏（不限 I2C1/I2C2 两个，有 I2C3 也一样改法）
 */

#ifndef I2C_H
#define I2C_H

#include "stm32f10x.h"

/*==========================================================
 * ★I2C 实例配置（换实例就改这 4 行，其余代码不用动）
 *   I2C_HW_X       外设指针：I2C1 / I2C2
 *   I2C_HW_RCC_BIT APB1ENR 时钟位：I2C1=21、I2C2=22
 *   I2C_HW_PORT    引脚所在 GPIO 口（F103 两个 I2C 都在 GPIOB）
 *   I2C_HW_SCL_PIN / I2C_HW_SDA_PIN 引脚号：I2C1=6/7、I2C2=10/11
 *==========================================================*/
#define I2C_HW_X         I2C1        /* I2C1 或 I2C2 */
#define I2C_HW_RCC_BIT   21          /* APB1ENR 位号 */
#define I2C_HW_PORT      GPIOB       /* SCL/SDA 所在 GPIO 口 */
#define I2C_HW_SCL_PIN   6           /* SCL 引脚号 */
#define I2C_HW_SDA_PIN   7           /* SDA 引脚号 */

/*==========================================================
 * I2C 速率配置
 *   FREQ  = PCLK1 频率（MHz），写进 CR2
 *   CCR   = 标准模式时钟分频：CCR = PCLK1 / (2 × fSCL)
 *           36MHz / (2 × 100kHz) = 180
 *   TRISE = 标准模式上升时间：1000ns / (1/PCLK1) + 1 = 37
 *==========================================================*/
#define I2C_HW_FREQ    36          /* PCLK1 = 36MHz */
#define I2C_HW_CCR     180         /* 100kHz 标准模式 */
#define I2C_HW_TRISE   37          /* 上升时间补偿 */

void I2c_Init(void);                          /* 配置 I2C（引脚+速率+使能） */
void I2c_Start(void);                         /* 起始信号（写 START 位，等 SB） */
void I2c_Stop(void);                          /* 停止信号（写 STOP 位） */
void I2c_SendByte(uint8_t data);              /* 发一个字节（写 DR，硬件自动移位） */
uint8_t I2c_WaitAck(void);                    /* 等本字节发完+应答：返回 0=有应答，1=无应答 */

#endif
