# AuraBar

### AI 工作进展，一眼可见。

**面向 AI 工作空间、专注音频与智能设备开发的嵌入式 Linux 交互屏平台。**

[English](README.md) · [产品影片](docs/product/demo.md) · [开始复现](docs/guides/reproduce.md) · [企业集成评估](docs/product/integration-brief.md) · [文档导航](docs/README.md)

![AuraBar 产品展示](assets/demos/AuraBar-01-Product-Hero.gif)

*产品概念视觉；[60 秒完整影片](assets/demos/AuraBar-Product-Film-1080p.mp4)同时展示现有软件和真实 PCB。*

AuraBar 为任务进展、结果审阅与专注音频提供独立的触控入口。开发、授权和决策保留在电脑上，屏幕负责呈现值得关注的信息。T113-S3 硬件、原生界面与电脑伴侣也构成可研究、可适配的嵌入式产品基础。

## 三种实用体验，共用一个平台

| 体验 | 核心功能 | 用户价值 |
|---|---|---|
| **AI 编程伴侣** | 呈现电脑伴侣启动的 Codex 任务，以及 Summary／Evidence／Next 卡片 | 将 AI 结果整理为便于人工审阅的交接信息 |
| **桌面任务屏** | 同步真实命令状态、耗时、退出结果和连接状态 | 长时间运行的任务无需占据主窗口 |
| **音乐与专注** | 本地 WAV/MP3、合成棕噪声和雨声／海浪质感，支持播放控制与睡眠定时 | 为工作空间提供独立的音频入口 |

### 从执行，到审阅

电脑启动任务 → 屏幕关注进展 → 阅读结果卡片 → 回到电脑审阅与操作。

![真实任务界面回放](assets/demos/AuraBar-02-Task-Workflow.gif)

*由真实设备截图剪辑而成；构建与 AI 审查属于不同的历史任务。AI 与模型在电脑侧执行。*

### 为专注留出空间

任务与音乐共用原生交互界面，在任务卡片和音频控制之间切换时，后台可以继续播放。

![音乐与专注](assets/demos/AuraBar-03-Music-Focus.gif)

*声波为功能示意；下方真实设备截图保留当时的暂停状态。*

## 面向研究、适配与集成的工程基础

- **硬件资料完整可查**：T113-S3 双核 Cortex-A7、片内 128 MB DDR3、SPI NAND、MIPI DSI 显示、触摸、音频与 USB。
- **系统分工清晰**：板端负责 LVGL、音频和状态展示；电脑负责进程、Codex 与模型访问。
- **接口行为有据可依**：本地认证服务、有界任务快照、USB10 同步、心跳与重连机制。
- **交付依据可追溯**：可编辑电子工程、图文电路分析、应用源码、版本记录、自动检查与板端验收记录。

[阅读双语硬件设计指南 →](docs/hardware/README.md)

## 按目标开始

| 目标 | 入口 |
|---|---|
| 评估产品方案与合作 | [企业集成说明](docs/product/integration-brief.md) |
| 复现软硬件流程 | [复现指南](docs/guides/reproduce.md) |
| 在已配置的兼容板卡上运行 | [快速开始](docs/guides/quickstart.md) · [安装与回退](docs/guides/installation.md) |
| 对比软件版本 | [发布目录](docs/releases/README.md) |
| 开发适配器或修改界面 | [系统架构](docs/architecture/README.md) · [开发环境](docs/development/README.md) |
| 查看完整演示和素材说明 | [演示素材库](docs/product/demo.md) |

**当前软件基线：Coding Companion 0.4.0-alpha.2。** 已整合 Music & Focus 和 Desktop Link 的共享基础，公开源码面向指定硬件与系统环境下的工程评估。预编译应用包目前仍为发布草稿，仓库尚不提供空白板卡所需的完整 BSP 镜像；采购、组装或部署前请核对[兼容性与下载状态](docs/releases/README.md)。

## 从桌面延伸到更多设备

| 智能生活 | 车内辅助显示 |
|---|---|
| ![智能生活概念](assets/renders/connected-kitchen.webp) | ![车内辅助屏概念](assets/renders/vehicle-display.webp) |

这些场景展示平台的应用可能性。家电数据接入、车辆集成与适用性验证属于独立开发工作，详见[能力状态](docs/product/status.md)和[路线图](docs/product/roadmap.md)。

## 工程资源

`hardware/`：电子工程与设计指南；`software/`：板端程序与电脑伴侣；`docs/`：产品、集成、复现、架构与验收；`assets/`：影片、GIF 与图像；`scripts/` 和 `.github/`：完整性检查与持续集成。

[![仓库检查](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/repository-checks.yml/badge.svg?branch=main)](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/repository-checks.yml)
[![软件回归](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/software-tests.yml/badge.svg?branch=main)](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/software-tests.yml)

[验证记录](docs/testing/README.md) · [变更日志](CHANGELOG.md) · [参与开发](CONTRIBUTING.md) · [安全反馈](SECURITY.md) · [许可](LICENSE) · [第三方声明](THIRD_PARTY_NOTICES.md)

非保密的集成需求可通过 [Issue](https://github.com/TYF009/AuraBar-T113S3/issues/new/choose) 提供使用场景、目标系统和接口要求。商业支持、合规与交付承诺需要单独约定。
