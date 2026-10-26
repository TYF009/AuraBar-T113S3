# Board interface map

Design reference for the existing T113-S3 board. Check the final schematic, installed image and actual connector orientation before wiring. Runtime pin multiplexing is image-dependent.

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

[Design overview](README.md) · [Schematic](../../hardware/schematic/AuraBar-Schematic-v1.0.pdf)
