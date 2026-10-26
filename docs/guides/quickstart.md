# Quick start

This path targets a matching AuraBar T113-S3 board already running the verified Tina system. It does not provision a blank board. The complete BSP image is not distributed here yet.

## 1 Check prerequisites

- Correct board/display baseline, root development ADB through USB10, and existing compatible `/usr/res` assets.
- Computer with Python 3.10+, ADB and a data-capable USB cable.
- Obtain a compatible application build using [installation](installation.md). Prepared release drafts are not public downloads; check the [catalog](../releases/README.md).

From the repository root, optional environment checks are:

```sh
python scripts/doctor.py --target companion --adb /path/to/adb
python scripts/check_repository.py
```

## 2 Start the board application

After installation, stop the known previous display process using its actual PID. Keep one display app running. In an ADB shell:

```sh
cd /usr/bin
export AURABAR_TASK_FILE=/tmp/aurabar-tasks.json
./aurabar-coding-companion-0.4.0-alpha.2 --tasks
```

This foreground command lasts while the shell remains open. Persistent startup is not configured by the preview. Reboot follows the existing system configuration.

## 3 Start the computer companion

From `software/companion`, keep the first two terminals open:

```sh
# Terminal 1
python aurabar_companion.py serve
# Terminal 2: replace executable and serial with your own values
python aurabar_companion.py bridge --adb /path/to/adb --serial YOUR_SERIAL
# Terminal 3: a small real task
python aurabar_companion.py run --title "Connection check" -- python -c "print('AuraBar connected')"
```

Expect a Connected badge and a completed task. Pairing is local and automatic through the default token file. Full logs stay on the computer. Unknown percentages are shown explicitly.

## 4 Try a Codex review

Install and sign in to the official Codex CLI separately. Then:

```sh
python aurabar_companion.py codex --cwd /path/to/trusted-git-project --prompt-file examples/static-review.txt --title "Boundary review"
```

The supplied-code example needs no project shell tools. Read the result under Summary/Evidence/Next, then review on the computer. Your configured provider, network and account limits apply. Windows users may need `--codex` pointing to the actual `.exe`.

## 5 Try audio

Use Music to choose a local track or Brown noise. Rain and ocean are synthesized textures. Return with Tasks; the backend can keep playing. Start at low volume. Your music library is separate from project fixtures.

[Detailed commands](../../software/companion/README.md) · [Troubleshooting](troubleshooting.md) · [Compatibility](../releases/README.md)
