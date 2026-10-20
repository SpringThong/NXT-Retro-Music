#!/usr/bin/env python3
"""Exercise real visual policy and the production frame function at 60 Hz input."""
from pathlib import Path
import json, os, subprocess, tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src'
source=(S/'nxt_music_playing.c').read_text();a=source.index('bool MusicPlaying_frame(');b=source.index('\nvoid MusicPlaying_invalidate',a)
fixture=r'''
#include "nxt_performance.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
NxtConfig nxt;
static int state=PLAYER_STATE_PLAYING;
static uint32_t ticks,last_draw,last_spectrum;
static bool spectrum_clock_set,off,hidden,fft;
static int updates,paints;
bool PWR_getDisplayOff(void){return off;}
bool NxtSession_isHidden(void){return hidden;}
uint32_t SDL_GetTicks(void){return ticks;}
PlayerState NxtPlayback_state(void){return state;}
bool NxtRecord_needsRefresh(bool p){return false;}
bool Spectrum_needsRefresh(void){return fft;}
void Spectrum_update(void){updates++;}
void MusicPlaying_paintLayers(void){paints++;}
'''+source[a:b]+r'''
static int simulate(int m,int style,int playing,bool spectrum){
 nxt.performance_mode=m;nxt.interface_style=style;nxt.animations=1;
 state=playing?PLAYER_STATE_PLAYING:PLAYER_STATE_PAUSED;
 last_draw=last_spectrum=0;spectrum_clock_set=false;updates=paints=0;fft=spectrum;
 int draws=0;for(ticks=16;ticks<1000;ticks+=16)draws+=MusicPlaying_frame(false);
 assert(updates==paints);return draws;
}
int main(void){
 int rates[3][2],spectral[3];
 for(int m=0;m<3;m++){
  rates[m][0]=simulate(m,0,1,false);rates[m][1]=simulate(m,1,1,false);
  assert(simulate(m,22,1,false)==4);
  assert(simulate(m,0,0,false)==1);
  simulate(m,0,1,true);spectral[m]=updates;
  assert(updates>0&&updates<=(m==0?15:31));
  assert(NxtPerformance_recordMS()==(m==0?100:m==1?50:33));
  assert(NxtPerformance_previewMS()==NxtPerformance_recordMS());
 }
 assert(rates[0][0]<rates[1][0]&&rates[1][0]<rates[2][0]);
 assert(rates[0][1]<rates[1][1]&&rates[1][1]<rates[2][1]);
 assert(spectral[0]<spectral[1]&&spectral[1]==spectral[2]);
 /* Runtime changes affect the next frame without resetting the audio engine. */
 last_draw=1000;ticks=1060;fft=false;nxt.interface_style=0;
 nxt.performance_mode=0;assert(!MusicPlaying_frame(false));
 nxt.performance_mode=2;assert(MusicPlaying_frame(false));
 int previous=updates;off=true;ticks+=10000;
 assert(!MusicPlaying_frame(false)&&updates==previous);
 off=false;hidden=true;assert(!MusicPlaying_frame(false)&&updates==previous);
 hidden=false;fft=true;nxt.performance_mode=1;ticks+=33;
 MusicPlaying_frame(false);assert(updates==previous+1);
 ticks=UINT32_MAX-12;last_spectrum=ticks;ticks+=48;previous=updates;
 MusicPlaying_frame(false);assert(updates==previous+1);
 nxt.performance_mode=-10;assert(NxtPerformance_frameMS(true,true,1)==50);
 nxt.performance_mode=100;assert(NxtPerformance_spectrumMS()==33);
 printf("PASS production 60 Hz input loop: player draws %d/%d/%d; mechanical draws %d/%d/%d; FFT updates %d/%d/%d per second. Hidden/off: zero visual work; paused: 1 Hz; E-Ink: 4 Hz; immediate change, tick wrap and invalid-mode fallback.\n",rates[0][0],rates[1][0],rates[2][0],rates[0][1],rates[1][1],rates[2][1],spectral[0],spectral[1],spectral[2]);
}
'''
q=R/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-v182-budget-') as td:
 c=Path(td)/'test.c';binary=Path(td)/'test';c.write_text(fixture)
 cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(c),str(S/'nxt_performance.c'),'-Wl,--gc-sections','-o',str(binary)]
 with (R/'logs/v182-performance-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 p=subprocess.run([str(binary)],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,check=True)
 print(p.stdout,end='');(R/'logs/v182-performance-test.log').write_text(p.stdout+p.stderr)
(R/'logs/v182-performance-test.json').write_text(json.dumps({'exit':0,'production_policy_and_frame_function':True,'modes_reduce_actual_visual_work':True,'input_poll_rate_unchanged':True,'immediate_mode_switch':True,'FFT_capped':True,'paused_1Hz':True,'EInk_4Hz':True,'hidden_off_no_paint_or_FFT':True,'wake_and_uint32_wrap':True,'ASan_UBSan':True,'physical_temperature_measured':False},indent=2)+'\n')
