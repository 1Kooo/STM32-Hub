/**
 * @file    spi.c
 * @brief   SPI 总线驱动（硬件外设）——主机模式全双工，寄存器操作
 * @note    核心概念：
 *          1. 交换字节模型：写 DR 的同一瞬间硬件同时从 MISO 移入 1 字节
 *             → "发一个字节" = "收一个字节"，收发必须成对（Spi_TransferByte）
 *          2. 两个标志：
 *             TXE （SR bit1）发送缓冲空，可以写 DR
 *             RXNE（SR bit0）接收缓冲有数据，读 DR 获取（读后自动清）
 *          3. 软件 NSS：SSM(bit9)=1 + SSI(bit8)=1
 *             ★SSI 必须置 1！SSI=0 会把 MSTR 清掉，主机变从机
 *
 * 配置五步：
 *   1. 开时钟：GPIO（APB2ENR）+ SPI（SPI_RCC_REG 宏定位，SPI1=APB2/SPI2=APB1）
 *   2. 引脚：SCK/MOSI 复用推挽（0xB）、MISO 浮空输入（0x4）
 *   3. CR1：MSTR + SSM + SSI + BR 分频（+CPOL/CPHA 模式 0）
 *   4. CR1 SPE 使能
 *   5. Spi_TransferByte 交换字节
 */

#include "spi.h"

/* 引脚所在寄存器（CRL=0~7 号，CRH=8~15 号），换实例自动适配（SPI2 在 CRH） */
#if SPI_SCK_PIN < 8
    #define SPI_SCK_REG    SPI_PORT->CRL
    #define SPI_SCK_SHIFT  (SPI_SCK_PIN * 4)
#else
    #define SPI_SCK_REG    SPI_PORT->CRH
    #define SPI_SCK_SHIFT  ((SPI_SCK_PIN - 8) * 4)
#endif

#if SPI_MISO_PIN < 8
    #define SPI_MISO_REG   SPI_PORT->CRL
    #define SPI_MISO_SHIFT (SPI_MISO_PIN * 4)
#else
    #define SPI_MISO_REG   SPI_PORT->CRH
    #define SPI_MISO_SHIFT ((SPI_MISO_PIN - 8) * 4)
#endif

#if SPI_MOSI_PIN < 8
    #define SPI_MOSI_REG   SPI_PORT->CRL
    #define SPI_MOSI_SHIFT (SPI_MOSI_PIN * 4)
#else
    #define SPI_MOSI_REG   SPI_PORT->CRH
    #define SPI_MOSI_SHIFT ((SPI_MOSI_PIN - 8) * 4)
#endif

void Spi_Init(void)
{
    /* 1. 开时钟：GPIO（APB2）+ SPI（SPI_RCC_REG 宏定位：SPI1=APB2，SPI2=APB1） */
    RCC->APB2ENR |= (1u << SPI_GPIO_RCC_BIT);   /* GPIO 端口时钟（都在 APB2） */
    SPI_RCC_REG |= (1u << SPI_RCC_BIT);         /* SPI 外设时钟 */

    /* 2. 引脚：SCK/MOSI 复用推挽（CNF=10+MODE=01 = 0xB），MISO 浮空输入（0x4）
     *    ★复用功能：引脚交给 SPI 外设接管（同 I2C 的套路） */
    SPI_SCK_REG  &= ~((uint32_t)0xF << SPI_SCK_SHIFT);
    SPI_SCK_REG  |=  (uint32_t)0xB << SPI_SCK_SHIFT;    /* SCK */
    SPI_MOSI_REG &= ~((uint32_t)0xF << SPI_MOSI_SHIFT);
    SPI_MOSI_REG |=  (uint32_t)0xB << SPI_MOSI_SHIFT;   /* MOSI */
    SPI_MISO_REG &= ~((uint32_t)0xF << SPI_MISO_SHIFT);
    SPI_MISO_REG |=  (uint32_t)0x4 << SPI_MISO_SHIFT;   /* MISO（输入方向） */

    /* 3. CR1 配置（SPE 最后单独开，配置期间外设保持关闭更安全）：
     *    MSTR(bit2)    主机模式
     *    SSM(bit9)     软件 NSS
     *    SSI(bit8)     ★内部拉高，否则 MSTR 被清、变从机
     *    CPOL/CPHA     模式 0（都是 0）
     *    BR[2:0]       分频 = 4 → 18MHz */
    SPI_X->CR1 = (1u << 2) | (1u << 9) | (1u << 8)
               | ((uint32_t)SPI_CPOL << 1) | (uint32_t)SPI_CPHA
               | ((uint32_t)SPI_BR << 3);

    /* 4. 使能 SPI（SPE bit6） */
    SPI_X->CR1 |= (1u << 6);
}

uint8_t Spi_TransferByte(uint8_t data)
{
    while (!(SPI_X->SR & (1u << 1)));   /* 等 TXE：发送缓冲空，能写 */
    SPI_X->DR = data;                   /* 写 DR：硬件自动逐位移出（同时移入） */

    while (!(SPI_X->SR & (1u << 0)));   /* 等 RXNE：接收缓冲有数据 */
    return (uint8_t)SPI_X->DR;          /* 读 DR：拿到从机回的字节（读后自动清 RXNE） */
}
