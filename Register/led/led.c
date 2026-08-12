/**
 * @file    led.c
 * @brief   LED 驱动（寄存器版）
 * @note    直接操作寄存器，与 StdPeriph 版对照学习：
 *          RCC->APB2ENR       == RCC_APB2PeriphClockCmd()
 *          CRL / CRH 配置     == GPIO_Init()
 *          BSRR / ODR 操作    == GPIO_WriteBit()
 */

#include "led.h"

/*==========================================================
 * 私有函数：配置单个引脚为推挽输出
 *
 * 流程三步（所有外设通用套路）：
 *  1. 开时钟  —— 外设默认断电（时钟门控），不开时钟写寄存器无效
 *  2. 配模式  —— 在 CRL/CRH 中写入引脚的 MODE + CNF 配置
 *  3. 完成
 *
 * 引脚配置布局（F103）：
 *  CRL（32位）管引脚 0~7，房间起点 = pin × 4
 *  CRH（32位）管引脚 8~15，房间起点 = (pin - 8) × 4
 *  每个引脚 4 位：bit[1:0] = MODE（速度），bit[3:2] = CNF（模式）
 *  0x3 = 二进制 0011 = MODE=11（50MHz 输出）+ CNF=00（推挽输出）
 *
 * 必须先清后写：不先清零，房间里的旧配置会和新增量叠加
 *==========================================================*/
static void Led_ConfigPort(GPIO_TypeDef *port, uint8_t pinNum, uint8_t rccBit)
{
    uint32_t shift;

    /* 1. 开时钟：APB2ENR 第 rccBit 位置 1 */
    RCC->APB2ENR |= (1u << rccBit);

    /* 2. 选寄存器并算房间起点：0~7 用 CRL，8~15 用 CRH */
    if (pinNum < 8)
    {
        shift = pinNum * 4;
        port->CRL &= ~(0xF << shift);   /* 先清房间（4 位全置 0） */
        port->CRL |=  (0x3 << shift);   /* 后写：50MHz + 推挽输出 */
    }
    else
    {
        shift = (pinNum - 8) * 4;
        port->CRH &= ~(0xF << shift);   /* 先清房间 */
        port->CRH |=  (0x3 << shift);   /* 后写 */
    }
}

/*==========================================================
 * 私有函数：按编号写电平（level = 1 亮，0 灭）
 *
 * 用 BSRR 寄存器（原子操作）：
 *  低 16 位写 1 → 对应引脚输出高电平
 *  高 16 位写 1 → 对应引脚输出低电平
 *  单条指令完成，不会被中断打断，比 ODR 读-改-写更安全
 *==========================================================*/
static void Led_Set(LedNum led, uint8_t level)
{
    GPIO_TypeDef *port;
    uint8_t pin;

    switch (led)
    {
    case LED1: port = LED1_PORT; pin = LED1_PIN_NUM; break;
    case LED2: port = LED2_PORT; pin = LED2_PIN_NUM; break;
    default:  return;
    }

    if (level)
        port->BSRR = (1u << pin);           /* 输出高电平 */
    else
        port->BSRR = (1u << (pin + 16));    /* 输出低电平 */
}

/* 初始化所有 LED：开时钟 -> 推挽输出 -> 默认全灭 */
void Led_Init(void)
{
    Led_ConfigPort(LED1_PORT, LED1_PIN_NUM, LED1_RCC_BIT);
    Led_ConfigPort(LED2_PORT, LED2_PIN_NUM, LED2_RCC_BIT);

    Led_Off(LED1);
    Led_Off(LED2);
}

void Led_On(LedNum led)
{
    Led_Set(led, LED_ACTIVE_HIGH);
}

void Led_Off(LedNum led)
{
    Led_Set(led, !LED_ACTIVE_HIGH);
}

/*==========================================================
 * 翻转：读当前状态取反再写回
 * BSRR 只能写不能读，读当前状态必须用 ODR（可读可写），
 * 异或 1 实现 0↔1 翻转
 *==========================================================*/
void Led_Toggle(LedNum led)
{
    GPIO_TypeDef *port;
    uint8_t pin;

    switch (led)
    {
    case LED1: port = LED1_PORT; pin = LED1_PIN_NUM; break;
    case LED2: port = LED2_PORT; pin = LED2_PIN_NUM; break;
    default:  return;
    }

    port->ODR ^= (1u << pin);
}
