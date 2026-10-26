# AuraBar — Linux Smart Bar Display

*A long, elegant desktop assistant display powered by the Allwinner T113-S3 — and a universal embedded HMI platform you can also put inside a fridge or behind a dashboard.*

**English** documentation is primary; **中文说明** follows each section.

---

> **AuraBar** 是一块基于全志 T113-S3（双核 Cortex-A7，运行 Linux）的长条智能助理屏，同时也是一套通用嵌入式 HMI 平台——可放桌面，也可嵌入冰箱、装进仪表。英文文档为主，每节附中文。

![System architecture](hardware/docs/images/system-architecture.jpg)

## Highlights / 产品特性

**Coding Companion 0.4 preview:** a desktop companion for AI coding tasks. Follow
actual Codex CLI events, read Summary / Evidence / Next cards, and review changes
on your computer. Includes USB task synchronization and the Music & Focus base.
[Setup](software/companion/README.md) · [Product scope](docs/software/coding-companion.md).
中文：键盘旁的 AI 编程任务伴侣，显示真实任务状态、命令执行证据和待验收事项。
Codex runs on the computer with the user's own CLI login; no model runs on the board.

**EN**

- **6.9" ultra-wide bar panel**, 280 × 1424, **MIPI DSI** 4-lane, with capacitive touch (**GT911, I2C**)
- **Allwinner T113-S3**: dual Cortex-A7 @ 1.2 GHz, HiFi4 DSP, **128 MB DDR3 in-package** — runs mainline-friendly Linux
- **1 Gbit SPI NAND** (W25N01) storage, FEL burning key on board
- **Audio**: integrated codec, electret MIC input + LM4871 speaker amplifier (8 Ω)
- **Wireless**: RTL8723DS **Wi-Fi (SDIO) + Bluetooth (UART)**, plus an optional **BLE Mesh** module socket
- **Dual USB-C 16-pin**: one for power (USB Host reserved), one for ADB / firmware (USB OTG), with CC 5.1 kΩ Rd, ESD protection and power OR-ing
- Four independent SY8088 buck rails (3.3 V / 1.8 V / 1.5 V / 0.9 V), MT9201 PWM backlight driver
- LQFP128 SoC and 0603/SOT-23 passives throughout — **hand-solderable, tutorial-friendly, JLCPCB/LCSC one-click BOM**
- **Music & Focus preview**: local WAV/MP3, direct MP3 streams, synthesized ambient sound, clock, weather, Pomodoro and To-Do. [Software and validation status](software/).

**中文**

- **6.9 寸超长条屏**，280×1424，**MIPI DSI** 4 通道，电容触摸（**GT911，I2C**）
- **全志 T113-S3**：双核 Cortex-A7、HiFi4 DSP、**内置 128MB DDR3**，可运行 Linux
- **1Gbit SPI NAND**（W25N01）存储，板载 FEL 烧录按键
- **音频**：内置 codec，驻极体 MIC 输入 + LM4871 功放（8Ω 喇叭）
- **无线**：RTL8723DS **WiFi（SDIO）+ 蓝牙（UART）**，另预留 **BLE Mesh** 模组座
- **双 USB-C 16Pin**：一个主供电（USB Host 预留），一个 ADB/烧录（USB OTG），含 CC 5.1kΩ 下拉、ESD 保护与双电源防倒灌
- 四路独立 SY8088 降压（3.3/1.8/1.5/0.9V），MT9201 PWM 背光驱动
- LQFP128 主控、0603/SOT-23 器件，**可手工焊接、适合教学复刻，立创 BOM 一键下单**
- **Music & Focus 预览版**：本地 WAV/MP3、MP3 网络流、合成环境音、时钟、天气、番茄钟与待办。[软件与实测状态](software/)。

## System Block Diagram / 系统框图

```mermaid
graph LR
  subgraph PWR[Power / 供电]
    USB9[USB-C Power / 主供电 USB9] --> D[Dual Schottky OR-ing]
    USB10[USB-C Debug / 调试 USB10] --> D
    D --> BUCK[4x SY8088: 3.3/1.8/1.5/0.9V]
  end
  BUCK --> T[T113-S3<br/>Dual A7 + 128MB DDR3]
  T -->|MIPI DSI x4| LCD[6.9" 280x1424 LCD]
  T -->|PWM| BL[MT9201 Backlight / 背光]
  TP[GT911 Touch / 触摸] -->|I2C INT RST| T
  T <-->|SPI0 + FEL key| FL[W25N01 1Gbit NAND]
  WF[RTL8723DS WiFi+BT] -->|SDIO + UART1| T
  BLE[MX-02 BLE Mesh / 预留] -->|UART0| T
  MIC[MIC / 麦克风] --> T
  T --> AMP[LM4871 Amp / 功放] --> SPK[8Ω Speaker / 喇叭]
  T --- LOG[UART3 Log Header]
```

