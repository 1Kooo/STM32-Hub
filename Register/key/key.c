/**
 * @file    key.c
 * @brief   按键驱动（寄存器版）
 * @note    与 StdPeriph 版对照学习：
 *          CRL/CRH + ODR 上拉配置 == GPIO_Init(GPIO_Mode_IPU)
 *          IDR 读取               == GPIO_ReadInputDataBit()
 *          消抖与边沿检测逻辑      == 两版完全一致（与寄存器无关）
 */

#include "key.h"

/*==========================================================
 * 私有函数：配置单个引脚为上拉输入
 *
 * 输入模式配置（F103）：
 *  4 位 = MODE[1:0] + CNF[1:0]
 *  MODE = 00（输入模式）
 *  CNF  = 10（上拉/下拉输入模式）→ 具体上拉还是下拉由 ODR 决定！
 *  组合值 = 0x8（二进制 1000）
 *  （CNF=11 即 0xC 是输出模式的"复用开漏"，输入模式下不合法）
 *
 * ⚠ 关键细节：ODR 在输入模式下是"上下拉选择器"
 *   ODR 对应位 = 1 → 上拉输入
 *   ODR 对应位 = 0 → 下拉输入（复位默认值！）
 *   只写 CNF=10 而不管 ODR → 实际是下拉输入 → 悬空读到低电平
 *   导致"上电误判按下"或"按键按下无变化"两个经典坑
 *
 * 上拉效果：按键一端接 GND，平时引脚被内部电阻拉到 3.3V（高），
 * 按下时引脚被拉到 GND（低）→ 读 IDR 取反后"按下=1"
 *==========================================================*/
static void Key_ConfigPort(GPIO_TypeDef *port, uint8_t pinNum, uint8_t rccBit)
{
    uint32_t shift;

    /* 1. 开时钟：APB2ENR 第 rccBit 位置 1 */
    RCC->APB2ENR |= (1u << rccBit);

    /* 2. 选寄存器并算房间起点：0~7 用 CRL，8~15 用 CRH */
    if (pinNum < 8)
    {
        shift = pinNum * 4;
        port->CRL &= ~(0xF << shift);   /* 先清房间 */
        port->CRL |=  (0x8 << shift);   /* 后写：MODE=00 + CNF=10（上下拉模式） */
        port->ODR |= (1u << pinNum);    /* ★ ODR 置 1 → 上拉输入 */
    }
    else
    {
        shift = (pinNum - 8) * 4;
        port->CRH &= ~(0xF << shift);
        port->CRH |=  (0x8 << shift);   /* CNF=10（上下拉模式） */
        port->ODR |= (1u << pinNum);    /* ★ ODR 置 1 → 上拉输入 */
    }
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
    Key_ConfigPort(KEY1_PORT, KEY1_PIN_NUM, KEY1_RCC_BIT);
    Key_ConfigPort(KEY2_PORT, KEY2_PIN_NUM, KEY2_RCC_BIT);
}

/*==========================================================
 * 读单个按键电平
 * IDR = 输入数据寄存器（读引脚当前电平，0~15 位对应引脚）
 * 按键按下为低电平，取反后对外统一"按下=1"
 *==========================================================*/
uint8_t Key_Read(KeyNum key)
{
    GPIO_TypeDef *port;
    uint8_t pin;

    switch (key)
    {
    case KEY1: port = KEY1_PORT; pin = KEY1_PIN_NUM; break;
    case KEY2: port = KEY2_PORT; pin = KEY2_PIN_NUM; break;
    default:  return 0;
    }

    return (port->IDR & (1u << pin)) ? 0 : 1;
}

/*==========================================================
 * 扫描按键（消抖 + 边沿检测）
 *
 * 流程：
 *  1. 读当前电平，和"上次稳定状态"比较
 *  2. 有变化 → 延时 10ms 消抖 → 再读确认
 *  3. 确认变化 → 更新状态 → 返回事件（按下/松开）
 *
 * 为什么返回"事件"而不是电平：
 *  电平 = 实时状态（按住期间一直返回 1）
 *  事件 = 变化瞬间（按下只返回一次 KEY_DOWN）
 *  没有边沿检测，main 循环里按一下会误触 N 次
 *
 * static 数组：记住每个按键的上次状态，函数退出不丢失
 * 按键编号从 1 开始，数组索引用 key-1
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
