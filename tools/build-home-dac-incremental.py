#!/usr/bin/env python3
"""Recompile changed NextUI release units with the recorded baseline flags."""
from pathlib import Path
import shlex,subprocess,shutil,os
r=Path(__file__).resolve().parents[1]
old='/workspace/scratch/766af1647819/build-v1'
line=next(s for s in (r/'logs/footer-native-build.log').read_text().splitlines() if 'gcc -c -O2' in s)
a=shlex.split(line.replace(old,str(r)))
flags=a[:a.index('-o')]
# SDK includes use the pinned shared compiler sysroot. Text units need only the
# public SDL_ttf declarations; their existing static libraries remain unchanged.
for name in ['module_common','nxt_ui','nxt_i18n','nxt_screens','nxt_skin','nxt_hardware','nxt_pcm','nxt_usb','nxt_spotify','nxt_soundcloud','background','nxt_session','lyrics','nxt_config','nxt_player_core','musicplayer','module_menu']:
    p=r/'relink/obj'/f'{name}.o';print('Compile',name,flush=True)
    subprocess.run(flags+['-o',str(p),name+'.c'],cwd=r/'upstream/src',check=True)
subprocess.run(flags+['-o',str(r/'relink/obj/ext/all/common/api.o'),'../NextUI/workspace/all/common/api.c'],cwd=r/'upstream/src',check=True)
subprocess.run(['bash','tools/relink-nextui.sh'],cwd=r,check=True)
binary=r/'upstream/bin/tg5040/musicplayer.elf';shutil.copyfile(r/'dist/relinked/musicplayer.elf',binary)
subprocess.run([str(r/'.nxt-sdk/tg5040/compiler/bin/aarch64-nextui-linux-gnu-strip'),str(binary)],check=True);binary.chmod(0o755)
print('NextUI HomeDAC release rebuilt; baseline dependency archives retained.')
subprocess.run(['python3','tools/sync-overlay.py'],cwd=r,check=True)
