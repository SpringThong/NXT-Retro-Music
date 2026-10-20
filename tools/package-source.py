#!/usr/bin/env python3
"""Freeze the unified Stock/NextUI/NX Redux source, runtime payloads and verified QA."""
from pathlib import Path
import hashlib
import json
import os
import shutil
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
METADATA = json.loads((ROOT / 'SOURCE.json').read_text())
if METADATA.get('release_build_paused_by_user'):
    raise SystemExit('Release build paused for Bluetooth investigation. Use a source-only checkpoint; current binaries predate the fix.')
VERSION = METADATA['version']
RELEASE_ROMS = METADATA.get('release_roms', ['stock', 'nextui', 'nxredux'])
NAME = f'NXT_Retro_Music_v{VERSION}_Source' + ('_Checkpoint' if not METADATA.get('release_ready', True) else '')
OUT = ROOT / 'dist/release'
SKIP = {'.rsync-tmp', '.rsync-partial', '.cache', '.nxt-sdk', '.host-tools', '.git', '__pycache__', 'dist',
        '.pytest_cache', '.userdata', 'target', 'toolchain', 'host-sdk'}
PRIVATE = {'groq-api-key.txt','groq-key.txt','translation-service.json', 'credentials.json', 'spotify-credentials.json', 'soundcloud-credentials.json',
           'playback.json', 'last-playback.json', 'last-art.png', 'NXT_App_Log.txt', 'NXT_Device_Log.txt', 'NXT_Input_Log.txt',
           'NXT_Spotify_Log.txt', 'NXT_SoundCloud_Log.txt'}

def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    overlay = json.loads((ROOT / 'overlay-manifest.json').read_text())
    for item in overlay['files']:
        relative = item['path']
        assert digest(ROOT / 'upstream' / relative) == item['sha256'], relative
        assert digest(ROOT / 'overlay' / relative) == item['sha256'], relative
    required = ['SOURCE.json', f'README_v{VERSION}.md', 'overlay-manifest.json',
                f'docs/HUONG_DAN_NXT_Retro_Music_v{VERSION}.md', f'RELEASE_QA_v{VERSION}.md',
                
                'upstream/res/ime/candidates.sqlite3', 'upstream/src/nxt_theme_v18.inc']
    for rom, label, binary in [('stock', 'Stock', 'stock/bin/tg5040/musicplayer.elf'),
                               ('nextui', 'NextUI', 'upstream/bin/tg5040/musicplayer.elf'),
                               ('nxredux','NXRedux','nxredux/bin/tg5040/musicplayer.elf')]:
        if rom not in RELEASE_ROMS:
            continue
        header = (ROOT / binary).read_bytes()[:64]
        assert header[:6] == b'\x7fELF\2\1' and struct.unpack_from('<H', header, 18)[0] == 183
        report_path = f'BUILD_REPORT_v{VERSION}_{label}.json'
        report = json.loads((ROOT / report_path).read_text())
        assert report['version'] == VERSION and report['rom'] == rom and report['build']['exit'] == 0
        assert report['binary_sha256'] == digest(ROOT / binary)
        input_path = ROOT / {'stock':'stock','nextui':'upstream','nxredux':'nxredux'}[rom] / f'BUILD_INPUTS_v{VERSION}.json'
        inputs = json.loads(input_path.read_text())
        assert inputs['rom'] == 'NXT_ROM_' + rom.upper()
        assert inputs['binary_sha256'] == report['binary_sha256']
        for relative, sha in inputs['sources'].items():
            assert digest(ROOT / relative) == sha, relative
        for name, evidence in report.get('checks', {}).items():
            assert digest(ROOT / 'logs' / name) == evidence['sha256'], name
        required.extend([binary, report_path])
    files = []
    for root, dirs, names in os.walk(ROOT, followlinks=False):
        dirs[:] = sorted(d for d in dirs if d not in SKIP and not (Path(root) / d).is_symlink()
                         and not (d.startswith('screens-v') and d != 'screens-v'+VERSION))
        for name in sorted(names):
            path = Path(root) / name
            if (path.is_file() and not path.is_symlink() and name not in PRIVATE
                and (not name.startswith('.') or name in {'.gitignore','.gitattributes','.gitmodules','.nxt-overlay-version','.editorconfig','.clang-format','.dockerignore','.npmrc','.yarnrc'})
                and not name.startswith('FILES_SHA256')
                and not name.endswith(('.pyc', '.building', '.writing', '.nxt-writing', '.part', '.tmp'))):
                files.append(path)
    hashes = {p.relative_to(ROOT).as_posix(): digest(p) for p in files}
    assert all(path in hashes for path in required)
    manifest = ROOT / ('FILES_SHA256_V'+VERSION.replace('.','')+'.json')
    manifest.write_text(json.dumps({'version': VERSION, 'roms': RELEASE_ROMS,
                                  'files': hashes}, ensure_ascii=False, indent=2) + '\n')
    files.append(manifest)
    archive = OUT / (NAME + '.zip')
    temporary = archive.with_suffix('.zip.building')
    with zipfile.ZipFile(temporary, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as output:
        for path in sorted(files):
            info = zipfile.ZipInfo.from_file(path, NAME + '/' + path.relative_to(ROOT).as_posix())
            info.compress_type = zipfile.ZIP_DEFLATED
            with path.open('rb') as stream:
                prefix = stream.read(4)
                stream.seek(0)
                if prefix == b'\x7fELF' or path.suffix == '.sh' or prefix[:2] == b'#!':
                    info.create_system = 3
                    info.external_attr = 0o100755 << 16
                with output.open(info, 'w') as target:
                    shutil.copyfileobj(stream, target, length=1024 * 1024)
    with zipfile.ZipFile(temporary) as output:
        assert output.testzip() is None
        for relative in required:
            assert hashlib.sha256(output.read(NAME + '/' + relative)).hexdigest() == hashes[relative], relative
        for relative in ['stock/bin/tg5040/musicplayer.elf', 'upstream/bin/tg5040/musicplayer.elf',
                         'nxredux/bin/tg5040/musicplayer.elf', 'stock/launch.sh', 'upstream/launch.sh','nxredux/launch.sh']:
            assert output.getinfo(NAME + '/' + relative).external_attr >> 16 & 0o111, relative
    with temporary.open('rb') as stream:
        os.fsync(stream.fileno())
    os.replace(temporary, archive)
    sha = digest(archive)
    archive.with_suffix('.zip.sha256').write_text(sha + '  ' + archive.name + '\n')
    print(json.dumps({'source': str(archive), 'files': len(files), 'size': archive.stat().st_size,
                      'sha256': sha}, indent=2), flush=True)

if __name__ == '__main__':
    main()
