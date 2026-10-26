"""AuraBar task companion: Python standard library only, no remote command execution."""
import argparse
import collections
import hmac
import json
import os
from pathlib import Path
import secrets
import re
import subprocess
import sys
import threading
import time
import urllib.error
import urllib.request
import uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

MAX_TASKS = 8
MAX_BODY = 32768
STATES = {'running', 'succeeded', 'failed', 'cancelled'}


def bounded(value, limit):
    if not isinstance(value, str) or len(value.encode('utf-8')) > limit:
        raise ValueError('Invalid or oversized text')
    return ''.join(c if c >= ' ' and c != '\x7f' else ' ' for c in value)


class TaskStore:
    def __init__(self):
        self.lock = threading.Lock()
        self.tasks = collections.OrderedDict()
        self.server_id = str(uuid.uuid4())
        self.revision = self.sample = 0

    def update(self, data):
        task_id = bounded(data.get('id'), 63)
        if not re.fullmatch(r'[A-Za-z0-9_-]{1,63}', task_id):
            raise ValueError('Missing id')
        state = data.get('state')
        if state not in STATES:
            raise ValueError('Invalid state')
        progress = data.get('progress')
        if progress is not None and (type(progress) not in (int, float) or not 0 <= progress <= 100):
            raise ValueError('Invalid progress')
        title = bounded(data.get('title', ''), 127)
        stage = bounded(data.get('stage', ''), 127)
        detail = bounded(data.get('detail', ''), 511)
        evidence = bounded(data.get('evidence', ''), 511)
        next_step = bounded(data.get('next_step', ''), 511)
        with self.lock:
            old = self.tasks.get(task_id)
            if old and old['state'] != 'running':
                raise ValueError('Terminal task is immutable; start a new task id')
            if old is None and len(self.tasks) >= MAX_TASKS:
                terminal = next((k for k, v in self.tasks.items() if v['state'] != 'running'), None)
                if terminal is None:
                    raise ValueError('All task slots are running')
                del self.tasks[terminal]
            clock = time.monotonic()
            self.tasks[task_id] = dict(id=task_id, title=title, state=state, stage=stage,
                                       detail=detail, evidence=evidence, next_step=next_step, progress=progress,
                                       started=old['started'] if old else clock,
                                       finished=clock if state != 'running' else None)
            self.revision += 1

    def snapshot(self):
        with self.lock:
            self.sample += 1
            tasks = []
            for task in reversed(list(self.tasks.values())):
                result = {k: v for k, v in task.items() if k not in ('started', 'finished')}
                result['elapsed_seconds'] = int((task['finished'] or time.monotonic()) - task['started'])
                tasks.append(result)
            return dict(schema=1, server_id=self.server_id, revision=self.revision,
                        sample=self.sample, tasks=tasks)


def handler_for(store, token):
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):
            pass  # Never log authorization or potentially private task descriptions.

        def reply(self, status, value):
            body = json.dumps(value, ensure_ascii=False, allow_nan=False).encode('utf-8')
            self.send_response(status)
            self.send_header('Content-Type', 'application/json; charset=utf-8')
            self.send_header('Cache-Control', 'no-store')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def authorized(self):
            supplied = self.headers.get('Authorization', '')
            try:
                return hmac.compare_digest(supplied, 'Bearer ' + token)
            except TypeError:
                return False

        def do_GET(self):
            if not self.authorized():
                return self.reply(401, {'error': 'Unauthorized'})
            if self.path != '/v1/tasks':
                return self.reply(404, {'error': 'Not found'})
            self.reply(200, store.snapshot())

        def do_POST(self):
            if not self.authorized():
                return self.reply(401, {'error': 'Unauthorized'})
            if self.path != '/v1/tasks':
                return self.reply(404, {'error': 'Not found'})
            try:
                size = int(self.headers.get('Content-Length', '0'))
                if not 0 < size <= MAX_BODY:
                    raise ValueError('Invalid body size')
                data = json.loads(self.rfile.read(size))
                if not isinstance(data, dict):
                    raise ValueError('Expected object')
                store.update(data)
            except (ValueError, TypeError):
                return self.reply(400, {'error': 'Invalid task update'})
            self.reply(200, {'ok': True})

        def setup(self):
            super().setup()
            self.connection.settimeout(5)
    return Handler


def token_at(path, create=False):
    path = Path(path)
    if create:
        path.parent.mkdir(parents=True, exist_ok=True)
        try:
            fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
        except FileExistsError:
            pass
        else:
            with os.fdopen(fd, 'w', encoding='ascii') as stream:
                stream.write(secrets.token_hex(32))
    token = path.read_text(encoding='ascii').strip()
    if not 32 <= len(token) <= 128 or not token.isascii() or not token.isalnum():
        raise ValueError('Invalid pairing token file')
    return token


