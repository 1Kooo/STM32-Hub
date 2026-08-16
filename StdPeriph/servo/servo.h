/**
 * @file    servo.h
 * @brief   舵机驱动（通用：PWM 脉宽协议）——应用层驱动
 * @note    与 Register/servo 内容相同（本驱动无寄存器操作）：
 *          分层思想：外设驱动管硬件（pwm），应用驱动管业务（servo）
 *          本版配 StdPeriph/pwm（库函数版），Register 版配 Register/pwm
 *
 * 协议（hobby servo 通用标准，SG90 已验证）：
 *   50Hz 周期 + 0.5ms~2.5ms 脉宽 = 0°~180°
 *   MG996R / MG995 / 数字舵机 同协议，直接能用
 *   总线舵机（UART 协议）不适用
 */

#ifndef SERVO_H
#define SERVO_H

#include "stm32f10x.h"

/*==========================================================
 * 舵机配置
 * ★前置条件：pwm.h 的 PWM_ARR 必须是 19999（50Hz）！
 *   （呼吸灯是 999/1kHz，与舵机互斥，不同时用）
 *==========================================================*/
#define SERVO_MAX_ANGLE  180        /* 最大角度（270° 舵机改这里+换算系数） */
#define SERVO_MIN_CCR    500        /* 0°  → 脉宽 0.5ms（计数器 1MHz，1计数=1µs） */
#define SERVO_MAX_CCR    2500       /* 180° → 脉宽 2.5ms */

void Servo_Init(void);              /* 初始化（内部调 Pwm_Init → 50Hz PWM） */
void Servo_SetAngle(uint8_t angle); /* 转指定角度 0~180°（★自动限幅防烧） */




#endif