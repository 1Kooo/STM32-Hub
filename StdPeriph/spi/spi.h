/**
 * @file    spi.h
 * @brief   SPI 总线驱动（硬件外设 SPI1）——主机模式，全双工
 * @note    SPI 是什么（对比 I2C 记忆）：
 *          - 4 根线：SCK（时钟）/ MOSI（主出从入）/ MISO（从出主入）/ CS（片选）
 *          - 无地址，多从机靠片选 CS（普通 GPIO 拉低 = 选中谁）
 *          - 全双工：每 1 个时钟 = 移出 1 位 + 移入 1 位 → 发字节和收字节是同一件事
 *
 * ★SPI1 引脚（无重映射，硬件固定）：
 *   PA5 = SCK、PA6 = MISO、PA7 = MOSI（PA4 = NSS，软件管理后可当普通 GPIO）
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
 *   ★引脚用库位掩码 GPIO_Pin_x，不是引脚号！
 *==========================================================*/
#define SPI_X           SPI1
#define SPI_RCC_CMD     RCC_APB2PeriphClockCmd    /* 外设时钟函数：SPI1=APB2；SPI2 改 RCC_APB1PeriphClockCmd */
#define SPI_RCC         RCC_APB2Periph_SPI1       /* 外设时钟位：SPI2 改 RCC_APB1Periph_SPI2 */
#define SPI_GPIO_RCC    RCC_APB2Periph_GPIOA      /* 端口时钟位（GPIO 都在 APB2）：SPI2 用 GPIOB 改 */
#define SPI_PORT        GPIOA                     /* 引脚所在端口：SPI2 改 GPIOB */
#define SPI_SCK_PIN     GPIO_Pin_5                /* SCK：SPI2 改 GPIO_Pin_13 */
#define SPI_MISO_PIN    GPIO_Pin_6                /* MISO：SPI2 改 GPIO_Pin_14 */
#define SPI_MOSI_PIN    GPIO_Pin_7                /* MOSI：SPI2 改 GPIO_Pin_15 */
#define SPI_FCK         72                        /* 时钟源 MHz（只供注释参考）：SPI1=72；SPI2=36 */
#define SPI_BR          1                         /* BR[2:0] 分频：0=2、1=4、2=8...  1 → 4 分频 */
/* ★模式 0（CPOL=0 空闲低 + CPHA=0 第一边沿采样）在 spi.c 里写死（SPI_CPOL_Low/SPI_CPHA_1Edge）
 *   ⚠️ 不要在这里定义 SPI_CPOL/SPI_CPHA 宏——库结构体字段就叫这两个名，会被宏污染！
 *   要换模式（1~3）直接改 spi.c 那两行 */

void Spi_Init(void);                       /* 配置 SPI1（时钟+引脚+模式+使能） */
uint8_t Spi_TransferByte(uint8_t data);    /* ★全双工交换一字节：发出 data，返回收到的字节 */

#endif
