# Installation and rollback

Prepared archives are application upgrades for an existing matching Tina system. They are not PhoenixSuit images and do not supply the kernel, bootloader or full root filesystem.

## Baseline

The physical development baseline is T113-S3, Tina Linux kernel 5.4.61 and the verified fl7707n_mipi display setup. The framebuffer is 720×1424 with a 1424×280 visible UI mapping. These numbers describe different layers; do not change panel timings solely to match UI width.

Compatible `/usr/res/image`, `/usr/res/font` and `/usr/res/music` resources are required. The archive carries assets; the current deploy helper installs the versioned executable and expects resources already provisioned. It does not automatically migrate arbitrary existing assets or user data.

## Verify a package

Each package has a `.sha256` file and internal per-file manifest. On a machine with the repository:

```sh
python software/app_sdk/scripts/verify_package.py /path/to/package.tar.gz
```

The verifier checks paths, file types, manifest membership and hashes without extraction. A checksum detects file changes; it is not a signed publisher identity.

## Install a locally built executable

```sh
python software/app_sdk/scripts/deploy.py --adb /path/to/adb --serial YOUR_SERIAL
```

The helper requires the development image's root ADB, refuses an existing target version, checks resources and host/device SHA256, then sets executable permissions. It preserves `/usr/bin/demo` and does not alter boot configuration. Package-only Coding Companion users can provide the extracted `bin/aurabar` with `--binary /path/to/aurabar` and the matching `--version`. The current helper accepts only the coding-companion profile; older Music/Desktop builds require the tools at their own historical source revision.

Stop the known active display process by its actual PID before launching the new version from `/usr/bin`. Avoid broad process killing. See [quick start](quickstart.md) for startup.

## Recover

Stop only the new display process and restart the preserved previous executable from `/usr/bin`. Existing `/usr/bin/demo` is the fallback. If resources were changed manually, restore their backup as well. Reboot uses the existing boot configuration; it does not automatically start this preview.

User data is outside the static resource directory, normally `/usr/var/lib/aurabar`. `/tmp` snapshots and test music can disappear on reboot. Keep user data, local settings and a known working binary backed up before deployment.

Blank-board provisioning remains dependent on the matching BSP/image project. [Tool references](../../tools/README.md) alone do not constitute a reproducible firmware build.
