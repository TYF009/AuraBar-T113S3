import argparse
import contextlib
import io
from pathlib import Path
import sys
import subprocess
import tempfile
import threading
import unittest
import urllib.error
from http.server import ThreadingHTTPServer
import aurabar_companion as companion


class CompanionTests(unittest.TestCase):
    def setUp(self):
        self.token = 'a' * 64
        self.store = companion.TaskStore()
        self.server = ThreadingHTTPServer(('127.0.0.1', 0), companion.handler_for(self.store, self.token))
        self.server.daemon_threads = True
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.url = 'http://127.0.0.1:' + str(self.server.server_port)

    def tearDown(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join()

    def test_auth_bounds_and_samples(self):
        with self.assertRaises(urllib.error.HTTPError) as error:
            companion.api(self.url, 'wrong')
        self.assertEqual(error.exception.code, 401)
        first = companion.api(self.url, self.token)
        second = companion.api(self.url, self.token)
        self.assertGreater(second['sample'], first['sample'])
        self.assertEqual(first['revision'], second['revision'])
        for data in ({'id': 'x', 'state': 'imaginary'},
                     {'id': 'x', 'state': 'running', 'progress': 101},
                     {'id': 'x', 'state': 'running', 'title': 'x' * 128}):
            with self.assertRaises(urllib.error.HTTPError) as error:
                companion.api(self.url, self.token, data)
            self.assertEqual(error.exception.code, 400)

    def test_slots_preserve_active_tasks_and_terminal_is_immutable(self):
        for i in range(8):
            self.store.update({'id': str(i), 'title': str(i), 'state': 'running'})
        with self.assertRaises(ValueError):
            self.store.update({'id': '9', 'state': 'running'})
        self.store.update({'id': '0', 'state': 'failed'})
        with self.assertRaises(ValueError):
            self.store.update({'id': '0', 'state': 'running'})
        self.store.update({'id': '9', 'state': 'running'})
        self.assertEqual(len(self.store.snapshot()['tasks']), 8)
        self.assertNotIn('0', self.store.tasks)

    def test_real_commands_report_exit_and_local_logs(self):
        with tempfile.TemporaryDirectory() as directory:
            for code in (0, 7):
                args = argparse.Namespace(command=[sys.executable, '-c', f'print("original output");raise SystemExit({code})'],
                                          title='Real regression', url=self.url, cwd=None,
                                          log=str(Path(directory) / 'task.log'))
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(companion.run_task(args, self.token), code)
                task = self.store.snapshot()['tasks'][0]
                self.assertEqual(task['state'], 'succeeded' if code == 0 else 'failed')
                self.assertEqual(task['progress'], 100 if code == 0 else None)
                self.assertIn('original output', Path(args.log).read_text())
                self.assertNotIn('original output', task['detail'])

    def test_pairing_token_is_stable(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'token'
            first = companion.token_at(path, create=True)
            self.assertEqual(first, companion.token_at(path, create=True))
            self.assertEqual(len(first), 64)

    def test_external_adapter_cli(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'token'
            path.write_text(self.token)
            result = subprocess.run([sys.executable, companion.__file__, '--url', self.url,
                                     '--token-file', str(path), 'report', '--id', 'adapter-1',
                                     '--title', 'Adapter task', '--state', 'running',
                                     '--stage', 'Reading source'], capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            task = self.store.snapshot()['tasks'][0]
            self.assertEqual(task['stage'], 'Reading source')
            self.assertIsNone(task['progress'])


if __name__ == '__main__':
    unittest.main()
