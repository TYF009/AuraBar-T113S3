# AuraBar Hardware Design Guide / 硬件设计指南

**English first; Chinese translation follows each section. / 英文为主，每节附中文对照。**

Project: AuraBar (internal project name `6.22_Screen`)
SoC: Allwinner T113-S3 (dual Cortex-A7, HiFi4 DSP, 128 MB DDR3 in-package)
EDA: LCEDA Pro (嘉立创EDA专业版), project `6.22_Screen.eprj2`, Board1 / Schematic1, v1.0
Schematic pages: T113S3 · NET · POWER · CONNECT · AUDIO · FLASH · DISPLAY

> **Revision notes / 版本迭代说明**
> The final BOM/PDF supersedes a few early reference screenshots: the amplifier is **LM4871** (not LM4890S), the backlight boost is **MT9201** (not SY7200AABC), and all four power rails use **SY8088AAC** (the early 1.8 V rail used an XC6206 LDO). This guide describes the final design.
>
> 最终 BOM 与原理图以本指南为准：功放最终为 **LM4871**（早期截图为 LM4890S），背光升压为 **MT9201**（早期框图为 SY7200AABC），四路电源全部采用 **SY8088AAC**（早期 1.8V 用 XC6206 LDO）。

---

## 1. SoC Selection / 主控选型

**EN**

The T113-S3 was chosen for two reasons:

1. **Accessible assembly and rework** — the LQFP128 package supports practical soldering, inspection and rework during prototype development, reducing the effort needed to validate the board.
2. **Strong configuration** — dual Cortex-A7 cores, one HiFi4 DSP, 128 MB DDR3 integrated in-package, 3 ADCs, 2 DACs, 2 I2S/PCM interfaces, MIPI DSI display output, and an integrated audio codec.

Datasheet: https://linux-sunxi.org/images/7/73/T113-s3_datasheet_v1.6.pdf

![T113-S3 feature list](images/t113-s3-features.png)

![T113-S3 official block diagram](images/t113-s3-block-diagram.png)

**中文**

选择 T113-S3 主要有两方面考虑：

1. **便于装配与返修** —— LQFP128 封装便于原型阶段焊接、检查和返修，降低板卡验证的实施门槛；
2. **配置不错** —— Cortex-A7 双核、单核 HiFi4 DSP、内置 128MB DDR3、3 个 ADC、2 个 DAC、2 路 I2S/PCM、MIPI DSI 显示输出、集成音频 codec。

数据手册：https://linux-sunxi.org/images/7/73/T113-s3_datasheet_v1.6.pdf

The custom multi-part symbol for the SoC is built from the datasheet pinout:
按数据手册引脚绘制的主控元件符号如下：

![SoC symbol](images/mcu-symbol-drawing.png)

---

## 2. Minimum System / 最小系统

**EN** The minimum system consists of four parts: clock circuit, reset circuit, power circuit, and flash circuit.

**中文** 最小系统包括四部分：时钟电路、复位电路、电源电路、Flash 电路。

### 2.1 Clock Circuit / 时钟电路

The T113 requires two crystals: a main crystal and a low-speed (RTC) crystal. The SoC can also generate and fan out a clock for other peripherals (unused here).
T113 有两个晶振要求：主晶振和低速晶振；芯片也可产生时钟给其它外设（扇出功能本设计未使用）。

#### 2.1.1 Main clock, 24 MHz / 主时钟

**EN**

Per the hardware design guide, the T113 module supports only a **24 MHz** main crystal, connected across **DXIN / DXOUT (pins 22–23)**. The smaller the frequency tolerance the better, since the output stays closer to its nominal value; a **±10 ppm** crystal is selected. The load capacitors match the crystal (12 pF), with some margin for IC and trace parasitics; they can be tuned after bring-up.

- Crystal: YXC XL2EL89COI-111YLC-24M (LCSC **C5444545**), 24 MHz, CL = 12 pF, ±10 ppm, −40~85 °C
- Load capacitors: 12 pF 0603 1% (LCSC **C519482**), C30 / C123

The main clock also supports fan-out through REFCLK-OUT for external modules (unused). When fan-out is used the tolerance should be ≤ ±10 ppm; without fan-out ≤ ±20 ppm is acceptable.

**中文**

参考硬件设计指南，T113 模块只支持 **24MHz** 主时钟，接至 **DXIN、DXOUT（22、23 号引脚）**。晶振频偏越小越好——频偏越小，输出频率越接近额定频率，因此选 ±10ppm。匹配电容按晶振选型取 12pF，并需考虑 IC 内部与走线寄生电容，后续可按实际时钟效果调整。

- 晶振：扬兴 XL2EL89COI-111YLC-24M（立创 **C5444545**），24MHz、负载 12pF、±10ppm、−40~85℃
- 匹配电容：12pF 0603 1%（立创 **C519482**），C30 / C123

主时钟还可通过 REFCLK-OUT 扇出给外部模块（本设计未用）：需要扇出时晶振频偏应 ≤±10ppm，不需要扇出时 ≤±20ppm 即可。

