# Reproduce AuraBar / 复现指南

Choose a path that matches your starting point. This guide connects the available source and documented acceptance steps; it does not imply that a blank board can boot from this repository alone.

请按现有条件选择路径。本指南串联公开源码与验收流程；空白板卡仍需要匹配的 BSP、SDK 和系统资源。

| Starting point / 起点 | What you can reproduce / 可复现内容 | Required outside this repository / 额外条件 |
|---|---|---|
| Computer only / 仅电脑 | Companion unit tests; native host regressions on Linux / 伴侣测试、Linux 原生回归 | Python 3.10+; Linux test dependencies / 测试依赖 |
| Matching provisioned board / 已运行兼容系统的板卡 | Device UI, task sync, audio and supplied-code review / 界面、任务、音频与给定代码审查 | Matching Tina image, runtime resources, ARM SDK; Codex setup for AI / 匹配系统、资源与 SDK |
| New electronics build / 新制造板卡 | Inspect/adapt EDA, schematics, BOM and documented bring-up sequence / 检查适配 EDA、电路和调试流程 | Fabrication/assembly outputs, component sourcing, BSP and hardware verification / 制造输出、采购、BSP 与硬件验证 |

## 1. Inspect and check / 检查资料

```sh
git clone https://github.com/TYF009/AuraBar-T113S3.git
cd AuraBar-T113S3
python scripts/check_repository.py
python scripts/check_hardware_sources.py
python scripts/doctor.py --target repo
python -m unittest discover -s software/companion -v
```

Python tests exercise local sockets and subprocess fixtures; run them in an environment permitting loopback. They do not require a model account. Native C/audio tests and ARM prerequisites are in [development](../development/README.md).

Python 测试使用本地回环与子进程，不需要模型账户；原生 C／音频测试和 ARM 条件见[开发环境](../development/README.md)。

## 2. Match hardware and runtime / 匹配硬件与运行环境

Use the [hardware guide](../hardware/README.md), final schematic and BOM together. Historical reference circuits are labeled in the handbook; do not treat every reference figure as the populated board.

先核对硬件指南、最终原理图和 BOM，手册中历史参考方案不等于实际装配电路。

The verified software baseline uses Tina kernel 5.4.61, the fl7707n_mipi setup and a 720×1424 framebuffer mapped to the 1424×280 visible UI. Match runtime libraries and `/usr/res`. Check the [release compatibility table](../releases/README.md) before deployment. Application archives are not full flash images; prepared binary drafts are not public downloads.

当前应用包不是完整烧录镜像，预编译草稿不对公众提供下载。系统、屏幕映射、动态库和资源需与验收基线匹配。

## 3. Build, verify and install / 构建、校验与安装

Follow [ARM development](../development/README.md) for the toolchain and SDK, then [installation and rollback](installation.md). Verify the package manifest before deployment and retain the prior application for rollback. Follow [quick start](quickstart.md) to run one device process, the companion service and the USB bridge.

按 ARM 开发说明准备工具链及 SDK，再执行安装和回退流程。部署前校验包，保留旧程序，并按快速开始运行唯一板端进程、伴侣服务与 USB 桥。

## 4. Demonstrate and record / 演示与留证

| Check / 检查 | Expected observation / 预期结果 | Record / 留证 |
|---|---|---|
| Generic successful command / 成功命令 | Connected → running → completed, actual exit code / 连接、运行、完成与真实退出码 | Task log and capture / 日志与截图 |
| Deliberate failed command / 失败命令 | Failed state with non-zero exit, not a false success / 非零退出显示失败 | Exit evidence / 退出证据 |
| USB disconnect/reconnect / USB 断开重连 | Stale state marked offline, then resynchronization / 离线标记后恢复同步 | Physical observation / 实物观察 |
| Review cards / 审查卡片 | Summary, Evidence, Next readable and scrollable / 卡片可读、可滚动 | Supplied-code review and screen / 给定代码审查及屏幕 |
| Ambient audio / 环境音 | Play, pause and navigation work on actual speaker/display / 实物播放暂停与切页正常 | Listening observation / 听感记录 |
| Completion while playing / 播放中完成任务 | Notification and expected playback restoration / 提示及预期播放恢复 | Physical listening check required / 需实物听感验收 |

Record source revision, package hash, board/system version, exact command, observed result and exceptions. This table is a repeatable evaluation procedure, not a claim that every deployment has already passed. The last notification/music physical acceptance remains open in the existing baseline.

记录源码提交、包哈希、板卡系统版本、命令、观察结果及异常。此表是复现流程，不代表每套环境已经通过；现有基线的提示音／音乐恢复最终实物验收仍待完成。

## 5. Prepare a product variant / 准备产品方案

Keep transport and task schema boundaries when adding an adapter. Review [security](../../SECURITY.md), [resource provenance](../../THIRD_PARTY_NOTICES.md), [mechanical references](../../hardware/mechanical/README.md) and the [integration brief](../product/integration-brief.md). Define the deployment-specific acceptance criteria before presenting a variant as production-ready.

新增适配器时保持协议边界；产品化前核对安全、资源来源、结构适配与集成条件，先定义并通过场景验收，再承诺正式交付。
