/**
 * @file    i2c.c
 * @brief   I2C 总线驱动（软件模拟版）——寄存器操作
 * @note    核心思想：开漏输出 + 上拉电阻 = 线与
 *          - 输出高电平 = 释放总线（由外部上拉电阻拉高）
 *          - 输出低电平 = 主动拉低总线
 *          所以 SCL/SDA 全程开漏输出即可：
 *          读应答时写 1（释放），从机才有机会把 SDA 拉低。
 *
 * 用法（主机写从机，如 SSD1306 OLED）：
 *   I2c_Start();
 *   I2c_SendByte(从机地址 << 1 | 0);    // 写方向
 *   I2c_WaitAck();
 *   I2c_SendByte(命令/数据);
 *   I2c_WaitAck();
 *   I2c_Stop();
 */

#include "i2c.h"

/* 引脚置高/置低（BSRR：低 16 位置 1，高 16 位清 0） */
#define I2C_SCL_H()  (I2C_SCL_PORT->BSRR = (uint32_t)1u << I2C_SCL_PIN)
#define I2C_SCL_L()  (I2C_SCL_PORT->BSRR = (uint32_t)1u << (I2C_SCL_PIN + 16))
#define I2C_SDA_H()  (I2C_SDA_PORT->BSRR = (uint32_t)1u << I2C_SDA_PIN)
#define I2C_SDA_L()  (I2C_SDA_PORT->BSRR = (uint32_t)1u << (I2C_SDA_PIN + 16))
#define I2C_SDA_IN() (I2C_SDA_PORT->IDR & ((uint32_t)1u << I2C_SDA_PIN))  /* 读 SDA 电平 */

/* 半周期延时（空循环，配合模块上拉，约 100kHz） */
static void I2c_Delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 20; i++) ;
}

void I2c_Init(void)
{
    /* 1. 开 GPIOB 时钟（APB2ENR bit3） */
    RCC->APB2ENR |= (1u << 3);

    /* 2. PB6/PB7 配开漏输出
     *    CNF[1:0] = 11（开漏输出）、MODE[1:0] = 01（10MHz）
     *    低 4 位 = 0111 = 0x7 */
    I2C_SCL_PORT->CRL &= ~((uint32_t)0xF << (I2C_SCL_PIN * 4));
    I2C_SCL_PORT->CRL |=  (uint32_t)0x7 << (I2C_SCL_PIN * 4);
    I2C_SDA_PORT->CRL &= ~((uint32_t)0xF << (I2C_SDA_PIN * 4));
    I2C_SDA_PORT->CRL |=  (uint32_t)0x7 << (I2C_SDA_PIN * 4);

    /* 3. 总线空闲：两根线都释放（高电平） */
    I2C_SCL_H();
    I2C_SDA_H();
}

void I2c_Start(void)
{
    I2C_SDA_H();                /* 先保证总线空闲 */
    I2C_SCL_H();
    I2c_Delay();
    I2C_SDA_L();                /* ★SCL 高时 SDA 由高→低 = 起始 */
    I2c_Delay();
    I2C_SCL_L();                /* 拉低 SCL，准备传数据（SDA 允许变化） */
}

void I2c_Stop(void)
{
    I2C_SDA_L();                /* 先拉低 SDA */
    I2C_SCL_H();                /* 拉高 SCL */
    I2c_Delay();
    I2C_SDA_H();                /* ★SCL 高时 SDA 由低→高 = 停止 */
    I2c_Delay();
}

void I2c_SendByte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        /* MSB 先发：data & 0x80 看最高位 */
        if (data & 0x80)
            I2C_SDA_H();
        else
            I2C_SDA_L();
        data <<= 1;             /* 左移，把下一位顶到最高位 */

        I2C_SCL_H();            /* ★SCL 高电平期间 SDA 必须稳定 */
        I2c_Delay();
        I2C_SCL_L();            /* 拉低，允许 SDA 变化 */
        I2c_Delay();
    }
}

uint8_t I2c_WaitAck(void)
{
    I2C_SDA_H();                /* ★释放 SDA，从机才有机会拉低 */
    I2C_SCL_H();                /* 第 9 拍：给从机一个时钟 */
    I2c_Delay();
    if (I2C_SDA_IN()) {         /* 读 SDA：还是高 = 无应答 NACK */
        I2C_SCL_L();
        return 1;
    }
    I2C_SCL_L();                /* 被拉低 = 有应答 ACK */
    return 0;
}
