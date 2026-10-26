# Music & Focus engineering review

## Scope

Reviewed the supplied application SDK and integration with the existing AuraBar
Tina Linux 5.4.61 board. This is not a full BSP or PCB layout review.
Panel/firmware settings were preserved after resource-path repair restored the display.

## Changes

| Finding | Change |
|---|---|
| Compiler failures were followed by exit 0 | Fail-fast script, checked toolchain, separate builds |
| Resources had duplicate directory levels | Explicit asset layout and checksummed packaging |
| Config operations leaked descriptors | Exact reads, closed descriptors, atomic durable writes |
| Configuration and assets shared a directory | Writable data directory, legacy read fallback |
| System V queues shared names and ignored timeouts | Per-process bounded pthread queues |
| HTTP worker invoked UI callbacks | Bounded value events and UI-thread dispatch |
| Search callbacks outlived their page | Owner cleanup and request-generation invalidation |
| Empty/failed search never completed | Explicit empty/error completion |
| TLS verification was disabled | Peer/hostname verification enabled |
| Wi-Fi callbacks touched LVGL from a worker | Main-thread status delivery |
| WPA request length was uninitialized/reused | Bounded per-request lengths |
| Connecting removed all saved networks | Configure/select only the new network |
| Passwords were displayed or logged on errors | Hide passwords and redact command logging |
| WPA events and replies shared a channel | Separate command/event sockets |
| Audio paths were interpolated into shell commands | Argument-based spawning, owned child only |
| Alerts used killall aplay | Music/notification arbitration |
| NTP blocked the UI | Single bounded background synchronization |
| Minute countdowns followed app uptime | Per-task monotonic countdowns |
| Idle detection counted timer calls | Actual LVGL inactivity |
| Clock cleanup used mismatched objects | Correct matching clock instances |
| Fault signals were treated as ordinary exit | Graceful INT/TERM; normal fault behavior |
| Unused upstream demos were built | Optional, off by default |

## Validation

Executed on 2026-10-25:

- Ubuntu native regressions passed, including local WAV/MP3 and localhost HTTP
  success/404 failure, pause/seek, notification coordination and storage/queue/countdown checks.
- T113 cross-compilation succeeded. Storage, queue, countdown and music tests
  passed on the actual ARM board; control tests used ALSA null.
- Separate real ALSA speaker checks played the generated WAV and MP3 for three
  seconds each at volume 15. Both reached PLAYING with position above 2.6 seconds.
  The owner confirmed both sounds were audible and the physical display was normal.
- The Music page was captured from the actual framebuffer and visually inspected.
  Missing symbol glyphs were removed rather than shipped as square placeholders.
- The installed version's SHA256 matched the host executable. The old /usr/bin/demo
  remains available; boot scripts were not switched to the preview.

The suite covers storage descriptors/damaged records, concurrent queue order/timeouts,
exact countdown boundaries, local PCM/MP3, HTTP success/failure, pause/seek, favorites,
notification coordination and sleep controls.

Touch interactions, network reconnection, prolonged playback, analog audio quality,
power cycles and live Internet MP3 streams still require separate acceptance checks.
A null audio sink verifies software behavior, not analog audio quality.

## Remaining work

- Audit older pages' overlay ownership and persistence schema versions.
- Measure memory/CPU/latency with music and UI together.
- Validate every touch control, direction changes and the full mixer range.
- Add bounded metadata, cover art, lyrics and a library rescan UI.
- Add verified startup/recovery and full image generation once BSP is available.
- Confirm weather service commercial configuration before commercial distribution.
