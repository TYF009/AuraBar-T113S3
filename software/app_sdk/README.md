# AuraBar Desktop Link — 0.3.0-alpha.1

Native LVGL 8.3.1 application for the fixed AuraBar T113-S3 board and enclosure.
Includes the Music & Focus base plus the [Desktop Link task companion](../companion/README.md).
The Tasks page displays real computer-task state, result details and heartbeat
status; music remains active across page changes. No AI model is bundled yet.

## Implemented in this preview

- Music page with a bounded queue, local PCM16 WAV and MP3 decoding.
- Direct HTTP/HTTPS MP3 streams configured in stations.m3u.
- Pause/resume, previous/next, local-file seek, repeat queue, persistent favorites.
- Monotonic sleep timer.
- Procedurally generated brown noise, rain-like and ocean-like textures.
- Notification audio temporarily releases the music device, then resumes playback.
- Existing weather, To-Do, alarms and Pomodoro UI.
- Storage, threading, countdown, build and deployment fixes described in
  [the engineering review](../../docs/software/review-v0.2.md).

Not included yet: Bluetooth audio, AAC/HLS, DRM music services, album artwork,
synchronized lyrics, AI and automatic whole-system updates.
Rain/ocean entries are synthesized textures, not field recordings.

## Build

Use Linux with CMake >=3.15, a C/C++ compiler and the platform's development dependencies.
ARM binary dependencies are supplied by the matching Tina SDK, not committed
as anonymous binaries. Populate platform/t113/lib from that SDK before cross-compiling.
Existing development workspaces already contain these libraries.

~~~sh
# PC simulator: development packages for SDL2, FreeType, curl, cJSON,
# OpenSSL, nghttp2, bz2, zlib and wpa_client are required.
bash build.sh -linux

# T113: directory containing arm-openwrt-linux-gcc/g++
export TOOLCHAIN_PATH="$HOME/toolchain-sunxi-glibc-gcc-830/toolchain/bin"
bash build.sh -t113 --jobs 4
~~~

Outputs: build/linux/app/demo and build/t113/app/demo.
Each platform has a separate CMake cache; failed commands return a failure status.
Build type defaults to RelWithDebInfo; AURABAR_BUILD_TYPE=Release selects a release build.

~~~sh
bash tests/run.sh
python3 scripts/package.py --platform t113 --version 0.3.0-alpha.1
python3 scripts/verify_package.py dist/aurabar-desktop-link-0.3.0-alpha.1-t113.tar.gz
~~~

Tests use ALSA null; they do not prove speaker quality.
An original generated MP3 fixture is included solely for regression testing.
Separate real-speaker tests on the T113 board played both generated formats;
the owner confirmed sound and display on 2026-10-25.

## Install / roll back

This preview is an application upgrade for the already verified Tina image.
The archive contains bin/aurabar, assets, license notices and a SHA256 manifest;
it is not a PhoenixSuit flash image. Do not flash it as one.

The helper installs a new versioned binary and checks its SHA256 without changing
startup or replacing /usr/bin/demo. Matching /usr/res resources must already exist.

~~~sh
python3 scripts/deploy.py --adb /path/to/adb --serial YOUR_SERIAL
~~~

Stop the existing UI before starting the new executable from /usr/bin.
Use --music to open the music page. For rollback, stop the new process and
restart /usr/bin/demo. Keep only one display application running at a time.
Reboot currently returns to the existing system startup configuration.

## Music library

On T113, add WAV (PCM, 16-bit, mono/stereo) or MP3 files to /usr/res/music/library.
The queue is capped at 64 entries, including streams and three synthesized ambient
entries. Stop playback and restart the application to rescan in this preview.

Optional stations.m3u in the same directory:

~~~m3u
#EXTM3U
#EXTINF:-1,Your MP3 stream
https://your-server.example/audio.mp3
~~~

Only direct MP3 audio is supported for network sources. HLS playlists, AAC,
login/DRM streaming and live-stream seek are not implemented.
Streaming retains a bounded compressed buffer rather than downloading the whole track.

AURABAR_MUSIC_DIR selects a different library directory.
User-added music is not bundled with a release.

## Runtime configuration

| Variable | Default / purpose |
|---|---|
| AURABAR_MUSIC_DIR | /usr/res/music/library |
| AURABAR_DATA_DIR | /usr/var/lib/aurabar; simulator uses its build directory |
| AURABAR_AUDIO_DEVICE | ALSA default; null is for automated tests |
| AURABAR_CA_BUNDLE | Optional CA bundle path; certificate verification stays enabled |

Assets remain under /usr/res. Keep exactly one image, font and music
directory below that root. Pushing a directory into an existing directory of the
same name can accidentally create image/image or font/font.

~~~sh
/usr/bin/aurabar-desktop-link-0.3.0-alpha.1 --music
/usr/bin/aurabar-desktop-link-0.3.0-alpha.1 --tasks
/usr/bin/aurabar-desktop-link-0.3.0-alpha.1 --version
~~~

The current physical panel works with the verified 720x1424 framebuffer and
1424x280 visible UI mapping. Do not rewrite panel timings based solely on visible width.

Relative countdowns restart stopped after an application restart. Tasks and favorites
persist. Sleep timers are process-local and do not resume after restart.

## Architecture

- LVGL runs on the main thread.
- HTTP and Wi-Fi results are delivered back to that thread.
- The music worker owns its ALSA output and decoder.
- A separate notification worker coordinates with the music worker.
- Storage writes exact records via a synchronized temporary file and atomic rename.

See [third-party notices](THIRD_PARTY.md) and the repository's software license.

中文：此版本以真实可用的音乐、背景声和专注体验为目标，属于预览版。
不要把应用升级包当作可直接烧录的完整系统镜像。升级前保留当前可恢复版本。