def api(url, token, data=None):
    request = urllib.request.Request(url.rstrip('/') + '/v1/tasks',
                                     data=None if data is None else json.dumps(data, allow_nan=False).encode(),
                                     headers={'Authorization': 'Bearer ' + token,
                                              'Content-Type': 'application/json'})
    # Task status is a direct local/LAN connection; ignore global HTTP proxy settings.
    with urllib.request.build_opener(urllib.request.ProxyHandler({})).open(request, timeout=4) as response:
        body = response.read(MAX_BODY + 1)
        if len(body) > MAX_BODY:
            raise ValueError('Response too large')
        return json.loads(body)


def run_task(args, token):
    command = args.command
    if command and command[0] == '--':
        command = command[1:]
    if not command:
        raise ValueError('Provide a command after --')
    task_id = str(uuid.uuid4())
    task = dict(id=task_id, title=args.title, state='running', stage='Starting',
                detail='Task started on the companion computer', progress=None)
    api(args.url, token, task)  # Do not execute a task if initial reporting fails.
    def report(**changes):
        task.update(changes)
        try:
            api(args.url, token, task)
        except (OSError, ValueError):
            print('Task status delivery failed; task itself continues.', file=sys.stderr)
    print('Task id: ' + task_id, flush=True)
    process = None
    try:
        with Path(args.log).open('wb') as output:
            process = subprocess.Popen(command, cwd=args.cwd, stdout=output, stderr=subprocess.STDOUT,
                                       shell=False)
            report(stage='Running', detail='Output is written to the local task log; no percentage is available.')
            code = process.wait()
        report(state='succeeded' if code == 0 else 'failed', stage='Completed' if code == 0 else 'Failed',
               detail=f'Process exited with code {code}. Log: {Path(args.log).name}',
               progress=100 if code == 0 else None)
        return code if 0 <= code <= 255 else 1
    except KeyboardInterrupt:
        if process and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        report(state='cancelled', stage='Cancelled', detail='Stopped by the local user')
        return 130
    except OSError:
        report(state='failed', stage='Could not start', detail='Check the local command, working directory and log path')
        return 1


def bridge(args, token):
    import tempfile
    adb = [args.adb, '-s', args.serial]
    failures = 0
    print('USB task bridge running. Ctrl+C stops it.', flush=True)
    with tempfile.TemporaryDirectory() as directory:
        snapshot = Path(directory) / 'tasks.json'
        while True:
            try:
                data = api(args.url, token)
                snapshot.write_text(json.dumps(data, ensure_ascii=False), encoding='utf-8')
                subprocess.run(adb + ['push', str(snapshot), '/tmp/aurabar-tasks.json.next'],
                               check=True, timeout=5, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                subprocess.run(adb + ['shell', 'mv /tmp/aurabar-tasks.json.next /tmp/aurabar-tasks.json'],
                               check=True, timeout=5, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                if failures:
                    print('USB task bridge reconnected.', flush=True)
                failures = 0
            except (OSError, ValueError, subprocess.SubprocessError):
                failures += 1
                if failures == 1:
                    print('Task service or USB unavailable; waiting to reconnect.', flush=True)
            time.sleep(1 if failures < 3 else 3)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--url', default='http://127.0.0.1:8765')
    parser.add_argument('--token-file', default=str(Path.home() / '.aurabar/task-token'))
    commands = parser.add_subparsers(dest='operation', required=True)
    serve = commands.add_parser('serve')
    serve.add_argument('--listen', default='127.0.0.1')
    serve.add_argument('--port', type=int, default=8765)
    run = commands.add_parser('run')
    run.add_argument('--title', required=True)
    run.add_argument('--cwd')
    run.add_argument('--log', default='aurabar-task.log')
    run.add_argument('command', nargs=argparse.REMAINDER)
    report = commands.add_parser('report', help='Publish a real task stage from an external adapter')
    report.add_argument('--id', required=True)
    report.add_argument('--title', required=True)
    report.add_argument('--state', choices=sorted(STATES), required=True)
    report.add_argument('--stage', default='')
    report.add_argument('--detail', default='')
    report.add_argument('--progress', type=float)
    usb = commands.add_parser('bridge')
    usb.add_argument('--adb', default='adb')
    usb.add_argument('--serial', required=True)
    from codex_link import add_parser, run_codex
    add_parser(commands)
    args = parser.parse_args()
    try:
        token = token_at(args.token_file, create=args.operation == 'serve')
        if args.operation == 'serve':
            server = ThreadingHTTPServer((args.listen, args.port), handler_for(TaskStore(), token))
            server.daemon_threads = True
            print(f'AuraBar task service ready on {args.listen}:{args.port}', flush=True)
            server.serve_forever()
        elif args.operation == 'run':
            return run_task(args, token)
        elif args.operation == 'report':
            api(args.url, token, dict(id=args.id, title=args.title, state=args.state,
                                     stage=args.stage, detail=args.detail, progress=args.progress))
        elif args.operation == 'codex':
            return run_codex(args, token, api)
        else:
            bridge(args, token)
    except KeyboardInterrupt:
        return 0
    except (OSError, ValueError) as error:
        parser.exit(1, f'{type(error).__name__}: check companion configuration and connectivity\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
