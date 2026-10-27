"""Verify public hardware figures without distributing private authoring drafts."""
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def check():
    manifest = json.loads((ROOT / 'hardware/docs/figure-provenance.json').read_text(encoding='utf-8'))
    handbook = ROOT / 'hardware/docs/hardware-design-guide.md'
    references = {
        (handbook.parent / link).resolve()
        for link in re.findall(r'!\[[^\]\n]*\]\(([^)\n]+)\)', handbook.read_text(encoding='utf-8'))
    }
    errors = []

    def resolve(relative):
        path = (ROOT / relative).resolve()
        if not path.is_relative_to(ROOT):
            raise ValueError(f'Path outside repository: {relative}')
        return path

    for path in (ROOT / 'hardware').rglob('*'):
        if path.is_file() and path.suffix.lower() in ('.doc', '.docx'):
            errors.append(f'Authoring draft must stay outside public hardware: {path.relative_to(ROOT)}')
    records = manifest['figures']
    media = {record['docx_media'] for record in records}
    if len(media) != manifest['media_count']:
        errors.append('Published source-figure count mismatch')
    if len(records) != manifest['image_occurrences']:
        errors.append('Published figure placement count mismatch')
    if [record['order'] for record in records] != list(range(1, len(records) + 1)):
        errors.append('Published figure order mismatch')
    for record in records:
        path = resolve(record['repository_image'])
        if digest(path.read_bytes()) != record['sha256']:
            errors.append(f'Published figure hash mismatch: {path.name}')
        if path not in references:
            errors.append(f'Published figure not embedded in handbook: {path.name}')
    if len({record['sha256'] for record in records}) != manifest['unique_image_count']:
        errors.append('Unique published image count mismatch')
    pdf = manifest['schematic_pdf']
    if digest(resolve(pdf['path']).read_bytes()) != pdf['sha256']:
        errors.append('Schematic PDF changed; regenerate/review its previews')
    pages = pdf['renders']
    if [page['page'] for page in pages] != list(range(1, 8)):
        errors.append('Expected all seven final schematic sheets')
    for page in pages:
        path = resolve(page['image'])
        if digest(path.read_bytes()) != page['sha256']:
            errors.append(f'Schematic preview hash mismatch: {path.name}')
        if path not in references:
            errors.append(f'Final schematic sheet not embedded: {path.name}')
    illustrations = {
        path.resolve() for path in (ROOT / 'hardware/docs/images').rglob('*')
        if path.is_file() and path.suffix.lower() in ('.png', '.jpg', '.jpeg')
    }
    for path in sorted(illustrations - references):
        errors.append(f'Hardware illustration not embedded: {path.name}')
    if errors:
        print('\n'.join(errors), file=sys.stderr)
        return 1
    print(f'Hardware sources verified: {len(media)} published source images, '
          f'{len(records)} placements, {len(pages)} final sheets, '
          f'{len(illustrations)} embedded illustrations.')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(check())
    except (OSError, ValueError, KeyError) as error:
        print(f'Hardware source verification failed: {error}', file=sys.stderr)
        sys.exit(1)
