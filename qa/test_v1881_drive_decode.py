#!/usr/bin/env python3
"""Real shipped FFmpeg/NXT decoder on ARM64: temporary Drive files and metadata."""
from pathlib import Path
import subprocess,tempfile,os,wave,math,struct,json
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/stock-build/headers';sdk=R/'.nxt-sdk/tg5040/compiler';sysroot=sdk/'aarch64-nextui-linux-gnu/libc';cc=sdk/'bin/aarch64-nextui-linux-gnu-gcc';rom=sysroot
includes=[H,H/'ffmpeg',H/'fribidi/lib',H/'harfbuzz/src',H/'freetype/include',H/'utf8proc',S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',sysroot/'usr/include',sysroot/'usr/include/SDL2']
with tempfile.TemporaryDirectory(prefix='nxt-drive-decode-') as directory:
 w=Path(directory);cache=w/'nxt-retro-music/drive/cache';cache.mkdir(parents=True);part=cache/'1AudioFixture00.wav.part'
 with wave.open(str(part),'wb') as f:
  f.setparams((2,2,44100,0,'NONE','not compressed'));f.writeframes(b''.join(struct.pack('<hh',int(10000*math.sin(i*.062)),int(10000*math.sin(i*.062))) for i in range(44100*2)))
 data=w/'userdata';(data/'drive/streams').mkdir(parents=True);(data/'drive/streams/PUBLICAUDIOFIX00.json').write_text(json.dumps({'url':'https://drive.google.com/file/d/PUBLICAUDIOFIX00/view','name':'2 Nhạc Việt.wav'}))
 source=w/'decode.c';source.write_text(r'''
#include "nxt_decoder.h"
#include "nxt_drive.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "nxt_stream.h"
#include <stdlib.h>
static char stream_file[1400];
struct NxtStream {FILE *file;int64_t size;};
int userdata_snpath(const char *rel,char *out,size_t n){return snprintf(out,n,"%s/%s",getenv("QA_DATA"),rel);}
bool Browser_isAudioFile(const char*p){return p&&strstr(p,".wav");}
unsigned NxtStream_epoch(void){return 1;}
NxtStream *NxtStream_open(const NxtDriveEntry*entry,unsigned epoch){NxtStream*s=calloc(1,sizeof(*s));assert(!strcmp(entry->id,"PUBLICAUDIOFIX00"));s->file=fopen(stream_file,"rb");assert(s->file);fseek(s->file,0,SEEK_END);s->size=ftell(s->file);rewind(s->file);return s;}
int NxtStream_read(NxtStream*s,void*p,size_t n){return fread(p,1,n,s->file);}
int64_t NxtStream_size(const NxtStream*s){return s->size;}int64_t NxtStream_position(const NxtStream*s){return ftell(s->file);}
int64_t NxtStream_seek(NxtStream*s,int64_t n){return fseek(s->file,n,SEEK_SET)?-1:ftell(s->file);}
bool NxtStream_seekable(const NxtStream*s){return true;}
void NxtStream_close(NxtStream*s){if(s){fclose(s->file);free(s);}}void NxtStream_rearm(NxtStream*s,unsigned epoch){}

int main(int argc,char **argv){TrackInfo info;assert(NxtDecoder_probe(argv[1],&info));assert(!strcmp(info.codec,"pcm_s16le")&&info.sample_rate==44100&&info.channels==2);char final[1200],name[1300];snprintf(final,sizeof(final),"%s",argv[1]);final[strlen(final)-5]=0;assert(!rename(argv[1],final));snprintf(name,sizeof(name),"%s.name",final);FILE *f=fopen(name,"w");assert(f);fputs("2 Nhạc Việt.wav",f);fclose(f);assert(NxtDecoder_probe(final,&info));assert(!strcmp(info.title,"2 Nhạc Việt")&&!strcmp(info.album,"Google Drive"));NxtDecoder *d=NxtDecoder_open(final,48000);assert(d);float pcm[2048];int n,total=0;float peak=0;while((n=NxtDecoder_read(d,pcm,1024))>0){total+=n;for(int i=0;i<n*2;i++){assert(isfinite(pcm[i]));if(fabsf(pcm[i])>peak)peak=fabsf(pcm[i]);}}assert(total>=95000&&peak>.2f&&peak<1);assert(NxtDecoder_seek(d,1000)&&NxtDecoder_read(d,pcm,1024)>0);NxtDecoder_close(d);snprintf(stream_file,sizeof(stream_file),"%s",final);d=NxtDecoder_open("nxtdrive://PUBLICAUDIOFIX00/audio.wav",48000);assert(d&&NxtDecoder_read(d,pcm,1024)>0);assert(NxtDecoder_info(d)->sample_rate==44100&&!strcmp(NxtDecoder_info(d)->title,"2 Nhạc Việt"));assert(NxtDecoder_seek(d,1000)&&NxtDecoder_read(d,pcm,1024)>0);NxtDecoder_close(d);puts("PASS ARM64 production FFmpeg decoder: .part audio detection, Drive UTF-8 sidecar metadata, 44.1-to-48 kHz PCM and seek using shipped libraries.");return 0;}
''')
 libs=R/'relink/lib';text=libs/'text';ff=libs/'ffmpeg'
 cmd=[str(cc),'-O2','-D_GNU_SOURCE','-DUSE_SDL2','-ffunction-sections','-fdata-sections',*['-I'+str(p) for p in includes],str(source),str(S/'nxt_decoder.c'),str(S/'nxt_drive.c'),str(S/'nxt_drive_parser.c'),str(S/'include/parson/parson.c'),str(S/'nxt_text.c'),'-Wl,--gc-sections','-L'+str(sysroot/'usr/lib'),'-Wl,-rpath-link,'+str(sysroot/'lib')+':'+str(sysroot/'usr/lib'),*map(str,[ff/'libavformat.a',ff/'libavcodec.a',ff/'libswresample.a',ff/'libavutil.a']),'-lSDL2_image',*map(str,[text/'libSDL2_ttf.a',text/'libharfbuzz.a',text/'libfreetype.a',text/'libutf8proc.a',text/'libfribidi.a']),'-lstdc++','-lSDL2','-lm','-lz','-lpthread','-o',str(w/'decode')]
 subprocess.run(cmd,check=True)
 env={**os.environ,'QA_DATA':str(data),'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')}
 subprocess.run([str(R/'.host-tools/usr/bin/qemu-aarch64'),'-L',str(rom),'-E','LD_LIBRARY_PATH='+str(sysroot/'lib')+':'+str(sysroot/'usr/lib'),str(w/'decode'),str(part)],env=env,timeout=30,check=True)
(R/'logs/v1881-drive-decode-test.json').write_text(json.dumps({'actual_arm64_decoder':{'exit':0,'pinned_SDK_libc':True,'synthesized_audio':True,'network_or_device_test':False,'production_Drive_URI_metadata_normal_decoder_seek':True,'HTTPS_buffer_transport_FILE_fixture':True}},indent=2)+'\n')
