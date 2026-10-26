# AuraBar

### An embedded Linux display platform for connected products and AI workspaces

[中文](README.zh-CN.md) · [Get started](docs/guides/quickstart.md) · [Versions](docs/releases/README.md) · [Documentation](docs/README.md)

![AuraBar AI workspace concept](assets/renders/ai-workspace.webp)

*Product concept visualization: redesigned enclosure and interface. Working hardware and software are documented separately.*

AuraBar gives information a place outside the main computer window. A T113-S3 Linux board, native touch UI, audio services and a computer companion form a reusable display platform. The current application preview brings music, real task status and Codex review cards to the desk.

The architecture also provides a foundation for embedded appliance interfaces. Connected-home and auxiliary vehicle scenes illustrate future applications; device-specific integration has its own [readiness status](docs/product/status.md).

## Three ways to use AuraBar

| AI workspace | Music and focus | Embedded products |
|---|---|---|
| Follow companion-launched Codex tasks. Read Summary, Evidence and Next cards; review code on the computer. | Play local WAV/MP3 and synthesized ambient sound, with playback controls and sleep timing. | Adapt the platform to appliances, furniture and information panels. Dedicated integrations are planned. |

| Connected living concept | Auxiliary vehicle display concept |
|---|---|
| ![Connected kitchen](assets/renders/connected-kitchen.webp) | ![Parked vehicle display](assets/renders/vehicle-display.webp) |

## Working software

**Coding Companion 0.4.0-alpha.2** is the current source preview, built on Music & Focus and Desktop Link.

- **Visible task state:** receive actual process and Codex CLI events through the existing USB debug port.
- **Useful review cards:** distinguish AI-generated summaries from observed command exits and human follow-up actions.
- **Explicit connectivity:** mark stale snapshots offline and resynchronize after reconnection.
- **Everyday audio:** local WAV/MP3, synthesized brown noise and rain/ocean textures, playback controls and sleep timing.
- **Native device UI:** LVGL and audio run on the board; Codex and model access stay on the computer.

![Actual T113 framebuffer](assets/screenshots/coding-companion.png)

*Actual board capture. Board validation confirmed tab navigation, scrolling, improved responsiveness and ambient playback/pause. Notification recovery has automated coverage; its final physical listening check remains open.*

This preview tracks tasks launched through the companion. Desktop-app-wide chat synchronization, screen approvals and screen-triggered tasks are outside its scope. Real AI acceptance used code supplied in the prompt; project-file execution on the development computer encountered a Codex sandbox-helper permission issue. See the [capability matrix](docs/product/status.md).

## Choose an experience

| Preview | Primary purpose | Evidence |
|---|---|---|
| Music & Focus 0.2.0-alpha.1 | Music, ambient audio and focus tools | [Acceptance](docs/software/release-0.2.0-alpha.1.md) |
| Desktop Link 0.3.0-alpha.1 | Generic computer tasks, elapsed time and results | [Acceptance](docs/software/release-0.3.0-alpha.1.md) |
| **Coding Companion 0.4.0-alpha.2** | **Codex events and structured review cards, with the audio base** | [Acceptance](docs/software/release-0.4.0-alpha.2.md) |

These are successive previews of one platform. Historical builds retain their original source revisions. The current packager targets Coding Companion; one-command builds of all profiles are not yet available.

Prepared binaries are **application upgrades for a matching Tina system**. A complete, publicly reproducible BSP image is not supplied yet. Download availability and compatibility are explicit in the [release catalog](docs/releases/README.md).

## Documented hardware

| Subsystem | Current design |
|---|---|
| Compute | Allwinner T113-S3, dual Cortex-A7, 128 MB in-package DDR3 |
| Storage | W25N01GVZEIG, 1 Gbit SPI NAND |
| Interaction | MIPI DSI display, I²C touch and PWM backlight |
| Audio | SoC codec, microphone input and LM4871 speaker amplifier |
| Connectivity | RTL8723DS hardware; USB10 is the verified companion transport |
| Engineering | LCEDA Pro project, seven schematic sheets, BOM and netlist |

Hardware connectivity and enabled software are tracked separately. Bluetooth audio, BLE Mesh control and physical Wi-Fi transport acceptance remain open. Explore the [design overview](docs/hardware/README.md), [circuit guide](hardware/docs/hardware-design-guide.md) and [interface map](docs/hardware/interfaces.md).

## Find your entry point

| Goal | Guide |
|---|---|
| Try the software on a matching board | [Quick start](docs/guides/quickstart.md) |
| Install, verify and recover | [Installation and rollback](docs/guides/installation.md) |
| Diagnose display, audio, USB or Codex issues | [Troubleshooting](docs/guides/troubleshooting.md) |
| Build or change the software | [Development](docs/development/README.md) |
| Understand services and data flow | [Architecture](docs/architecture/README.md) · [Protocol](docs/architecture/task-protocol.md) |
| Evaluate an integration | [Status](docs/product/status.md) · [Roadmap](docs/product/roadmap.md) |

## Repository map

```text
hardware/       Electronics sources, schematics, BOM and mechanical reference entry
software/       Device runtime, companion and profile metadata
docs/           Product, architecture, guides, development, testing and release records
scripts/        Repository checks and environment diagnostics
assets/         Concept visuals, actual screenshots and provenance
.github/        Automated checks and contribution templates
```

The verified device build remains under `software/app_sdk/`. Self-authored components, vendored LVGL and platform adaptation retain clear boundaries and notices.

## Engineering and participation

[![Software regression](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/software-tests.yml/badge.svg?branch=main)](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/software-tests.yml)
[![Repository checks](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/repository-checks.yml/badge.svg?branch=main)](https://github.com/TYF009/AuraBar-T113S3/actions/workflows/repository-checks.yml)

Host regressions cover storage, queues, timers, music and task transport. ARM builds and board checks are recorded explicitly; badges do not certify hardware or a full BSP build. See [validation](docs/testing/README.md), [changelog](CHANGELOG.md), [contributing](CONTRIBUTING.md) and [security reporting](SECURITY.md).

Hardware uses CERN-OHL-S-2.0, software uses MIT, and documentation follows [LICENSE](LICENSE). Original hardware-guide notices are retained. Third-party materials retain their own terms: [notices and provenance](THIRD_PARTY_NOTICES.md).
