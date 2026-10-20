#!/usr/bin/env python3
"""Regression for 10 local formats + DSP with the production ARM64 decoder."""
from pathlib import Path
import json
import os
import subprocess
import tempfile

R = Path(__file__).resolve().parents[1]
S = R / 'upstream/src'
H = R / '.cache/stock-build/headers'
sdk = R / '.nxt-sdk/tg5040/compiler'
sysroot = sdk / 'aarch64-nextui-linux-gnu/libc'
libs = R / 'relink/lib'
include = [H, H/'ffmpeg', H/'zip/lib', H/'samplerate', H/'fribidi/lib',
           H/'harfbuzz/src', H/'freetype/include', H/'utf8proc', S, S/'include',
           R/'upstream/NextUI/workspace/all/common',
           R/'upstream/NextUI/workspace/tg5040/platform',
           R/'upstream/NextUI/workspace/tg5040/libmsettings',
           sysroot/'usr/include', sysroot/'usr/include/SDL2']
with tempfile.TemporaryDirectory(prefix='nxt-nextui-local-decode-') as directory:
    w = Path(directory)
    command = [str(sdk/'bin/aarch64-nextui-linux-gnu-gcc'), '-O2', '-D_GNU_SOURCE',
               '-DUSE_SDL2','-DNXT_ROM_STOCK', '-ffunction-sections', '-fdata-sections',
               *['-I'+str(p) for p in include], str(R/'qa/test_native_audio.c'),
               *[str(S/name) for name in ['nxt_decoder.c', 'nxt_dsp.c', 'nxt_text.c',
                                          'nxt_drive.c', 'nxt_drive_parser.c', 'nxt_stream.c',
                                          'browser.c', 'nxt_player_core.c', 'file_utils.c',
                                          'include/parson/parson.c']],
               '-Wl,--gc-sections', '-L'+str(sysroot/'usr/lib'),
               '-Wl,-rpath-link,'+str(sysroot/'usr/lib')+':'+str(sysroot/'lib'),
               *[str(libs/('ffmpeg/'+n+'.a')) for n in ['libavformat', 'libavcodec', 'libswresample', 'libavutil']],
               '-lSDL2_image', *[str(libs/('text/'+n+'.a')) for n in
                                  ['libSDL2_ttf', 'libharfbuzz', 'libfreetype', 'libutf8proc', 'libfribidi']],
               '-lstdc++', '-lSDL2', '-lm', '-lz', '-lpthread', '-o', str(w/'decoder')]
    subprocess.run(command, check=True)
    subprocess.run([str(R/'.host-tools/usr/bin/qemu-aarch64'), '-L', str(R/'.cache/stock-research/rootfs'),
                    '-E', 'LD_LIBRARY_PATH=/usr/trimui/lib:/usr/lib:/lib', str(w/'decoder'), str(R/'qa/media')],
                   env={**os.environ, 'LD_LIBRARY_PATH': str(R/'.host-tools/usr/lib/x86_64-linux-gnu')},
                   check=True, timeout=30)
(R/'logs/stock-decoder-test.json').write_text(json.dumps({
    'local_10_formats': {'exit': 0, 'actual_arm64_decoder': True},
    'dsp_replaygain_eq_limiter': {'exit': 0}}, indent=2)+'\n')
