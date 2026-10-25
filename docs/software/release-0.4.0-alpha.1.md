# Coding Companion 0.4.0-alpha.1 — development acceptance record

Date: 2026-10-25. This is a source preview, not a completed hardware release.
The Music & Focus 0.2 and Desktop Link 0.3 iterations remain on their existing branches.

## Delivered source

- A standard-library Python Codex CLI adapter consumes actual exec JSONL events.
- A schema-constrained review card separates AI summary, observed command exits,
  AI verification notes and human follow-up actions.
- A model-reported blocked outcome is not shown as a completed task.
- Per-run local prompt, event, stderr and result files; bounded input/events,
  local cancellation, timeout, and result preservation if delivery fails.
- Optional schema-1 evidence/next_step fields retain older-reader compatibility.
- Native task-page source adds Summary / Evidence / Next tabs and scrollable text.
- Product scope, setup, example prompt and package/deploy profile updated for 0.4.

## Verified on the development computer and USB transport

The Python regression suite covers: command exits and duplicate events, UTF-8 bounds,
card validation, invalid/missing results after a zero CLI exit, model-reported blockers,
initial-delivery failure preventing execution, timeout, local result preservation after
service loss, and the existing companion authentication/command tests.
The test CLI is a local fixture; these regressions do not invoke a model.

Separately, an actual logged-in Codex CLI (0.164.0-alpha.11.1) completed a Chinese
static review of a small percentage function supplied in the task. The card identified
unknown/zero denominator handling and follow-up boundary tests, and explicitly stated
that no commands, tests or file modifications were performed. The adapter recorded
zero observed command exits, saved the full result, and the USB bridge delivered the
matching task id and Chinese result to /tmp/aurabar-tasks.json on the real board.
This verifies a real model result and USB delivery, not an end-to-end coding/test verdict.

An earlier attempt using a stale system proxy timed out and reported failure. Retrying
with the current system proxy restored model connectivity. A project-file reading task
then returned a card stating that the local Codex shell sandbox could not initialize
due to access denial on its sandbox helper directory. That did not verify file reading
or test execution. The blocked-outcome mapping was added after this observation and is
covered by regression tests; the earlier task was not retroactively rewritten.

## Pending acceptance / 待验收

- The VMware Ubuntu build environment was not running during this iteration.
  Restart the existing vm-build-worker.py to run native C regressions and build T113.
- The board is still running the preserved Desktop Link 0.3 executable. Do not claim
  the 0.4 tabs, scrolling or full touch path have been verified on it.
- Build and verify the matching 0.4 archive before creating a downloadable release asset.
- Recheck terminal notification audio during music, followed by music recovery.
- Resolve the computer's Codex sandbox permission issue before claiming project-file
  reading, command execution or actual test-suite integration on this machine.

## Scope limits

This tracks tasks launched by the companion, not every conversation in the desktop
Codex app. It does not offer screen approvals, task launch/cancel buttons, durable
task storage, automatic startup, whole-system flashing or a bundled local model.
AI-generated summaries and outcomes still need human review.

See [setup](../../software/companion/README.md) and [product scope](coding-companion.md).
