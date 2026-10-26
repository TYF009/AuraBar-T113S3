"""Verify original Word figure preservation and final schematic preview integrity."""
import hashlib
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

ROOT = Path(__file__).resolve().parents[1]
NS = {
    'a': 'http://schemas.openxmlformats.org/drawingml/2006/main',
    'r': 'http://schemas.openxmlformats.org/officeDocument/2006/relationships',
}


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

    source = resolve(manifest['source'])
    if digest(source.read_bytes()) != manifest['source_sha256']:
        errors.append('Original DOCX hash differs from figure provenance')
    with zipfile.ZipFile(source) as archive:
        media = {name for name in archive.namelist() if name.startswith('word/media/')}
        if len(media) != manifest['media_count']:
            errors.append('Word media count mismatch')
        rels = {rel.attrib['Id']: rel.attrib['Target'] for rel in ET.fromstring(archive.read('word/_rels/document.xml.rels'))}
        document = ET.fromstring(archive.read('word/document.xml'))
        placements = [
            'word/' + rels[blip.attrib['{' + NS['r'] + '}embed']]
            for blip in document.findall('.//a:blip', NS)
            if '{' + NS['r'] + '}embed' in blip.attrib
        ]
        records = manifest['figures']
        if placements != [record['docx_media'] for record in records]:
            errors.append('Word placement order differs from provenance')
        if len(placements) != manifest['image_occurrences']:
            errors.append('Word placement count mismatch')
        if media != {record['docx_media'] for record in records}:
            errors.append('Original Word media missing from provenance')
        for record in records:
            path = resolve(record['repository_image'])
            original = archive.read(record['docx_media'])
            if digest(original) != record['sha256'] or path.read_bytes() != original:
                errors.append(f'Original figure not preserved: {record["docx_media"]}')
            if path not in references:
                errors.append(f'Original figure not embedded in handbook: {path.name}')
        if len({record['sha256'] for record in records}) != manifest['unique_image_count']:
            errors.append('Unique Word image count mismatch')
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
    print(f'Hardware sources verified: {len(media)} original Word images, '
          f'{len(placements)} placements, {len(pages)} final sheets, '
          f'{len(illustrations)} embedded illustrations.')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(check())
    except (OSError, ValueError, KeyError, zipfile.BadZipFile, ET.ParseError) as error:
        print(f'Hardware source verification failed: {error}', file=sys.stderr)
        sys.exit(1)
