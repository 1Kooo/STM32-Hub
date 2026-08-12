/**
 * @file    led.h
 * @brief   LED 驱动（寄存器版）
 * @note    直接操作寄存器，不依赖标准外设库，用于学习底层原理
 *          API 与 StdPeriph 版完全一致，可对照学习
 *
 * 使用方法：
 *  1. 修改下方宏配置引脚（端口 / 位号 / 时钟位 / 极性）
 *  2. 调用 Led_Init() 完成初始化
 *  3. 用 Led_On / Led_Off / Led_Toggle 控制 LED
 *
 * 时钟位速查（RCC->APB2ENR）：
 *  GPIOA=2  GPIOB=3  GPIOC=4  GPIOD=5  GPIOE=6
 */

#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

/* 极性：1 = 高电平点亮，0 = 低电平点亮
 * 判断方法：
 *   LED 阳极接 3.3V（共阳接法）→ 引脚低电平点亮 → 0
 *   LED 阴极接地（共阴接法）→ 引脚高电平点亮 → 1 */
#define LED_ACTIVE_HIGH      0

/* ---------- LED1 配置 ---------- */
#define LED1_PORT            GPIOC      /* 端口 */
#define LED1_PIN_NUM         13         /* 引脚号（0~15） */
#define LED1_RCC_BIT         4          /* 该端口在 APB2ENR 的时钟位 */

/* ---------- LED2 配置 ---------- */
#define LED2_PORT            GPIOB      /* 端口 */
#define LED2_PIN_NUM         1          /* 引脚号（0~15） */
#define LED2_RCC_BIT         3          /* 该端口在 APB2ENR 的时钟位 */

typedef enum
{
    LED1 = 1,
    LED2 = 2,
} LedNum;

void Led_Init(void);          /* 初始化所有 LED（推挽输出，默认全灭） */
void Led_On(LedNum led);      /* 点亮 */
void Led_Off(LedNum led);     /* 熄灭 */
void Led_Toggle(LedNum led);  /* 翻转 */
#endif
