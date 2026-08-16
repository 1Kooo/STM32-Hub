/**
 * @file    pwm.h
 * @brief   PWM 输出驱动（标准外设库版）——与 Register 版 API 完全一致
 * @note    配置宏与 Register 版一一对应，移植时同样只改配置区
 *          ★注意：本驱动只开 1 个通道，且不要和定时中断驱动共用
 *           同一个定时器实例（PSC/ARR 是共享资源，谁后 Init 谁覆盖谁）
 */

#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

/*==========================================================
 * PWM 配置区 ★移植时改这里★
 *
 * 引脚（★PWM 波形只在固定复用引脚上出）：
 *   TIM2_CH1=PA0  TIM2_CH2=PA1  TIM2_CH3=PA2  TIM2_CH4=PA3
 *
 * 公式：
 *   PWM 频率 = TIM时钟(72MHz) / ((PSC+1) × (ARR+1))
 *   占空比   = CCR / (ARR+1)
 *   呼吸灯：ARR=999 → 1kHz；舵机：ARR=19999 → 50Hz
 *==========================================================*/
#define PWM_TIM        TIM2                  /* 定时器实例 */
#define PWM_RCC        RCC_APB1Periph_TIM2   /* TIM2 时钟宏（APB1） */
#define PWM_PSC        71                    /* 预分频（PSC+1=72 → 计数器时钟 1MHz） */
#define PWM_ARR        999                   /* 计数上限（PWM 1kHz；舵机改 19999 → 50Hz） */
#define PWM_CH_PORT    GPIOA                 /* 通道引脚端口（PA0） */
#define PWM_CH_PIN     GPIO_Pin_0            /* 通道引脚（TIM2_CH1 = PA0） */
#define PWM_GPIO_RCC   RCC_APB2Periph_GPIOA  /* GPIOA 时钟宏 */

void Pwm_Init(void);              /* 初始化 PWM 输出（初始占空比 0） */
void Pwm_SetDuty(uint16_t duty);  /* 设置占空比（0 ~ PWM_ARR，超范围自动限幅） */

#endif
