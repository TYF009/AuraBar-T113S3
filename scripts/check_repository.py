"""Check local documentation, asset integrity and current release metadata (stdlib only)."""
import hashlib
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]


def check():
    errors = []
    docs = set(ROOT.glob('*.md')) | set((ROOT / 'docs').rglob('*.md'))
    for folder in ('hardware', 'assets', 'software', 'software/app_sdk', 'software/companion', 'hardware/mechanical'):
        docs.update((ROOT / folder).glob('*.md'))
    docs.update((ROOT / 'hardware/docs').glob('*.md'))
    for path in (ROOT / 'docs/architecture').glob('*.json'):
        try:
            json.loads(path.read_text(encoding='utf-8'))
        except (ValueError, OSError) as error:
            errors.append(f'Invalid architecture JSON: {path.name}: {error}')
    links = 0
    for path in sorted(docs):
        text = path.read_text(encoding='utf-8-sig')
        body = re.sub(r'```[\s\S]*?```|~~~[\s\S]*?~~~', '', text)
        for link in re.findall(r'!?\[[^\]\n]*\]\(([^)\n]+)\)', body):
            link = link.strip().split(' "', 1)[0].strip('<>')
            if not link or link.startswith('#') or urlsplit(link).scheme:
                continue
            target = unquote(link.split('#', 1)[0].split('?', 1)[0])
            resolved = (path.parent / target).resolve()
            if not resolved.is_relative_to(ROOT):
                errors.append(f'{path.relative_to(ROOT)}: link outside repository: {link}')
            elif not resolved.exists():
                errors.append(f'{path.relative_to(ROOT)}: missing link: {link}')
            links += 1
    version = (ROOT / 'VERSION').read_text().strip()
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?', version):
        errors.append('Invalid VERSION')
    patterns = {
        'software/app_sdk/app/main.c': r'puts\("AuraBar Coding Companion ([^"]+)"\)',
        'software/app_sdk/app/ui/page_about_us.c': r'Coding Companion ([0-9][^"]+)"',
        'software/app_sdk/scripts/package.py': r'--version[^\n]*default=[\'"]([^\'"]+)',
        'software/app_sdk/scripts/deploy.py': r'--version[^\n]*default=[\'"]([^\'"]+)',
    }
    for name, pattern in patterns.items():
        found = re.search(pattern, (ROOT / name).read_text(encoding='utf-8'))
        if not found or found.group(1) != version:
            errors.append(f'Current version mismatch: {name}')
    profile = json.loads((ROOT / 'software/profiles/coding-companion.json').read_text())
    if profile.get('version') != version:
        errors.append('Current profile version mismatch')
    catalog = json.loads((ROOT / 'software/releases.json').read_text())
    if catalog.get('current') != version or not any(r['version'] == version for r in catalog['releases']):
        errors.append('Release catalog current version mismatch')
    tags = set()
    for release in catalog['releases']:
        if release['tag'] in tags:
            errors.append('Duplicate release tag')
        tags.add(release['tag'])
        if release.get('publication') not in ('draft', 'prerelease', 'published'):
            errors.append('Invalid release availability')
        if not re.fullmatch('[0-9a-f]{40}', release['source_revision']):
            errors.append('Invalid release revision')
        digest = release.get('sha256')
        if digest is not None and not re.fullmatch('[0-9a-fA-F]{64}', digest):
            errors.append('Invalid release SHA256')
        if not (ROOT / release['acceptance']).is_file():
            errors.append('Missing release acceptance record')
        if release['artifact_kind'] != 'application-upgrade' or release['bootable'] is not False:
            errors.append('Unexpected artifact-kind claim')
    assets = json.loads((ROOT / 'assets/manifest.json').read_text(encoding='utf-8'))['assets']
    for asset in assets:
        target = (ROOT / asset['path']).resolve()
        if not target.is_relative_to(ROOT) or not target.is_file():
            errors.append(f'Missing/invalid asset path: {asset["path"]}')
            continue
        if hashlib.sha256(target.read_bytes()).hexdigest() != asset['sha256']:
            errors.append(f'Asset digest mismatch: {asset["path"]}')
        if asset.get('kind') not in ('concept', 'framebuffer', 'cad-render'):
            errors.append(f'Unclassified asset: {asset["path"]}')
    fonts = json.loads((ROOT / 'software/resource-provenance.json').read_text(encoding='utf-8'))['fonts']
    for font in fonts:
        target = (ROOT / font['path']).resolve()
        if not target.is_relative_to(ROOT) or not target.is_file():
            errors.append('Invalid font provenance path')
        elif hashlib.sha256(target.read_bytes()).hexdigest() != font['sha256']:
            errors.append(f'Font provenance digest mismatch: {font["path"]}')
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        return 1
    print(f'Checked {len(docs)} documents, {links} local links, {len(assets)} assets, {len(fonts)} font records and {len(tags)} releases; version {version}.')
    return 0


if __name__ == '__main__':
    raise SystemExit(check())