## Hardware Specifications / 硬件规格

| Item / 项目 | Specification / 规格 |
|---|---|
| SoC | Allwinner T113-S3, LQFP128, dual Cortex-A7 + HiFi4 DSP |
| RAM | 128 MB DDR3 (in-package), VCC-DRAM 1.5 V |
| Storage | Winbond W25N01GVZEIG, 1 Gbit SPI NAND |
| Display | 6.9" bar, 280×1424, MIPI DSI 4-lane, FPC 30P 0.5 mm |
| Touch | Capacitive, GT911 class, I2C, FPC 6P 0.5 mm |
| Backlight | MT9201 boost, PWM dimming, ~20 mA, 2 strings × 4 LEDs |
| Audio | SoC codec; MICIN differential mic input; HPOUT → LM4871 → 8 Ω speaker |
| Wi-Fi / BT | RTL8723DS, Wi-Fi over SDIO (SDC1), BT over UART1, ceramic antenna |
| BLE | MX-02 socket on UART0 (optional, for BLE Mesh) |
| USB | 2 × USB-C 16-pin (USB0 OTG / ADB / FEL; USB1 Host + 5 V in) |
| Debug | UART3 1.25 mm 3-pin log header; FEL key SW1; reset key SW2; user key SW5 |
| Power rails | 5 V USB in → 3.3 V / 1.8 V / 1.5 V / 0.9 V, 4 × SY8088AAC |
| Clocks | 24 MHz ±10 ppm main; 32.768 kHz RTC |
| EDA | LCEDA Pro (嘉立创EDA专业版), project v1.0, A4 schematic × 7 pages |
| BOM | 50 unique parts / 136 components, LCSC numbers included |

## Repository Structure / 仓库结构

```text
AuraBar-T113S3/
├── README.md                     # This file / 本说明
├── hardware/
│   ├── lceda/6.22_Screen.eprj2   # LCEDA Pro project / 嘉立创EDA专业版工程
│   ├── schematic/
│   │   ├── AuraBar-Schematic-v1.0.pdf   # 7-page schematic PDF
│   │   ├── netlist.net           # Protel netlist w/ LCSC metadata / 网表
│   │   └── dxf/                  # Per-page DXF (CAD) / 分页 DXF
│   ├── bom/                      # BOM xlsx + csv / 物料清单
│   └── docs/
│       ├── hardware-design-guide.md    # Bilingual design guide / 双语设计指南
│       ├── hardware-design-guide-source.docx
│       └── images/               # 33 design figures / 设计过程图
├── docs/requirements/            # Product requirements / 需求分析
├── tools/                        # Burning-tool download notes / 烧录工具说明
└── software/                     # Music & Focus application / 应用源码、测试与版本配置
```

## Documentation / 文档

- 📘 [Bilingual Hardware Design Guide](hardware/docs/hardware-design-guide.md) — selection rationale and per-module circuit design / 双语硬件设计指南：选型理由与逐模块电路设计
- 📄 [Schematic PDF](hardware/schematic/AuraBar-Schematic-v1.0.pdf) · [DXF (CAD)](hardware/schematic/dxf/) · [Netlist](hardware/schematic/netlist.net)
- 📋 [BOM (xlsx)](hardware/bom/AuraBar-BOM-v1.0.xlsx) / [csv](hardware/bom/AuraBar-BOM-v1.0.csv)
- 🎯 [Product Requirements](docs/requirements/README.md)

**Schematic pages / 原理图分页**：`T113S3` (MCU + clocks + reset) · `POWER` (4 bucks) · `CONNECT` (USB-C / LED / key / UART) · `NET` (Wi-Fi + BLE) · `AUDIO` (MIC + amp) · `FLASH` (SPI NAND + FEL) · `DISPLAY` (MIPI + touch + backlight)

## Bring-up & Firmware Burning / 烧录与上手

**EN**

1. **T113-S3 firmware (Linux)**: power via the **debug USB-C (USB10, USB0 OTG)**. Hold **SW1** while powering on — the SoC fails to detect the NAND and enters **FEL mode**. Flash an Allwinner image with **PhoenixSuit / LiveSuit** (or `sunxi-fel`); release SW1 afterwards. USB0 also provides **ADB** once Linux boots.
2. **Logs**: connect a 3.3 V USB-UART adapter to the **UART3 1.25 mm 3-pin header (U24)** (GND/TX/RX).
3. **Optional BLE module (MX-02)**: if it is ESP-based, program it with the Espressif **Flash Download Tool**. Tool binaries are not committed to this repo for licensing/size reasons — see [`tools/README.md`](tools/README.md) for official download links.
4. The main USB-C (USB9) powers the board through a Schottky OR-ing diode; both ports can be attached safely.

