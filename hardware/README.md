# Hardware Sources / 硬件设计资料

| Directory / 目录 | Purpose / 用途 |
|---|---|
| [lceda](lceda/) | Editable LCEDA Pro electronics project / 可编辑 EDA 工程 |
| [schematic](schematic/) | PDF, DXF and netlist exports / 原理图与网表导出 |
| [bom](bom/) | Spreadsheet and CSV bill of materials / 物料表 |
| [docs](docs/hardware-design-guide.md) | Bilingual circuit handbook and published figures / 双语电路手册与公开图片 |
| [mechanical](mechanical/README.md) | Enclosure provenance and mechanical-reference status / 外壳来源与机械参考状态 |

Start with the [hardware design guide / 硬件设计指南](../docs/hardware/README.md), then use the final schematic/BOM as the source of component and interface facts. This repository is not yet a complete, verified manufacturing handoff. [Hardware license](LICENSE) retains its original scope.

先阅读[硬件设计指南](../docs/hardware/README.md)，再通过最终原理图、BOM 和网表核对器件与连接。仓库尚未形成已完整验证的制造交付包，[硬件许可](LICENSE)维持原有范围。

The [figure index / 图像索引](docs/figure-index.md) maps all original Word images to preserved files and records the final schematic previews. Run `python scripts/check_hardware_sources.py` from the repository root to verify the published image and final schematic hashes; no author draft is required.

[图像索引](docs/figure-index.md)记录原始 Word 图片与保留文件的对应关系，并列出最终原理图预览。在仓库根目录运行 `python scripts/check_hardware_sources.py` 可独立检查公开图片与最终原理图哈希，无需作者原稿。
