# Troubleshooting

Capture the software version, board revision, baseline image, last action and the smallest relevant log before changing configuration. Redact tokens, credentials and private task content.

| Symptom | Check |
|---|---|
| ADB list is empty | USB10 debug port, data cable, host driver and `adb devices`; power alone is insufficient |
| Display is blank or corrupted | One active display app, matching panel setup, complete assets, no accidental `image/image` or `font/font` nesting |
| UI width differs from framebuffer | Preserve the verified 720×1424 framebuffer and 1424×280 visible mapping; do not rewrite device-tree timing from UI size |
| Waiting for companion / Disconnected | Service and bridge running, selected device serial, token path and fresh snapshot updates; unchanged old files do not establish a live link |
| HTTP unauthorized | All companion commands must use the same token file; place global `--token-file` before the subcommand |
| Codex fails before result | Confirm CLI login/configuration, configured provider/network/proxy, timeout and local stderr; view the run's `result.json` |
| Codex cannot read project files | The development PC hit a sandbox-helper directory permission issue. Fix the CLI environment; the supplied-code static example can verify model/card transport independently |
| Card says completed but tests are unclear | Read Evidence. Exit codes and AI notes do not establish test-suite coverage; inspect local logs and run appropriate tests |
| No audio | Library format/path, mixer, speaker path, pause state, one PCM owner and notification activity; use low-volume fixtures to isolate the chain |
| Network station fails | Only direct MP3 streams are supported; check actual server status, format and CA chain; HLS/AAC/DRM are not supported |
| New library files are missing | Current preview rescans on application restart; queue capacity is bounded |
| History vanished | Companion task history is in memory. USB reconnect and service restart have different recovery semantics |
| Restart returns to old UI | Startup is intentionally unchanged by the deploy helper; previews are launched manually |

## Useful read-only diagnostics

```sh
adb devices
adb -s YOUR_SERIAL shell uname -a
adb -s YOUR_SERIAL shell cat /sys/class/graphics/fb0/bits_per_pixel
python scripts/doctor.py --target companion --adb /path/to/adb
```

The installer does not define a universal log path; logs depend on how you launched the process. Do not upload raw Codex event logs without checking them for private project data.

[Report a reproducible issue](https://github.com/TYF009/AuraBar-T113S3/issues/new/choose) · [Security reporting](../../SECURITY.md)
