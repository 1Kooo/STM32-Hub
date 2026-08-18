/**
 * @file    oled.c
 * @brief   OLED 显示驱动（SSD1306，128×64，I2C 接口）——应用层驱动
 * @note    ★分层思想：本驱动不碰寄存器、不碰库函数，
 *          所有字节收发只调 I2c_*（i2c 驱动管协议，oled 管显示业务）。
 *          所以本版与 Register/oled 代码完全相同——
 *          版本差异全被配对的 i2c 驱动吸收掉了（本版配 StdPeriph/i2c）。
 *
 * SSD1306 使用要点：
 *   1. 控制字节区分命令/数据：0x00 = 命令，0x40 = 数据
 *   2. 页寻址模式（0x20,0x02）：定位 = 页地址(0xB0+y) + 列高4位(0x10|) + 列低4位
 *   3. 页模式下连续写数据，列自动 +1，写满 128 列回卷到该页开头（不跨页）
 *   4. 字模：F8X16，16 字节/字符，前 8 字节上半页、后 8 字节下半页
 */

#include "oled.h"
#include "i2c.h"

#include "oled_font.h"     /* 8×16 ASCII 字库（独立文件，纯数据） */

/* 写命令：从机地址 + 控制字节 0x00（命令） + 命令本身 */
static void Oled_WriteCmd(uint8_t cmd)
{
    I2c_Start();
    I2c_SendByte(OLED_ADDR);        /* 0x78 = 7 位地址 0x3C 左移 1 位，写方向 */
    I2c_WaitAck();
    I2c_SendByte(0x00);             /* ★控制字节：0x00 = 命令 */
    I2c_WaitAck();
    I2c_SendByte(cmd);
    I2c_WaitAck();
    I2c_Stop();
}

/* 写数据：从机地址 + 控制字节 0x40（数据） + 数据本身 */
static void Oled_WriteData(uint8_t data)
{
    I2c_Start();
    I2c_SendByte(OLED_ADDR);
    I2c_WaitAck();
    I2c_SendByte(0x40);             /* ★控制字节：0x40 = 数据 */
    I2c_WaitAck();
    I2c_SendByte(data);
    I2c_WaitAck();
    I2c_Stop();
}

void Oled_Init(void)
{
    I2c_Init();                     /* 依赖 I2C 驱动（软件模拟，PB6/PB7） */

    Oled_WriteCmd(0xAE);            /* 关显示（配置期间防花屏） */
    Oled_WriteCmd(0xD5); Oled_WriteCmd(0x80);   /* 显示时钟分频/振荡频率 */
    Oled_WriteCmd(0xA8); Oled_WriteCmd(0x3F);   /* multiplex 比率 = 64（128×64 屏） */
    Oled_WriteCmd(0xD3); Oled_WriteCmd(0x00);   /* 显示偏移 = 0 */
    Oled_WriteCmd(0x40);            /* 显示起始行 = 0（★错位/上下漂移就查这里） */
    Oled_WriteCmd(0x8D); Oled_WriteCmd(0x14);   /* ★开电荷泵（内部升压，不开不亮） */
    Oled_WriteCmd(0x20); Oled_WriteCmd(0x02);   /* 内存寻址模式 = 页模式（最简单） */
    Oled_WriteCmd(0xA1);            /* 段重映射：列 0 在最左 */
    Oled_WriteCmd(0xC8);            /* COM 扫描方向：从上到下 */
    Oled_WriteCmd(0xDA); Oled_WriteCmd(0x12);   /* COM 引脚硬件配置（128×64 屏） */
    Oled_WriteCmd(0x81); Oled_WriteCmd(0xCF);   /* 对比度 */
    Oled_WriteCmd(0xD9); Oled_WriteCmd(0xF1);   /* 预充电周期 */
    Oled_WriteCmd(0xDB); Oled_WriteCmd(0x40);   /* VCOMH 取消选择电平 */
    Oled_WriteCmd(0xA4);            /* 恢复 RAM 内容显示 */
    Oled_WriteCmd(0xA6);            /* 正常显示（非反色） */
    Oled_WriteCmd(0xAF);            /* 开显示 */

    Oled_Clear();                   /* 上电清屏 */
}

void Oled_Clear(void)
{
    uint8_t page, col;
    for (page = 0; page < 8; page++) {      /* 8 页 */
        Oled_SetPos(0, page);
        for (col = 0; col < 128; col++)     /* 每页 128 列 */
            Oled_WriteData(0x00);           /* 全灭 */
    }
}

void Oled_SetPos(uint8_t x, uint8_t y)
{
    Oled_WriteCmd(0xB0 + y);                        /* 页地址 0xB0~0xB7 */
    Oled_WriteCmd(((x >> 4) & 0x0F) | 0x10);        /* 列地址高 4 位（0x10~0x17） */
    Oled_WriteCmd(x & 0x0F);                        /* 列地址低 4 位（0x00~0x0F） */
}

void Oled_ShowChar(uint8_t x, uint8_t y, char ch)
{
    uint8_t i;

    Oled_SetPos(x, y);                      /* 上半页 */
    for (i = 0; i < 8; i++)                 /* 字模前 8 字节 = 上半部分 */
        Oled_WriteData(F8X16[(ch - 0x20) * 16 + i]);

    Oled_SetPos(x, y + 1);                  /* ★下半页（y+1 才是下一页） */
    for (i = 8; i < 16; i++)                /* 字模后 8 字节 = 下半部分 */
        Oled_WriteData(F8X16[(ch - 0x20) * 16 + i]);
}

void Oled_ShowString(uint8_t x, uint8_t y, char *str)
{
    while (*str) {
        Oled_ShowChar(x, y, *str++);
        x += 8;                             /* 字符宽 8 像素 */
        if (x > 120) {                      /* 超行自动换行 */
            x = 0;
            y += 2;                         /* ★16 像素高 = 2 页 */
            if (y > 6) break;               /* 到底了就不写了 */
        }
    }
}