**中文**

1. **T113 固件（Linux）**：用**调试 Type-C（USB10，USB0 OTG）**接电脑，上电时按住 **SW1**，主控检测不到 NAND 即进入 **FEL 模式**，用全志 **PhoenixSuit / LiveSuit**（或 `sunxi-fel`）烧录镜像，之后松开；Linux 启动后该口也用于 **ADB**。
2. **看日志**：3.3V USB 转串口接 **UART3 的 1.25mm 3P 端子（U24）**（GND/TX/RX）。
3. **可选 BLE 模组（MX-02）**：若为 ESP 方案，用乐鑫 **Flash Download Tool** 烧录。出于版权与体积考虑，工具 exe 未入库，官方下载链接见 [`tools/README.md`](tools/README.md)。
4. 主供电 Type-C（USB9）经肖特基防倒灌二极管供电，两个 Type-C 可同时安全连接。

## Pin / Interface Map / 接口映射

| Function / 功能 | Port / Group | Pins / Connector |
|---|---|---|
| SPI NAND | SPI0 (PF) | CS/CLK/MOSI/MISO + FEL key SW1 |
| Wi-Fi SDIO | SDC1 (PG0–PG5) | CLK/CMD/D0–D3, 33 Ω on CLK |
| BT UART | UART1 (PG6–PG9) | TX/RX/RTS/CTS |
| BLE Mesh | UART0 + PD05/PD06 mode | MX-02 socket U5 |
| Runtime log | UART3 (PB8/PB9) | 1.25 mm 3P header U24 |
| MIPI DSI | PD13–PD19 | FPC2 30P (4 data lanes + clock + TE + RST) |
| Touch | I2C (PB) + INT/RST | FPC7 6P, pins 42–45 |
| Backlight PWM | LCD_PWM (pin 52) | MT9201 EN |
| MIC | MIC_IN_3P/3N | pins 87/88 |
| Speaker | HPOUTR → LM4871 | CN2 2P |
| User LED / key | LED1 = PD11, BTN1 = PD10 | LED4 / SW5 |
| USB0 OTG / USB1 Host | — | USB10 / USB9 Type-C |
| Reset | RESET pin 27 | SW2 + 10 kΩ pull-up + TVS |

## Cloning the Board / 复刻说明

**EN** Open `hardware/lceda/6.22_Screen.eprj2` with **LCEDA Pro (嘉立创EDA专业版)**; the BOM and netlist carry LCSC (立创商城) part numbers for one-click ordering/PCBA. The board uses hand-solderable packages (LQFP128, 0603, SOT-23-5/6, SOIC-8). Read the [design guide](hardware/docs/hardware-design-guide.md) before modifying power, audio (AGND/bead) or DDR-configuration circuitry.

**中文** 用**嘉立创EDA专业版**打开 `hardware/lceda/6.22_Screen.eprj2`；BOM 与网表均带立创商城编号，可一键下单/贴片。封装均适合手工焊接（LQFP128、0603、SOT-23-5/6、SOIC-8）。改动电源、音频（AGND/磁珠）或 DRAM 配置电路前请先阅读[设计指南](hardware/docs/hardware-design-guide.md)。

## Software Roadmap / 软件路线图

**EN** [Music & Focus 0.2.0-alpha.1](software/) includes the LVGL application, regression tests, edition manifest and checksummed application packaging. The ARM application and generated WAV/MP3 output were tested on the existing board. Full bootable firmware awaits a reproducible BSP build; AI and desktop integration editions remain planned.

**中文** [Music & Focus 0.2.0-alpha.1](software/) 已加入 LVGL 应用、回归测试、版本配置及带校验的应用打包工具。ARM 程序及生成的 WAV/MP3 测试音已在现有板卡验证。完整可烧录固件仍需可复现的 BSP 构建，AI 与桌面联动版本属于后续计划。

## License / 许可

**EN** Hardware design files are released under **CERN-OHL-S-2.0** (open hardware); documentation under **CC BY-SA 4.0**; application software under **MIT**. Component trademarks belong to their respective owners.

**中文** 硬件设计文件以 **CERN-OHL-S-2.0**（开源硬件）许可发布；文档采用 **CC BY-SA 4.0**；软件（待补充）采用 **MIT**。元器件商标归各自所有者。

---

*AuraBar · hardware v1.0 · Designed with LCEDA Pro · Built around the Allwinner T113-S3*