![Main 24 MHz clock](images/clock-main-24m.png)

#### 2.1.2 Low-speed clock, 32.768 kHz / 低速时钟

**EN**

The RTC uses a **32.768 kHz** crystal across **X32KIN / X32KOUT (pins 24–25)**. A **10 MΩ** resistor is wired across the crystal to allow fine frequency trimming.

- Crystal: YXC X321532768KGD2SI (LCSC **C620155**), X2
- 10 MΩ across-crystal resistor: LCSC **C7250**, R14
- Load capacitors: 12 pF (LCSC C519482), C120 / C121

**中文**

低速时钟使用 **32.768kHz** 晶振，接 **X32KIN、X32KOUT（24、25 号引脚）**。晶振两端跨接一只 **10MΩ** 电阻，可用于频率微调。

- 晶振：扬兴 X321532768KGD2SI（立创 **C620155**），X2
- 跨接电阻 10MΩ：立创 **C7250**，R14
- 匹配电容 12pF（C519482），C120 / C121

![RTC crystal](images/clock-rtc-32k-schematic.png)
![Clock pins](images/clock-rtc-pins.png)

### 2.2 Reset / 系统复位

**EN**

The RESET signal (pin 27) is pulled up to VCC-IO (VCC-RTC is also acceptable) through a resistor; the official guide recommends 100 kΩ, but **10 kΩ (R13, LCSC C25804)** is used here to reduce resistor variety. RC and TVS devices are kept so the reset line is not disturbed during system-level ESD testing.

- A manual reset key **SW2** (TS-1088, LCSC C720477) is added for convenience.
- Capacitor C41 = 100 nF (LCSC C14663).
- ESD/TVS diode U41 = ESD5451N (LCSC **C2936977**).

**中文**

系统复位信号（27 脚）可通过电阻上拉到 VCC-IO（或 VCC-RTC），官方建议 100kΩ；这里为减少元器件种类选 **10kΩ（R13，C25804）**。产品设计保留 RC 与 TVS 器件，防止系统级静电测试时复位被干扰。

- 增加手动复位按键 **SW2**（TS-1088，C720477）；
- 电容 C41 = 100nF（C14663）；
- ESD 管 U41 = ESD5451N（**C2936977**）。

![Reset circuit](images/reset-circuit.png)

### 2.3 Power Design / 电源设计

**EN**

Unlike a simple MCU, the T113-S3 contains many modules that need independent rails: VDD-CORE / VDD-SYS at **0.9 V**, DRAM at **1.5 V** (note: T113-**S3** DRAM uses 1.5 V while T113-**i** uses 1.8 V — do not change this), the audio domain at 1.8 V, and I/O at 3.3 V. The 1.2 V rail belongs to the CSI module which is unused, so it is omitted.

| Rail / 电源轨 | Current / 电流 | Feeds / 供电对象 |
|---|---|---|
| 0.9 V | 2 A | VDD-CPU (CORE), VDD-SYS |
| 1.5 V | 1 A | VCC-DRAM (internal DDR3) |
| 1.8 V | ~0.4 A | VCC-PLL, VDD18-DRAM, VCC-LVDS, VCC-RTC, HPVCC |
| 3.3 V | 2 A | VCC-IO/PD/PE/PF/PG/PC, WiFi, LCD I/O, etc. |

