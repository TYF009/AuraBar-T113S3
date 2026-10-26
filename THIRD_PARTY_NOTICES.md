# Third-party notices and provenance

Repository component licenses are scoped in [LICENSE](LICENSE). They do not replace notices for dependencies, media or external models.

| Material | Record |
|---|---|
| LVGL 8.3.1 | Vendored source and [MIT notice](software/app_sdk/lvgl/LICENCE.txt) |
| minimp3 | [CC0/public-domain dedication](software/app_sdk/third_party/minimp3/LICENSE) |
| Platform APIs and runtime libraries | curl, OpenSSL, FreeType, zlib, bzip2, nghttp2, cJSON, ALSA and wpa_supplicant; matching SDK notices required |
| Audio fixtures / synthesized textures | Project-generated regression tones and procedural output; not third-party music recordings |
| Existing UI fonts/images/notification sound | Project-provided source assets; complete redistribution records are not yet collected |
| Concept visuals | Generated with imagegen for this project, clearly classified in the [asset registry](assets/README.md) |
| External enclosure | [Source reference](hardware/mechanical/README.md); original STL terms are not replaced by the electronics license |

The [resource provenance record](software/resource-provenance.json) captures exact font hashes and embedded notices:

- `Aqum-2-Classic-2.otf` has a copyright notice but no embedded license text or license URL. Its original distribution license remains unrecorded.
- `Library-3-am-3.otf` embeds a free-use agreement from Igor Kosinsky, including a restriction on selling the fonts. The embedded text is recorded verbatim.
- These particular Source Han Sans CN light/regular files embed 2014 Adobe Apache-2.0 notices; do not infer their terms from a different modern font release.

Embedded metadata is evidence, not a complete redistribution review. Before public binary publication, collect applicable distribution notices and verify bundled image/audio provenance. Prepared binaries remain drafts while this release item is open.

The existing hardware guide and figures retain the scope of [hardware/LICENSE](hardware/LICENSE); newly authored repository documentation follows the root documentation scope. No existing third-party component has been relicensed by this reorganization.

Detailed application dependency notes: [THIRD_PARTY.md](software/app_sdk/THIRD_PARTY.md).
