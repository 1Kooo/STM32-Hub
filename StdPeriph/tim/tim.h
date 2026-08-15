/**
 * @file    tim.h
 * @brief   定时器驱动（标准外设库版）——与 Register 版 API 完全一致
 * @note    配置宏与 Register 版一一对应，移植时同样只改配置区：
 *          TIM_RCC 时钟宏      == 寄存器版时钟位（RCC_APB1Periph_TIM2 = 0x1 = bit0）
 *          ★注意 TIM2/3/4 实际时钟 72MHz（APB1 分频≠1 时定时器 ×2）
 */

#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"

/*==========================================================
 * 定时器配置区 ★移植时改这里★
 *
 * F103 定时器速查（C8T6 有 TIM1/2/3/4）：
 *   TIM1 → RCC_APB2Periph_TIM1（APB2，高级定时器）
 *   TIM2 → RCC_APB1Periph_TIM2 ★通用，最常用
 *   TIM3 → RCC_APB1Periph_TIM3
 *   TIM4 → RCC_APB1Periph_TIM4
 *
 * 更新频率 = TIM时钟(72MHz) / ((PSC+1) × (ARR+1))
 * 本配置：72MHz / (72 × 1000) = 1kHz = 1ms 一次中断
 *==========================================================*/
#define TIM_X          TIM2                  /* 定时器实例 */
#define TIM_RCC        RCC_APB1Periph_TIM2   /* 时钟宏 */
#define TIM_PSC        71                    /* 预分频（PSC+1=72 → 计数器时钟 1MHz） */
#define TIM_ARR        999                   /* 计数上限（ARR+1=1000 → 1ms 更新） */
#define TIM_IRQn       TIM2_IRQn             /* 中断号 */
#define TIM_IRQ_PRIO   2                     /* 抢占优先级 */

void Tim_Init(void);              /* 初始化定时器中断（1ms tick） */
uint32_t Tim_Tick(void);          /* 读毫秒计数 */

#endif
