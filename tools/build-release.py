#!/usr/bin/env python3
"""Build any/all ROMs from the same tree; keep each frontend and OTA separate."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TARGETS = {
    'stock': ('Stock', 'stock', 'Apps/NXT_Retro_Music'),
    'nextui': ('NextUI', 'upstream', 'Tools/tg5040/NXT Retro Music.pak'),
    'nxredux': ('NXRedux', 'nxredux', 'Tools/NXT Retro Music.pak'),
}


def positive(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError('jobs must be at least 1')
    return number


def preflight(roms):
    metadata = json.loads((ROOT / 'SOURCE.json').read_text())
    version = metadata['version']
    header = (ROOT / 'upstream/src/nxt.h').read_text()
    match = re.search(r'^#define NXT_VERSION "([^"]+)"', header, re.M)
    if not match or match[1] != version:
        raise ValueError('SOURCE.json and NXT_VERSION do not match')
    paths = ['stock/frontend-sources.json', 'relink/lib/libopus.a',
             'relink/lib/libmbedtls.a', 'relink/lib/libsamplerate.a',
             'relink/lib/libzip.a', 'upstream/res/ime/candidates.sqlite3',
             'upstream/bin/tg5040/nxt-spotify-backend',
             'upstream/bin/tg5040/nxt-soundcloud-backend']
    paths += ['relink/lib/ffmpeg/' + n + '.a' for n in
              ['libavformat', 'libavcodec', 'libswresample', 'libavutil']]
    paths += ['relink/lib/text/' + n + '.a' for n in
              ['libSDL2_ttf', 'libharfbuzz', 'libfreetype', 'libutf8proc', 'libfribidi']]
    paths += json.loads((ROOT / 'stock/frontend-sources.json').read_text())
    for path in paths:
        if not (ROOT / path).is_file():
            raise FileNotFoundError('Required bundled input missing: ' + path)
    # SDK is deliberately separate: build-stock.py downloads the pinned SDK
    # automatically if its compiler is absent. No existing checkout is needed.
    for rom in roms:
        label, folder, app = TARGETS[rom]
        config = json.loads((ROOT / folder / 'config.json').read_text())
        if (config.get('version') != version or config.get('rom') != rom or
                config.get('release_filename') != f'NXT_Retro_Music_v{version}_{label}.zip'):
            raise ValueError('Wrong version/ROM/package metadata for ' + rom)
        feed = ROOT / folder / f'ota-source-{rom}.url'
        if feed.read_text().strip() != metadata['official_OTA_sources'][rom]:
            raise ValueError('Official OTA source mismatch for ' + rom)
        print(f'{label} v{version}: {app}', flush=True)
    return version


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', choices=['all', *TARGETS], default='all')
    parser.add_argument('--jobs', type=positive, default=None)
    parser.add_argument('--build-only', action='store_true', help='Compile without installer/OTA packaging')
    parser.add_argument('--check', action='store_true', help='Check bundled inputs and metadata without compiling/downloading')
    args = parser.parse_args()
    roms = list(TARGETS) if args.rom == 'all' else [args.rom]
    version = preflight(roms)
    if args.check:
        print('PASS unified source inputs; SDK will be downloaded automatically when needed.')
        return
    for rom in roms:
        env = dict(os.environ, NXT_BUILD_ROM='NXT_ROM_' + rom.upper(), NXT_PACKAGE_ROM=rom)
        if args.jobs is not None:
            env['NXT_JOBS'] = str(args.jobs)
        commands = [['build-stock.py']]  # Explicit define above selects the ROM.
        if not args.build_only:
            commands += [[f'package-{rom}.py'], ['package-ota-kit.py']]
        for command in commands:
            subprocess.run([sys.executable, str(ROOT / 'tools' / command[0])],
                           cwd=ROOT, env=env, check=True)
    if not args.build_only:
        print(f'Done v{version}; installers and OTA kits: dist/<rom>-release/.', flush=True)
    print('No live OTA feed was uploaded. Physical ROM/DAC testing is separate.', flush=True)


if __name__ == '__main__':
    main()
