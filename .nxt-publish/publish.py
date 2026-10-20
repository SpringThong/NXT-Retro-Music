#!/usr/bin/env python3
"""One-time import into SpringThong/NXT-Retro-Music-Source, then private release.

Requires explicit owner approval to run with repository contents:write.
Does not change repository visibility or rebuild application binaries.
Transfer credentials are supplied at execution time, never printed.
"""
from pathlib import Path, PurePosixPath
import base64
import gzip
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile

REPOSITORY = 'SpringThong/NXT-Retro-Music-Source'
VERSION = '1.9.7.7'
SOURCE_SHA256 = 'e5cb7b0488af2d6775c9be9a3a2e86c00a829159f5c3056e10a61f10b60ae992'
SOURCE_PREFIX = 'NXT_Retro_Music_v1.9.7.7_Source/'
DOCS = ['README.md', 'README.vi.md', 'docs/GITHUB_PUBLISH.vi.md',
        'docs/screenshots/auralis-now-playing.png',
        'docs/screenshots/auralis-full-lyrics.png',
        'docs/screenshots/cassette-player.png',
        'docs/screenshots/studio-meter-player.png',
        'docs/screenshots/forest-dial-menu.png']

def digest(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''): h.update(block)
    return h.hexdigest()

def download(helper, transfer, destination):
    if not transfer.get('download_url'):
        raise ValueError('A fresh authorized download transfer is required')
    process = subprocess.run([sys.executable, str(helper), 'materialize', str(destination)],
        input=json.dumps(transfer), text=True, capture_output=True)
    if process.returncode: raise RuntimeError('Authorized download failed; no release published')

def extract_source(archive, root):
    names = []
    with zipfile.ZipFile(archive) as z:
        for info in z.infolist():
            if not info.filename.startswith(SOURCE_PREFIX): raise ValueError('Unexpected source root')
            p = PurePosixPath(info.filename[len(SOURCE_PREFIX):])
            if not p.parts: continue
            if p.is_absolute() or '..' in p.parts or '.git' in p.parts: raise ValueError('Unsafe archive path')
            mode = info.external_attr >> 16
            if mode & 0o170000 == 0o120000: raise ValueError('Unexpected symbolic link')
            target = root.joinpath(*p.parts)
            if info.is_dir(): target.mkdir(parents=True, exist_ok=True); continue
            if info.file_size >= 100 * 1024 * 1024: raise ValueError('Git file size limit exceeded')
            target.parent.mkdir(parents=True, exist_ok=True)
            with z.open(info) as src, target.open('wb') as dst: shutil.copyfileobj(src, dst)
            target.chmod(0o755 if mode & 0o111 else 0o644)
            names.append(p.as_posix())
    manifest = json.loads((root / 'FILES_SHA256_V1977.json').read_text())
    for name, expected in manifest['files'].items():
        if digest(root / name) != expected: raise ValueError('Release manifest mismatch: ' + name)
    return names, manifest

