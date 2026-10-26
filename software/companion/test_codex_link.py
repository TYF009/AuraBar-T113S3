"""Adapter regressions use a local CLI fixture, never a model or paid service."""
import argparse
import contextlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import aurabar_companion as companion
from codex_link import CodexEvents, clip, parse_result, run_codex

CARD = dict(outcome='completed', summary='Added validation', changes=['Input guard'],
            verification_notes=['No test suite was run'], next_steps=['Review the diff'])

FIXTURE = '''import json, pathlib, sys, time
args = sys.argv
mode = pathlib.Path(args[args.index('--cd') + 1], 'mode').read_text()
sys.stdin.read()
def emit(value):
    print(json.dumps(value), flush=True)
emit({'type':'thread.started','thread_id':'fixture-thread'})
emit({'type':'turn.started'})
if mode == 'timeout':
    time.sleep(15)
emit({'type':'item.completed','item':{'id':'cmd-1','type':'command_execution','exit_code':7,'command':'private command','aggregated_output':'private output'}})
emit({'type':'item.completed','item':{'id':'cmd-1','type':'command_execution','exit_code':7}})
if mode == 'malformed':
    print('not json', flush=True)
elif mode == 'failed':
    emit({'type':'turn.failed','error':{'message':'private error'}})
else:
    emit({'type':'turn.completed','usage':{}})
card = {'outcome':'completed','summary':'Added validation','changes':['Input guard'],'verification_notes':['No test suite was run'],'next_steps':['Review the diff']}
if mode == 'blocked':
    card['outcome'] = 'blocked'
if mode != 'missing-card':
    pathlib.Path(args[args.index('--output-last-message') + 1]).write_text(json.dumps(card))
'''


class CodexLinkTests(unittest.TestCase):
    def test_events_distinguish_evidence_from_claims(self):
        events = CodexEvents()
        events.accept({'type': 'item.completed', 'item': {'type': 'agent_message', 'text': 'All tests passed'}})
        self.assertIn('0 exit 0', events.evidence())
        for code in (0, 7, None):
            item = dict(type='command_execution', id=str(code), exit_code=code, command='secret')
            events.accept({'type': 'item.completed', 'item': item})
            events.accept({'type': 'item.completed', 'item': item})
        self.assertIn('1 exit 0, 1 nonzero, 1 unknown', events.evidence())
        self.assertNotIn('secret', events.detail)
        events.accept({'type': 'error', 'message': 'temporary'})
        self.assertFalse(events.turn_failed)
        events.accept({'type': 'future.event'})

    def test_utf8_bounds_and_card_validation(self):
        text = clip('中' * 400 + '\x00\x7f')
        self.assertLessEqual(len(text.encode('utf-8')), 511)
        self.assertNotIn('\x00', text)
        self.assertEqual(parse_result(json.dumps(CARD).encode()), CARD)
        for value in ([], {}, dict(CARD, changes='string'), dict(CARD, next_steps=[False])):
            with self.assertRaises(ValueError):
                parse_result(json.dumps(value).encode())

    def run_fixture(self, directory, mode, offline_after_start=False):
        root = Path(directory)
        (root / 'mode').write_text(mode)
        (root / 'prompt.txt').write_text('Inspect this project', encoding='utf-8')
        (root / 'fixture.py').write_text(FIXTURE, encoding='utf-8')
        args = argparse.Namespace(cwd=root, prompt_file=root / 'prompt.txt', title='Codex regression',
                                  output_dir=root / 'runs', codex='codex', sandbox='read-only',
                                  timeout=5 if mode == 'timeout' else 30, url='http://unused')
        store = companion.TaskStore()
        reports = 0
        def api(url, token, task):
            nonlocal reports
            reports += 1
            if offline_after_start and reports > 1:
                raise OSError('Service stopped')
            store.update(task)
        launch = subprocess.Popen
        def fixture(command, **kwargs):
            self.assertIn('read-only', command)
            self.assertNotIn('--dangerously-bypass-approvals-and-sandbox', command)
            return launch([sys.executable, str(root / 'fixture.py')] + command[1:], **kwargs)
        with patch('codex_link.subprocess.Popen', side_effect=fixture), contextlib.redirect_stdout(io.StringIO()):
            code = run_codex(args, 'test-token', api)
        task = store.snapshot()['tasks'][0]
        record = json.loads(next((root / 'runs').glob('*/result.json')).read_text(encoding='utf-8'))
        return code, task, record

    def test_cli_process_completed_card_and_command_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            code, task, record = self.run_fixture(directory, 'success')
        self.assertEqual(code, 0)
        self.assertEqual(task['state'], 'succeeded')
        self.assertEqual(task['stage'], 'Ready for human review')
        self.assertIsNone(task['progress'])
        self.assertIn('1 nonzero', task['evidence'])
        self.assertIn('Human review required', task['next_step'])
        self.assertNotIn('private', json.dumps(task))
        self.assertEqual(record['result'], CARD)

    def test_zero_exit_without_valid_completion_is_failure(self):
        for mode in ('failed', 'missing-card', 'malformed'):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as directory:
                code, task, record = self.run_fixture(directory, mode)
                self.assertEqual(code, 1)
                self.assertEqual(task['state'], 'failed')
                self.assertEqual(record['adapter_exit_code'], 1)

    def test_model_reported_blocker_is_not_completed(self):
        with tempfile.TemporaryDirectory() as directory:
            code, task, record = self.run_fixture(directory, 'blocked')
        self.assertEqual(code, 1)
        self.assertEqual(task['state'], 'failed')
        self.assertEqual(task['stage'], 'Blocked - check computer')
        self.assertEqual(record['result']['outcome'], 'blocked')

    def test_timeout_stops_cli_and_saves_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            code, task, record = self.run_fixture(directory, 'timeout')
        self.assertEqual(code, 1)
        self.assertEqual(task['stage'], 'Timed out')
        self.assertEqual(record['task']['state'], 'failed')

    def test_completion_saved_when_service_disappears(self):
        with tempfile.TemporaryDirectory() as directory:
            code, task, record = self.run_fixture(directory, 'success', offline_after_start=True)
        self.assertEqual(code, 0)
        self.assertEqual(task['state'], 'running')  # The server never received completion.
        self.assertEqual(record['task']['state'], 'succeeded')
        self.assertTrue(record['delivery_failed'])

    def test_initial_delivery_failure_prevents_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            prompt = root / 'prompt.txt'
            prompt.write_text('Hello')
            args = argparse.Namespace(cwd=root, prompt_file=prompt, output_dir=root / 'runs',
                                      timeout=30, title='No connection', codex='codex',
                                      sandbox='read-only', url='unused')
            with patch('codex_link.subprocess.Popen') as process:
                with self.assertRaises(OSError):
                    run_codex(args, 'test', lambda *args: (_ for _ in ()).throw(OSError()))
                process.assert_not_called()


if __name__ == '__main__':
    unittest.main()
