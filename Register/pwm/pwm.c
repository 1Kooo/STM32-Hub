/**
 * @file    pwm.c
 * @brief   PWM 输出驱动（寄存器版）——TIM2_CH1 @ PA0
 * @note    在定时中断驱动基础上新增 3 步：
 *          时基之后 → 通道模式(CCMR1) → 输出使能(CCER) → CCR 定占空比
 *          与 StdPeriph 版对照学习：
 *          CCMR1/CCER 配置        == TIM_OC1Init()
 *          OC1PE 预装载           == TIM_OC1PreloadConfig()
 *          CCR1 写入              == TIM_SetCompare1()
 *
 * PWM 原理：CNT 从 0 数到 ARR，期间和 CCR 赛跑
 *   CNT <  CCR  → 输出有效电平（亮/通电）
 *   CNT >= CCR  → 输出无效电平（灭/断电）
 *   占空比 = CCR / (ARR+1) —— CCR 越大，有效时间越长
 *   频率够高（≥几百Hz）人眼/设备看到的是"平均效果"
 *
 * 舵机应用（50Hz）：
 *   SG90 看高电平脉宽：0.5ms→0°、1.5ms→90°、2.5ms→180°
 *   1MHz 计数下 CCR = 脉宽µs（500~2500），角度换算在 main 里做
 */

#include "pwm.h"

/*==========================================================
 * 初始化 PWM 输出（PA0 = TIM2_CH1）
 *==========================================================*/
void Pwm_Init(void)
{
    /* 1. 开时钟：GPIO + TIM（和串口一样要两个） */
    PWM_GPIO_RCC_REG |= (1u << PWM_GPIO_RCC_BIT);   /* GPIOA */
    PWM_RCC_REG |= (1u << PWM_RCC_BIT);             /* TIM2 */

    /* 2. 配引脚：PA0 复用推挽 50MHz（0xB，和 USART TX 一个配置）
     *    ★复用：引脚控制权交给 TIM2 内部电路 */
    PWM_CH_PORT->CRL &= ~(0xF << (PWM_CH_PIN * 4));   /* 先清房间（PA0 房间 0） */
    PWM_CH_PORT->CRL |=  (0xB << (PWM_CH_PIN * 4));   /* 复用推挽 + 50MHz */

    /* 3. 时基（和定时中断一样）：PSC 分频、ARR 上限、CNT 清零 */
    PWM_TIM->PSC = PWM_PSC;
    PWM_TIM->ARR = PWM_ARR;
    PWM_TIM->CNT = 0;

    /* 4. 通道模式——CCMR1 的 OC1M 选 PWM 模式1
     *    OC1M[6:4] = 110 → PWM 模式1（CNT<CCR 输出有效）
     *    OC1PE(bit3) = 1 → 输出预装载（改 CCR 周期边界才生效，防波形撕裂）
     *    先清后写：清 3 位旧值再写 110 */
    PWM_TIM->CCMR1 &= ~(0x7 << 4);
    PWM_TIM->CCMR1 |=  (0x6 << 4);
    PWM_TIM->CCMR1 |=  (1u << 3);           /* OC1PE */

    /* 5. 输出使能——CCER 的 CC1E=1（不打开没波形！） */
    PWM_TIM->CCER |= (1u << 0);

    /* 6. CCR 初值 0（占空比 0 = 全灭） */
    PWM_TIM->CCR1 = 0;

    /* 7. 启动计数：CR1 的 CEN */
    PWM_TIM->CR1 |= (1u << 0);
}

/*==========================================================
 * 设置占空比（0 ~ PWM_ARR）
 * 本质：改 CCR1 比较值 → 硬件自动切换输出电平比例
 * 呼吸灯/舵机/电机调速全都只调这个函数
 *==========================================================*/
void Pwm_SetDuty(uint16_t duty)
{
    if (duty > PWM_ARR)                 /* 限幅：超出上限钳到上限（防乱传值） */
        duty = PWM_ARR;
    PWM_TIM->CCR1 = duty;               /* 写 CCR 即生效 */
}
