/**
 * @file    servo.c
 * @brief   舵机驱动（通用：PWM 脉宽协议）——应用层驱动
 * @note    ★分层思想：本驱动不碰寄存器，只做"角度 → 脉宽"换算，
 *          输出交给 PWM 驱动（Pwm_SetDuty）。
 *          所以 Register 版和 StdPeriph 版代码完全相同——
 *          版本差异全被配对的 pwm 驱动吸收掉了。
 *
 * 角度换算原理：
 *   0.5ms~2.5ms 对应 0°~180°，线性关系
 *   CCR = 500 + 角度 × (2000µs / 180°)
 *   （计数器 1MHz 下 1 计数 = 1µs，CCR 直接等于脉宽微秒数）
 */

#include "servo.h"
#include "pwm.h"        /* 复用 PWM 驱动（标准库版：StdPeriph/pwm） */

void Servo_Init(void)
{
    Pwm_Init();         /* 输出 50Hz PWM（确认 pwm.h 的 ARR=19999） */
}

void Servo_SetAngle(uint8_t angle)
{
    uint16_t ccr;

    if (angle > SERVO_MAX_ANGLE)            /* ★限幅：超 0.5~2.5ms 舵机会烧 */
        angle = SERVO_MAX_ANGLE;

    ccr = SERVO_MIN_CCR + (uint32_t)angle * 2000 / 180;   /* 500 + 角度×11.1 */
    Pwm_SetDuty(ccr);                       /* 交给 PWM 驱动输出 */
}

