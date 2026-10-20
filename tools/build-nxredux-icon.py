#!/usr/bin/env python3
"""Rebuild official NX Redux v2.1.2 with one additional Tools icon mapping."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,re,shutil
R=Path(__file__).resolve().parents[1]
S=R/'nxredux/launcher-source';W=S/'workspace/all/nextui'
C=R/'.nxt-sdk/tg5040/compiler';CC=C/'bin/aarch64-nextui-linux-gnu-gcc'
SYS=C/'aarch64-nextui-linux-gnu/libc';H=R/'.cache/nxredux-build/headers'
B=R/'.cache/nxredux-icon-build';B.mkdir(parents=True,exist_ok=True)
P=R/'nxredux/integration';L=R/'relink/lib'
(B/'link').mkdir(exist_ok=True)
for n in ['msettings','gametimedb']:shutil.copy2(P/('lib'+n+'-reference.so'),B/'link'/('lib'+n+'.so'))
flags=['-O3','-Ofast','-flto','-fomit-frame-pointer','-mcpu=cortex-a53','-D_GNU_SOURCE','-DUSE_SDL2','-DUSE_GLES','-DGL_GLEXT_PROTOTYPES','-DPLATFORM="tg5040"','-std=gnu99']
includes=[W,W/'../common',W/'../common/ui',W/'../ratools',W/'../libgametimedb',S/'workspace/tg5040/platform',S/'workspace/tg5040/libmsettings',S/'rcheevos/include',H/'samplerate',H/'samplerate/src',H/'harfbuzz/src',H/'freetype/include',H/'utf8proc',H/'fribidi/lib',SYS/'usr/include',SYS/'usr/include/SDL2']
flags+=['-I'+str(p) for p in includes]
local=B/'include/rcheevos';local.mkdir(parents=True,exist_ok=True)
for p in (S/'rcheevos/include').glob('*.h'):shutil.copy2(p,local/p.name)
flags+=['-I'+str(B/'include')]
make=(W/'Makefile').read_text();line=next(l for l in make.splitlines() if l.startswith('SOURCE ='))
ui=(W/'../common/ui/ui.mk').read_text();match=re.search(r'UI_COMPONENT_SRCS = (.*?)(?:\n\n)',ui,re.S)
parts=match.group(1).replace('\\\n',' ').split()
line=line.replace('$(TARGET)','nextui').replace('$(UI_COMPONENT_SRCS)',' '.join(parts)).replace('$(UI_LISTVIEW_SRCS)','$(UI_DIR)/ui_listview.c').replace('$(UI_DIR)','../common/ui').replace('$(PLATFORM)','tg5040')
sources=[(W/p).resolve() for p in line.split('=',1)[1].split()]
rc=[p for p in (S/'rcheevos/src').rglob('*.c') if p.name not in ['rc_libretro.c','rc_client_external.c','rc_client_raintegration.c'] and 'lua' not in p.parts]
def compile_one(item):
 idx,source=item;obj=B/(str(idx)+'.o');f=flags+(['-DRC_DISABLE_LUA','-DRC_CLIENT_SUPPORTS_HASH','-I'+str(S/'rcheevos/src')] if source in rc else [])
 subprocess.run([str(CC),*f,'-c',str(source),'-o',str(obj)],check=True);return obj
with ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile_one,enumerate(sources+rc)))
subprocess.run([str(C/'bin/aarch64-nextui-linux-gnu-ar'),'rcs',str(B/'librcheevos.a'),*map(str,objects[len(sources):])],check=True)
link=[str(CC),'-O3','-flto',*map(str,objects[:len(sources)]),str(B/'librcheevos.a'),'-Wl,--as-needed','-L'+str(B/'link'),'-L'+str(SYS/'usr/lib'),'-L'+str(SYS/'lib'),'-Wl,-rpath-link,'+str(B/'link')+':'+str(SYS/'usr/lib')+':'+str(SYS/'lib'),'-lmsettings','-lgametimedb','-lSDL2_image','-lSDL2_ttf',str(L/'libsamplerate.a'),'-ltinyalsa','-lSDL2','-lGLESv2','-lsqlite3','-lpthread','-ldl','-lm','-lz','-s','-o',str(P/'nextui-v2.1.2-nxt-icon.elf')]
subprocess.run(link,check=True)
print('Built official v2.1.2 launcher with NXT Tools icon; original mappings retained.')
