# AuraBar software editions

The hardware and enclosure are fixed. Software editions share the same display,
touch, storage and audio base; each edition has its own version and release assets.

| Edition | Status | Contents |
|---|---|---|
| Music & Focus | 0.2.0-alpha.1 preview | Music, procedural ambient sound, focus tools |
| AI Assistant | Planned | Voice/text interaction and approved device actions |
| Desktop Link | 0.3.0-alpha.1 preview | Real computer-task status, USB companion and completion notifications; includes the music base |

See the [current application source and build guide](app_sdk/README.md).
For the current source version, see [Desktop Link setup](companion/README.md).
[Desktop Link acceptance record / 实机验收记录](../docs/software/release-0.3.0-alpha.1.md)
The 0.2.0 preview remains on its original branch/release; this source builds 0.3.0.
![Music page captured from the T113 framebuffer](../docs/software/images/music-focus-board.png)

[Preview acceptance record / 预览版验收记录](../docs/software/release-0.2.0-alpha.1.md)
[Profile metadata](profiles/music-focus.json) explicitly distinguishes application
upgrade packages from bootable firmware images.

## Release policy

- Every tested edition receives an immutable version/tag and checksummed assets.
- A preview is not described as production-ready.
- The present deliverable is an **application upgrade**, used with the existing,
  tested Tina Linux base. It does not replace the bootloader or device tree.
- Complete flashable images require the matching BSP, kernel/device-tree sources
  and image packaging project; those are not included in this repository yet.
- Existing versions remain available so visitors can choose an edition.
- User data is kept outside the read-only asset directory.

中文：当前先开发音乐与专注版本。源码、应用升级包与完整系统镜像会明确区分，
未经实机验证的功能不会标成已完成；后续版本保留下载和回退说明。
