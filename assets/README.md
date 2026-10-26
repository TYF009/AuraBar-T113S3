# Visual asset registry

The [machine-readable manifest](manifest.json) records classification, provenance and SHA256 for public visual assets.

| Directory | Meaning | Allowed interpretation |
|---|---|---|
| `renders/` | AI-generated product concepts | Future industrial design and application scenarios; illustrative UI/data |
| `screenshots/` | Actual board framebuffer | Recorded UI appearance; not proof of every physical interaction |
| `engineering/` | Actual supplied CAD geometry rendered with illustrative materials | Geometric reference; not assembly/manufacturing acceptance |

The three concept images were generated on 2026-10-26 using imagegen. The desktop image established the product language for the kitchen and vehicle images. Original PNG masters remain in the project production workspace; optimized WebP copies are used here. Temperature, charging and file-count displays are example data.

Screenshots link to the [acceptance records](../docs/testing/README.md). Geometry-reference colors and lighting do not represent measured physical materials. Electronics, enclosure reference and product concept identities remain distinct.
