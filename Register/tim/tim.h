/**
 * @file    tim.h
 * @brief   定时器驱动（寄存器版）——宏配置，移植只改配置区
 * @note    与 StdPeriph/tim 对照学习：
 *          配置区宏          == TIM_TimeBaseInitTypeDef 参数
 *          时基+中断配置      == TIM_TimeBaseInit + TIM_ITConfig + NVIC_Init
 *
 * 功能：定时中断，1ms 节拍 tick（时间片轮询的基石）
 * 用法：Tim_Init() → 主循环读 Tim_Tick()
 *       PWM 输出是定时器的另一个功能，独立成 pwm 驱动
 */

#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"

/*==========================================================
 * 定时器配置区 ★移植时改这里★
 *
 * F103 定时器速查（C8T6 有 TIM1/2/3/4）：
 *   TIM1     → APB2ENR bit11（高级定时器，PWM 电机用）
 *   TIM2     → APB1ENR bit0  ★通用，最常用
 *   TIM3     → APB1ENR bit1
 *   TIM4     → APB1ENR bit2
 *
 * ★★时钟大坑★★：
 *   TIM2/3/4 挂 APB1，但 APB1 分频 ≠ 1 时定时器时钟自动 ×2！
 *   所以 TIM2 实际时钟 = 36MHz × 2 = 72MHz（不是 36MHz！）
 *   拿 36MHz 算 PSC/ARR = 定时时间差一倍
 *
 * 更新频率 = TIM时钟 / ((PSC+1) × (ARR+1))
 * 本配置：72MHz / (72 × 1000) = 1kHz = 1ms 一次中断
 *==========================================================*/
#define TIM_X          TIM2          /* 定时器实例 */
#define TIM_RCC_REG    RCC->APB1ENR  /* 时钟寄存器：TIM1→APB2ENR，TIM2/3/4→APB1ENR */
#define TIM_RCC_BIT    0             /* 时钟位：TIM1=11，TIM2=0，TIM3=1，TIM4=2 */
#define TIM_PSC        71            /* 预分频（PSC+1=72 → 计数器时钟 1MHz） */
#define TIM_ARR        999           /* 计数上限（ARR+1=1000 → 1ms 更新） */
#define TIM_IRQn       TIM2_IRQn     /* 中断号 */
#define TIM_IRQ_PRIO   2             /* 抢占优先级（数越小越优先） */

void Tim_Init(void);              /* 初始化定时器中断（1ms tick） */
uint32_t Tim_Tick(void);          /* 读毫秒计数（中断里每秒 +1000） */

#endif
