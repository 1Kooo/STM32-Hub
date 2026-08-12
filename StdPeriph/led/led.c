/**
 * @file    led.c
 * @brief   LED 驱动（标准外设库版，基于 STM32F10x 标准外设库）
 * @note    与 Register 版 API 完全一致，可对照学习：
 *          RCC_APB2PeriphClockCmd()  == RCC->APB2ENR 位操作
 *          GPIO_Init()               == CRL / CRH 配置
 *          GPIO_WriteBit()           == BSRR / ODR 操作
 *
 * 库函数封装了寄存器操作，底层实现可在 Keil 中
 * 右键函数名 -> Go To Definition Of 查看
 */

#include "led.h"

/*==========================================================
 * 私有函数：配置单个引脚为推挽输出
 *
 * 库函数内部完成：参数检查 -> 位编码换算 -> CRL/CRH 写入
 * 对应寄存器版 Led_ConfigPort 中的 APB2ENR + CRL/CRH 操作
 *==========================================================*/
static void Led_ConfigPort(GPIO_TypeDef *port, uint16_t pin, uint32_t rcc)
{
    GPIO_InitTypeDef gpio;

    /* 1. 开时钟：对应寄存器版 RCC->APB2ENR |= (1u << rccBit) */
    RCC_APB2PeriphClockCmd(rcc, ENABLE);

    /* 2. 配模式：对应寄存器版 CRL/CRH 先清后写
       GPIO_Mode_Out_PP = CNF 00（推挽输出）
       GPIO_Speed_50MHz = MODE 11（50MHz） */
    gpio.GPIO_Pin   = pin;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &gpio);
}

/* 私有：按编号写电平（level = 1 亮，0 灭） */
static void Led_Set(LedNum led, uint8_t level)
{
    switch (led)
    {
    case LED1:
        GPIO_WriteBit(LED1_PORT, LED1_PIN, (BitAction)level);
        break;
    case LED2:
        GPIO_WriteBit(LED2_PORT, LED2_PIN, (BitAction)level);
        break;
    default:
        break;
    }
}

/* 初始化所有 LED：开时钟 -> 推挽输出 -> 默认全灭 */
void Led_Init(void)
{
    Led_ConfigPort(LED1_PORT, LED1_PIN, LED1_RCC);
    Led_ConfigPort(LED2_PORT, LED2_PIN, LED2_RCC);

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
 * 翻转：读当前电平取反再写回（1 - 当前值 = 取反）
 * 对应寄存器版 ODR 异或操作：
 * GPIO_ReadOutputDataBit 读当前电平 == 读 ODR
 * GPIO_WriteBit 写回              == 写 ODR
 *==========================================================*/
void Led_Toggle(LedNum led)
{
    switch (led)
    {
    case LED1:
        GPIO_WriteBit(LED1_PORT, LED1_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED1_PORT, LED1_PIN)));
        break;
    case LED2:
        GPIO_WriteBit(LED2_PORT, LED2_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED2_PORT, LED2_PIN)));
        break;
    default:
        break;
    }
}
