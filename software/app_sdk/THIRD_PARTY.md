# Third-party components

AuraBar application changes follow the software license in ../LICENSE.
That license does not replace third-party component licenses.

- **LVGL 8.3.1**, with the supplied platform modifications: MIT.
  https://github.com/lvgl/lvgl/tree/v8.3.1
- **minimp3**: CC0/public-domain dedication. Full text is in
  third_party/minimp3/LICENSE; source: https://github.com/lieff/minimp3.
- Platform headers/libraries include APIs from curl, OpenSSL, FreeType, zlib,
  bzip2, nghttp2, cJSON, ALSA and wpa_supplicant. Their original notices and licenses
  apply. Obtain runtime binaries and redistribution notices from the corresponding SDK.
- ALSA is loaded dynamically from the existing system library.
- Generated regression audio and procedural textures contain no third-party recordings.
- Existing UI fonts, images and notification sounds remain project-provided assets.
  Keep their provenance/licensing records with product distribution.

Unused LVGL demo applications and platform SDK binary libraries are excluded from Git.
