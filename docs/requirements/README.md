# Product Requirements / 产品需求

> **AuraBar** — a universal embedded smart-display solution built around a long, wide auxiliary screen.

## 1. Product Vision / 产品愿景

**EN — A universal embedded product solution.** A long, large display that sits on a user's desk and acts as a personal desktop assistant: touch control, time, weather, and a Pomodoro timer, with screen-off control and volume control. The platform can later be extended with mini-games, HID shortcut keys, and BLE-Mesh multi-device central control. The same design can be embedded into a refrigerator or serve as an automotive digital instrument cluster (mileage, current gear, current speed).

**中文 —— 一款通用的嵌入式产品解决方案。** 一块长且大的屏幕，放在使用者工作台上，作为个人桌面助理屏：支持触摸控制，可查看时间、天气，设置番茄时钟，具备熄屏控制与音量控制功能。后续可扩展小游戏、HID 快捷键，以及 BLE Mesh 组网多设备中控。同一方案也可以内嵌进冰箱，或作为汽车数字仪表盘，显示里程、当前档位与当前速度。

## 2. Application Scenarios / 应用场景

| Scenario / 场景 | Key functions / 核心功能 |
|---|---|
| Desktop assistant / 桌面助理屏 | Clock, weather, Pomodoro, touch, volume, screen-off / 时间、天气、番茄钟、触摸、音量、熄屏 |
| Smart-home hub / 智能家居中控 | BLE Mesh central control of multiple devices / BLE Mesh 多设备中控 |
| White-goods HMI / 家电内嵌（冰箱） | Embedded UI panel for appliances / 冰箱等家电内嵌显示面板 |
| Automotive cluster / 汽车仪表 | Mileage, gear, speed display / 里程、档位、速度显示 |

Reference concept images: [fridge example](./images/fridge-example.png), [car dashboard example](./images/car-dashboard-example.png).
参考概念图：[冰箱实例](./images/fridge-example.png)、[汽车实例](./images/car-dashboard-example.png)。

## 3. Why not STM32F103 / 为什么不选用 STM32F103

**EN**

1. The core frequency is too low, which leads to an insufficient refresh frame rate when driving a larger screen.
2. It has no native display interface (no MIPI DSI / RGB / LVDS suitable for this panel).
3. It has no hardware audio codec.

**中文**

1. 主频太低，驱动较大屏幕时刷新帧率不够；
2. 没有显示接口（无 MIPI DSI / RGB / LVDS 等合适接口）；
3. 没有音频编解码器。

These requirements drive the choice of the Allwinner **T113-S3** application processor (dual Cortex-A7, capable of running Linux, with MIPI DSI and an integrated audio codec).
这些需求最终指向全志 **T113-S3** 应用处理器（双核 Cortex-A7，可运行 Linux，自带 MIPI DSI 显示接口与音频 codec）。
