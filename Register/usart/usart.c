/**
 * @file    usart.c
 * @brief   串口驱动（寄存器版）——USART1 @ 115200，宏配置见 usart.h
 * @note    与 StdPeriph 版对照学习：
 *          Usart_ConfigGpio()       == GPIO_Init()（TX 复用推挽 / RX 浮空输入）
 *          BRR 计算 + CR1/CR2 配置  == USART_Init()（库函数内部也是这么算的）
 *          等 TXE / 等 RXNE          == USART_GetFlagStatus()
 *          写 DR / 读 DR             == USART_SendData() / USART_ReceiveData()
 *
 * 波特率计算原理（★核心）：
 *   USARTDIV = fPCLK / (16 × 波特率)
 *   = 72000000 / (16 × 115200) = 39.0625
 *   整数部分 39（0x27）→ BRR 高 12 位，小数部分 0.0625×16=1 → BRR 低 4 位
 *   最终 BRR = 0x271
 *
 * 发送/接收握手（对称的）：
 *   发送：等 TXE（发送寄存器空，可以塞新数据）→ 写 DR
 *   接收：等 RXNE（收到数据了）→ 读 DR
 *   写 DR = 发送，读 DR = 接收，一个字节在 DR 里进进出出
 */

#include "usart.h"

/*==========================================================
 * 私有函数：配 TX 引脚（复用推挽输出 50MHz）
 *
 * ★复用（AF）：
 *   普通输出时引脚归 GPIO 管；复用模式把引脚控制权借给
 *   外设（USART/SPI/TIM），由外设内部电路驱动引脚电平
 *
 * 引脚配置（和 LED/按键一样的"房间"套路）：
 *   pinNum < 8  → CRL，房间起点 = pinNum × 4
 *   pinNum >= 8 → CRH，房间起点 = (pinNum - 8) × 4
 *   每个房间 4 位 = MODE[1:0]（速度）+ CNF[1:0]（模式）
 *
 * TX 编码 0xB = 1011：
 *   MODE = 11 → 50MHz 输出
 *   CNF  = 10 → 复用推挽输出
 *==========================================================*/
static void Usart_SetTxPin(GPIO_TypeDef *port, uint16_t pinNum)
{
    volatile uint32_t *crReg;
    uint32_t shift;

    if (pinNum < 8)
    {
        crReg = &port->CRL;
        shift = pinNum * 4;
    }
    else
    {
        crReg = &port->CRH;
        shift = (pinNum - 8) * 4;
    }

    *crReg &= ~(0xF << shift);    /* 先清房间（否则新旧值叠加） */
    *crReg |=  (0xB << shift);    /* CNF=10 复用推挽 + MODE=11 50MHz */
}

/*==========================================================
 * 私有函数：配 RX 引脚（浮空输入）
 *
 * RX 编码 0x4 = 0100：
 *   MODE = 00 → 输入模式
 *   CNF  = 01 → 浮空输入（不拉高不拉低，靠外部电平）
 *   （也可以配上拉输入，但串口对端有驱动，浮空就够）
 *==========================================================*/
static void Usart_SetRxPin(GPIO_TypeDef *port, uint16_t pinNum)
{
    volatile uint32_t *crReg;
    uint32_t shift;

    if (pinNum < 8)
    {
        crReg = &port->CRL;
        shift = pinNum * 4;
    }
    else
    {
        crReg = &port->CRH;
        shift = (pinNum - 8) * 4;
    }

    *crReg &= ~(0xF << shift);    /* 先清房间 */
    *crReg |=  (0x4 << shift);    /* CNF=01 浮空输入 + MODE=00 输入 */
}

/*==========================================================
 * 初始化串口（配置五步，套路固定）
 *==========================================================*/
void Usart_Init(void)
{
    /* 1. 开时钟：GPIO + USART 两个都要开！
     *    ★易错点：只开 USART 时钟，配引脚是无效的（时钟关着寄存器不工作）
     *    （点灯只开 GPIO 时钟，串口要开俩——和点灯不一样！） */
    USART_GPIO_RCC_REG |= (1u << USART_GPIO_RCC_BIT);  /* GPIO 时钟 */
    USART_RCC_REG |= (1u << USART_RCC_BIT);            /* USART 时钟 */

    /* 2. 配引脚：TX 复用推挽 50MHz，RX 浮空输入 */
    Usart_SetTxPin(USART_TX_PORT, USART_TX_PIN);
    Usart_SetRxPin(USART_RX_PORT, USART_RX_PIN);

    /* 3. 波特率：BRR 按频率自动算（不用查表）
     *    公式：USARTDIV = fPCLK / (16 × 波特率)
     *    整数部分左移 4 位，小数部分 ×16 取整拼到低 4 位
     *    ★移植时频率别填错：U1=72MHz，U2/U3=36MHz，填错波特率就乱码 */
    {
        float usartDiv = (float)USART_FCK / (16u * USART_BAUD);
        uint16_t mantissa = (uint16_t)usartDiv;                     /* 整数部分 */
        uint16_t fraction = (uint16_t)((usartDiv - mantissa) * 16); /* 小数 ×16 */
        USART_X->BRR = (uint16_t)((mantissa << 4) | fraction);
    }

    /* 4. 数据格式：
     *    CR1：M=0 → 8 位数据（bit12），PCE=0 → 无校验（bit10）
     *    CR2：STOP=00 → 1 位停止位（bit13:12）
     *    （8N1 = 8 位数据 + 无校验 + 1 位停止位，串口最常见格式） */
    USART_X->CR1 &= ~(1u << 12);    /* M = 0：8 位数据 */
    USART_X->CR1 &= ~(1u << 10);    /* PCE = 0：无校验 */
    USART_X->CR2 &= ~(0x3 << 12);   /* STOP = 00：1 位停止位 */

    /* 5. 使能：UE=bit13 总开关、TE=bit3 发送、RE=bit2 接收 */
    USART_X->CR1 |= (1u << 13) | (1u << 3) | (1u << 2);
}

/*==========================================================
 * 发送一个字节
 * 流程：等 TXE（发送寄存器空，上一个字节被硬件搬进移位寄存器）→ 写 DR
 * 写 DR 即触发硬件按波特率逐位移出（起始位/数据/停止位硬件自动加）
 *==========================================================*/
void Usart_SendByte(uint8_t data)
{
    while (!(USART_X->SR & (1u << 7)));   /* 等 TXE=1（bit7） */
    USART_X->DR = data;                   /* 写 DR 即发送 */
}

/*==========================================================
 * 发送字符串：逐字符发送，遇到 '\0' 结束
 *==========================================================*/
void Usart_SendString(char *str)
{
    while (*str)
    {
        Usart_SendByte((uint8_t)*str);
        str++;
    }
}

/*==========================================================
 * 接收一个字节（阻塞等待）
 * 流程：等 RXNE（收到数据，硬件把字节搬进 DR）→ 读 DR（读自动清标志）
 * ★阻塞的含义：对方不发数据，程序就卡在这一行——等接收时干不了别的
 *  （这就是以后学中断/时间片轮询的动机：轮询的短板）
 *==========================================================*/
uint8_t Usart_ReceiveByte(void)
{
    while (!(USART_X->SR & (1u << 5)));   /* 等 RXNE=1（bit5） */
    return (uint8_t)(USART_X->DR & 0xFF); /* 只取低 8 位 */
}
