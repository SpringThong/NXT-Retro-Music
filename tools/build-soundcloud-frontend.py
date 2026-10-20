#!/usr/bin/env python3
"""Recompile provider menus and branding; relink pinned native objects."""
from pathlib import Path
import shutil, subprocess, tarfile

r = Path(__file__).resolve().parents[1]
sdk = r/'.nxt-sdk/tg5040'
cc = sdk/'compiler/bin/aarch64-nextui-linux-gnu-gcc'
if not cc.is_file():
    subprocess.run(['python3', str(r/'tools/sdk-assets.py')], check=True)
include = sdk/'local/include'
include.mkdir(parents=True, exist_ok=True)
if not (include/'samplerate.h').is_file():
    with tarfile.open(r/'sources/libsamplerate-0.2.2-source.tar.gz') as tar:
        member = next(m for m in tar.getmembers() if Path(m.name).name == 'samplerate.h')
        (include/'samplerate.h').write_bytes(tar.extractfile(member).read())
libraries = sdk/'local/lib'
libraries.mkdir(parents=True, exist_ok=True)
for name in ('libzip.a', 'libsamplerate.a'):
    shutil.copyfile(r/'relink/lib'/name, libraries/name)
(sdk/'.libraries-ready').write_text('Pinned ARM64 static libraries reused from relink/lib\n')
src = r/'upstream/src'
flags = [str(cc), '-c', '-O2', '-D_GNU_SOURCE', '-DUSE_SDL2', '-DUSE_GLES',
         '-DPLATFORM="tg5040"', '-std=gnu99', '-Werror=implicit-function-declaration',
         '-include', str(src/'nxt_i18n.h')]
for path in (src, src/'include', include, sdk/'compiler/aarch64-nextui-linux-gnu/libc/usr/include/SDL2',
             r/'upstream/NextUI/workspace/all/common', r/'upstream/NextUI/workspace/tg5040/platform',
             r/'upstream/NextUI/workspace/tg5040/libmsettings'):
    flags.append('-I'+str(path))
for name in ('nxt_skin', 'nxt_spotify', 'nxt_soundcloud'):
    print('Compile', name, flush=True)
    subprocess.run(flags+['-o',str(r/'relink/obj'/f'{name}.o'),str(src/f'{name}.c')], check=True)
subprocess.run(['bash',str(r/'tools/relink-nextui.sh')], cwd=r, check=True)
binary = r/'upstream/bin/tg5040/musicplayer.elf'
shutil.copyfile(r/'dist/relinked/musicplayer.elf', binary)
subprocess.run([str(sdk/'compiler/bin/aarch64-nextui-linux-gnu-strip'),str(binary)], check=True)
binary.chmod(0o755)
subprocess.run(['python3',str(r/'tools/sync-overlay.py')], check=True)
