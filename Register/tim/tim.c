/**
 * @file    tim.c
 * @brief   定时器驱动（寄存器版）——TIM2 定时中断，1ms 节拍
 * @note    与 StdPeriph 版对照学习：
 *          时基三件套 PSC/ARR/CNT  == TIM_TimeBaseInit()
 *          DIER 开中断             == TIM_ITConfig()
 *          NVIC_EnableIRQ          == NVIC_Init()
 *          SR 清标志               == TIM_ClearITPendingBit()
 *          中断服务逻辑两版完全一致（与寄存器无关）
 *
 * 时基单元三级节拍器：
 *   TIM时钟(72MHz) → ①PSC 分频(PSC+1) → ②CNT 每时钟 +1 → ③计到 ARR 溢出
 *   溢出 = 更新事件(UEV) → UIF 置位 → 进中断 → CNT 清零重来
 *
 * 为什么用 TIM 而不用 DWT 延时：
 *   延时是"死等"（阻塞）；定时中断是"后台跑"（非阻塞）——
 *   中断里 tick++，主循环只看 tick，就能同时干多件事（时间片轮询）
 */

#include "tim.h"

/* sTick：ms 计数，中断里 ++，主循环里读
 * ★volatile 必须加：这变量被中断函数改，不加的话编译器可能
 *   把它优化进寄存器（认为没人改），主循环读到永远旧值 */
static volatile uint32_t sTick = 0;

/*==========================================================
 * 初始化 TIM2 定时中断（1ms 一次）
 *==========================================================*/
void Tim_Init(void)
{
    /* 1. 开时钟 */
    TIM_RCC_REG |= (1u << TIM_RCC_BIT);

    /* 2. 时基三件套：PSC 分频、ARR 上限、CNT 清零
     *    ★PSC/ARR 都是 +1 法则（写 0 = 1 倍分频/计 1 个数） */
    TIM_X->PSC = TIM_PSC;
    TIM_X->ARR = TIM_ARR;
    TIM_X->CNT = 0;

    /* 3. 清更新标志 UIF（防止上电残留标志直接进中断） */
    TIM_X->SR &= ~(1u << 0);          /* SR bit0 = UIF，写 0 清除 */

    /* 4. 开更新中断：DIER bit0 = UIE */
    TIM_X->DIER |= (1u << 0);

    /* 5. NVIC 使能中断（ISER = 中断使能寄存器，TIM2_IRQn=28 → ISER[0] bit28） */
    NVIC_SetPriority(TIM_IRQn, TIM_IRQ_PRIO);   /* 设抢占优先级 */
    NVIC_EnableIRQ(TIM_IRQn);                   /* CMSIS 封装，内部就是 ISER 置位 */

    /* 6. 启动计数：CR1 bit0 = CEN */
    TIM_X->CR1 |= (1u << 0);
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
 * ★注意：如果 stm32f10x_it.c 里已有空的 TIM2_IRQHandler，
 *   要把它删掉，否则重复定义编译报错
 * ★必须清 UIF 标志：不清 = 一直认为"溢出" = 死循环进中断
 *==========================================================*/
void TIM2_IRQHandler(void)
{
    if (TIM_X->SR & (1u << 0))        /* 是更新中断吗（UIF=1）？ */
    {
        TIM_X->SR &= ~(1u << 0);      /* 清标志（写 0） */
        sTick++;                      /* 1ms 到了 */
    }
}
