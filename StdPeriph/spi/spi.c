/**
 * @file    spi.c
 * @brief   SPI 总线驱动（硬件外设 SPI1）——主机模式全双工，标准库版
 * @note    与 Register/spi 同逻辑，寄存器操作换成库函数：
 *          - 配置：SPI_Init() + SPI_Cmd()
 *          - 发字节：SPI_I2S_SendData()（对应写 DR）
 *          - 等标志：SPI_I2S_GetFlagStatus()（对应查 SR）
 *          - 收字节：SPI_I2S_ReceiveData()（对应读 DR）
 *
 * 核心概念：
 *          1. 交换字节模型：写 DR 的同一瞬间硬件同时从 MISO 移入 1 字节
 *             → "发一个字节" = "收一个字节"，收发必须成对（Spi_TransferByte）
 *          2. 两个标志：
 *             TXE （SR bit1）发送缓冲空，可以写 DR
 *             RXNE（SR bit0）接收缓冲有数据，读 DR 获取（读后自动清）
 *          3. 软件 NSS：SPI_NSS_Soft（库内部同时置 SSM+SSI）
 */

#include "spi.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

void Spi_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    /* 1. 开时钟：GPIO + SPI（SPI_RCC_CMD 宏定位：SPI1=APB2 函数，SPI2 换 APB1 函数） */
    SPI_RCC_CMD(SPI_GPIO_RCC, ENABLE);
    SPI_RCC_CMD(SPI_RCC, ENABLE);

    /* 2. 引脚：SCK/MOSI 复用推挽（★复用功能），MISO 浮空输入 */
    GPIO_InitStructure.GPIO_Pin   = (uint16_t)(SPI_SCK_PIN | SPI_MOSI_PIN);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;    /* 复用推挽 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;  /* 浮空输入 */
    GPIO_Init(SPI_PORT, &GPIO_InitStructure);

    /* 3. SPI1 配置：主机/全双工/8 位/模式 0/软件 NSS/4 分频/MSB 先行 */
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;  /* 全双工 */
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;                  /* 主机 */
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;                  /* 8 位 */
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;                     /* 模式 0 */
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;                   /* 模式 0 */
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;                     /* ★软件 NSS */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;          /* 4 分频 */
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;                 /* MSB 先发 */
    SPI_InitStructure.SPI_CRCPolynomial     = 7;                                /* CRC 多项式（不用也给） */
    SPI_Init(SPI_X, &SPI_InitStructure);

    /* 4. 使能 SPI */
    SPI_Cmd(SPI_X, ENABLE);
}

uint8_t Spi_TransferByte(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI_X, SPI_I2S_FLAG_TXE) == RESET);  /* 等 TXE */
    SPI_I2S_SendData(SPI_X, data);                                    /* 写 DR */

    while (SPI_I2S_GetFlagStatus(SPI_X, SPI_I2S_FLAG_RXNE) == RESET); /* 等 RXNE */
    return (uint8_t)SPI_I2S_ReceiveData(SPI_X);                       /* 读 DR */
}
