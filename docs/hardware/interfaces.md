# Board Interface Map / 板卡接口表

The following mapping is derived from the final exported netlist. Check the installed image and connector orientation before wiring. T1 package pin numbers and Linux GPIO numbers are different identifiers; runtime multiplexing is image-dependent.

下表按最终导出网表整理。接线前核对运行系统与连接器朝向；T1 封装脚号与 Linux GPIO 编号不同，运行时复用由镜像配置决定。

| Function / 功能 | Port / Group | Pins / Connector |
|---|---|---|
| SPI NAND | SPI0 (PC2–PC7) | CLK/CS/MOSI/MISO/WP/HOLD = T1 pins 19/18/17/16/15/14; FEL key SW1 |
| Wi-Fi SDIO | SDC1 (PG0–PG5) | CLK/CMD/D0–D3, 33 Ω on CLK |
| BT UART | UART1 (PG6–PG9) | TX/RX/RTS/CTS |
| Optional module / 可选模块 | UART0 TX/RX = PE2/PE3, pins 35/33 | MX-02 socket U5; module-specific mode pins / 模式脚按具体模块核对 |
| Runtime log / 日志 | UART3 TX/RX = PB6/PB7, pins 80/79 | 1.25 mm 3P header U24 |
| MIPI DSI pairs / 差分对 | PD0–PD9, pins 55–64 | Data0, Data1, CLK, Data2, Data3 P/N → FPC2 |
| LCD control / 显示控制 | TE = PD21 pin 53; RST = PD20 pin 54 | FPC2 30P |
| Touch / 触摸 | INT/RST/SCL/SDA = PE5/PE4/PE0/PE1 | T1 pins 42/43/44/45 → FPC7 contacts 2/5/4/3 |
| Touch supply / 触摸供电 | FPC7 contact 1 GND, contact 6 3V3 | Contacts 7/8 additional GND pads / 附加接地焊盘 |
| Backlight PWM / 背光 | LCD_PWM = PD22, pin 52 | MT9201 EN |
| MIC | MIC_IN_3P/3N | pins 87/88 |
| Speaker / 扬声器 | HPOUTR → LM4871 | CN2 contacts 1/2 = VO−/VO+; neither is GND / 两端均非地 |
| Amplifier control / 功放控制 | PA_SHDN = PB2, pin 86 | U53 shutdown pin; R138 pull-down / 下拉 |
| User LED / key | LED1 = PD11, BTN1 = PD10 | LED4 / SW5 |
| USB0 OTG / USB1 signals | — | USB10 / USB9 Type-C; USB9 is the main power input / USB9 为主供电输入 |
| Reset | RESET pin 27 | SW2 + 10 kΩ pull-up + TVS |

[Hardware guide / 硬件指南](README.md) · [Circuit handbook / 电路手册](../../hardware/docs/hardware-design-guide.md) · [Schematic / 原理图](../../hardware/schematic/AuraBar-Schematic-v1.0.pdf) · [Netlist / 网表](../../hardware/schematic/netlist.net)
