/**
 * @file    spi.h
 * @brief   SPI 总线驱动（硬件外设）——主机模式，全双工
 * @note    SPI 是什么（对比 I2C 记忆）：
 *          - 4 根线：SCK（时钟）/ MOSI（主出从入）/ MISO（从出主入）/ CS（片选）
 *          - 无地址，多从机靠片选 CS（普通 GPIO 拉低 = 选中谁）
 *          - 全双工：每 1 个时钟 = 移出 1 位 + 移入 1 位 → 发字节和收字节是同一件事
 *
 * ★引脚是硬件固定的（无重映射）：
 *   SPI1 → PA5(SCK)/PA6(MISO)/PA7(MOSI)，挂 APB2 = 72MHz
 *   SPI2 → PB13(SCK)/PB14(MISO)/PB15(MOSI)，挂 APB1 = 36MHz
 *   换实例 = 改下面宏（不限 SPI1/SPI2 两个，改法一样）
 *
 * ★软件 NSS（SSM=1 + SSI=1）：不依赖硬件 NSS 引脚，
 *   片选 CS 用任意 GPIO 自己拉低/拉高
 *
 * CPOL/CPHA（时钟极性/相位，"语言口音"，主从必须一致）：
 *   模式 0 = CPOL=0（空闲低）+ CPHA=0（第一边沿采样）——最通用
 */

#ifndef SPI_H
#define SPI_H

#include "stm32f10x.h"

/*==========================================================
 * ★SPI 实例配置（换实例就改这几行，其余代码不用动）
 *   SPI1：APB2（时钟位 12），引脚 PA5/6/7，时钟 72MHz
 *   SPI2：APB1（时钟位 14），引脚 PB13/14/15，时钟 36MHz
 *==========================================================*/
#define SPI_X           SPI1
#define SPI_RCC_REG     RCC->APB2ENR    /* 外设时钟寄存器：SPI1=APB2；SPI2 改 RCC->APB1ENR */
#define SPI_RCC_BIT     12              /* 外设时钟位：SPI1=12；SPI2=14 */
#define SPI_GPIO_RCC_BIT 2              /* 端口时钟位（GPIO 都在 APB2）：GPIOA=2；SPI2 用 GPIOB 改 3 */
#define SPI_PORT        GPIOA           /* 引脚所在端口：SPI2 改 GPIOB */
#define SPI_SCK_PIN     5               /* SCK 引脚号：SPI2 改 13 */
#define SPI_MISO_PIN    6               /* MISO 引脚号：SPI2 改 14 */
#define SPI_MOSI_PIN    7               /* MOSI 引脚号：SPI2 改 15 */
#define SPI_FCK         72              /* 时钟源 MHz（只供注释参考）：SPI1=72；SPI2=36 */
#define SPI_BR          1               /* BR[2:0] 分频：0=2、1=4、2=8...  1 → 4 分频 */
#define SPI_CPOL        0               /* 时钟极性：0=空闲低 */
#define SPI_CPHA        0               /* 时钟相位：0=第一边沿采样（模式 0） */

void Spi_Init(void);                       /* 配置 SPI（时钟+引脚+模式+使能） */
uint8_t Spi_TransferByte(uint8_t data);    /* ★全双工交换一字节：发出 data，返回收到的字节 */

#endif
