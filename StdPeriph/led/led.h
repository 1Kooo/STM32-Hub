/**
 * @file    led.h
 * @brief   LED 驱动（标准外设库版）
 * @note    基于 STM32F10x 标准外设库，与 Register 版 API 一致
 *
 * 使用方法：
 *  1. 修改下方宏配置引脚（端口 / 引脚 / 时钟宏 / 极性）
 *  2. 调用 Led_Init() 完成初始化
 *  3. 用 Led_On / Led_Off / Led_Toggle 控制 LED
 *
 * 时钟宏速查：
 *  GPIOA -> RCC_APB2Periph_GPIOA   GPIOB -> RCC_APB2Periph_GPIOB
 *  GPIOC -> RCC_APB2Periph_GPIOC   GPIOD -> RCC_APB2Periph_GPIOD
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
#define LED1_PORT            GPIOC
#define LED1_PIN             GPIO_Pin_13
#define LED1_RCC             RCC_APB2Periph_GPIOC

/* ---------- LED2 配置 ---------- */
#define LED2_PORT            GPIOB
#define LED2_PIN             GPIO_Pin_1
#define LED2_RCC             RCC_APB2Periph_GPIOB

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