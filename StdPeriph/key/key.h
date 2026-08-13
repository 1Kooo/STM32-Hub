/**
 * @file    key.h
 * @brief   按键驱动（标准外设库版，基于 STM32F10x 标准外设库）
 * @note    与 Register 版 API 完全一致，可对照学习
 *
 * 使用方法：
 *  1. 修改下方宏配置引脚（端口 / 引脚 / 时钟宏）
 *  2. 调用 Key_Init() 完成初始化
 *  3. 用 Key_Scan() 获取按键事件（消抖 + 边沿检测）
 *
 * 接线说明：按键一端接引脚，另一端接 GND，引脚内部上拉
 * 平时读到高电平（松开），按下读到低电平（按下）
 *
 * 时钟宏速查：
 *  GPIOA -> RCC_APB2Periph_GPIOA   GPIOB -> RCC_APB2Periph_GPIOB
 *  GPIOC -> RCC_APB2Periph_GPIOC   GPIOD -> RCC_APB2Periph_GPIOD
 */

#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* ---------- KEY1 配置 ---------- */
#define KEY1_PORT            GPIOA
#define KEY1_PIN             GPIO_Pin_2
#define KEY1_RCC             RCC_APB2Periph_GPIOA

/* ---------- KEY2 配置 ---------- */
#define KEY2_PORT            GPIOB
#define KEY2_PIN             GPIO_Pin_0
#define KEY2_RCC             RCC_APB2Periph_GPIOB

typedef enum
{
    KEY1 = 1,
    KEY2 = 2,
} KeyNum;

typedef enum
{
    KEY_NONE = 0,   /* 无事件 */
    KEY_UP,         /* 松开（仅触发一次） */
    KEY_DOWN,       /* 按下（仅触发一次） */
} KeyEvent;

void Key_Init(void);              /* 初始化所有按键（上拉输入） */
uint8_t Key_Read(KeyNum key);     /* 读电平：按下返回 1，松开返回 0 */
KeyEvent Key_Scan(KeyNum key);    /* 带消抖扫描，返回按键事件 */
#endif