The official EVB power tree (12 V adapter → first-stage DC/DC → 5 V bus → multiple bucks; plus the SoC's internal LDOA/LDOB) is shown for reference. AuraBar is powered from 5 V USB, so the 12 V adapter stage is not needed.
官方 EVB 电源树（12V 适配器→一级 DC/DC→5V 母线→多路降压，另有 SoC 内部 LDOA/LDOB）仅作参考；AuraBar 由 USB 5V 供电，不需要 12V 适配级。

![EVB power tree reference](images/power-tree-evb-reference.png)

**EN** There are two common ways to generate the rails:

1. **Integrated PMIC**, e.g. the EA3059 with four independently configurable outputs — compact but harder to hand-solder.
2. **Discrete DC/DC bucks**, e.g. SY8088AAC — chosen here for hand soldering.

**中文** 电源产生一般有两种方案：

1. **集成电源芯片**（如 EA3059，支持 4 路独立输出）—— 紧凑但不好手工焊接；
2. **四路独立降压芯片**（如 SY8088AAC）—— 为方便焊接，选择本方案。

![Integrated PMIC option (not used)](images/power-pmic-option.png)

**EN**

The 1.8 V rail draws little current and could use an XC6206P182MR LDO, while the other rails use SY8088AAC (1 A) / SY8089AAAC (2 A). In the final design **all four rails use SY8088AAC (U1–U4)** — verified to work in practice.

- Inductor: 2.2 µH (LCSC C383369 class; BOM uses MGFL2012F2R2MT, L1–L4)
- Output capacitor: 10 µF / 25 V 0603 (LCSC C96446 class)
- Output voltage divider: **Vout = 0.6 × (1 + RH/RL)**

| Rail | IC | RH (top) / RL (bottom) |
|---|---|---|
| 5 V → 3.3 V | U1 SY8088AAC | R1 150 kΩ / R2 68 kΩ |
| 5 V → 1.5 V | U2 SY8088AAC | R5 200 kΩ / R3 300 kΩ |
| 5 V → 0.9 V | U3 SY8088AAC | R8 300 kΩ / R4 150 kΩ |
| 5 V → 1.8 V | U4 SY8088AAC | R7 150 kΩ / R6 300 kΩ |

**中文**

1.8V 需求电流较低，原本可用 XC6206P182MR LDO，其余几路用 SY8088AAC(1A) 与 SY8089AAAC(2A)；最终设计 **四路全部使用 SY8088AAC（U1–U4）**，实测可用。

- 电感 2.2µH（BOM 为麦捷 MGFL2012F2R2MT，L1–L4）；
- 输出电容 10µF/25V 0603；
- 分压公式 **Vout = 0.6×(1+RH/RL)**。

| 电源 | 芯片 | 上拉 RH / 下拉 RL |
|---|---|---|
| 5V→3.3V | U1 | 150kΩ / 68kΩ |
| 5V→1.5V | U2 | 200kΩ / 300kΩ |
| 5V→0.9V | U3 | 300kΩ / 150kΩ |
| 5V→1.8V | U4 | 150kΩ / 300kΩ |

![Discrete DC/DC rails](images/power-dcdc-discrete.png)

**EN**

*Internal LDOs (not used):* the SoC integrates LDOA (default 1.8 V / 200 mA, can feed VCC-PLL, VCC18-DRAM, VCC-LVDS) and LDOB (default 1.35 V / 400 mA, can feed VCC-DRAM, though external DC/DC is recommended because DDR current is high). LDOIN accepts 2.4–3.63 V. Since the internal LDOs are unused, these pins are left floating.

**中文**

*内部 LDO（未使用）：* SoC 集成 LDOA（默认 1.8V/200mA，可给 VCC-PLL、VCC18-DRAM、VCC-LVDS 供电）与 LDOB（默认 1.35V/400mA，可给 VCC-DRAM 供电，但 DDR 功耗大建议外部 DC/DC）；LDOIN 输入范围 2.4–3.63V。本设计未用内部 LDO，相关引脚悬空处理。

![Power domains](images/power-domains.jpg)

### 2.4 Power Ripple / 电源纹波处理

**EN**

The T113 is sensitive to rail ripple, so bypass capacitors are added on every rail: core, DRAM and analog rails get a mix of µF- and nF-class capacitors on both input and output; I/O rails get nF-class capacitors only.

**中文**

T113 对各路电源纹波要求较高，需在各路电源添加旁路电容：核心、DRAM、模拟信号电源的输入输出均用 µF 与 nF 搭配；IO 外设部分只加 nF 级电容。

![Decoupling capacitors](images/power-decoupling.png)

**EN Special audio treatment / 音频部分特别处理：**

- AGND and GND are joined through a **0 Ω resistor** to separate analog and digital ground (R79 / R127 in different sheet revisions).
- AVCC is isolated through a **ferrite bead**, chosen primarily by impedance and current rating; a 0603 bead rated **600 Ω @ 100 MHz** (Sunlord GZ1608D601TF, L8) is used. For a shipping product, the bead must be matched to the measured EMI interference frequency.
- VRA1 and VRA2 get nF/µF low-ESR capacitors to ground for high-frequency noise; **470 nF** is the default here (C82 / C83).

These values must be tuned on the manufactured board according to the measured audio quality and EMI results — layout, fabrication and application all change the interference.
AGND 与 GND 之间用 **0Ω 电阻**做数模地隔离；AVCC 用**磁珠**隔离（0603、600Ω@100MHz，顺络 GZ1608D601TF，L8；量产时需按 EMI 实测干扰频率适配磁珠）；VRA1/VRA2 加 nF 与 µF 低 ESR 电容接地滤高频，默认 **470nF**（C82/C83）。这些器件需在板卡生产后结合实际音频效果与 EMI 测试调整。

### 2.5 Final MCU Schematic / 主控最终设计

**EN** The complete T113-S3 with audio ferrite bead, ground isolation and all power pins is shown below.

**中文** 含音频磁珠、地隔离与全部电源引脚的主控原理图如下。

![Final MCU schematic](images/mcu-final-schematic.png)

---

## 3. DRAM / 内置 DRAM 配置

**EN**

The T113-S3 has **128 MB DDR3 in-package**, with VCC-DRAM at **1.5 V**. Per the official hardware design recommendation, pin 47 is pulled to ground through a **240 Ω ±1% resistor (R123, LCSC C23350)**. (The corresponding configuration on T113-i / 1.8 V parts differs — follow the datasheet.)

**中文**

T113-S3 内置 **128MB DDR3**，VCC-DRAM 供电 **1.5V**。按官方硬件设计建议，在 **Pin47 通过 240Ω、精度 1% 的电阻下拉（R123，C23350）**。（T113-i / 1.8V 版本配置不同，以手册为准。）

![DRAM config resistor](images/dram-config.png)

---

## 4. Flash / 闪存

**EN**

The T113 supports eMMC and SPI NOR / SPI NAND flash. A **Winbond W25N01GVZEIG, 1 Gbit SPI NAND (U6, LCSC C90768)** is selected from the Allwinner-validated list (which also includes GigaDevice GD5F series, Macronix MX35LF series, ESMT F50L series, etc.).

- SPI CS / CLK / MOSI / MISO go to the SoC on **SPI0 (pins 14–19 area, PF group)**.
- **WP and HOLD are tied high to 3.3 V** (10 kΩ R145 / R93) and are not driven by the SoC:
  - **WP (Write Protect)**: when pulled low the chip becomes read-only, blocking write/erase/program operations.
  - **HOLD**: when low while CS is low, the current SPI operation is paused, DO goes high-Z, and DI/CLK are ignored; communication resumes at the pause point once HOLD goes high.
  - Both must stay high for normal operation.
- An extra key **SW1** is added on the SPI0 MOSI line: **if SW1 is held during power-on, the SoC cannot detect the flash and enters FEL firmware-burning mode.**

**中文**

T113 支持 eMMC 与 SPI NOR / SPI NAND。从全志验证型号清单（兆易 GD5F 系列、旺宏 MX35LF 系列、ESMT F50L 系列等）中选择华邦 **W25N01GVZEIG，1Gbit SPI NAND（U6，C90768）**。

- SPI 的 CS/CLK/MOSI/MISO 直接接主控 **SPI0（PF 组引脚）**；
- **WP、HOLD 直接 3.3V 上拉（10kΩ R145/R93），不接 T113**：WP 拉低进入只读模式，禁止写入/擦除/编程；HOLD 在 CS 为低时拉低会暂停 SPI 通信、DO 高阻、忽略 DI/CLK，拉高后从暂停点继续。正常使用两脚均须为高；
- 额外增加按键 **SW1**：上电时按住 SW1，芯片检测不到 Flash，即进入 **FEL 烧录模式**。

![SPI NAND flash and FEL key](images/spi-nand-flash.png)

---

## 5. USB / USB 接口

**EN**

Two parts of the board need USB: a firmware/ADB port and a power port. Common USB-C connector options are 24-pin (full-featured), 16-pin (USB 2.0 only — no need to waste a 24-pin part when the MCU lacks USB 3.0), and 6-pin (power only, for toys / toothbrushes). This board uses two **KH-TYPE-C-16P** connectors (LCSC C709357):

- **USB10 → USB0 (OTG)**: firmware burning and ADB debugging. VBUS = VIN5V (debug supply).
- **USB9 → USB1 (Host)**: reserved for future USB functions; its VBUS is the main **5 V** supply.
- **CC1 / CC2 each use a 5.1 kΩ pull-down to ground (R141/R142, R113/R114)** to advertise sink attachment. The documented input is 5 V. Rd resistors alone do not implement USB Power Delivery negotiation or establish support for higher input voltages.
- The two VBUS paths are OR-ed through **Schottky diodes D22 / D23 (LMBR240FT1G, SOD-123FL)** to isolate the supplies and prevent back-powering the host PC. Selection: low forward voltage Vf (less loss) and If ≥ the maximum system current (e.g. 2 A).
- USB data pairs are protected by ESD5451N TVS arrays (U33–U36).
- A power indicator LED (LED3 + 10 kΩ R115) shows power is present.

**中文**

板上两部分需要 USB：烧录/ADB 口与供电口。常见 Type-C 有 24Pin（全功能）、16Pin（仅 USB2.0，MCU 无 USB3.0 时不浪费 24Pin）、6Pin（仅取电，用于玩具、牙刷）。本板使用两个 **KH-TYPE-C-16P（C709357）**：

- **USB10 接 USB0（OTG）**：烧录与 ADB 调试，VBUS 为 VIN5V（调试供电）；
- **USB9 接 USB1（Host）**：预留扩展其它 USB 功能，VBUS 为**主供电 5V**；
- **CC1/CC2 各用 5.1kΩ 电阻接地（R141/R142、R113/R114）**：下拉电阻用于声明取电端连接，当前文档的输入为 5V。仅有 Rd 电阻不构成 USB PD 协商能力，也不能据此宣称支持更高输入电压；
- 两路 VBUS 通过 **肖特基二极管 D22/D23（LMBR240FT1G，SOD-123FL）串联隔离**，避免外部供电倒灌电脑；选型看正向压降 Vf 越低越好、电流 If ≥ 系统最大电流（如 2A）；
- USB 数据线用 ESD5451N（U33–U36）做静电保护；
- 电源指示灯 LED3 + 10kΩ R115，方便判断是否通电。

![Debug Type-C](images/usb-debug-typec.png)
![Power OR-ing diode and indicator](images/power-oring-diode.png)

Type-C connector comparison / Type-C 接口对比：[24-pin](images/typec-24pin.png) · [16-pin](images/typec-16pin.png) · [6-pin](images/typec-6pin.png)

---

## 6. LED and Keys / LED 与按键

**EN** Any GPIO-capable pins that do not conflict with a peripheral can be used. In this project **LED1 → PD11** (LED4, NCD0603B1, with 10 kΩ R124) and **BTN1 → PD10** (SW5 with 100 nF debounce C124 and ESD U42).

**中文** 任意支持 GPIO 且不占用外设的引脚均可：**LED1 接 PD11**（LED4，NCD0603B1，串 10kΩ R124）；**BTN1 接 PD10**（SW5，100nF 消抖 C124，ESD 管 U42）。

![LED and button](images/led-and-button.png)

---

## 7. UART / 串口

**EN**

The T113 provides six UARTs; **UART0 is the default debug console** (remappable in software; GPIO mux is described in datasheet §4.3). Two UARTs are used on this board:

- **UART1 (PG6–PG9, with RTS/CTS)** → the RTL8723DS Bluetooth channel (see §9).
- **UART3 (PB8/PB9)** → a runtime log header, a ZX-MX1.25-3PWT 3-pin terminal (U24, LCSC C7430469) with ESD5451N protection (U37/U38). Connecting here shows the boot/runtime log.
- UART0 (pin 33/35) is wired to the optional BLE Mesh module socket U5 (MX-02), see §9b.

**中文**

T113 内置 6 组 UART，**UART0 默认用于 Debug**（可软件修改；IO 映射见数据手册 4.3 节 GPIO Multiplex Function）。本板用到：

- **UART1（PG6–PG9，含 RTS/CTS）**：接 RTL8723DS 蓝牙（见第 9 节）；
- **UART3（PB8/PB9）**：运行日志，预留 ZX-MX1.25-3PWT 3P 端子（U24，C7430469），带 ESD5451N（U37/U38）保护，接此口可查看运行日志；
- UART0（33/35 脚）：接 BLE Mesh 模组预留座 U5（MX-02），见 9b。

![UART3 log port](images/uart3-debug-port.png)

---

## 8. Audio / 音频接口

**EN** The T113-S3 provides 2 I2S digital audio interfaces, 1 analog output (HPOUT) and 3 analog inputs (MICIN, FMIN, LINEIN). This design uses **MICIN for the microphone** and **HPOUT for the speaker amplifier**.

**中文** T113-S3 支持 2 路 I2S 数字音频、1 路模拟输出 HPOUT、3 路模拟输入（MICIN、FMIN、LINEIN）。本设计用 **MICIN 作麦克风输入**，**HPOUT 接功放驱动扬声器**。

### 8.1 Microphone / MIC 电路

**EN**

A classic electret-condenser microphone biasing circuit is used, with the GMI4015P-2C-42dB mic (LCSC **C233292**, MIC2).

- **C46 / C47 (100 nF)** — DC-blocking / AC-coupling caps. At 1 kHz, the capacitive reactance of 100 nF is approximately 1.59 kΩ: Xc = 1/(2πfC). The resulting high-pass response depends on the surrounding input impedance and must be evaluated for the actual signal chain.
- **R27 (2 kΩ) / R34 (1 kΩ)** — MIC bias from AVCC. R27 sets the bias current and therefore sensitivity: too large gives too small current and low output level (weak sounds are lost as noise); too small raises noise. 2 kΩ is the usual value; 1 kΩ gives higher sensitivity.
- **C87 (100 pF)** — differential-mode high-frequency noise filter (>10 MHz switch/crosstalk noise sees low impedance and is shorted).
- **R148 (75 Ω) + C143 (10 µF)** — RC low-pass on the AVCC supply, blocking switching-noise and clock interference into the mic chain.
- Inputs arrive at MIC_IN_3P / MIC_IN_3N (pins 87/88) as a differential pair.

**中文**

参考经典驻极体电容麦电路，麦克风为 GMI4015P-2C-42dB（**C233292**，MIC2）：

- **C46/C47（100nF）**：隔直通交，100nF 在 1kHz 时容抗约为 1.59kΩ，计算式为 Xc=1/(2πfC)。高通响应取决于周边输入阻抗，需按实际信号链评估；
- **R27（2kΩ）/R34（1kΩ）**：由 AVCC 提供偏置；R27 影响偏置电流即灵敏度——过大会使输入电流过小、输出电平偏低、小声被当噪声丢弃；一般取 2kΩ，高灵敏度可选 1kΩ；
- **C87（100pF）**：滤除信号线间差模高频噪声（对 >10MHz 呈低阻短路）；
- **R148（75Ω）+ C143（10µF）**：AVCC 电源 RC 低通，阻隔开关电源噪声与数字时钟干扰；
- 信号以差分对进入 MIC_IN_3P / MIC_IN_3N（87/88 脚）。

![MIC reference](images/mic-reference-circuit.png)
![MIC schematic](images/mic-circuit.png)

### 8.2 Speaker Amplifier / 扬声器功放

**EN**

Amplifier classes compared: Class A is simple and clean-sounding but inefficient and hot; Class B is efficient but suffers crossover distortion; Class AB combines both advantages; Class D uses PWM for high efficiency and power but needs filtering and can introduce digital noise.

The early design used an LM4890S; the **final BOM uses an LM4871 (U53, SOIC-8, LCSC C7494970)** — chosen because the small LM4890 package is harder to assemble; LM4871 is the recommended alternative. It drives an **8 Ω speaker** via the HC-1.25-2PWT 2-pin terminal CN2.

- **R135 (22 Ω) with C135 (100 nF)** — classic low-pass to ground, filtering highs at HPOUT.
- **C142 (470 nF)** — input coupling; blocks DC and forms a high-pass with R136 (20 kΩ).
- **C138 (1 µF)** — Bypass/reference pin filter.
- **C139 (10 µF)** — supply filtering.
- **R149 (20 kΩ) / R136** — closed-loop gain feedback network.
- **R138 (100 kΩ)** — pull-down defining the pin level before boot.
- **HPOUT_FB (pin 100)** — feedback unused; per datasheet tied through 100 nF (C91) to ground.
- HPOUTR/HPOUTL are stereo outputs; two amplifiers/speakers are possible, but typical compact devices use one speaker (HPOUTR is routed here).
- ESD5451N (U49/U50) protects the speaker terminal; the amplifier and speaker side reference AGND.

**中文**

功放类别对比：A 类简单音质好但效率低易发热；B 类效率高功率大但有交越失真；AB 类兼顾音质与效率；D 类 PWM 调制效率高功率大但需滤波器、易有数字噪声。

早期选 LM4890S，**最终 BOM 采用 LM4871（U53，SOIC-8，C7494970）**——LM4890 封装小、装配难，LM4871 是推荐替代。经 HC-1.25-2PWT 2P 端子 CN2 驱动 **8Ω 扬声器**：

- **C135 100nF 串 R135 22Ω 接地**：典型低通，滤高频；
- **C142（470nF）**：输入耦合隔直，并与 R136（20kΩ）构成高通；
- **C138（1µF）**：Bypass 参考电源脚滤波；
- **C139（10µF）**：电源滤波；
- **R149（20kΩ）与 R136**：设置闭环增益的反馈网络；
- **R138（100kΩ）**：下拉，确定未启动时引脚电平；
- **HPOUT_FB（100 脚）**：未用反馈，按手册经 100nF（C91）接地；
- HPOUTR/HPOUTL 为左右声道，可接两颗功放两只喇叭，紧凑设备通常只接一只（本设计走 HPOUTR）；
- 喇叭端子由 ESD5451N（U49/U50）保护，功放与喇叭侧参考 AGND。

![LM4890 reference (early)](images/amplifier-lm4890-reference.png)
![Final amplifier output](images/amplifier-output.png)

---

## 9. WiFi / WiFi 与蓝牙

**EN**

A Realtek **RTL8723DS Wi-Fi + BT module (U11, LCSC C2920724)** is used. Wi-Fi runs over SDIO and Bluetooth over UART; BT is not used in the current product but is wired for future use. Of the module's pins (power, enable, Wi-Fi, BT UART, BT audio PCM, low-power), only power, enable, Wi-Fi SDIO and BT UART are connected.

- AGND → ground; WL_BT_ANT → 2.4 GHz ceramic antenna AN2051-245 (L10, LCSC C99727).
- VBAT_IN → 3.3 V; WL_DIS (Wi-Fi enable) pulled high; BT_DIS (BT enable) pulled high.
- HOST_WAKE_BT / BT_WAKE_HOST (classic-BT wake) not needed without sleep — left unconnected.
- WL_HOST_WAKE not needed — pulled high by default; SUSCLK_IN (low-power clock input) unused, pulled high to avoid floating.
- PCM (BT audio) unused; MAIN_XTAL_IN = NC; VBAT_EN (NC pin) left floating.
- SDIO D0–D3 / CMD / CLK connect to the SoC **SDC1**; a 33 Ω series resistor (R90) is placed on CLK.
- BT UART (TX/RX/RTS/CTS) connects to SoC **UART1**.
- RN2/RN3 are 10 kΩ resistor arrays for the pulls; C55 = 10 µF at VDDIO; C54/C63 = 10 µF/100 nF at VBAT.

| Signal | RTL8723DS | T113-S3 pin |
|---|---|---|
| SDC1_CLK | CLK | PG0 (pin 120) |
| SDC1_CMD | CMD | PG1 |
| SDC1_D0–D3 | D0–D3 | PG2–PG5 |
| UART1_TX / RX | UART_OUT / UART_IN | PG6 / PG7 |
| UART1_RTS / CTS | UART_RTS / UART_CTS | PG8 / PG9 |

**中文**

WiFi 选用 Realtek **RTL8723DS WiFi+蓝牙模组（U11，C2920724）**：WiFi 走 SDIO，蓝牙走 UART；当前产品未用蓝牙，但先接上。模组引脚含电源、使能、WiFi、蓝牙 UART、蓝牙音频、低功耗等，本设计只接电源、使能、WiFi、蓝牙 UART：

- AGND 接地；WL_BT_ANT 接 2.4G 陶瓷天线 AN2051-245（L10，C99727）；
- VBAT_IN 接 3.3V；WL_DIS（WiFi 使能）上拉使能；BT_DIS（蓝牙使能）上拉使能；
- HOST_WAKE_BT / BT_WAKE_HOST 为经典蓝牙唤醒，不做睡眠不接；
- WL_HOST_WAKE 用不到，默认上拉；SUSCLK_IN 低功耗时钟未用，上拉固定高电平避免悬空；
- PCM 蓝牙音频未用留空；MAIN_XTAL_IN 为 NC；VBAT_EN（NC 脚）悬空；
- SDIO D0–D3/CMD/CLK 接主控 **SDC1**，CLK 串 33Ω 电阻 R90；
- 蓝牙 UART（TX/RX/RTS/CTS）接主控 **UART1**；
- RN2/RN3 为 10kΩ 排阻上拉，VDDIO 处 C55=10µF，VBAT 处 C54/C63 为 10µF/100nF。

![WiFi module](images/wifi-module-rtl8723ds.png)
![WiFi to MCU wiring](images/wifi-mcu-connections.png)

### 9b. BLE Mesh socket (optional) / BLE Mesh 预留座

**EN**

An **MX-02** module footprint (U5) is reserved on **UART0**, with two mode-control pins on PD05/PD06: per the on-sheet note, **PD05/PD06 = 0 → normal mode (can receive and transmit / connect); = 1 → transparent-transmission mode (transmit only, cannot receive)**. This socket is reserved for the future BLE-Mesh multi-device central-control feature; it can be left unpopulated. Firmware for ESP-based BLE modules can be written with the Espressif Flash Download Tool (see `/tools`).

**中文**

在 **UART0** 上预留 **MX-02** 模组封装（U5），PD05/PD06 为模式脚：按图中注释，**0 = 正常模式（可收发连接数据）；1 = 透传模式（只能发送，不能接收）**。该座为后续 BLE Mesh 多设备中控功能预留，可不贴。若该模组为 ESP 系，可用乐鑫 Flash Download Tool 烧录（见 `/tools`）。

---

## 10. Display, Touch and Backlight / 屏幕、触摸与背光

**EN**

The T113-S3 supports RGB888/RGB666, dual-channel LVDS (pin-muxed with RGB) and MIPI DSI (also pin-muxed). This project uses a **6.9-inch** bar panel:

- **Display: MIPI DSI**, 280(H) × 1424(V), through an **FPC 30-pin 0.5 mm pitch** connector (Xunpu FPC-05F-30PH20, FPC2, LCSC C2856808).
- **Touch: I2C (Goodix GT911 class)**, through an **FPC 6-pin 0.5 mm top-contact** connector (AFC07-S06ECA-00, FPC7, LCSC C262579).

**中文**

T113-S3 支持 RGB888/RGB666、双路 LVDS（与 RGB 复用引脚）、MIPI DSI（同样复用）。本项目屏幕为 **6.9 寸长条屏**：显示部分 **MIPI DSI**，分辨率 280×1424，用 **FPC 30pin、间距 0.5mm** 端子（讯普 FPC-05F-30PH20，FPC2，C2856808）；触摸部分 **I2C（GT911 类）**，用 **6pin 0.5mm 上接端子**（AFC07-S06ECA-00，FPC7，C262579）。

### 10.1 MIPI screen connector / 屏幕端子

**EN**

FPC2 carries 4 MIPI data lanes (MIPI_0–3 P/N), the differential clock (LCD_CLK_P/N), MIPI_LCD_TE (tearing-effect output from the LCD), MIPI_LCD_RST (active-low reset from the SoC), LCD_VCC (3.3 V, with 22 µF C132 / 1 µF C133 bulk), and the backlight pins LEDA (anode) / LEDK (cathode, two pads). Pins 1–7 and inter-lane pins are GND.

**中文**

FPC2 包含 4 组 MIPI 数据通道（MIPI_0–3 P/N）、差分时钟 LCD_CLK_P/N、MIPI_LCD_TE（LCD 输出的 TE 状态脚）、MIPI_LCD_RST（主控输出，拉低复位）、LCD_VCC（3.3V，配 22µF C132 与 1µF C133）、背光 LEDA（阳极）/LEDK（阴极，两脚）。1–7 脚及各通道间为 GND。

![MIPI FPC30](images/mipi-lcd-fpc30.png)

### 10.2 Touch (I2C) / TP 设计

**EN**

I2C outputs are open-drain, so SDA/SCL (plus RST/INT) are pulled up to 3.3 V through a 10 kΩ resistor array (RN1). FPC7 pinout: 1 INT, 2 GND, 3 SDA, 4 SCL, 5 RST, 6 3.3 V (pins 7/8 GND). TP_SCL/TP_SDA/TP_INT/TP_RST run to PB-port I2C pins (44/45/42/43).

**中文**

I2C 输出级为开漏结构，SDA/SCL（及 RST/INT）经 10kΩ 排阻 RN1 上拉至 3.3V。FPC7 线序：1 INT、2 GND、3 SDA、4 SCL、5 RST、6 3.3V（7/8 脚 GND）。TP_SCL/SDA/INT/RST 接 PB 口 I2C 引脚（44/45/42/43 脚）。

![Touch FPC6 and pulls](images/touch-i2c-fpc6.png)

### 10.3 Backlight / 背光

**EN**

The panel backlight is two strings of four LEDs each. Per the panel datasheet the recommended forward current is ~40 mA at 12 V forward voltage; to extend panel life the current is set lower, **~20 mA**. The boost LED driver selection criteria: single channel, PWM dimming, ~1 MHz switching. An **MT9201 (U52, SOT-23-6, LCSC C182966)** boost converter designed for multi-string LED driving is used.

- **Rcs = 10 Ω (R121)** sets the LED current to within 20 mA.
- **Cin / Cout = 1 µF (C141 / C119)**; use X7R or low-ESR dielectrics to reduce noise.
- **Cout voltage rating must be high (>30 V)**: with no panel attached the open-circuit output voltage can run high and puncture a low-rated cap.
- The freewheeling diode D22 (LMBR240FT1G) is rated ≥ 30 V / 500 mA; inductor L18 = 10 µH (WPN201610H100MT).
- **EN has a 10 kΩ pull-down (R120)** to avoid screen flashing during power-up; LCD_PWM (pin 52) drives EN for PWM dimming.
- LEDA/LEDK route to FPC2 pins 30 / 28–29.

**中文**

屏幕背光为两串、每串 4 颗 LED；按屏幕手册正向电流建议 40mA、正向电压 12V。为延长寿命，在不影响显示效果下把背光电流设低至约 **20mA**。LED 驱动选型要点：单通道、支持 PWM 调光、频率约 1MHz。采用专为多串 LED 设计的升压转换器 **MT9201（U52，SOT-23-6，C182966）**：

- **Rcs = 10Ω（R121）**：将输出电流控制在 20mA 内；
- **Cin/Cout = 1µF（C141/C119）**：优选 X7R 或低 ESR 电容以降低噪声；
- **Cout 耐压要高（>30V）**：未接屏时空载输出电压过高，容易击穿低耐压电容；
- 续流二极管 D22（LMBR240FT1G）需 30V/500mA 以上；电感 L18 = 10µH（WPN201610H100MT）；
- **EN 脚加 10kΩ 下拉（R120）**，防止上电过程闪屏；LCD_PWM（52 脚）接 EN 实现 PWM 调光；
- LEDA/LEDK 分别接 FPC2 的 30 脚与 28/29 脚。

![MT9201 reference](images/backlight-mt9201-reference.png)
![Backlight schematic](images/backlight-mt9201.png)

---

## 11. Key Component Summary / 关键器件汇总

| Function / 功能 | Part / 型号 | Designator | LCSC / 备注 |
|---|---|---|---|
| SoC | Allwinner T113-S3 (LQFP128) | T1 | dual A7 + 128 MB DDR3 |
| SPI NAND | W25N01GVZEIG 1 Gbit | U6 | C90768 |
| Wi-Fi/BT | RTL8723DS module | U11 | C2920724 |
| BLE Mesh (optional) | MX-02 socket | U5 | on UART0, may be DNP |
| Bucks ×4 | SY8088AAC | U1–U4 | C79313 |
| Audio amp | LM4871 (SOIC-8) | U53 | C7494970 |
| Backlight boost | MT9201 | U52 | C182966 |
| Microphone | GMI4015P-2C-42dB | MIC2 | C233292 |
| USB-C ×2 | KH-TYPE-C-16P | USB9/10 | C709357 |
| LCD FPC | FPC-05F-30PH20 (30P 0.5) | FPC2 | C2856808 |
| TP FPC | AFC07-S06ECA-00 (6P 0.5) | FPC7 | C262579 |
| Log UART | ZX-MX1.25-3PWT | U24 | C7430469 |
| Speaker terminal | HC-1.25-2PWT | CN2 | C2845379 |
| Main crystal | 24 MHz ±10 ppm | X1 | C5444545 |
| RTC crystal | 32.768 kHz | X2 | C620155 |
| Antenna | AN2051-245 ceramic | L10 | C99727 |
| ESD/TVS | ESD5451N ×11 | U21…U50 | C2936977 |
| OR-ing diodes | LMBR240FT1G ×2 | D22/D23 | C383230 |

The full 50-line BOM (with all passives) is in [`../bom/`](../bom/AuraBar-BOM-v1.0.xlsx).
完整 50 项 BOM（含全部阻容感）见 [`../bom/`](../bom/AuraBar-BOM-v1.0.xlsx)。
