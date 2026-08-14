/**
 * @file    usart.h
 * @brief   串口驱动（寄存器版）——宏配置，移植只改配置区
 * @note    与 StdPeriph/usart 对照学习：
 *          配置区宏           == USART_InitTypeDef 结构体参数
 *          所有 USART_X 宏    == USART1 / USART2 等实例
 *          用法三步：改配置区 → Usart_Init() → Send/Receive
 *
 * 配置五步套路（写死版也是这五步）：
 *  ① 开时钟（GPIO + USART，两个都要开！）
 *  ② 配引脚（TX 复用推挽，RX 浮空输入）
 *  ③ 波特率（BRR，按频率算）
 *  ④ 数据格式（8 位数据、1 位停止位、无校验）
 *  ⑤ 使能（UE + TE + RE）
 */

#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

/*==========================================================
 * 串口配置区 ★移植时改这里就行★
 *
 * F103 各串口速查（照着填）：
 *   USART1: TX=PA9  RX=PA10 | 时钟寄存器 APB2ENR bit14 | 频率 72MHz
 *   USART2: TX=PA2  RX=PA3  | 时钟寄存器 APB1ENR bit17 | 频率 36MHz
 *   USART3: TX=PB10 RX=PB11 | 时钟寄存器 APB1ENR bit18 | 频率 36MHz
 *
 * ★移植必改的 4 处：
 *   ① USART_X         串口实例
 *   ② 时钟（寄存器 + 位号：U1 在 APB2，U2/U3 在 APB1！）
 *   ③ 引脚（TX/RX 端口和引脚号）
 *   ④ USART_FCK       频率（U1=72MHz，U2/U3=36MHz，算波特率用）
 *==========================================================*/
#define USART_X          USART1        /* 串口实例：USART1/2/3 */
#define USART_RCC_REG    RCC->APB2ENR  /* USART 时钟寄存器：U1→APB2ENR，U2/U3→APB1ENR */
#define USART_RCC_BIT    14            /* USART 时钟使能位：U1=14，U2=17，U3=18 */
#define USART_GPIO_RCC_REG  RCC->APB2ENR  /* GPIO 时钟寄存器（U1/U2 引脚在 A，U3 在 B） */
#define USART_GPIO_RCC_BIT  2             /* GPIO 时钟位：GPIOA=2、GPIOB=3、GPIOC=4 */
#define USART_TX_PORT    GPIOA         /* TX 端口 */
#define USART_TX_PIN     9             /* TX 引脚号 */
#define USART_RX_PORT    GPIOA         /* RX 端口 */
#define USART_RX_PIN     10            /* RX 引脚号 */
#define USART_BAUD       115200        /* 波特率（常用 115200 / 9600） */
#define USART_FCK        72000000      /* 串口时钟频率：U1=72MHz，U2/U3=36MHz */

/* API（无参，配置全靠上面的宏） */
void Usart_Init(void);               /* 初始化串口（五步） */
void Usart_SendByte(uint8_t data);   /* 发送一个字节（阻塞，等 TXE） */
void Usart_SendString(char *str);    /* 发送字符串（遇到 '\0' 结束） */
uint8_t Usart_ReceiveByte(void);     /* 接收一个字节（阻塞等待 RXNE） */

#endif
