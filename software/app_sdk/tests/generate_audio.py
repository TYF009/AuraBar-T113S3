"""Generate original diagnostic audio; contains no third-party recordings."""
import math
import shutil
import struct
import subprocess
import sys
import wave
from pathlib import Path

root = Path(sys.argv[1])
root.mkdir(parents=True, exist_ok=True)
with wave.open(str(root / "01-test-tone.wav"), "wb") as wav:
    wav.setparams((1, 2, 22050, 0, "NONE", "not compressed"))
    for i in range(22050 * 12):
        # Quiet tone with smooth entrance/exit.
        envelope = min(1, i / 2205, (22050 * 12 - i) / 2205)
        wav.writeframesraw(struct.pack("<h", int(1500 * envelope * math.sin(2 * math.pi * 440 * i / 22050))))
if shutil.which("ffmpeg"):
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(root / "01-test-tone.wav"),
                    "-codec:a", "libmp3lame", "-b:a", "64k", str(root / "02-test-tone.mp3")], check=True)
else:
    fixture = Path(__file__).parent / "fixtures" / "02-test-tone.mp3"
    if fixture.is_file() and fixture.resolve() != (root / fixture.name).resolve():
        shutil.copyfile(fixture, root / fixture.name)
print("Generated original audio fixtures")
