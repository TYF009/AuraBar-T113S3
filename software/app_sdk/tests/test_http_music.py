"""Run the music state tests against a local MP3 server and a failing endpoint."""
import http.server
import os
import subprocess
import sys
import threading
from functools import partial
from pathlib import Path

binary, library = sys.argv[1], Path(sys.argv[2])
class Handler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *args):
        pass
    def handle(self):
        try:
            super().handle()
        except (BrokenPipeError, ConnectionResetError):
            pass
server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), partial(Handler, directory=str(library)))
threading.Thread(target=server.serve_forever, daemon=True).start()
port = server.server_address[1]
(library / "stations.m3u").write_text(
    f"#EXTINF:-1,HTTP test\nhttp://127.0.0.1:{port}/02-test-tone.mp3\n"
    f"#EXTINF:-1,HTTP failure\nhttp://127.0.0.1:{port}/missing.mp3\n", encoding="utf-8")
env = os.environ.copy()
env["AURABAR_MUSIC_DIR"] = str(library)
try:
    subprocess.run([binary], env=env, check=True, timeout=45)
finally:
    server.shutdown()
