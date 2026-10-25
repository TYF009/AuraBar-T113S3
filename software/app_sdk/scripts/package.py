"""Build a checksummed application upgrade bundle (not a bootable image)."""
import argparse
import hashlib
import json
import re
import shutil
import tarfile
import tempfile
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--platform", choices=("linux", "t113"), required=True)
parser.add_argument("--version", default="0.4.0-alpha.1")
parser.add_argument("--profile", choices=("coding-companion",), default="coding-companion")
parser.add_argument("--output", type=Path, default=Path("dist"))
args = parser.parse_args()
if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]{0,63}", args.version):
    parser.error("Invalid version")
root = Path(__file__).resolve().parents[1]
binary = root / "build" / args.platform / "app" / "demo"
if not binary.is_file():
    parser.error("Build the selected platform first")
output = args.output.resolve()
output.mkdir(parents=True, exist_ok=True)
name = f"aurabar-{args.profile}-{args.version}-{args.platform}"
with tempfile.TemporaryDirectory() as temporary:
    stage = Path(temporary) / name
    (stage / "bin").mkdir(parents=True)
    (stage / "notices").mkdir()
    (stage / "companion").mkdir()
    for filename in ("aurabar_companion.py", "codex_link.py", "codex_result.schema.json", "README.md"):
        shutil.copyfile(root.parent / "companion" / filename, stage / "companion" / filename)
    shutil.copytree(root.parent / "companion" / "examples", stage / "companion" / "examples")
    for source, notice_name in ((root.parent / "LICENSE", "AuraBar-MIT.txt"),
                         (root / "lvgl/LICENCE.txt", "LVGL-MIT.txt"),
                         (root / "third_party/minimp3/LICENSE", "minimp3-CC0.txt"),
                         (root / "THIRD_PARTY.md", "THIRD_PARTY.md")):
        shutil.copyfile(source, stage / "notices" / notice_name)
    shutil.copyfile(binary, stage / "bin" / "aurabar")
    (stage / "bin" / "aurabar").chmod(0o755)
    allowed = {".png", ".jpg", ".jpeg", ".gif", ".otf", ".ttf", ".wav"}
    for kind in ("image", "font", "music"):
        for source in (root / "app" / "res" / kind).rglob("*"):
            if not source.is_file() or source.suffix.lower() not in allowed:
                continue
            target = stage / "assets" / kind / source.relative_to(root / "app" / "res" / kind)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
    manifest = {
        "profile": args.profile, "version": args.version, "platform": args.platform,
        "artifact_kind": "application-upgrade", "bootable": False,
        "hardware": "AuraBar T113-S3 v1.0, existing verified fl7707n_mipi firmware",
        "resources": "/usr/res", "user_data": "/usr/var/lib/aurabar",
        "runtime_dependencies": ["libasound.so.2", "libcurl.so.4", "existing SDK runtime libraries", "aplay"],
        "files": {},
    }
    for file in sorted(stage.rglob("*")):
        if file.is_file():
            manifest["files"][file.relative_to(stage).as_posix()] = hashlib.sha256(file.read_bytes()).hexdigest()
    (stage / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    archive = output / (name + ".tar.gz")
    with tarfile.open(archive, "w:gz") as tar:
        tar.add(stage, arcname=name)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    (output / (archive.name + ".sha256")).write_text(f"{digest}  {archive.name}\n", encoding="ascii")
    print(archive.name)
