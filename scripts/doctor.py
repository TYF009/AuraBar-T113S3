"""Read-only environment diagnosis; does not install dependencies or read credentials."""
import argparse
import json
import os
from pathlib import Path
import platform
import shutil
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target', choices=('repo', 'companion', 'host', 'arm'), default='repo')
    parser.add_argument('--adb', help='ADB executable or absolute path')
    parser.add_argument('--toolchain', help='Directory containing arm-openwrt-linux-gcc')
    parser.add_argument('--json', action='store_true')
    args = parser.parse_args()
    records = []

    def record(name, ok, detail):
        records.append(dict(name=name, status='ok' if ok else 'missing', detail=detail))

    record('python', sys.version_info >= (3, 10), platform.python_version())
    record('repository metadata', (ROOT / 'VERSION').is_file(), 'VERSION in repository root')
    if args.target == 'companion':
        executable = args.adb or 'adb'
        record('adb', bool(shutil.which(executable)), 'Use --adb /path/to/adb when it is not on PATH')
        records.append(dict(name='Codex', status='info', detail='Optional for generic tasks; configure the official CLI separately for Codex tasks'))
    if args.target in ('host', 'arm'):
        record('Linux build host', platform.system() == 'Linux', platform.system())
        for command in ('cmake', 'make', 'pkg-config'):
            record(command, bool(shutil.which(command)), 'Executable on PATH')
    if args.target == 'host':
        record('C compiler', bool(shutil.which(os.environ.get('CC', 'cc'))), 'Native compiler required for C regressions')
        records.append(dict(name='Host libraries', status='info', detail='Install curl/cJSON development libraries, ALSA and ffmpeg; full simulator additionally needs SDL2/FreeType/platform libraries'))
    if args.target == 'arm':
        directory = args.toolchain or os.environ.get('TOOLCHAIN_PATH')
        compiler = Path(directory) / 'arm-openwrt-linux-gcc' if directory else None
        record('ARM compiler', compiler is not None and compiler.is_file(), 'Set TOOLCHAIN_PATH or --toolchain; known baseline GCC 8.3')
        libraries = ROOT / 'software/app_sdk/platform/t113/lib'
        names = [p.name for p in libraries.iterdir()] if libraries.is_dir() else []
        for prefix in ('libcurl', 'libssl', 'libcrypto', 'libfreetype', 'libcjson', 'libwpa_client'):
            record(prefix, any(n.startswith(prefix) for n in names), 'Matching Tina SDK library required')
        records.append(dict(name='ABI and board', status='info', detail='Presence checks do not certify ABI compatibility or hardware behavior'))
    if args.json:
        print(json.dumps(dict(target=args.target, checks=records), indent=2))
    else:
        for item in records:
            print(f'{item["status"].upper():7} {item["name"]}: {item["detail"]}')
    return 1 if any(item['status'] == 'missing' for item in records) else 0


if __name__ == '__main__':
    raise SystemExit(main())
