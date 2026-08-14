/**
 * @file    usart.h
 * @brief   串口驱动（标准外设库版）——与 Register 版 API 完全一致
 * @note    配置宏与 Register 版一一对应，移植时同样只改配置区：
 *          库的时钟宏   == 寄存器版的位掩码（RCC_APB2Periph_USART1 = 0x4000 = bit14）
 *          库的引脚宏   == 引脚位掩码（GPIO_Pin_9 = 0x0200）
 *          API 无参     == 配置全靠下面的宏
 */

#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

/*==========================================================
 * 串口配置区 ★移植时改这里就行★
 *
 * F103 各串口速查（照着填）：
 *   USART1: TX=PA9  RX=PA10 | APB2 时钟宏 RCC_APB2Periph_USART1
 *   USART2: TX=PA2  RX=PA3  | APB1 时钟宏 RCC_APB1Periph_USART2
 *   USART3: TX=PB10 RX=PB11 | APB1 时钟宏 RCC_APB1Periph_USART3
 *
 * ★移植必改的 3 处：
 *   ① USART_X         串口实例
 *   ② USART_RCC / USART_GPIO_RCC   时钟宏（U1 在 APB2，U2/U3 在 APB1！）
 *   ③ 引脚（TX/RX 端口和引脚位掩码）
 *   （波特率库函数自动算，不用管频率——这是库版比寄存器版省心的地方）
 *==========================================================*/
#define USART_X          USART1              /* 串口实例：USART1/2/3 */
#define USART_RCC        RCC_APB2Periph_USART1  /* USART 时钟宏 */
#define USART_GPIO_RCC   RCC_APB2Periph_GPIOA   /* GPIO 时钟宏 */
#define USART_TX_PORT    GPIOA               /* TX 端口 */
#define USART_TX_PIN     GPIO_Pin_9          /* TX 引脚（库用位掩码，不是引脚号！） */
#define USART_RX_PORT    GPIOA               /* RX 端口 */
#define USART_RX_PIN     GPIO_Pin_10         /* RX 引脚 */
#define USART_BAUD       115200              /* 波特率（库自动算 BRR） */

/* API（与 Register 版一致） */
void Usart_Init(void);               /* 初始化串口 */
void Usart_SendByte(uint8_t data);   /* 发送一个字节（阻塞） */
void Usart_SendString(char *str);    /* 发送字符串 */
uint8_t Usart_ReceiveByte(void);     /* 接收一个字节（阻塞等待） */

#endif
