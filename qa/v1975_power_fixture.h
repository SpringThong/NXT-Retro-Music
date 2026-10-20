/* The production power functions are appended by test_v1975_screenoff.py.
 * Physical button, backlight and clock operations are deterministic fixtures. */
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "defines.h"
#include "api.h"
#include "msettings.h"
static struct {int display_only,display_off,display_guard,display_consumed,screen_off_controls;
 int requested_sleep,requested_wake,resume_tick,can_sleep;
 SDL_atomic_t is_charging;} pwr;
static Uint32 ticks=10000;static int down,just,up,repeat,lit=1,backlight_calls,poweroff_calls,sleep_calls;
Uint32 SDL_GetTicks(void){return ticks;}
int PAD_anyPressed(void){return !!down;}
int PAD_isPressed(int k){return !!(down&k);}
int PAD_justPressed(int k){return !!(just&k);}
int PAD_justReleased(int k){return !!(up&k);}
int PAD_justRepeated(int k){return !!(repeat&k);}
void PLAT_enableBacklight(int v){lit=v;backlight_calls++;}
uint32_t CFG_getScreenTimeoutSecs(void){return 60;}
int CFG_getScreenTimeout(void){return 0;}
int InitializedSettings(void){return 0;}
int GetMute(void){return 0;}
int PWR_preventAutosleep(void){return 0;}
void PWR_powerOff(int r){poweroff_calls++;}
void PWR_sleep(void){sleep_calls++;}
void LEDS_applyRules(void){}
void LOG_note(int n,const char*s,...){}
static int no_system(const char *s){assert(!"unexpected power off/system command");return 0;}
#define system no_system
