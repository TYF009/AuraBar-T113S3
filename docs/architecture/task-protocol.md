# Task protocol and evidence semantics

The companion exposes authenticated GET/POST `/v1/tasks`. Both require a Bearer pairing token. POST accepts a task update, never a remote shell command. The server binds to `127.0.0.1:8765` by default. USB mode does not copy the token to the board.

## Schema 1

The [snapshot JSON Schema](task-snapshot.schema.json) describes structural constraints. UTF-8 byte limits, duplicate task ids, control characters and freshness rules remain runtime checks; the schema alone is not a full validator.

```json
{"schema":1,"server_id":"session-1","revision":2,"sample":8,"tasks":[{"id":"job-1","title":"Code review","state":"succeeded","stage":"Ready for human review","detail":"AI-generated summary","evidence":"Observed command exits","next_step":"Review the diff","progress":null,"elapsed_seconds":16}]}
```

| Field | Contract |
|---|---|
| `server_id` | Changes on server restart |
| `revision` / `sample` | Revision follows updates; sample changes on reads for freshness |
| `id` | 1–63 ASCII letters, digits, dash or underscore; fresh id per run |
| `state` | `running`, `succeeded`, `failed`, `cancelled`; terminal entries are immutable |
| `title` / `stage` | At most 127 UTF-8 bytes each |
| `detail` / `evidence` / `next_step` | At most 511 UTF-8 bytes each; last two are optional |
| `progress` | Null when unknown; otherwise 0–100 |
| `elapsed_seconds` | Monotonic elapsed time, frozen at completion |

Bodies are bounded to 32 KiB and the in-memory service holds eight tasks. Terminal entries are evicted before active ones; if all slots are active a new task is rejected.

The bridge writes `/tmp/aurabar-tasks.json.next`, then renames it atomically. The board needs fresh samples and marks the snapshot disconnected after five seconds without freshness. First-loaded history and server restarts do not replay old completion sounds.

## Codex result interpretation

The adapter requires a successful CLI exit, completed turn, no failed turn, and a valid schema-constrained card. A reported `blocked` outcome is shown as blocked/failed. Summary, changes, verification notes and next steps are AI-generated; command exit counts are derived from events, deduplicated by item id. An exit-0 command is not a test-suite verdict.

The [result schema](../../software/companion/codex_result.schema.json) is machine-readable. A model task completed through this adapter does not imply human acceptance.

## Failure and data lifecycle

Service history is not persisted. Completion delivery after service failure has no durable queue; local Codex results/logs are retained. USB interruption can recover the latest live service state. Ctrl+C/timeout stops the immediate child; child process trees may need separate cleanup. Network deployments beyond loopback need their own TLS/reverse-proxy arrangement.

See [companion commands](../../software/companion/README.md) for all options.
