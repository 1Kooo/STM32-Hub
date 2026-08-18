/**
 * @file    i2c.c
 * @brief   I2C 总线驱动（软件模拟版）——标准库版
 * @note    与 Register/i2c 完全同逻辑，只是把寄存器操作换成库函数：
 *          - BSRR 置/清位  → GPIO_SetBits / GPIO_ResetBits
 *          - IDR 读电平    → GPIO_ReadInputDataBit
 *          - CRL 配模式    → GPIO_Init（GPIO_Mode_Out_OD 开漏输出）
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
#include "stm32f10x_gpio.h"

/* 引脚置高/置低/读电平（库函数封装）
 * I2C_SCL_PIN 已是库掩码（GPIO_Pin_6 = 0x0040），直接传即可 */
#define I2C_SCL_H()  GPIO_SetBits(I2C_SCL_PORT, I2C_SCL_PIN)
#define I2C_SCL_L()  GPIO_ResetBits(I2C_SCL_PORT, I2C_SCL_PIN)
#define I2C_SDA_H()  GPIO_SetBits(I2C_SDA_PORT, I2C_SDA_PIN)
#define I2C_SDA_L()  GPIO_ResetBits(I2C_SDA_PORT, I2C_SDA_PIN)
#define I2C_SDA_IN() GPIO_ReadInputDataBit(I2C_SDA_PORT, I2C_SDA_PIN)

/* 半周期延时（空循环，配合模块上拉，约 100kHz） */
static void I2c_Delay(void)
{
    volatile uint32_t i;
    for (i = 0; i < 20; i++) ;
}

void I2c_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 1. 开 GPIOB 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 2. PB6/PB7 配开漏输出（★开漏是 I2C 的关键）
     * GPIO_Pin 是库掩码，I2C_SCL_PIN | I2C_SDA_PIN = GPIO_Pin_6|GPIO_Pin_7 */
    GPIO_InitStructure.GPIO_Pin   = (uint16_t)(I2C_SCL_PIN | I2C_SDA_PIN);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;   /* 开漏输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 3. 总线空闲：两根线都释放（高电平） */
    GPIO_SetBits(I2C_SCL_PORT, (uint16_t)(I2C_SCL_PIN | I2C_SDA_PIN));
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
