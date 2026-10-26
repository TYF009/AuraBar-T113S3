# Release catalog and compatibility

Current source: **0.4.0-alpha.2**. All four prepared binary publications currently remain **draft prereleases**, so visitors cannot download those draft assets. The public [Releases page](https://github.com/TYF009/AuraBar-T113S3/releases) is the authoritative download entry once a publication is released.

| Experience/build | Source revision | Binary availability | Evidence |
|---|---|---|---|
| Music & Focus 0.2.0-alpha.1 | `2e2883d` | Draft | [Record](../software/release-0.2.0-alpha.1.md) |
| Desktop Link 0.3.0-alpha.1 | `06eb464` | Draft | [Record](../software/release-0.3.0-alpha.1.md) |
| Coding Companion 0.4.0-alpha.1 | `1486971` | Draft, superseded preview | [Record](../software/release-0.4.0-alpha.1.md) |
| **Coding Companion 0.4.0-alpha.2** | `07a8a17` | **Draft, recommended source preview** | [Record](../software/release-0.4.0-alpha.2.md) |

The source revisions identify the original application preview, not later documentation updates. Release hashes are recorded in the acceptance documents and machine-readable [catalog](../../software/releases.json).

## Compatibility baseline

| Requirement | Current preview |
|---|---|
| Board | Existing AuraBar T113-S3 design |
| System | Matching Tina development image, kernel 5.4.61 in physical acceptance |
| Display | Verified fl7707n_mipi setup; 720×1424 framebuffer / 1424×280 visible UI mapping |
| Resources | Matching `/usr/res` images, fonts and music assets |
| Runtime | SDK-compatible runtime libraries, `libasound.so.2`, `libcurl.so.4`, `aplay` |
| Companion | Python 3.10+ and ADB; Codex CLI/login separately for Codex tasks |
| First transport | USB10; physical Wi-Fi task transport not accepted yet |
| Startup | Manual preview launch; installer leaves existing boot configuration unchanged |

An application archive is not a PhoenixSuit image. Blank-board installation cannot be completed solely from the current application source. [Install/rollback](../guides/installation.md) and [build dependencies](../development/README.md) are prerequisites, not optional footnotes.

## Publication gate

Before making a binary draft public, verify its manifest/hash, target commit, hardware/system baseline, installation path, rollback, known issues and notices for bundled fonts/images/audio. Several project-provided resource notices still need a provenance review; see [third-party records](../../THIRD_PARTY_NOTICES.md). Existing hardware/software code licenses are not silently applied to third-party media.

[Release procedure](process.md) · [Changelog](../../CHANGELOG.md)
