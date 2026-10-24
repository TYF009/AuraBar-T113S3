"""Exercise the actual C polling worker against the Python HTTP companion."""
import os
from pathlib import Path
import queue
import subprocess
import sys
import tempfile
import threading
import time
from http.server import ThreadingHTTPServer

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'companion'))
from aurabar_companion import TaskStore, handler_for

token = 'b' * 64
store = TaskStore()
store.update({'id': 'tracked', 'title': 'Real client test', 'state': 'running'})
server = ThreadingHTTPServer(('127.0.0.1', 0), handler_for(store, token))
port = server.server_port
thread = threading.Thread(target=server.serve_forever, daemon=True)
thread.start()
lines = queue.Queue()
seen = []
client = None
try:
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / 'token'
        path.write_text(token)
        env = dict(os.environ, AURABAR_TASK_URL=f'http://127.0.0.1:{port}/v1/tasks', AURABAR_TASK_TOKEN_FILE=str(path))
        env.pop('AURABAR_TASK_FILE', None)
        client = subprocess.Popen([sys.argv[1]], env=env, stdout=subprocess.PIPE, text=True)
        def read():
            for line in client.stdout:
                lines.put(line.strip())
        threading.Thread(target=read, daemon=True).start()
        def until(predicate, timeout=10):
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                try:
                    line = lines.get(timeout=0.5)
                    seen.append(line)
                    if predicate(line):
                        return
                except queue.Empty:
                    pass
            raise AssertionError('Expected state not received: ' + repr(seen))
        until(lambda line: 'connected=1 count=1' in line)
        store.update({'id': 'tracked', 'title': 'Real client test', 'state': 'succeeded', 'progress': 100})
        until(lambda line: 'notice=1' in line)
        server.shutdown()
        server.server_close()
        thread.join()
        until(lambda line: 'connected=0 count=1' in line, 12)
        # A new server identity must not replay historical completion sounds.
        replacement = TaskStore()
        replacement.update({'id': 'historical', 'title': 'Historical result', 'state': 'failed'})
        server = ThreadingHTTPServer(('127.0.0.1', port), handler_for(replacement, token))
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        until(lambda line: 'connected=1 count=1' in line)
        assert sum('notice=1' in line for line in seen) == 1, seen
        print('task client: authenticated HTTP, completion, disconnect and server-restart recovery passed')
finally:
    if client:
        client.terminate()
        client.wait(timeout=6)
        client.stdout.close()
    server.shutdown()
    server.server_close()
    thread.join()
