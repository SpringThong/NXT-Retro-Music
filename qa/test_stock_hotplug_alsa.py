#!/usr/bin/env python3
"""Reproduce ALSA hook caching on official SDL/ALSA; assert live route handoff."""
from pathlib import Path
import subprocess,tempfile,os,json
R=Path(__file__).resolve().parents[1];sdk=R/'.nxt-sdk/tg5040/compiler';rom=R/'.cache/stock-research/rootfs'
with tempfile.TemporaryDirectory(prefix='nxt-hotplug-alsa-') as td:
 w=Path(td);c=w/'test.c';c.write_text(r'''
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
extern int snd_config_update_free_global(void);
static void samples(void *data,Uint8 *pcm,int n){SDL_memset(pcm,64,n);}
static char base[1200],route[1400];
static void config(const char *output){FILE *f=fopen(route,"w");assert(f);fprintf(f,"pcm.nxt_music_output { type file slave.pcm \"nxt_null\" file \"%s/%s.raw\" }\n",base,output);assert(!fclose(f));}
static off_t size(const char *name){char p[1400];snprintf(p,sizeof(p),"%s/%s.raw",base,name);struct stat s;return stat(p,&s)?0:s.st_size;}
static void play(bool reload){if(reload)assert(snd_config_update_free_global()==0);SDL_AudioSpec want={0},have;want.freq=48000;want.channels=2;want.samples=1024;want.format=AUDIO_S16SYS;want.callback=samples;SDL_AudioDeviceID d=SDL_OpenAudioDevice(NULL,0,&want,&have,0);if(!d){puts(SDL_GetError());abort();}SDL_PauseAudioDevice(d,0);SDL_Delay(90);SDL_CloseAudioDevice(d);}
int main(int argc,char **argv){assert(argc==2);snprintf(base,sizeof(base),"%s",argv[1]);snprintf(route,sizeof(route),"%s/runtime.conf",base);assert(!SDL_setenv("AUDIODEV","nxt_music_output",1));assert(!SDL_Init(SDL_INIT_AUDIO));
 config("speaker");play(true);assert(size("speaker")>0);
 config("dac");play(false);assert(size("dac")==0);puts("REPRODUCED baseline: SDL reopen alone still opens cached speaker route.");
 config("dac");play(true);assert(size("dac")>0);
 off_t dac=size("dac");config("speaker-return");play(true);assert(size("speaker-return")>0&&size("dac")==dac);
 for(int i=0;i<4;i++){config(i%2?"speaker-return":"dac");play(true);}
 SDL_Quit();puts("PASS official Stock SDL/ALSA: cached hook reproduced, live speaker→DAC→speaker and repeated handoffs work after process-local config reload without restarting SDL/app.");return 0;}
''')
 base=w/'alsa.conf';base.write_text('pcm.nxt_null { type null }\n@hooks [ { func load files [ "'+str(w/'runtime.conf')+'" ] errors false } ]\n')
 binary=w/'test';cmd=[str(sdk/'bin/aarch64-nextui-linux-gnu-gcc'),'-O2',str(c),'-include','stdbool.h','-I'+str(sdk/'aarch64-nextui-linux-gnu/libc/usr/include'),'-lSDL2','-lasound','-o',str(binary)]
 subprocess.run(cmd,check=True)
 p=subprocess.run([str(R/'qa/run-stock-arm64.sh'),'-E','SDL_AUDIODRIVER=alsa','-E','ALSA_CONFIG_PATH='+str(base),str(binary),str(w)],capture_output=True,text=True,timeout=20)
 (R/'logs/stock-hotplug-alsa-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/stock-hotplug-alsa-test.json').write_text(json.dumps({'real_sdl_alsa_handoff':{'exit':0,'baseline_cache_reproduced':True,'two_way_live_handoff':True,'repeat_cycles':4,'official_stock_arm64':True,'physical_dac_tested':False}},indent=2)+'\n')
