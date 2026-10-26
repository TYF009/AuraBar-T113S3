# Software

The current source preview is **Coding Companion 0.4.0-alpha.2**. Music & Focus and Desktop Link are earlier iterations of the same shared platform.

| Area | Entry |
|---|---|
| Device application, LVGL and platform adaptation | [app_sdk](app_sdk/README.md) |
| Python service, USB bridge, process/Codex adapters | [companion](companion/README.md) |
| Experience and preview metadata | [profiles](profiles/) |
| Build/environment | [Development guide](../docs/development/README.md) |
| Install and connect | [Quick start](../docs/guides/quickstart.md) |
| Versions, compatibility and downloads | [Release catalog](../docs/releases/README.md) |

`app_sdk` stays in place to preserve tested build paths. Its components have explicit UI/service/platform boundaries; vendored libraries retain their own licenses. SDK binary libraries, local build outputs and user data are excluded from version control.

The current package script builds Coding Companion. Historical source revisions and prepared application bundles are documented separately; the profile metadata is not a promise of independent one-command feature builds.

Whole-system BSP images are not supplied. A compatible Tina system and resources are required. [Software license](LICENSE) · [Third-party notices](../THIRD_PARTY_NOTICES.md)
