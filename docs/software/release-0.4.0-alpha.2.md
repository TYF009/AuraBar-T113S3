# Coding Companion 0.4.0-alpha.2 — scroll update

Date: 2026-10-25. Builds on [alpha.1](release-0.4.0-alpha.1.md), whose task tabs,
long-text scrolling and ambient audio were confirmed functional by the owner.
The owner also reported choppy scrolling on the task page.

## What changed

The 250 ms task-page callback previously reset every label and style even when
content did not change. It now compares text, colors, visibility and progress
before mutating LVGL objects. Static terminal tasks therefore no longer repeatedly
invalidate otherwise unchanged labels. Tab/selection actions refresh immediately,
instead of waiting for the next polling callback. The text panel disables elastic
overscroll and vertical scroll chaining, while retaining normal vertical scrolling.

Row taps also select the task id actually displayed, avoiding an index mismatch if
a new task arrives between rendering and the tap. This is a UI optimization, not
a claim of measured frame rate or a guarantee that all sources of latency are solved.

## Build and installation

The Ubuntu incremental T113 build coding-04-build-scroll-02 exited zero. The
4,737,676-byte binary was installed separately from alpha.1 and started on the board:

```text
/usr/bin/aurabar-coding-companion-0.4.0-alpha.2
SHA256 034354521bfd95e06bd23d98f1c2c05234c7141b2e319c3566397756dcb071d7
```

Host/device SHA256 and --version matched. The existing boot configuration and all
older preview executables were preserved. The alpha.2 archive passed verification
of all 142 manifest files, including the Codex schema and sample prompt:

```text
aurabar-coding-companion-0.4.0-alpha.2-t113.tar.gz
SHA256 2fca94dc3570427cd545a1466b74e340370ee6dfd0135047cfe87a3508aa7b24
```

The earlier full Ubuntu regression, ARM model test, real Codex result and USB
delivery records remain in alpha.1's acceptance document. The adapter, task protocol
and audio backend are unchanged in this update. GitHub checks cover the updated source.

## Remaining acceptance

- The owner has been asked to compare scrolling on alpha.2; feedback is pending.
- Ambient sound was audible on alpha.1, but notification interruption and recovery
  still need physical confirmation after the restart.
- The local Codex sandbox-helper permission issue still prevents claiming a verified
  project-file/command-execution workflow. Actual model acceptance used supplied code.

This remains a draft application-upgrade preview, not a bootable firmware image.
Review cards and AI-reported outcomes require human review. See the
[product scope](coding-companion.md) and [setup](../../software/companion/README.md).
