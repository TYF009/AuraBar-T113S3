# Development environment

The repository has two build scopes: a standard-library Python companion and a native C/LVGL device application. A full Tina BSP/kernel/image build is outside the supplied source baseline.

## Companion

Python 3.10+ is required. The service, bridge and adapter use only the standard library; a model or API key is not needed for local regressions.

```sh
python -m unittest discover -s software/companion -v
python scripts/check_repository.py
python scripts/doctor.py --target companion --adb /path/to/adb
```

The tests use local loopback sockets and subprocess fixtures. On restricted Windows sandboxes, local networking may be denied independently of application correctness. Do not work around that by disabling authentication.

## Native host regressions

Ubuntu 24.04 CI installs build-essential, pkg-config, curl/cJSON development libraries, ALSA runtime and ffmpeg. Then:

```sh
bash software/app_sdk/tests/run.sh
```

These tests use ALSA null and fixtures, not a physical speaker. Full simulator builds additionally need SDL2, FreeType, platform libraries and wpa_client; the host test job is not a complete simulator-build guarantee.

## T113 application

Known compiler: `arm-openwrt-linux-gcc` 8.3 from the matching sunxi/Tina toolchain. Use Linux, CMake 3.15+ and the matching SDK libraries under `software/app_sdk/platform/t113/lib`.

```sh
export TOOLCHAIN_PATH=/path/to/toolchain/bin
python3 scripts/doctor.py --target arm
cd software/app_sdk
bash build.sh -t113 --jobs 4
python3 scripts/package.py --platform t113
python3 scripts/verify_package.py dist/aurabar-coding-companion-0.4.0-alpha.2-t113.tar.gz
```

SDK libraries are intentionally not committed. The repository does not yet provide a universally obtainable, redistributable SDK bundle or container. Obtain matching headers/libraries from your board's SDK and preserve their license notices. A toolchain path alone is insufficient. The environment checker reports missing items; it does not fetch or certify them.

Outputs remain under `software/app_sdk/build/{linux,t113}` and archives under `dist`. [Application details](../../software/app_sdk/README.md) cover runtime variables and audio sources.

## Changes and versions

`VERSION` identifies the current application preview. `scripts/check_repository.py` checks its agreement with executable text, About text, current profile and package/deploy defaults. Historical documents and profiles retain their own versions. Generated build metadata is a future improvement; the current check prevents silent disagreement across existing definitions.

Run the affected tests and document evidence. Keep directory moves separate from behavior changes. Never commit local SDK binaries, user music, tokens, raw private logs or workstation build helpers. [Contributing](../../CONTRIBUTING.md) describes review expectations.
