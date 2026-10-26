# Validation and evidence

Validation is recorded by layer. A passing host job does not certify real speaker behavior, touch feel, a whole-system image or an automotive integration.

| Layer | Coverage | Entry / record |
|---|---|---|
| Repository | Local links, asset registry, current version consistency, release metadata | `python scripts/check_repository.py` |
| Companion | Authentication, slots, process exits, Codex fixtures, blockers, timeout, offline local result preservation | `python -m unittest discover -s software/companion -v` |
| Native C | Storage, queues, timers, music, task parser, actual HTTP polling | `bash software/app_sdk/tests/run.sh` |
| ARM compile | Application and standalone test binaries with matching SDK | Build guide and version acceptance records |
| Physical board | Display, speaker tones, USB task/reconnect, card navigation and scrolling | Owner confirmations and framebuffer captures |

## Recorded previews

- [0.2 music and foundation](../software/release-0.2.0-alpha.1.md)
- [0.3 task transport and results](../software/release-0.3.0-alpha.1.md)
- [0.4 first Codex card and ARM build](../software/release-0.4.0-alpha.1.md)
- [0.4 alpha.2 owner-confirmed scroll/audio controls](../software/release-0.4.0-alpha.2.md)

The Codex regression fixture is not a real model. Separately, an authenticated real model reviewed supplied code and its card reached the board. That run observed no command exits and is not an end-to-end repository editing/test acceptance.

## Open physical checks

Notification completion followed by music resumption, physical Wi-Fi/HTTPS transport, long-duration memory/CPU behavior, restart/cold-start and the full range of legacy widgets need additional acceptance. Capture the tested commit, binary hash, baseline system, action, expected/observed behavior and known limitations for each check.

Tests should protect behavior or boundaries, not mirror implementation. UI performance claims require measurement or explicit owner feedback; the alpha.2 record states noticeably smoother scrolling without inventing an FPS figure.
