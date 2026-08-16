# STM32-Hub

STM32 常用外设驱动与函数封装集合，**标准外设库（SPL）+ HAL + 寄存器版** 三套并收。

- 驱动代码直接拷贝进 Keil 工程即可使用，按需修改头文件里的引脚宏配置
- 三套版本 API 完全一致，换版本只换文件、不改调用代码
- 寄存器版带详细注释，作为学习底层原理的对照材料

## 支持平台

| 平台 | 状态 |
|---|---|
| STM32F103C8T6 | ✅ 默认平台 |
| 其他 STM32 系列 | 🚧 待适配 |

## 目录结构

```
STM32-Hub/
├── StdPeriph/      # 标准外设库版（Keil MDK + SPL）
│   ├── led/        #   LED 驱动
│   ├── key/        #   按键驱动
│   ├── usart/      #   串口驱动（宏配置可移植，115200）
│   ├── tim/        #   定时器驱动（1ms 定时中断 tick）
│   ├── pwm/        #   PWM 输出驱动（呼吸灯/舵机）
│   └── servo/      #   舵机驱动（应用层，复用 PWM）
├── HAL/            # HAL 库版（CubeMX + HAL）
│   ├── led/
│   ├── key/
│   ├── usart/
│   ├── tim/
│   ├── pwm/
│   └── servo/
└── Register/       # 寄存器版（学习用，逐行注释原理）
    ├── led/
    ├── key/
    ├── usart/
    ├── tim/
    ├── pwm/
    └── servo/
```

## 收录清单

| 外设 | 说明 | StdPeriph | HAL | Register |
|---|---|---|---|---|
| LED | 多路 LED 点亮/熄灭/翻转，极性可配 | ✅ | 🚧 待开发 | ✅ |
| 按键 | 上拉输入 + 消抖 + 边沿检测 | ✅ | 🚧 待开发 | ✅ |
| 串口 USART | 收发 + printf 重定向，宏配置可移植（U1/U2/U3） | ✅ | 🚧 待开发 | ✅ |
| 定时器 TIM | 1ms 定时中断 tick，时间片轮询基石（宏配置可移植） | ✅ | 🚧 待开发 | ✅ |
| PWM | 占空比输出（呼吸灯/舵机/电机），宏配置可移植 | ✅ | 🚧 待开发 | ✅ |
| 舵机 Servo | 角度控制（应用层驱动，复用 PWM，两版同代码） | ✅ | 🚧 待开发 | ✅ |

## 三版关系说明

```
main.c（业务代码，调用 Led_On(LED1) 等，三版通用）
  ├── StdPeriph 版  → 调用 ST 标准外设库函数
  ├── HAL 版        → 调用 HAL 库函数（初始化可由 CubeMX 生成）
  └── Register 版   → 直接操作寄存器（不依赖任何库）
```

各版底层对比（以点灯为例）：

| 操作 | StdPeriph 版 | Register 版 |
|---|---|---|
| 开时钟 | `RCC_APB2PeriphClockCmd()` | `RCC->APB2ENR \|= (1u << bit)` |
| 配模式 | `GPIO_Init()` | `CRL/CRH` 先清后写 |
| 输出电平 | `GPIO_WriteBit()` | `BSRR` |
| 翻转 | `GPIO_ReadOutputDataBit()` | `ODR ^=` |
| 串口配置 | `USART_Init()`（波特率自动算） | 五步：双时钟→引脚→BRR→格式→使能 |
| 串口收发 | `USART_SendData()` / `USART_ReceiveData()` | 等 TXE/RXNE → 写/读 DR |

## 使用方法

1. 将对应版本的 `.c` / `.h` 文件拷贝进 Keil 工程（如 `StdPeriph/led/`）
2. 在头文件中修改引脚宏配置（端口、引脚号、极性）
3. 在 `main.c` 中调用初始化函数后即可使用

## 代码规范

- 变量命名：小驼峰（如 `curBattery`）
- 函数命名：大驼峰（如 `Led_Init()`）
- 宏/常量：全大写 + 下划线（如 `LED1_PIN`）
- 注释：中文
- 每个 `.c` 配一个 `.h`
