"""Codex exec JSONL adapter. Uses the user's CLI login; no credentials on the board."""
import json
import math
from pathlib import Path
import subprocess
import time
import uuid

LINE_LIMIT = 1024 * 1024
EVENT_LIMIT = 32 * 1024 * 1024
PROMPT_LIMIT = 64 * 1024
RESULT_LIMIT = 64 * 1024


def clip(value, limit=511):
    """Bound UTF-8 text without splitting a character; remove display control codes."""
    value = ''.join(c if c >= ' ' and c != '\x7f' else ' ' for c in str(value))
    encoded = value.encode('utf-8')
    if len(encoded) <= limit:
        return value
    return encoded[:limit - 3].decode('utf-8', errors='ignore') + '...'


def parse_result(raw):
    if len(raw) > RESULT_LIMIT:
        raise ValueError('Oversized result')
    result = json.loads(raw)
    if not isinstance(result, dict) or set(result) != {'outcome', 'summary', 'changes', 'verification_notes', 'next_steps'}:
        raise ValueError('Invalid result card')
    if result['outcome'] not in ('completed', 'blocked'):
        raise ValueError('Invalid task outcome')
    if not isinstance(result['summary'], str) or not result['summary'].strip():
        raise ValueError('Missing summary')
    for name in ('changes', 'verification_notes', 'next_steps'):
        if not isinstance(result[name], list) or len(result[name]) > 32:
            raise ValueError('Invalid result list')
        if any(not isinstance(item, str) or len(item.encode('utf-8')) > 4096 for item in result[name]):
            raise ValueError('Invalid result text')
    return result


class CodexEvents:
    """Derive evidence only from command events, never from an agent's prose."""
    def __init__(self):
        self.thread_id = None
        self.turn_completed = False
        self.turn_failed = False
        self.commands = {}
        self.files = set()
        self.final_message = ''
        self.stage = 'Starting Codex'
        self.detail = 'Waiting for the first Codex event'

    def accept(self, event):
        if not isinstance(event, dict):
            raise ValueError('Invalid Codex event')
        kind = event.get('type')
        if kind == 'thread.started':
            self.thread_id = event.get('thread_id')
            self.stage, self.detail = 'Connected to Codex', 'The coding task is starting'
        elif kind == 'turn.started':
            self.stage, self.detail = 'Codex working', 'Waiting for command or file events'
        elif kind == 'turn.completed':
            self.turn_completed = True
            self.stage = 'Preparing review card'
        elif kind == 'turn.failed':
            self.turn_failed = True
            self.stage, self.detail = 'Codex failed', 'See the local Codex log for the failure'
        elif kind == 'error':
            # A recoverable stream error is not proof that the entire turn failed.
            self.stage, self.detail = 'Codex reported an issue', 'Waiting for recovery or a final task result'
        elif kind in ('item.started', 'item.updated', 'item.completed'):
            item = event.get('item', {})
            if not isinstance(item, dict):
                raise ValueError('Invalid Codex item')
            category = item.get('type')
            if category == 'command_execution':
                self.stage = 'Running command' if kind != 'item.completed' else 'Command finished'
                # Commands/output may contain secrets; only counts and exit codes reach the device.
                self.detail = 'Command details stay in the local log'
                if kind == 'item.completed':
                    key = item.get('id')
                    code = item.get('exit_code')
                    if not isinstance(key, str) or (code is not None and type(code) is not int):
                        raise ValueError('Invalid command result')
                    self.commands[key] = code
            elif category == 'file_change':
                self.stage, self.detail = 'Updating files', 'File changes reported by Codex; review the diff on the computer'
                if kind == 'item.completed':
                    changes = item.get('changes', [])
                    if not isinstance(changes, list):
                        raise ValueError('Invalid file changes')
                    for change in changes:
                        if isinstance(change, dict) and isinstance(change.get('path'), str):
                            self.files.add(change['path'])
            elif category == 'agent_message' and kind == 'item.completed':
                message = item.get('text', '')
                if isinstance(message, str):
                    self.final_message = message
            elif category == 'mcp_tool_call':
                self.stage, self.detail = 'Using a tool', 'External tool work reported by Codex'
        return self.stage, self.detail

    def evidence(self):
        codes = list(self.commands.values())
        ok = sum(code == 0 for code in codes)
        bad = sum(code is not None and code != 0 for code in codes)
        unknown = sum(code is None for code in codes)
        return (f'Observed commands: {ok} exit 0, {bad} nonzero, {unknown} unknown. '
                f'File changes reported: {len(self.files)}. '
                'Command exit codes do not establish test coverage. Review on computer.')


def add_parser(commands):
    parser = commands.add_parser('codex', help='Run Codex and send real events and a review card to AuraBar')
    parser.add_argument('--codex', default='codex', help='Path to the official Codex executable')
    parser.add_argument('--cwd', type=Path, required=True, help='Trusted Git project directory')
    parser.add_argument('--prompt-file', type=Path, required=True, help='UTF-8 task instructions')
    parser.add_argument('--title', default='Codex task')
    parser.add_argument('--sandbox', choices=('read-only', 'workspace-write'), default='read-only')
    parser.add_argument('--output-dir', type=Path, default=Path.home() / '.aurabar/codex-runs')
    parser.add_argument('--timeout', type=float, default=900, help='Maximum run time in seconds (5..7200)')


def stop_process(process):
    if process and process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()


