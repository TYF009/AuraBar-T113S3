"""Install a versioned application alongside /usr/bin/demo using ADB.

Existing matching display firmware and /usr/res assets are prerequisites.
This does not change boot configuration or install a whole-system image.
"""
import argparse
import hashlib
import re
import shlex
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--adb', default='adb')
parser.add_argument('--serial', required=True)
parser.add_argument('--version', default='0.3.0-alpha.1')
parser.add_argument('--profile', choices=('desktop-link',), default='desktop-link')
parser.add_argument('--binary', type=Path)
args = parser.parse_args()
if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._-]{0,63}', args.version):
    parser.error('Invalid version')
root = Path(__file__).resolve().parents[1]
binary = args.binary or root / 'build/t113/app/demo'
if not binary.is_file():
    parser.error('Build the T113 target or provide --binary')
adb = [args.adb, '-s', args.serial]
def shell(command):
    return subprocess.check_output(adb + ['shell', command], text=True).strip()
if shell('id -u') != '0':
    parser.error('The verified Tina development image must provide root ADB')
target = '/usr/bin/aurabar-' + args.profile + '-' + args.version
quoted = shlex.quote(target)
if shell('test -e ' + quoted + ' && echo exists || echo missing') == 'exists':
    parser.error('Version already installed. Stop it and use a new version name; never overwrite a running executable.')
if shell('test -r /usr/res/image/main_bg.png && echo yes || echo no') != 'yes':
    parser.error('Existing /usr/res assets are missing; provision the matching resources first')
digest = hashlib.sha256(binary.read_bytes()).hexdigest()
subprocess.run(adb + ['push', str(binary), target], check=True)
actual = shell('sha256sum ' + quoted).split()[0]
if actual != digest:
    raise SystemExit('SHA256 mismatch; application was not launched')
shell('chmod 755 ' + quoted)
print(shell(quoted + ' --version'))
print('Verified SHA256: ' + digest)
print('Installed alongside /usr/bin/demo. Boot configuration is unchanged.')
print('Stop the currently running UI before starting: ' + target + (' --tasks' if args.profile == 'desktop-link' else ' --music'))
print('Rollback: stop this version, then start /usr/bin/demo from /usr/bin.')
