#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT
cc="${CC:-cc}"
common=(-std=c11 -Wall -Wextra -g -pthread)
"$cc" "${common[@]}" -Werror -DPROJECT_RES_URL='"/nonexistent/"' -I"$root/component/storage" "$root/tests/test_storage.c" "$root/component/storage/file_save.c" -o "$out/storage"
"$out/storage"
"$cc" "${common[@]}" -Werror -I"$root/component/osal" "$root/tests/test_queue.c" "$root/component/osal/osal_queue.c" -o "$out/queue"
"$out/queue"
"$cc" "${common[@]}" -I"$root/component/clock" "$root/tests/test_countdown.c" -o "$out/countdown"
"$out/countdown"
python3 -m unittest discover -s "$root/../companion" -v
json_flags=(-I"$root/platform/x86linux/include" "$root/platform/x86linux/lib/libcjson.a")
if pkg-config --exists libcjson; then
    read -r -a json_libs <<< "$(pkg-config --cflags --libs libcjson)"
    json_flags=(-I"$root/platform/x86linux/include" "${json_libs[@]}")
fi
"$cc" "${common[@]}" -Werror -I"$root/component/tasks" "$root/tests/test_task_model.c" "$root/component/tasks/task_model.c" "${json_flags[@]}" -lm -o "$out/task-model"
"$out/task-model"
export AURABAR_DATA_DIR="$out/data"
python3 "$root/tests/generate_audio.py" "$out/library"
curl_flags=(-I"$root/platform/x86linux/include" -l:libcurl.so.4)
if pkg-config --exists libcurl; then read -r -a curl_flags <<< "$(pkg-config --cflags --libs libcurl)"; fi
"$cc" "${common[@]}" -Werror -I"$root/component/tasks" "$root/tests/task_probe.c" "$root/component/tasks/task_model.c" "$root/component/tasks/task_status.c" "${json_flags[@]}" "${curl_flags[@]}" -lm -o "$out/task-probe"
python3 "$root/tests/test_task_client.py" "$out/task-probe"
"$cc" "${common[@]}" -DSIMULATOR_LINUX -DPROJECT_RES_URL='"/usr/res/"' -I"$root/component/storage" -I"$root/component/music" -I"$root/component/hal" -I"$root/third_party/minimp3" "$root/tests/test_music.c" "$root/component/music/music_player.c" "$root/component/music/pcm_output.c" "$root/component/storage/file_save.c" "$root/platform/x86linux/src/hal/em_hal_audio.c" "${curl_flags[@]}" -ldl -lm -o "$out/music"
if [[ -f "$out/library/02-test-tone.mp3" ]]; then
    python3 "$root/tests/test_http_music.py" "$out/music" "$out/library"
else
    AURABAR_MUSIC_DIR="$out/library" "$out/music"
fi
if [[ -n "${TOOLCHAIN_PATH:-}" ]]; then bash "$root/tests/build_arm_tests.sh"; fi
