/**
 * @file    tim.c
 * @brief   定时器驱动（标准外设库版）——TIM2 定时中断，1ms 节拍
 * @note    与 Register 版 API 完全一致，可逐行对照学习：
 *          RCC_APB1PeriphClockCmd()  == RCC->APB1ENR |= 位掩码
 *          TIM_TimeBaseInit()        == PSC/ARR/CNT 时基三件套（含 PSC+1/ARR+1 法则）
 *          TIM_ITConfig()            == DIER 开中断
 *          NVIC_Init()               == NVIC_EnableIRQ + NVIC_SetPriority
 *          TIM_ClearITPendingBit()   == SR 清标志
 *          TIM_Cmd(ENABLE)           == CR1 的 CEN
 *          中断服务逻辑两版完全一致（与寄存器无关）
 *
 * ★库版的两个注意点：
 *   ① TIM_TimeBaseInit 内部会触发一次更新事件（UIF 置位），
 *      必须在开中断前清一次标志，否则上电立即进一次中断
 *   ② ISR 里必须 TIM_ClearITPendingBit 清标志，否则重复进中断
 */

#include "tim.h"

/* sTick：ms 计数，中断里 ++，主循环里读（★volatile 必须加） */
static volatile uint32_t sTick = 0;

/*==========================================================
 * 初始化 TIM2 定时中断（1ms 一次）
 *==========================================================*/
void Tim_Init(void)
{
    TIM_TimeBaseInitTypeDef t;
    NVIC_InitTypeDef n;

    /* 1. 开时钟（对应寄存器版 APB1ENR 置位） */
    RCC_APB1PeriphClockCmd(TIM_RCC, ENABLE);

    /* 2. 时基配置（对应寄存器版 PSC/ARR/CNT 三件套）
     *    ★PSC/ARR 填的是"实际值 - 1"（+1 法则，库内部写寄存器） */
    t.TIM_Prescaler         = TIM_PSC;              /* 预分频 */
    t.TIM_Period            = TIM_ARR;              /* 计数上限 */
    t.TIM_CounterMode       = TIM_CounterMode_Up;   /* 向上计数 */
    t.TIM_ClockDivision     = TIM_CKD_DIV1;         /* 时钟分频（滤波用，不用管） */
    t.TIM_RepetitionCounter = 0;                    /* 仅高级定时器用，通用填 0 */
    TIM_TimeBaseInit(TIM_X, &t);

    /* 3. ★清更新标志（TimeBaseInit 产生过一次更新事件，不清理会马上进中断） */
    TIM_ClearFlag(TIM_X, TIM_FLAG_Update);

    /* 4. 开更新中断（对应寄存器版 DIER 的 UIE） */
    TIM_ITConfig(TIM_X, TIM_IT_Update, ENABLE);

    /* 5. NVIC 使能（对应寄存器版 NVIC_EnableIRQ + SetPriority） */
    n.NVIC_IRQChannel                   = TIM_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = TIM_IRQ_PRIO;  /* 抢占优先级 */
    n.NVIC_IRQChannelSubPriority        = 0;             /* 子优先级 */
    n.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&n);

    /* 6. 启动计数（对应寄存器版 CR1 的 CEN） */
    TIM_Cmd(TIM_X, ENABLE);
}

/*==========================================================
 * 读毫秒计数（1ms 中断里 +1）
 *==========================================================*/
uint32_t Tim_Tick(void)
{
    return sTick;
}

/*==========================================================
 * 中断服务函数
 * ★注意：如果 stm32f10x_it.c 里已有空的 TIM2_IRQHandler，删掉它
 *==========================================================*/
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM_X, TIM_IT_Update) != RESET)  /* 是更新中断？ */
    {
        TIM_ClearITPendingBit(TIM_X, TIM_IT_Update);     /* ★清标志 */
        sTick++;
    }
}
