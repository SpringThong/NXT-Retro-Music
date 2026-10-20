/* Real app-level routing and mapped controls, hardware buttons are fixtures. */
#include "nxt.h"
#include "api.h"
#include "nxt_pad.c"
#include "nxt_controls.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
NxtConfig nxt;static int down,just,up,repeat,off,consumed,vol=85,light=5;static bool locked,select_held,select_long;static Uint32 select_started,ticks;
int PAD_isPressed(int k){return !!(down&k);}int PAD_justPressed(int k){return !!(just&k);}int PAD_justReleased(int k){return !!(up&k);}int PAD_justRepeated(int k){return !!(repeat&k);}
int PWR_getDisplayOff(void){return off;}int PWR_displayInputConsumed(void){return consumed;}
bool Nxt_isLocked(void){return locked;}bool ModuleCommon_stopSignalled(void){return false;}
bool Nxt_selectLongUsed(void){return select_long;}void Nxt_selectConsume(void){select_long=true;}
Uint32 SDL_GetTicks(void){return ticks;}
ToastToken Toast_show(const char *s,uint32_t ms){return 0;}
const char *Nxt_text(const char *a,const char*b){return a;}
int GetBrightness(void){return light;}void SetBrightness(int v){light=v;}
int NxtVolume_getPercent(void){return vol;}void NxtVolume_setPercent(int v){vol=v;}
bool Nxt_save(void){return true;}void Nxt_setShuffle(bool v){}void Nxt_setRepeat(int v){}void Nxt_stop(void){}
void PWR_setDisplayOff(int v){off=v;}void Nxt_screenOff(void){off=1;}void ModuleCommon_requestQuit(void){}
/* Actual Nxt_lockInput is inserted below by the runner. */

static int vocal_toggles;bool NxtVocal_toggle(void){vocal_toggles++;return true;}
