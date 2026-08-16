/**
 * @file    pwm.h
 * @brief   PWM 输出驱动（寄存器版）——宏配置，移植只改配置区
 * @note    与 StdPeriph/pwm 对照学习：
 *          配置区宏          == TIM_OCInitTypeDef 参数
 *          通道模式配置      == TIM_OC1Init + TIM_OC1PreloadConfig
 *          CCR 写入          == TIM_SetCompare1()
 *
 * 功能：TIM2_CH1 输出 PWM（呼吸灯/舵机/电机调速/背光调光）
 * 用法：Pwm_Init() → Pwm_SetDuty(值)
 *
 * ★注意：本驱动只开 1 个通道（单实例）。
 *   一个定时器的 PSC/ARR 是共享资源——不要和定时中断驱动共用同一个
 *   定时器实例（谁后 Init 谁覆盖谁）！多功能用不同定时器。
 *
 * 呼吸灯 vs 舵机（同驱动改配置）：
 *   呼吸灯：ARR=999  → PWM 1kHz，duty 0~999 渐变
 *   舵机：  ARR=19999 → PWM 50Hz，duty 500~2500（脉宽 0.5~2.5ms）
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
 *   占空比   = CCR / (ARR+1)   ← 亮度/角度由 CCR 定
 *==========================================================*/
#define PWM_TIM        TIM2          /* 定时器实例 */
#define PWM_RCC_REG    RCC->APB1ENR  /* TIM2 时钟寄存器（APB1） */
#define PWM_RCC_BIT    0             /* TIM2=bit0 */
#define PWM_PSC        71            /* 预分频（PSC+1=72 → 计数器时钟 1MHz） */
#define PWM_ARR        999           /* 计数上限（ARR+1=1000 → PWM 1kHz）
                                      * 舵机改 19999 → 50Hz */
#define PWM_CH_PORT    GPIOA         /* 通道引脚端口（PA0） */
#define PWM_CH_PIN     0             /* 通道引脚号（0=CH1） */
#define PWM_GPIO_RCC_REG  RCC->APB2ENR   /* GPIOA 时钟寄存器 */
#define PWM_GPIO_RCC_BIT  2              /* GPIOA=bit2 */

void Pwm_Init(void);              /* 初始化 PWM 输出（初始占空比 0） */
void Pwm_SetDuty(uint16_t duty);  /* 设置占空比（0 ~ PWM_ARR，超范围自动限幅） */

#endif
