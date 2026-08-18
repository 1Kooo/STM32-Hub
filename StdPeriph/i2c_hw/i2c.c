/**
 * @file    i2c.c
 * @brief   I2C 总线驱动（★硬件外设版）——标准库版
 * @note    与 Register/i2c_hw 同逻辑，寄存器操作换成库函数：
 *          - 配置：I2C_Init() + I2C_Cmd()（内部自动算 CCR/TRISE，只需给速率）
 *          - 起始：I2C_GenerateSTART()（对应写 START 位）
 *          - 停止：I2C_GenerateSTOP()
 *          - 发字节：I2C_SendData()（对应写 DR）
 *          - 等标志：I2C_CheckEvent() / I2C_GetFlagStatus()（对应查 SR1）
 *
 * 标志对照（和寄存器版同一个东西）：
 *   I2C_EVENT_MASTER_MODE_SELECT = SB 置位
 *   I2C_FLAG_BTF / I2C_FLAG_TXE / I2C_FLAG_AF / I2C_FLAG_ADDR
 *
 * 换实例 = 改 i2c.h 的 I2C_HW_X / I2C_HW_RCC / 引脚宏
 */

#include "i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"

void I2c_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /* 1. 开时钟：GPIO（APB2）+ I2C（APB1，实例宏定位）——★两个时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(I2C_HW_RCC, ENABLE);

    /* 2. SCL/SDA 配复用开漏（★复用功能：引脚交给 I2C 外设接管） */
    GPIO_InitStructure.GPIO_Pin   = (uint16_t)(I2C_HW_SCL_PIN | I2C_HW_SDA_PIN);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;    /* 复用开漏 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(I2C_HW_PORT, &GPIO_InitStructure);

    /* 3. I2C 配置：标准模式 100kHz（速率直接给 Hz，库函数自动算 CCR/TRISE） */
    I2C_InitStructure.I2C_Mode                  = I2C_Mode_I2C;
    I2C_InitStructure.I2C_ClockSpeed            = 100000;              /* 100kHz */
    I2C_InitStructure.I2C_DutyCycle             = I2C_DutyCycle_2;     /* 标准模式不用 */
    I2C_InitStructure.I2C_OwnAddress            = 0x00;                /* 自己地址（从机模式才用） */
    I2C_InitStructure.I2C_Ack                   = I2C_Ack_Enable;      /* 自动应答 */
    I2C_InitStructure.I2C_AcknowledgedAddress   = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C_HW_X, &I2C_InitStructure);

    /* 4. 使能外设 */
    I2C_Cmd(I2C_HW_X, ENABLE);
}

void I2c_Start(void)
{
    I2C_GenerateSTART(I2C_HW_X, ENABLE);        /* 写 START 位 */
    while (!I2C_CheckEvent(I2C_HW_X, I2C_EVENT_MASTER_MODE_SELECT));   /* 等 SB */
}

void I2c_Stop(void)
{
    I2C_GenerateSTOP(I2C_HW_X, ENABLE);         /* 写 STOP 位 */
}

void I2c_SendByte(uint8_t data)
{
    while (I2C_GetFlagStatus(I2C_HW_X, I2C_FLAG_TXE) == RESET);   /* 等 DR 空 */
    I2C_SendData(I2C_HW_X, data);               /* 写 DR */
}

uint8_t I2c_WaitAck(void)
{
    while (I2C_GetFlagStatus(I2C_HW_X, I2C_FLAG_BTF) == RESET);   /* 等字节移位完成 */

    if (I2C_GetFlagStatus(I2C_HW_X, I2C_FLAG_AF) == SET) {        /* 无应答 */
        I2C_ClearFlag(I2C_HW_X, I2C_FLAG_AF);                     /* 清 AF */
        return 1;
    }

    if (I2C_GetFlagStatus(I2C_HW_X, I2C_FLAG_ADDR) == SET)        /* 地址阶段 */
        I2C_ClearADDRFlag(I2C_HW_X);                              /* ★读 SR1+SR2 清 ADDR */

    return 0;                                                     /* 有应答 */
}
