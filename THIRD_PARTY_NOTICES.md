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

Font files include `Aqum-2-Classic-2.otf`, `Library-3-am-3.otf` and Source Han Sans CN light/regular files. Their embedded names are not a complete license record. Before public binary publication, collect the applicable font notices and verify bundled image/audio provenance. Prepared binaries remain drafts while this release item is open.

The existing hardware guide and figures retain the scope of [hardware/LICENSE](hardware/LICENSE); newly authored repository documentation follows the root documentation scope. No existing third-party component has been relicensed by this reorganization.

Detailed application dependency notes: [THIRD_PARTY.md](software/app_sdk/THIRD_PARTY.md).
