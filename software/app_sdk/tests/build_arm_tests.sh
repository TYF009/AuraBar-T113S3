#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
toolchain="${TOOLCHAIN_PATH:-$HOME/toolchain-sunxi-glibc-gcc-830/toolchain/bin}"
cc="$toolchain/arm-openwrt-linux-gcc"
export STAGING_DIR="${STAGING_DIR:-$(dirname "$toolchain")/arm-openwrt-linux-gnueabi}"
out="$root/build/tests-arm"
mkdir -p "$out/library"
python3 "$root/tests/generate_audio.py" "$out/library"
common=(-std=c11 -O2 -g -pthread -march=armv7-a -mtune=cortex-a7 -mfpu=neon -mfloat-abi=hard)
"$cc" "${common[@]}" -DPROJECT_RES_URL='"/usr/res/"' -I"$root/component/storage" "$root/tests/test_storage.c" "$root/component/storage/file_save.c" -o "$out/storage"
"$cc" "${common[@]}" -I"$root/component/osal" "$root/tests/test_queue.c" "$root/component/osal/osal_queue.c" -o "$out/queue"
"$cc" "${common[@]}" -DPROJECT_RES_URL='"/usr/res/"' -I"$root/component/storage" -I"$root/component/music" -I"$root/component/hal" -I"$root/third_party/minimp3" -I"$root/platform/t113/include" "$root/tests/test_music.c" "$root/component/music/music_player.c" "$root/component/music/pcm_output.c" "$root/component/storage/file_save.c" "$root/platform/t113/src/hal/em_hal_audio.c" -L"$root/platform/t113/lib" -Wl,-rpath-link,"$root/platform/t113/lib" -lcurl -lssl -lcrypto -lz -lnghttp2 -ldl -lm -o "$out/music"
"$cc" "${common[@]}" -I"$root/component/clock" "$root/tests/test_countdown.c" -o "$out/countdown"
"$cc" "${common[@]}" -DPROJECT_RES_URL='"/usr/res/"' -I"$root/component/storage" -I"$root/component/music" -I"$root/component/hal" -I"$root/third_party/minimp3" -I"$root/platform/t113/include" "$root/tests/audio_smoke.c" "$root/component/music/music_player.c" "$root/component/music/pcm_output.c" "$root/component/storage/file_save.c" "$root/platform/t113/src/hal/em_hal_audio.c" -L"$root/platform/t113/lib" -Wl,-rpath-link,"$root/platform/t113/lib" -lcurl -lssl -lcrypto -lz -lnghttp2 -ldl -lm -o "$out/audio-smoke"
echo "ARM test executables built"
