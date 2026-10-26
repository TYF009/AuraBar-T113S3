# Hardware design overview

The current T113-S3 board is the engineering reference. Product renders explore redesigned enclosures and interfaces; source electronics and actual captures establish what was built.

![Imported PCB geometry reference](../../assets/engineering/pcb-reference.png)

*Rendering from the supplied STEP, using illustrative materials. It does not establish assembly clearance or manufacturing acceptance.*

## Design rationale

| Decision | Engineering intent |
|---|---|
| T113-S3 with in-package DDR3 | Linux/display/audio capability with an accessible leaded package and fewer external memory interconnects |
| Separate supply rails | Supply the SoC, DDR and I/O domains according to the final schematic; four SY8088AAC devices are in the BOM |
| MIPI display + I²C touch + PWM | Keep pixel transport, touch input and backlight control as distinct responsibilities |
| SPI NAND and FEL entry | Provide persistent system storage and a documented recovery/programming path |
| Dual USB-C | Separate the main-power and development/debug roles, with isolation/protection in the circuit |
| Codec and LM4871 output | Support local audio and notifications without adding a separate media computer |
| RTL8723DS connections | Provide a networking hardware foundation; software acceptance is tracked separately |

## Source documents

- [LCEDA Pro project](../../hardware/lceda/6.22_Screen.eprj2)
- [Schematic PDF](../../hardware/schematic/AuraBar-Schematic-v1.0.pdf)
- [BOM spreadsheet](../../hardware/bom/AuraBar-BOM-v1.0.xlsx) and [CSV](../../hardware/bom/AuraBar-BOM-v1.0.csv)
- [Netlist](../../hardware/schematic/netlist.net) and [DXF sheets](../../hardware/schematic/dxf/)
- [Detailed design guide](../../hardware/docs/hardware-design-guide.md)
- [Interface map](interfaces.md) and [mechanical references](../../hardware/mechanical/README.md)

The supplied PCB STEP indicates approximately 75×38 mm board outline and 76.42×37.97×8.41 mm overall component envelope. These are model bounds, not an assembly drawing. CAD exports and a complete enclosure/screen mounting baseline are not yet packaged as a manufacturing release.

The readable guide is maintained against the final BOM; its original Word source is retained as historical design input. Early references may show alternate amplifier/power choices. Measured EMI, temperature, power and certification results are not implied by schematic intent.
