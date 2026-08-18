/**
 * @file    i2c.c
 * @brief   I2C 总线驱动（★硬件外设版）——寄存器操作
 * @note    核心差异（对比软件版 i2c/）：
 *          软件版：自己翻 GPIO 产生时序，没有"状态"概念
 *          硬件版：外设状态机自动产生时序，代码要"等标志"：
 *            SB   （SR1 bit0）起始条件已发送
 *            ADDR （SR1 bit1）地址已发送且被应答（★读 SR2 清除）
 *            BTF  （SR1 bit2）字节移位完成（数据已发出、ACK 已采样）
 *            TXE  （SR1 bit7）发送数据寄存器 DR 空（可以写下一个字节）
 *            AF   （SR1 bit4）无应答标志（从机没理我们）
 *
 * 发送一字节的事务流程（对应软件版四件套）：
 *   I2c_Start();            → 写 START 位 → 等 SB
 *   I2c_SendByte(addr);     → 等 TXE → 写 DR
 *   I2c_WaitAck();          → 等 BTF → 查 AF → 清 ADDR
 *   I2c_SendByte(data);     → 等 TXE → 写 DR
 *   I2c_WaitAck();          → 等 BTF → 查 AF
 *   I2c_Stop();             → 写 STOP 位（硬件自动出停止时序）
 */

#include "i2c.h"

/* 等 SR1 里某个标志置位 */
#define I2C_WAIT_FLAG(flag) while (!(I2C_HW_X->SR1 & (flag)))

/* 引脚所在寄存器（CRL=0~7 号，CRH=8~15 号），换实例自动适配 */
#if I2C_HW_SCL_PIN < 8
    #define I2C_HW_SCL_REG    I2C_HW_PORT->CRL
    #define I2C_HW_SCL_SHIFT  (I2C_HW_SCL_PIN * 4)
#else
    #define I2C_HW_SCL_REG    I2C_HW_PORT->CRH
    #define I2C_HW_SCL_SHIFT  ((I2C_HW_SCL_PIN - 8) * 4)
#endif

#if I2C_HW_SDA_PIN < 8
    #define I2C_HW_SDA_REG    I2C_HW_PORT->CRL
    #define I2C_HW_SDA_SHIFT  (I2C_HW_SDA_PIN * 4)
#else
    #define I2C_HW_SDA_REG    I2C_HW_PORT->CRH
    #define I2C_HW_SDA_SHIFT  ((I2C_HW_SDA_PIN - 8) * 4)
#endif

void I2c_Init(void)
{
    /* 1. 开时钟：GPIO（APB2ENR bit3）+ I2C（APB1ENR，实例宏定位）——★两个时钟 */
    RCC->APB2ENR |= (1u << 3);
    RCC->APB1ENR |= (1u << I2C_HW_RCC_BIT);

    /* 2. SCL/SDA 配复用开漏（CNF=11 + MODE=01 = 0x7）
     *    ★复用功能：引脚交给 I2C 外设接管，不再是普通 IO
     *    ★引脚硬件固定（I2C1=PB6/7、I2C2=PB10/11），换实例改 i2c.h 宏 */
    I2C_HW_SCL_REG &= ~((uint32_t)0xF << I2C_HW_SCL_SHIFT);
    I2C_HW_SCL_REG |=  (uint32_t)0x7 << I2C_HW_SCL_SHIFT;
    I2C_HW_SDA_REG &= ~((uint32_t)0xF << I2C_HW_SDA_SHIFT);
    I2C_HW_SDA_REG |=  (uint32_t)0x7 << I2C_HW_SDA_SHIFT;

    /* 3. CR2：FREQ = PCLK1 频率（MHz），外设用它算内部时序 */
    I2C_HW_X->CR2 = I2C_HW_FREQ;

    /* 4. CCR：时钟分频 → SCL = PCLK1 / (2 × CCR) = 100kHz */
    I2C_HW_X->CCR = I2C_HW_CCR;

    /* 5. TRISE：SCL 上升时间补偿（标准模式 = 1000ns 换算成时钟周期 +1） */
    I2C_HW_X->TRISE = I2C_HW_TRISE;

    /* 6. 使能：ACK（bit10，自动应答）+ PE（bit0，外设总开关） */
    I2C_HW_X->CR1 |= (1u << 10);        /* ACK */
    I2C_HW_X->CR1 |= (1u << 0);         /* PE */
}

void I2c_Start(void)
{
    I2C_HW_X->CR1 |= (1u << 8);         /* 写 START 位：硬件自动出起始时序 */
    I2C_WAIT_FLAG((1u << 0));           /* 等 SB：起始条件已发送 */
}

void I2c_Stop(void)
{
    I2C_HW_X->CR1 |= (1u << 9);         /* 写 STOP 位：硬件自动出停止时序 */
}

void I2c_SendByte(uint8_t data)
{
    I2C_WAIT_FLAG((1u << 7));           /* 等 TXE：DR 空了才能写（没空写=覆盖） */
    I2C_HW_X->DR = data;                /* 写 DR：硬件自动逐位移出，自动收 ACK */
}

uint8_t I2c_WaitAck(void)
{
    I2C_WAIT_FLAG((1u << 2));           /* 等 BTF：字节移位完成（ACK 阶段已结束） */

    if (I2C_HW_X->SR1 & (1u << 4)) {    /* AF 置位 = 从机没应答 */
        I2C_HW_X->SR1 &= ~(1u << 4);    /* 清 AF（只写 0 到位，不影响其它标志） */
        return 1;
    }

    if (I2C_HW_X->SR1 & (1u << 1))      /* ADDR 置位（地址阶段才有） */
        (void)I2C_HW_X->SR2;            /* ★读 SR2 清除 ADDR（双寄存器清法，必须） */

    return 0;                           /* 有应答 */
}
