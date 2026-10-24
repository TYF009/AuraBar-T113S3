# AuraBar Desktop Link companion — 0.3.0-alpha.1

A small Python 3.10+ service that reports actual command/task state to AuraBar.
Only the Python standard library is required. The device keeps its native LVGL UI.
The companion does not execute commands received over HTTP; commands run only
when the local user invokes the run subcommand.

## First connection: USB

Keep the board connected through its USB10 debug port. Use three terminals on
the computer. Replace python with your Python executable and SERIAL with adb devices output.
Run commands from this directory.

~~~sh
# Terminal 1: local authenticated task service; leave running.
python aurabar_companion.py serve

# Terminal 2: bridge the service to the selected device; leave running.
python aurabar_companion.py bridge --adb /path/to/adb --serial SERIAL
~~~

The service listens only on 127.0.0.1:8765 by default. On first startup it creates
a random pairing token at ~/.aurabar/task-token. All companion commands use the
same file. --token-file PATH can select a different location; place this option
before serve, bridge, run or report. Do not commit or publish token files.
Tokens are not copied to the board for USB mode.

Install the matching application separately, stop the previous display app,
then start on the board from /usr/bin:

~~~sh
export AURABAR_TASK_FILE=/tmp/aurabar-tasks.json
/usr/bin/aurabar-desktop-link-0.3.0-alpha.1 --tasks
~~~

Keep one display application running. The installer preserves /usr/bin/demo and
does not change boot configuration. The USB bridge uses a temporary file followed
by atomic rename, and retries when the service or cable is unavailable. The old
board ADB daemon does not need reverse-port support.

~~~sh
# Terminal 3: run an actual command and report its exit status.
python aurabar_companion.py run --title "Software regression" --log regression.log -- python -m unittest discover -s . -v
~~~

Start/Running/Completed/Failed are real execution states. A command with no
progress API displays "Percentage unavailable" rather than a simulated percentage.
Exit 0 becomes Completed; a nonzero exit becomes Failed. The wrapper returns the
command's exit code and writes full stdout/stderr only to the local log. Ctrl+C
marks the task Cancelled and terminates the immediate child process; subprocess
trees created by that command may require their own cleanup.

## Report stages from an AI/build adapter

Choose a fresh id for each task. Keep title and id consistent across updates.
Only report percentages when the real task supplies them.

~~~sh
python aurabar_companion.py report --id job-2026-001 --title "Document summary" --state running --stage "Reading source"
# The external program actually performs the work, then reports its result:
python aurabar_companion.py report --id job-2026-001 --title "Document summary" --state succeeded --stage "Summary ready" --detail "3 decisions and 2 action items" --progress 100
~~~

These examples describe an adapter interface; no AI summarization model is bundled.
A terminal task cannot be reopened with the same id. The service holds up to eight
tasks in memory and evicts terminal entries before active ones. Restarting the
service clears that history; it does not recover tasks already running elsewhere.
If the companion service itself is stopped, command completion reports are not
durably queued. USB disconnects are recovered by retrieving the latest server state.

## HTTP contract and optional network mode

GET /v1/tasks reads a snapshot. POST /v1/tasks accepts one task update, never a shell
command. Both require Authorization: Bearer TOKEN. Requests/responses are bounded
to 32 KiB. Task text is bounded by UTF-8 byte length; ids use letters, digits, dash
or underscore. Unknown progress is JSON null. Snapshot schema:

~~~json
{
  "schema": 1,
  "server_id": "unique-per-service-start",
  "revision": 2,
  "sample": 17,
  "tasks": [{
    "id": "job-1", "title": "ARM build", "state": "running",
    "stage": "Compiling", "detail": "Building the application",
    "progress": null, "elapsed_seconds": 32
  }]
}
~~~

States: running, succeeded, failed, cancelled. Newest task starts first in the list.
Elapsed time comes from the computer's monotonic clock and freezes on completion.
The sample counter changes on each read, including when no task changed.
The device marks the last snapshot disconnected after five seconds without a
fresh sample. Initial historical completions and service restarts do not replay sounds.
The task page follows the newest running task; tapping a row pins it, Latest resumes
following. Music remains active when moving between Music and Tasks.

For network mode, configure the board's AURABAR_TASK_URL to the full endpoint,
e.g. https://your-companion.example/v1/tasks, and AURABAR_TASK_TOKEN_FILE to a
local token file. Do not set AURABAR_TASK_FILE at the same time (file mode wins).
The built-in HTTP server is for a local/trusted development connection. Use a TLS
reverse proxy for deployments beyond USB/loopback; TLS certificate verification
stays enabled. Local task connections ignore global proxy variables.
Network mode and USB share the same parser/state model; this preview's physical
transport acceptance uses USB, not Wi-Fi.

## Tests and limitations

~~~sh
python -m unittest discover -s . -v
~~~

Native C tests additionally exercise the actual polling worker, HTTP authentication,
disconnect/reconnect, restart handling and notification deduplication. Tests are in
../app_sdk/tests and run in GitHub Actions.

No remote command runner, automatic boot switching, persistent job database,
task cancellation from the screen, or autonomous AI execution is included.