def main():
    root = Path.cwd()
    setup = root / '.nxt-publish'
    config = json.loads((setup / 'transfers.json').read_text())
    if config['repository'] != REPOSITORY: raise ValueError('Wrong target repository')
    actual = json.loads(subprocess.check_output(['gh', 'api', 'repos/' + REPOSITORY], text=True))
    if not actual.get('private'): raise ValueError('Repository must remain private')
    with tempfile.TemporaryDirectory(prefix='nxt-publish-', dir=os.environ.get('RUNNER_TEMP')) as tmp:
        tmp = Path(tmp)
        source = tmp / ('NXT_Retro_Music_v' + VERSION + '_Source.zip')
        helper = setup / 'library_file_transfer.py'
        download(helper, config['source'], source)
        if digest(source) != SOURCE_SHA256: raise ValueError('Original source checksum mismatch')
        overlay = tmp / 'documentation'
        for name in DOCS:
            target = overlay / name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(root / name, target)
        names, manifest = extract_source(source, root)
        original_verified = len(manifest['files'])
        for name in DOCS:
            (root / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(overlay / name, root / name)
        # Lossless transfer recipes copy original compressed bytes from the verified
        # release snapshot; they do not compile or rebuild the application.
        source_bytes = source.read_bytes()
        (tmp / 'assets').mkdir()
        for rom in ('Stock', 'NextUI', 'NXRedux'):
            recipe = json.loads(gzip.decompress((setup / (rom + '.recipe.json.gz')).read_bytes()))
            name = f'NXT_Retro_Music_v{VERSION}_{rom}.zip'
            if recipe['target_name'] != name: raise ValueError('Unexpected installer recipe')
            target = tmp / 'assets' / name
            with target.open('wb') as out:
                for operation in recipe['operations']:
                    if 'copy' in operation:
                        offset, length = operation['copy']
                        if offset < 0 or length < 0 or offset + length > len(source_bytes): raise ValueError('Invalid copy range')
                        out.write(source_bytes[offset:offset + length])
                    elif 'literal' in operation:
                        out.write(base64.b64decode(operation['literal'], validate=True))
                    else: raise ValueError('Only original-byte transfer is permitted')
            if target.stat().st_size != recipe['target_size'] or digest(target) != recipe['target_sha256']:
                raise ValueError('Installer transfer checksum mismatch')
        del source_bytes
        assets, checksum_lines = [], []
        for rom in ('Stock', 'NextUI', 'NXRedux'):
            report = json.loads((root / f'BUILD_REPORT_v{VERSION}_{rom}.json').read_text())
            installer = tmp / 'assets' / report['installer']
            sha = digest(installer)
            if sha != report['installer_sha256']: raise ValueError('Original installer checksum mismatch: ' + rom)
            with zipfile.ZipFile(installer) as z:
                if z.testzip() is not None: raise ValueError('Installer ZIP CRC failure: ' + rom)
            assets.append(installer)
            checksum_lines.append(sha + '  ' + installer.name)
        checksum_lines.append(SOURCE_SHA256 + '  ' + source.name)
        checksums = tmp / 'SHA256SUMS.txt'
        checksums.write_text('\n'.join(checksum_lines) + '\n')
        report_name = 'docs/GITHUB_IMPORT_v1.9.7.7.json'
        (root / report_name).write_text(json.dumps({'version':VERSION,
            'original_source_sha256':SOURCE_SHA256,'original_files':len(names),
            'original_manifest_hashes_verified':original_verified,
            'application_source_modified':False,'documentation_overlay':DOCS,
            'original_installer_sha256_verified':True}, indent=2) + '\n')
        manifest['github_import_original_source_sha256'] = SOURCE_SHA256
        manifest['files'] = {name:digest(root / name) for name in sorted(set(names) | set(DOCS) | {report_name}) if name != 'FILES_SHA256_V1977.json'}
        (root / 'FILES_SHA256_V1977.json').write_text(json.dumps(manifest, indent=2) + '\n')
        notes = tmp / 'release-notes.md'
        shutil.copyfile(setup / 'release-notes.md', notes)
        shutil.rmtree(setup)
        subprocess.run(['git','config','user.name','github-actions[bot]'], check=True)
        subprocess.run(['git','config','user.email','41898282+github-actions[bot]@users.noreply.github.com'], check=True)
        subprocess.run(['git','add','--all','--force'], check=True)
        subprocess.run(['git','commit','-m','Import complete NXT Retro Music v1.9.7.7 source for all three ROMs'], check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['git','push','origin','HEAD:main'], check=True)
        head = subprocess.check_output(['git','rev-parse','HEAD'], text=True).strip()
        subprocess.run(['gh','release','create','v'+VERSION,'--repo',REPOSITORY,'--target',head,
            '--title','NXT Retro Music v'+VERSION,'--notes-file',str(notes),'--draft',
            *map(str, assets),str(source),str(checksums)], check=True)
        subprocess.run(['gh','release','edit','v'+VERSION,'--repo',REPOSITORY,'--draft=false'], check=True)
        print('Private source and original Stock/NextUI/NX Redux installers published and verified.')

if __name__ == '__main__': main()
