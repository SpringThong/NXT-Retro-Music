#!/usr/bin/env python3
"""Exercise production Stock shortcuts and both analogue axes with SDL events."""
from pathlib import Path
import subprocess,tempfile,json
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';api=(R/'upstream/NextUI/workspace/all/common/api.c').read_text()
Q=R/'.cache/home-dac';inc=Q/'include'
incs=['-I'+str(inc),'-I'+str(S),'-I'+str(S/'include'),'-I'+str(R/'upstream/NextUI/workspace/all/common'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings')]
flags=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-ffunction-sections','-fdata-sections',*incs]
with tempfile.TemporaryDirectory(prefix='nxt-stock-input-') as directory:
 w=Path(directory)
 controls=w/'controls.c';controls.write_text(r'''
#include "module_common.c"
#include <assert.h>
NxtConfig nxt;
static int pressed,repeated,volume=10,brightness=5,consumed;static bool locked;
int PAD_isPressed(int b){return pressed&b;}int PAD_justRepeated(int b){return repeated&b;}
bool Nxt_isLocked(void){return locked;}void Nxt_selectConsume(void){consumed++;}bool Nxt_selectLongUsed(void){return consumed>0;}
int GetVolume(void){return volume;}void SetVolume(int v){volume=v;}
int GetBrightness(void){return brightness;}void SetBrightness(int v){brightness=v;}
ToastToken Toast_show(const char *t,uint32_t d){return 0;}
const char *Nxt_text(const char *a,const char *b){return a;}
Uint32 SDL_GetTicks(void){return 1234;}
int PWR_getDisplayOff(void){return 0;}int PWR_displayInputConsumed(void){return 0;}
int main(void){int show=0;const int keys[]={BTN_UP,BTN_DOWN,BTN_LEFT,BTN_RIGHT,BTN_ANALOG_UP,BTN_ANALOG_DOWN,BTN_ANALOG_LEFT,BTN_ANALOG_RIGHT};
 for(int i=0;i<8;i++){volume=10;brightness=5;consumed=0;pressed=BTN_SELECT|keys[i];repeated=keys[i];assert(nxt_adjust_levels(&show));assert(consumed==1);assert(volume==10+(i%4==0)-(i%4==1));assert(brightness==5+(i%4==3)-(i%4==2));}
 for(int i=0;i<8;i++){pressed=repeated=keys[i];consumed=0;int v=volume,b=brightness;assert(!nxt_adjust_levels(&show));assert(volume==v&&brightness==b);}
 volume=20;pressed=BTN_SELECT|BTN_UP;repeated=BTN_UP;nxt_adjust_levels(&show);assert(volume==20);volume=0;repeated=BTN_DOWN;nxt_adjust_levels(&show);assert(volume==0);
 brightness=10;repeated=BTN_RIGHT;nxt_adjust_levels(&show);assert(brightness==10);brightness=0;repeated=BTN_LEFT;nxt_adjust_levels(&show);assert(brightness==0);
 locked=true;volume=10;repeated=BTN_UP;assert(!nxt_adjust_levels(&show)&&volume==10);puts("PASS SELECT + D-pad/analogue directions: matching volume/brightness, boundaries, lock, consumed SELECT and ordinary navigation retained.");return 0;}
''')
 subprocess.run(flags+[str(controls),str(S/'nxt_pad.c'),'-Wl,--gc-sections','-o',str(w/'controls')],check=True);subprocess.run([str(w/'controls')],check=True)
 analog=api.split('void PAD_setAnalog(int neg_id',1)[1].split('\nvoid PAD_reset',1)[0]
 poll=api.split('FALLBACK_IMPLEMENTATION void PLAT_pollInput(void)',1)[1].split('FALLBACK_IMPLEMENTATION int PLAT_shouldWake',1)[0]
 source=w/'axes.c';source.write_text(r'''
#include "defines.h"
#include "api.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
PAD_Context pad;LID_Context lid;int is_brick=0,is_brickpro=1;
static SDL_Event next;static bool ready;
void LOG_note(int level,const char *fmt,...){}
Uint32 SDL_GetTicks(void){return 100;}
int SDL_PollEvent(SDL_Event *event){if(!ready)return 0;ready=false;*event=next;return 1;}
void PAD_update(const SDL_Event *e){}void PWR_powerOff(int e){}int PLAT_lidChanged(int *s){return 0;}
#define AXIS_DEADZONE 0x4000
void PAD_setAnalog(int neg_id'''+analog+'\nvoid PLAT_pollInput(void)'+poll+r'''
static void axis(int a,int value){next=(SDL_Event){0};next.type=SDL_JOYAXISMOTION;next.jaxis.axis=a;next.jaxis.value=value;ready=true;PLAT_pollInput();}
int main(void){axis(AXIS_LX,-30000);assert(pad.is_pressed&BTN_ANALOG_LEFT);axis(AXIS_RX,0);assert(pad.is_pressed&BTN_ANALOG_LEFT);axis(AXIS_LX,0);assert(!(pad.is_pressed&BTN_ANALOG_LEFT));axis(AXIS_RX,30000);assert(pad.is_pressed&BTN_ANALOG_RIGHT);axis(AXIS_LX,0);assert(pad.is_pressed&BTN_ANALOG_RIGHT);axis(AXIS_RX,0);assert(!(pad.is_pressed&BTN_ANALOG_RIGHT));axis(AXIS_LY,-30000);assert(pad.is_pressed&BTN_ANALOG_UP);axis(AXIS_RY,0);assert(pad.is_pressed&BTN_ANALOG_UP);axis(AXIS_LY,0);axis(AXIS_RY,30000);assert(pad.is_pressed&BTN_ANALOG_DOWN);puts("PASS real SDL poll: both sticks and idle-stick events preserve the other stick's direction.");return 0;}
''')
 subprocess.run(flags+[str(source),'-Wl,--gc-sections','-o',str(w/'axes')],check=True);subprocess.run([str(w/'axes')],check=True)
(R/'logs/stock-input-test.json').write_text(json.dumps({'shortcuts':{'exit':0},'both_sticks':{'exit':0}},indent=2)+'\n')
