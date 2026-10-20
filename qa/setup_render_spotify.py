#!/usr/bin/env python3
"""Prepare native renderer QA from bundled source archives and public SDK headers.
Requires Linux SDL2/FreeType/HarfBuzz/FriBidi/samplerate runtimes, gcc, Python/Pillow.
"""
from pathlib import Path
import tarfile,shutil,subprocess,re,struct
from PIL import Image,ImageDraw,ImageFont
r=Path(__file__).resolve().parents[1];q=r/'.cache/render-sp';q.mkdir(parents=True,exist_ok=True);headers=q/'include';headers.mkdir(exist_ok=True)
sdk=r/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include'
for name in ['SDL2','GLES2','EGL','KHR']:shutil.copytree(sdk/name,headers/name,dirs_exist_ok=True)
archives={'sdl-ttf':'SDL2_ttf-2.24.0.tar.gz','freetype':'freetype-2.13.2.tar.gz','harfbuzz':'harfbuzz-8.3.0.tar.gz','fribidi':'fribidi-1.0.13.tar.xz','utf8proc':'utf8proc-2.9.0.tar.gz'}
for name,archive in archives.items():
 dest=q/'src'/name
 if not dest.is_dir():
  dest.mkdir(parents=True)
  with tarfile.open(r/'sources'/archive) as tar:
   root=tar.getmembers()[0].name.split('/')[0]
   for member in tar.getmembers():
    if not member.name.startswith(root+'/'):continue
    member.name=member.name[len(root)+1:]
    if member.name:tar.extract(member,dest,filter='data')
shutil.copyfile(q/'src/sdl-ttf/SDL_ttf.h',headers/'SDL2/SDL_ttf.h')
shutil.copytree(q/'src/fribidi/lib',headers/'fribidi',dirs_exist_ok=True)
x=(headers/'fribidi/fribidi-config.h.in').read_text()
for key,value in {'FRIBIDI_VERSION':'1.0.13','FRIBIDI_MAJOR_VERSION':'1','FRIBIDI_MINOR_VERSION':'0','FRIBIDI_MICRO_VERSION':'13','FRIBIDI_INTERFACE_VERSION':'4','SIZEOF_INT':'4'}.items():x=x.replace('@'+key+'@',value)
x=x.replace('@FRIBIDI_MSVC_BUILD@','').replace('@FRIBIDI_MSVC_BUILD_PLACEHOLDER@','');(headers/'fribidi/fribidi-config.h').write_text(x)
incs=['-I'+str(headers),'-I'+str(headers/'SDL2'),'-I'+str(headers/'fribidi'),'-I'+str(q/'src/harfbuzz/src'),'-I'+str(q/'src/freetype/include'),'-I'+str(q/'src/utf8proc')]
with (q/'setup.log').open('w') as log:
 for src,name,extra in [(q/'src/sdl-ttf/SDL_ttf.c','ttf.o',['-DTTF_USE_HARFBUZZ=1']),(q/'src/utf8proc/utf8proc.c','utf8proc.o',[])]:subprocess.run(['gcc','-O2','-fPIC',*incs,*extra,'-c',str(src),'-o',str(q/name)],stdout=log,stderr=log,check=True)
(q/'includes.txt').write_text('\n'.join(incs))
stage=q/'stage';stage.mkdir(exist_ok=True);(stage/'res').mkdir(exist_ok=True);shutil.copytree(r/'upstream/res/fonts',stage/'res/fonts',dirs_exist_ok=True)
for f in (r/'upstream/res/icons').glob('*.png'):
 target=stage/'res/icons'/f.name;target.parent.mkdir(exist_ok=True);im=Image.open(f).convert('RGBA');target.with_suffix('.png.rgba').write_bytes(struct.pack('<II',*im.size)+im.tobytes('raw','BGRA'))
cover=Image.new('RGB',(300,300));d=ImageDraw.Draw(cover)
for y in range(300):d.line((0,y,300,y),fill=(26+y//10,40+y//7,36+y//9))
d.ellipse((47,40,253,246),outline=(157,228,182),width=5);d.ellipse((104,97,196,189),fill=(17,26,24));d.line((150,28,150,263),fill=(234,244,237),width=2);font=ImageFont.truetype(str(r/'upstream/res/fonts/notosans.ttf'),18);d.text((74,270),'NXT DEMO',font=font,fill=(234,244,237));f=stage/'cover.png';f.with_suffix('.png.rgba').write_bytes(struct.pack('<II',*cover.size)+cover.convert('RGBA').tobytes('raw','BGRA'))
(q/'fixture.lrc').write_text('[00:00.000]Nhạc thử nghiệm · 한국어 日本語 中文 ภาษาไทย\n[01:00.000]Một dòng lời thử nghiệm\n')
