from pathlib import Path
import subprocess,tempfile,json,os
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';H=R/'.cache/nxredux-build/headers'
with tempfile.TemporaryDirectory(prefix='nxt-1891-led-') as td:
 w=Path(td);led=w/'led';led.mkdir();hdr=w/'include';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2')
 for name in ['max_scale','max_scale_lr','max_scale_f1f2','max_scale_rear']:(led/name).write_text('20')
 for name in ['m','lr','f1','f2','rear']:
  (led/('effect_rgb_hex_'+name)).write_text('88FF88 ');(led/('effect_'+name)).write_text('1')
 (led/'effect_enable').write_text('1');(led/'anim_frames_enable').write_text('1');(led/'effect_names').write_text((R/'qa/fixtures/brick-pro-stock-led-effects.txt').read_text())
 hw=(S/'nxt_hardware.c').read_text();decl=hw[hw.index('static uint32_t next_probe'):hw.index('static void identify')];tick=hw[hw.index('led_update:')+len('led_update:'):hw.index('void NxtHardware_release')];release=hw[hw.index('void NxtHardware_release'):hw.index('static void diagnostic_attr')]
 c=w/'led.c';c.write_text('''#include "nxt.h"
#include "nxt_led.h"
#include "player.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
NxtConfig nxt;
PlayerState Player_getState(void){return PLAYER_STATE_PLAYING;}
bool NxtSpotify_isPlaying(void){return false;}bool NxtSoundCloud_isPlaying(void){return false;}
float NxtDSP_peak(void){return .5f;}float NxtBit_peak(void){return .5f;}
void NxtLog_step(const char*c,const char*s,bool ok,const char*fmt,...){}
const char *Nxt_text(const char*a,const char*b){return a;}
'''+decl+'\nvoid led_tick(uint32_t now){'+tick+release+'''
static unsigned value(const char *name,int hex){char p[512];snprintf(p,sizeof(p),"%s/%s",NXT_LED_ROOT,name);FILE*f=fopen(p,"r");assert(f);unsigned v;assert(fscanf(f,hex?"%x":"%u",&v)==1);fclose(f);return v;}
int main(void){nxt.led=1;nxt.led_strength=60;nxt.led_rgb2=0xffa4c9;
 for(int i=0;i<16;i++){nxt.led_color=i;led_tick(100+i*60);assert(value("effect_rgb_hex_m",1)==NxtLED_rgb());assert(value("effect_m",0)==4&&value("anim_frames_enable",0)==0);}
 FILE*f=fopen(NXT_LED_ROOT "/anim_frames_enable","w");fputs("1",f);fclose(f);led_tick(1200);assert(value("anim_frames_enable",0)==0);
 nxt.led_color=16;nxt.led_rgb=0x123456;led_tick(1300);assert(value("effect_rgb_hex_m",1)==0x123456);
 nxt.led_effect=1;led_tick(1400);assert(value("effect_rgb_hex_lr",1)==0x123456&&value("effect_rgb_hex_rear",1)==0xffa4c9);
 nxt.led_effect=2;led_tick(1500);unsigned a=value("effect_rgb_hex_m",1);led_tick(1800);assert(value("effect_rgb_hex_m",1)!=a);
 NxtHardware_release();assert(value("effect_rgb_hex_m",1)==0x88ff88&&value("effect_m",0)==1&&value("anim_frames_enable",0)==1&&value("max_scale",0)==20);
 puts("PASS live LED: static4, 16 presets, custom RGB, gradient/rainbow, native animation reclaim and restore.");}
''')
 incs=[hdr,H,S,S/'include',SDK/'usr/include/SDL2',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']
 b=w/'led-test';cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NXREDUX','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-DNXT_LED_ROOT="'+str(led)+'"',*['-I'+str(p) for p in incs],str(c),str(S/'nxt_led.c'),'-Wl,--gc-sections','-lm','-o',str(b)]
 with (R/'logs/v1891-led-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(b)],capture_output=True,text=True,timeout=20,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1'});(R/'logs/v1891-led-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);p.check_returncode()
(R/'logs/v1891-led-test.json').write_text(json.dumps({'exit':0,'production_LED_loop':True,'ASan_UBSan':True,'driver_fixture_static4':True,'live_16_colors_gradient_rainbow':True,'native_frame_animation_disabled':True,'reclaim_and_restore':True,'physical_device_tested':False},indent=2)+'\n')
