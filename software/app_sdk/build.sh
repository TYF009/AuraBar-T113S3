#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
usage() { echo "Usage: $0 {-linux|-t113|-clean} [--jobs N]"; }
platform=""
jobs="${AURABAR_BUILD_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"
while (($#)); do
    case "$1" in
        -linux) platform=linux ;;
        -t113) platform=t113 ;;
        -clean) cmake -E remove_directory "$root/build"; exit 0 ;;
        --jobs) shift; jobs="${1:?--jobs requires a value}" ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; exit 2 ;;
    esac
    shift
done
[[ -n "$platform" && "$jobs" =~ ^[1-9][0-9]*$ ]] || { usage >&2; exit 2; }
args=(-S "$root" -B "$root/build/$platform" -DCMAKE_BUILD_TYPE="${AURABAR_BUILD_TYPE:-RelWithDebInfo}")
if [[ "$platform" == linux ]]; then
    args+=(-DCMAKE_TOOLCHAIN_FILE="$root/platform/x86linux/linux.cmake" -DSIMULATOR_LINUX=ON)
else
    toolchain="${TOOLCHAIN_PATH:-$HOME/toolchain-sunxi-glibc-gcc-830/toolchain/bin}"
    [[ -x "$toolchain/arm-openwrt-linux-gcc" ]] || { echo "Missing ARM compiler: set TOOLCHAIN_PATH to its bin directory" >&2; exit 1; }
    export STAGING_DIR="${STAGING_DIR:-$(dirname "$toolchain")/arm-openwrt-linux-gnueabi}"
    args+=(-DCMAKE_TOOLCHAIN_FILE="$root/platform/t113/t113.cmake" -DTOOLCHAIN_PATH="$toolchain" -DSIMULATOR_LINUX=OFF)
fi
cmake "${args[@]}"
cmake --build "$root/build/$platform" --parallel "$jobs"
