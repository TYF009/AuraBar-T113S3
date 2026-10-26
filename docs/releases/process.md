# Preview release procedure

1. Select a source commit and application version; keep historical acceptance records immutable except for factual corrections.
2. Run repository checks, relevant host regressions and the actual target build. Record compiler/SDK baseline and binary hash.
3. Run the necessary physical checks and record remaining limitations explicitly. Host-null audio does not replace a listening check.
4. Package using `software/app_sdk/scripts/package.py`, verify the archive and preserve its SHA256 file.
5. Check bundled resource/dependency notices and record source provenance. Do not assume software MIT covers fonts, SDK binaries or enclosure models.
6. Verify installation and rollback on the matching baseline, including resource requirements and startup behavior.
7. Prepare a prerelease with exact target commit, package, checksum, compatibility and known issues. Check uploaded asset digests.
8. Publish only when required release evidence/notices are present; update `software/releases.json` and the readable catalog to the same availability.

Tags preserve an application build. Later documentation changes do not retroactively rebuild that binary. Current profile changes require a deliberate new build; labels in `software/profiles` alone do not switch compiled features.

The source default branch and binary publication are separate: main can contain a tested source preview while prepared binaries remain draft. [Compatibility](README.md) is the user-facing authority.
