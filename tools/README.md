# Tools / 工具下载

Third-party tool binaries are **not committed** to this repository (file size + redistribution licensing). Download them from the official sources below.
第三方工具的二进制文件**不放入本仓库**（体积与版权原因），请从以下官方地址下载。

## 1. Allwinner T113-S3 firmware flashing / T113 固件烧录

- **PhoenixSuit** (Windows) / **LiveSuit** (Linux/macOS) — Allwinner FEL-mode flashing tools. Search the Allwinner developer portal / linux-sunxi community:
  - linux-sunxi: <https://linux-sunxi.org/Tools>
  - FEL mode entry: hold **SW1** while powering on the board via the debug USB-C (USB0 / USB10).
- Command-line alternative: `sunxi-fel` from the **sunxi-tools** project — <https://github.com/linux-sunxi/sunxi-tools>
- USB driver on Windows: Allwinner FEL USB driver (usually bundled with PhoenixSuit).

中文：T113 烧录使用全志 **PhoenixSuit（Windows）/ LiveSuit（Linux/macOS）**，或开源命令行 **sunxi-fel**；上电时按住 **SW1** 进入 FEL 模式（接调试 Type-C / USB10）。

## 2. Optional BLE module (MX-02) / 可选 BLE 模组烧录

If the MX-02 BLE-Mesh module is ESP-based, use the Espressif **Flash Download Tool** (the `flash_download_tool_3.9.11` folder in the original hardware workspace was prepared for this purpose):
若 MX-02 BLE Mesh 模组基于 ESP，可使用乐鑫 **Flash Download Tool**（原始硬件工作区中的 `flash_download_tool_3.9.11` 目录即为此准备）：

- Official download / 官方下载: <https://www.espressif.com/en/support/download/other-tools>
- Choose the target chip (e.g. ESP32-C3) and load the BLE-Mesh firmware `.bin` at the address given by the firmware build.
- 中文：选择对应目标芯片（如 ESP32-C3），按固件工程给出的地址加载 `.bin` 文件烧录。

## 3. EDA / 设计软件

- **LCEDA Pro / 嘉立创EDA专业版** (to open `hardware/lceda/6.22_Screen.eprj2`): <https://pro.lceda.cn/>
- Any CAD package supporting **DXF** can open the schematic exports under `hardware/schematic/dxf/`.
