/* Actual global/key-mapped/HID input paths; ROM button/mixer are fixtures. */
#include "module_common.c"
#include "nxt_controls.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
NxtConfig nxt;
const int Nxt_keyIds[16]={BTN_UP,BTN_DOWN,BTN_LEFT,BTN_RIGHT,BTN_A,BTN_B,BTN_X,BTN_Y,BTN_L1,BTN_R1,BTN_L2,BTN_R2,BTN_START,BTN_SELECT,BTN_PLUS,BTN_MINUS};
static int pressed,repeated,vol=85,light=5,volume_calls,consumed;static char message[256];
int PAD_isPressed(int mask){return (pressed&mask)!=0;}int PAD_justRepeated(int mask){return (repeated&mask)!=0;}int PAD_justPressed(int mask){return 0;}int PAD_justReleased(int mask){return 0;}
int PWR_getDisplayOff(void){return 0;}int PWR_displayInputConsumed(void){return 0;}
bool Nxt_isLocked(void){return false;}bool Nxt_selectLongUsed(void){return false;}void Nxt_selectConsume(void){consumed++;}
int NxtPad_directionMask(int mask){return mask;}
int GetVolume(void){return vol;}int GetBrightness(void){return light;}void SetBrightness(int v){light=v;}
int NxtVolume_getPercent(void){return vol;}void NxtVolume_setPercent(int v){vol=v;volume_calls++;}
const char *Nxt_text(const char*a,const char*b){return a;}
ToastToken Toast_show(const char*s,uint32_t ms){snprintf(message,sizeof(message),"%s",s);return 0;}
Uint32 SDL_GetTicks(void){return 123;}
void LOG_note(int n,const char*s,...){}int GetAudioSink(void){return 2;}
bool Nxt_save(void){return true;}void Nxt_setShuffle(bool v){}void Nxt_setRepeat(int v){}void Nxt_stop(void){}
void PWR_setDisplayOff(int off){}
int main(void){
 nxt.bit_perfect=1;pressed=BTN_SELECT|BTN_UP;repeated=BTN_UP;int show=0;
 assert(nxt_adjust_levels(&show)&&vol==86&&volume_calls==1&&consumed==1&&show==2&&!strstr(message,"DAC"));
 pressed=0;repeated=BTN_MINUS;assert(nxt_adjust_levels(&show)&&vol==85&&volume_calls==2&&!strstr(message,"DAC"));
 assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP)&&vol==86&&volume_calls==3);assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_DOWN)&&vol==85&&volume_calls==4);
 for(int i=0;i<16;i++)nxt.playing_keys[i]=NC_NONE;nxt.playing_keys[0]=NC_VOLUME_UP;repeated=BTN_UP;assert(NxtControl_aux()&&vol==86&&volume_calls==5&&!strstr(message,"DAC"));
 nxt.playing_keys[0]=NC_VOLUME_DOWN;assert(NxtControl_aux()&&vol==85&&volume_calls==6&&!strstr(message,"DAC"));
 puts("PASS actual input paths while raw mode active: SELECT+direction, PLUS/MINUS, DAC HID and mapped volume keys reach volume controller; no forced-DAC warning or blocked key.");
}
