"""Verify an application archive without extracting it."""
import argparse
import hashlib
import json
import tarfile
from pathlib import Path, PurePosixPath

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('archive', type=Path)
args = parser.parse_args()
archive = args.archive
expected = Path(str(archive) + '.sha256').read_text(encoding='ascii').split()[0]
if hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
    raise SystemExit('Archive SHA256 mismatch')
with tarfile.open(archive, 'r:gz') as tar:
    members = tar.getmembers()
    roots = set()
    files = {}
    for member in members:
        path = PurePosixPath(member.name)
        if path.is_absolute() or '..' in path.parts or not path.parts:
            raise SystemExit('Unsafe archive path')
        roots.add(path.parts[0])
        if not member.isdir() and not member.isfile():
            raise SystemExit('Unsupported archive member')
        if member.isfile():
            if member.size > 128 * 1024 * 1024:
                raise SystemExit('Unexpectedly large member')
            key = '/'.join(path.parts[1:])
            if key in files:
                raise SystemExit('Duplicate archive member')
            files[key] = member
    if len(roots) != 1 or 'manifest.json' not in files:
        raise SystemExit('Invalid package layout')
    manifest = json.load(tar.extractfile(files.pop('manifest.json')))
    if manifest.get('artifact_kind') != 'application-upgrade' or manifest.get('bootable') is not False:
        raise SystemExit('Unexpected artifact type')
    if set(manifest['files']) != set(files):
        raise SystemExit('Manifest does not match archive contents')
    for name, member in files.items():
        if hashlib.sha256(tar.extractfile(member).read()).hexdigest() != manifest['files'][name]:
            raise SystemExit('File SHA256 mismatch: ' + name)
print(f"Verified {len(files)} files; {manifest['profile']} {manifest['version']} ({manifest['platform']})")
