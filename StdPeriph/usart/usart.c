/**
 * @file    usart.c
 * @brief   串口驱动（标准外设库版）——USART1 @ 115200，宏配置见 usart.h
 * @note    与 Register 版 API 完全一致，可逐行对照学习：
 *          RCC_APB2PeriphClockCmd()  == RCC->APB2ENR |= 位掩码
 *          GPIO_Init()               == CRL/CRH 房间配置
 *          USART_Init()              == BRR 计算 + CR1/CR2 格式配置（波特率自动算）
 *          USART_Cmd(ENABLE)         == CR1 |= UE|TE|RE
 *          USART_GetFlagStatus()     == 读 SR 标志位
 *          USART_SendData()          == 写 DR
 *          USART_ReceiveData()       == 读 DR
 *          等待逻辑两版完全一致（与寄存器无关）
 *
 * ★库常量和寄存器位值的关系（看库源码会发现）：
 *   USART_FLAG_TXE = 0x0080 = SR 的 bit7
 *   USART_FLAG_RXNE = 0x0020 = SR 的 bit5
 *   USART_Mode_Tx   = 0x0008 = CR1 的 bit3（TE）
 *   USART_Mode_Rx   = 0x0004 = CR1 的 bit2（RE）
 *   —— 库 = 寄存器封装 + 参数检查 + 人话接口，就这么回事
 */

#include "usart.h"

/*==========================================================
 * 私有函数：配置串口引脚
 * TX：复用推挽输出 50MHz（对应寄存器版 0xB：CNF=10 + MODE=11）
 * RX：浮空输入（对应寄存器版 0x4：CNF=01 + MODE=00）
 *==========================================================*/
static void Usart_ConfigGpio(void)
{
    GPIO_InitTypeDef gpio;

    /* TX 引脚：复用推挽（★复用 = 控制权交给 USART） */
    gpio.GPIO_Pin   = USART_TX_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;     /* 复用推挽输出 */
    gpio.GPIO_Speed = GPIO_Speed_50MHz;    /* 50MHz */
    GPIO_Init(USART_TX_PORT, &gpio);

    /* RX 引脚：浮空输入（输入模式，GPIO_Speed 无效可以不写） */
    gpio.GPIO_Pin   = USART_RX_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;   /* 浮空输入 */
    GPIO_Init(USART_RX_PORT, &gpio);
}

/*==========================================================
 * 初始化串口（五步，和寄存器版一一对应）
 *==========================================================*/
void Usart_Init(void)
{
    USART_InitTypeDef usart;

    /* 1. 开时钟：GPIO + USART 两个都要开（对应寄存器版 APB2ENR 置两个位） */
    RCC_APB2PeriphClockCmd(USART_GPIO_RCC | USART_RCC, ENABLE);

    /* 2. 配引脚 */
    Usart_ConfigGpio();

    /* 3. 串口参数（对应寄存器版 BRR 计算 + CR1/CR2 格式配置）
     *    ★波特率库函数自动算：内部就是 USARTDIV = fck / (16 × baud)
     *    8N1：8 位数据、无校验、1 位停止位 */
    usart.USART_BaudRate            = USART_BAUD;
    usart.USART_WordLength          = USART_WordLength_8b;   /* 8 位数据 */
    usart.USART_StopBits            = USART_StopBits_1;      /* 1 位停止位 */
    usart.USART_Parity              = USART_Parity_No;       /* 无校验 */
    usart.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx; /* 收发都开 */
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART_X, &usart);

    /* 4. 使能串口（对应寄存器版 CR1 |= UE，与 TE/RE 由 Mode 配置写入） */
    USART_Cmd(USART_X, ENABLE);
}

/*==========================================================
 * 发送一个字节：等 TXE（发送寄存器空）→ 写 DR
 *==========================================================*/
void Usart_SendByte(uint8_t data)
{
    /* USART_GetFlagStatus 返回 SET/RESET，RESET = 标志没置位 = 还没空 */
    while (USART_GetFlagStatus(USART_X, USART_FLAG_TXE) == RESET);
    USART_SendData(USART_X, data);   /* 写 DR */
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
 * 接收一个字节（阻塞等待）：等 RXNE（收到数据）→ 读 DR
 *==========================================================*/
uint8_t Usart_ReceiveByte(void)
{
    while (USART_GetFlagStatus(USART_X, USART_FLAG_RXNE) == RESET);
    return (uint8_t)USART_ReceiveData(USART_X);   /* 读 DR（读自动清标志） */
}
