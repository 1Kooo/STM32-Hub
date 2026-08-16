/**
 * @file    pwm.c
 * @brief   PWM 输出驱动（标准外设库版）——TIM2_CH1 @ PA0
 * @note    与 Register 版 API 完全一致，可逐行对照学习：
 *          RCC_APB1PeriphClockCmd()  == RCC->APB1ENR |= 位掩码
 *          GPIO_Init(AF_PP)          == CRL 复用推挽配置（0xB）
 *          TIM_TimeBaseInit()        == PSC/ARR/CNT 时基三件套
 *          TIM_OC1Init(PWM1)         == CCMR1 的 OC1M=110（PWM 模式1）
 *          TIM_OC1PreloadConfig()    == CCMR1 的 OC1PE（预装载，防波形撕裂）
 *          TIM_SetCompare1()         == 写 CCR1（定占空比）
 *          TIM_Cmd(ENABLE)           == CR1 的 CEN
 *
 * PWM 原理：CNT 从 0 数到 ARR，期间和 CCR 赛跑
 *   CNT <  CCR → 输出有效电平；CNT >= CCR → 输出无效电平
 *   占空比 = CCR / (ARR+1)
 */

#include "pwm.h"

/*==========================================================
 * 初始化 PWM 输出（PA0 = TIM2_CH1）
 *==========================================================*/
void Pwm_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef t;
    TIM_OCInitTypeDef oc;

    /* 1. 开时钟：GPIO + TIM（和串口一样要两个） */
    RCC_APB2PeriphClockCmd(PWM_GPIO_RCC, ENABLE);
    RCC_APB1PeriphClockCmd(PWM_RCC, ENABLE);

    /* 2. 配引脚：PA0 复用推挽 50MHz（★复用 = 控制权交给 TIM2） */
    gpio.GPIO_Pin   = PWM_CH_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PWM_CH_PORT, &gpio);

    /* 3. 时基：PSC 分频、ARR 上限（决定 PWM 频率） */
    t.TIM_Prescaler         = PWM_PSC;
    t.TIM_Period            = PWM_ARR;
    t.TIM_CounterMode       = TIM_CounterMode_Up;
    t.TIM_ClockDivision     = TIM_CKD_DIV1;
    t.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(PWM_TIM, &t);

    /* 4. 通道配置为 PWM 模式1（对应寄存器版 CCMR1 的 OC1M=110 + CCER 的 CC1E）
     *    TIM_OutputState_Enable = CC1E 输出使能
     *    TIM_Pulse = CCR 初值（占空比） */
    oc.TIM_OCMode      = TIM_OCMode_PWM1;        /* PWM 模式1：CNT<CCR 输出有效 */
    oc.TIM_OutputState = TIM_OutputState_Enable; /* 打开输出通道 */
    oc.TIM_Pulse       = 0;                      /* CCR 初值 0（全灭） */
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;    /* 有效电平 = 高 */
    TIM_OC1Init(PWM_TIM, &oc);

    /* 5. 输出预装载使能（对应寄存器版 OC1PE）——改 CCR 周期边界才生效 */
    TIM_OC1PreloadConfig(PWM_TIM, TIM_OCPreload_Enable);

    /* 6. 启动计数：CR1 的 CEN */
    TIM_Cmd(PWM_TIM, ENABLE);
}

/*==========================================================
 * 设置占空比（0 ~ PWM_ARR）
 * 本质：改 CCR1 比较值 → 硬件自动切换输出电平比例
 *==========================================================*/
void Pwm_SetDuty(uint16_t duty)
{
    if (duty > PWM_ARR)                 /* 限幅：超出上限钳到上限 */
        duty = PWM_ARR;
    TIM_SetCompare1(PWM_TIM, duty);     /* 写 CCR1 */
}
