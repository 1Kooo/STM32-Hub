/**
 * @file    key.c
 * @brief   按键驱动（标准外设库版，基于 STM32F10x 标准外设库）
 * @note    与 Register 版 API 完全一致，可对照学习：
 *          GPIO_Init(GPIO_Mode_IPU) == CRL/CRH + ODR 上拉配置
 *          GPIO_ReadInputDataBit()  == IDR 读取
 *          消抖与边沿检测逻辑        == 两版完全一致（与寄存器无关）
 *
 * 库函数封装了寄存器操作，底层实现可在 Keil 中
 * 右键函数名 -> Go To Definition Of 查看
 * （GPIO_Mode_IPU 内部 = CNF=10 + ODR 置位，就是 Register 版那两行）
 */

#include "key.h"

/*==========================================================
 * 私有函数：配置单个引脚为上拉输入
 *
 * GPIO_Mode_IPU = 上拉输入（Input Pull-Up）
 * 库函数内部完成：CNF=10（上拉/下拉模式）+ ODR 置 1（选上拉）
 * 对应 Register 版的 CRL |= 0x8 + ODR |= (1 << pin)
 *==========================================================*/
static void Key_ConfigPort(GPIO_TypeDef *port, uint16_t pin, uint32_t rcc)
{
    GPIO_InitTypeDef gpio;

    /* 1. 开时钟：对应寄存器版 RCC->APB2ENR |= (1u << rccBit) */
    RCC_APB2PeriphClockCmd(rcc, ENABLE);

    /* 2. 配模式：上拉输入（内部完成 ODR 置位，无需手动） */
    gpio.GPIO_Pin  = pin;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(port, &gpio);
}

/*==========================================================
 * 私有函数：消抖延时（约 ms 毫秒）
 * 空循环 + __NOP() 防止编译器把循环优化掉
 * 若工程已有延时函数（Delay_ms 等），可直接替换本函数
 *==========================================================*/
static void Key_DelayMs(uint16_t ms)
{
    uint16_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 7200; j++)
            __NOP();
}

/* 初始化所有按键：开时钟 -> 上拉输入 */
void Key_Init(void)
{
    Key_ConfigPort(KEY1_PORT, KEY1_PIN, KEY1_RCC);
    Key_ConfigPort(KEY2_PORT, KEY2_PIN, KEY2_RCC);
}

/*==========================================================
 * 读单个按键电平
 * 对应寄存器版：IDR 位读取后取反（按下=1）
 *==========================================================*/
uint8_t Key_Read(KeyNum key)
{
    switch (key)
    {
    case KEY1:
        return GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) ? 0 : 1;
    case KEY2:
        return GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) ? 0 : 1;
    default:
        return 0;
    }
}

/*==========================================================
 * 扫描按键（消抖 + 边沿检测）
 * 逻辑与 Register 版完全一致：
 *  变化 → 消抖 10ms → 确认 → 返回事件
 *==========================================================*/
KeyEvent Key_Scan(KeyNum key)
{
    static uint8_t lastState[2];   /* 上次稳定状态（支持 KEY1~KEY2） */
    uint8_t index = key - 1;
    uint8_t now;
    KeyEvent event = KEY_NONE;

    if (key < KEY1 || key > KEY2)
        return KEY_NONE;

    now = Key_Read(key);

    if (now != lastState[index])       /* 检测到电平变化 */
    {
        Key_DelayMs(10);               /* 消抖：等 10ms */
        now = Key_Read(key);           /* 再读确认 */

        if (now != lastState[index])   /* 变化被确认 */
        {
            lastState[index] = now;
            event = now ? KEY_DOWN : KEY_UP;
        }
    }
    return event;
}