def run_codex(args, token, api):
    if not math.isfinite(args.timeout) or not 5 <= args.timeout <= 7200:
        raise ValueError('Timeout must be 5..7200 seconds')
    cwd = args.cwd.resolve(strict=True)
    if not cwd.is_dir():
        raise ValueError('Project must be a directory')
    raw = args.prompt_file.read_bytes()
    if not raw or len(raw) > PROMPT_LIMIT:
        raise ValueError('Prompt must contain 1..65536 UTF-8 bytes')
    prompt = raw.decode('utf-8-sig')
    task_id = str(uuid.uuid4())
    directory = args.output_dir.resolve() / task_id
    directory.mkdir(parents=True)
    result_path = directory / 'codex-final.json'
    instruction = (prompt + '\n\nReturn a concise review card matching the supplied JSON schema. '
                   'Use the language of the task. Distinguish observations from assumptions. '
                   'List actual verification and anything not checked in verification_notes. '
                   'Do not claim human acceptance. Limit each list to 5 brief entries. '
                   'Set outcome to blocked if missing access, permissions, input or another obstacle '
                   'prevented fulfilling the task; completed means the requested work was performed, '
                   'not that human review has passed. '
                   'Do not include credentials or secret values in the card.')
    (directory / 'prompt.txt').write_text(instruction, encoding='utf-8')
    command = [args.codex, 'exec', '--json', '--color', 'never', '--sandbox', args.sandbox,
               '--cd', str(cwd), '--output-schema', str(Path(__file__).with_name('codex_result.schema.json')),
               '--output-last-message', str(result_path), '-']
    task = dict(id=task_id, title=clip(args.title, 127), state='running', stage='Starting Codex',
                detail='Connecting to the computer Codex CLI', evidence='', next_step='', progress=None)
    api(args.url, token, task)  # Never start work without initial reporting.
    print(f'Codex task: {task_id}\nLocal results: {directory}', flush=True)
    delivery_failed = False

    def report(**changes):
        nonlocal delivery_failed
        task.update(changes)
        try:
            api(args.url, token, task)
            delivery_failed = False
        except (OSError, ValueError):
            if not delivery_failed:
                print('Companion unavailable; Codex continues and results are saved locally.', flush=True)
            delivery_failed = True

    events = CodexEvents()
    process = None
    outcome = None
    code = 1
    try:
        with (directory / 'events.jsonl').open('wb') as output, (directory / 'stderr.log').open('wb') as errors:
            process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=output, stderr=errors, shell=False)
            process.stdin.write(instruction.encode('utf-8'))
            process.stdin.close()
            started = time.monotonic()
            last_report = 0
            pending = b''
            read_count = 0
            with (directory / 'events.jsonl').open('rb') as stream:
                while True:
                    chunk = stream.read(65536)
                    read_count += len(chunk)
                    if read_count > EVENT_LIMIT:
                        raise ValueError('Codex event log limit exceeded')
                    pending += chunk
                    while b'\n' in pending:
                        line, pending = pending.split(b'\n', 1)
                        if len(line) > LINE_LIMIT:
                            raise ValueError('Codex event too large')
                        if line.strip():
                            events.accept(json.loads(line))
                    if len(pending) > LINE_LIMIT:
                        raise ValueError('Codex event too large')
                    now = time.monotonic()
                    if now - last_report > 0.5:
                        report(stage=events.stage, detail=clip(events.detail), evidence=events.evidence())
                        last_report = now
                    if now - started > args.timeout:
                        raise TimeoutError('Codex task timed out')
                    if process.poll() is not None and not chunk:
                        if pending.strip():
                            events.accept(json.loads(pending))
                        break
                    time.sleep(0.1)
            code = process.wait()
        if code != 0 or events.turn_failed or not events.turn_completed:
            raise ValueError('Codex did not complete successfully')
        # Use the schema-constrained final artifact; never treat arbitrary prose as acceptance.
        outcome = parse_result(result_path.read_bytes())
        detail = clip(outcome['summary'] + ' | ' + ' / '.join(outcome['changes']))
        next_step = clip('Human review required. ' + ' / '.join(outcome['next_steps']))
        blocked = outcome['outcome'] == 'blocked'
        report(state='failed' if blocked else 'succeeded',
               stage='Blocked - check computer' if blocked else 'Ready for human review', detail=detail,
               evidence=clip(events.evidence() + ' AI-reported notes (unverified): ' +
                             ' / '.join(outcome['verification_notes'])),
               next_step=next_step, progress=None)
        code = 1 if blocked else 0
    except KeyboardInterrupt:
        stop_process(process)
        report(state='cancelled', stage='Cancelled locally', detail='The local user stopped this Codex run',
               evidence=events.evidence(), next_step='Review partial changes on the computer.')
        code = 130
    except (OSError, ValueError, TimeoutError) as error:
        stop_process(process)
        report(state='failed', stage='Timed out' if isinstance(error, TimeoutError) else 'Needs attention',
               detail='Codex did not produce a valid completed result. Check the local logs.',
               evidence=events.evidence(), next_step='Review partial changes and logs before retrying.')
        code = 1
    finally:
        # Save even when the companion is offline; task delivery is not a durable job queue.
        record = dict(task=task, thread_id=events.thread_id, result=outcome,
                      adapter_exit_code=code, delivery_failed=delivery_failed,
                      evidence_source='Codex command_execution events; not a test-suite verdict')
        (directory / 'result.json').write_text(json.dumps(record, ensure_ascii=False, indent=2), encoding='utf-8')
    print('Codex result: ' + task['stage'] + '. See local result.json.', flush=True)
    return code
