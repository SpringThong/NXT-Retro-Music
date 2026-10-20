/* SPDX-License-Identifier: GPL-3.0-only
 * NXT legacy API -> the installed NX Redux firmware API. Never map, resize,
 * migrate or copy the firmware's private SharedSettings/msettings.bin layout.
 * Use a distinct library name so dlopen cannot resolve to this adapter. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "msettings.h"
#include <alsa/asoundlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define GETTERS(X) \
 X(GetBrightness,GetBrightness,2) \
 X(GetColortemp,GetColortemp,20) \
 X(GetContrast,GetContrast,0) \
 X(GetSaturation,GetSaturation,0) \
 X(GetExposure,GetExposure,0) \
 X(GetDisplayCalEnabled,GetDisplayCalEnabled,0) \
 X(GetDisplayCalRedGain,GetDisplayCalRedGain,100) \
 X(GetDisplayCalGreenGain,GetDisplayCalGreenGain,100) \
 X(GetDisplayCalBlueGain,GetDisplayCalBlueGain,100) \
 X(GetVolume,GetVolume,8) \
 X(GetJack,GetJack,0) \
 X(GetAudioSink,GetAudioSink,0) \
 X(GetHDMI,GetHDMI,0) \
 X(GetMute,GetFnMode,0) \
 X(GetMusicVolume,GetMusicVolume,20) \
 X(GetSpeakerMute,GetSpeakerMute,0) \
 X(GetMutedBrightness,GetFnBrightness,-69) \
 X(GetMutedColortemp,GetFnColortemp,-69) \
 X(GetMutedContrast,GetFnContrast,-69) \
 X(GetMutedSaturation,GetFnSaturation,-69) \
 X(GetMutedExposure,GetFnExposure,-69) \
 X(GetMutedVolume,GetFnVolume,-69) \
 X(GetMuteDisablesDpad,GetFnDpadDisabled,0) \
 X(GetMuteEmulatesJoystick,GetFnDpadJoystick,0) \
 X(GetMuteTurboA,GetFnTurboA,0) \
 X(GetMuteTurboB,GetFnTurboB,0) \
 X(GetMuteTurboX,GetFnTurboX,0) \
 X(GetMuteTurboY,GetFnTurboY,0) \
 X(GetMuteTurboL1,GetFnTurboL1,0) \
 X(GetMuteTurboL2,GetFnTurboL2,0) \
 X(GetMuteTurboR1,GetFnTurboR1,0) \
 X(GetMuteTurboR2,GetFnTurboR2,0)
#define SETTERS(X) \
 X(SetRawBrightness,SetRawBrightness) \
 X(SetRawColortemp,SetRawColortemp) \
 X(SetRawContrast,SetRawContrast) \
 X(SetRawSaturation,SetRawSaturation) \
 X(SetRawExposure,SetRawExposure) \
 X(SetBrightness,SetBrightness) \
 X(SetColortemp,SetColortemp) \
 X(SetContrast,SetContrast) \
 X(SetSaturation,SetSaturation) \
 X(SetExposure,SetExposure) \
 X(SetDisplayCalEnabled,SetDisplayCalEnabled) \
 X(SetDisplayCalRedGain,SetDisplayCalRedGain) \
 X(SetDisplayCalGreenGain,SetDisplayCalGreenGain) \
 X(SetDisplayCalBlueGain,SetDisplayCalBlueGain) \
 X(SetJack,SetJack) \
 X(SetAudioSink,SetAudioSink) \
 X(SetMusicVolume,SetMusicVolume) \
 X(SetHDMI,SetHDMI) \
 X(SetMute,SetFnMode) \
 X(SetMutedBrightness,SetFnBrightness) \
 X(SetMutedColortemp,SetFnColortemp) \
 X(SetMutedContrast,SetFnContrast) \
 X(SetMutedSaturation,SetFnSaturation) \
 X(SetMutedExposure,SetFnExposure) \
 X(SetMutedVolume,SetFnVolume) \
 X(SetMuteDisablesDpad,SetFnDpadDisabled) \
 X(SetMuteEmulatesJoystick,SetFnDpadJoystick) \
 X(SetMuteTurboA,SetFnTurboA) \
 X(SetMuteTurboB,SetFnTurboB) \
 X(SetMuteTurboX,SetFnTurboX) \
 X(SetMuteTurboY,SetFnTurboY) \
 X(SetMuteTurboL1,SetFnTurboL1) \
 X(SetMuteTurboL2,SetFnTurboL2) \
 X(SetMuteTurboR1,SetFnTurboR1) \
 X(SetMuteTurboR2,SetFnTurboR2)

static void *firmware;
static int initialized;
static void (*api_init)(void),(*api_quit)(void);
static void (*api_volume)(int),(*api_raw_volume)(int);
static void (*api_displaycal)(int,int,int,int);
#define DEFINE_GET(name,target,fallback) static int (*api_##name)(void); int name(void){return api_##name?api_##name():fallback;}
GETTERS(DEFINE_GET)
#define DEFINE_SET(name,target) static void (*api_##name)(int); void name(int v){if(api_##name)api_##name(v);}
SETTERS(DEFINE_SET)

static void *bind(const char *target,const char *legacy){
    void *p=dlsym(firmware,target);return p?p:dlsym(firmware,legacy);
}
void InitSettings(void){
    if(initialized)return;
    const char *path=getenv("NXT_NXREDUX_SETTINGS_LIB");char resolved[PATH_MAX];
    if(!path||!*path){const char *system=getenv("SYSTEM_PATH");int n=snprintf(resolved,sizeof(resolved),"%s/lib/libmsettings.so",system?system:"");if(!system||n<0||n>=(int)sizeof(resolved)){fprintf(stderr,"NXT: NX Redux SYSTEM_PATH is unavailable\n");exit(EXIT_FAILURE);}path=resolved;}
    if(path[0]!='/'){fprintf(stderr,"NXT: NX Redux settings library must use an absolute path\n");exit(EXIT_FAILURE);}
    firmware=dlopen(path,RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND);
    if(!firmware){fprintf(stderr,"NXT: cannot load NX Redux settings API: %s\n",dlerror());exit(EXIT_FAILURE);}
#define BIND_GET(name,target,fallback) api_##name=bind(#target,#name);
    GETTERS(BIND_GET)
#define BIND_SET(name,target) api_##name=bind(#target,#name);
    SETTERS(BIND_SET)
    api_init=bind("InitSettingsNoApply","InitSettings");api_quit=dlsym(firmware,"QuitSettings");
    api_volume=dlsym(firmware,"SetVolume");api_raw_volume=dlsym(firmware,"SetRawVolume");
    api_displaycal=dlsym(firmware,"SetRawDisplayCal");
    if(!api_init||!api_quit||!api_volume||!api_raw_volume||!api_GetVolume||!api_GetAudioSink||!api_SetAudioSink||!api_GetBrightness||!api_SetBrightness){fprintf(stderr,"NXT: unsupported NX Redux settings API\n");exit(EXIT_FAILURE);}
    api_init();initialized=1;
}
int InitializedSettings(void){return initialized;}
void QuitSettings(void){if(initialized){initialized=0;api_quit();}}
void SetRawDisplayCal(int on,int red,int green,int blue){if(api_displaycal)api_displaycal(on,red,green,blue);}

/* Volume-only updates in some NX Redux releases leave the BlueALSA switch
 * muted. Repair just the current A2DP control, respecting master/FN/OSD mute.
 * This runs only on volume/output actions, never in the PCM callback. */
static void bluetooth_switch(void){
    if(!initialized||GetAudioSink()!=AUDIO_SINK_BLUETOOTH)return;
    snd_mixer_t *m=NULL;if(snd_mixer_open(&m,0)<0)return;
    if(snd_mixer_attach(m,"default")>=0&&snd_mixer_selem_register(m,NULL,NULL)>=0&&snd_mixer_load(m)>=0){
        for(snd_mixer_elem_t *e=snd_mixer_first_elem(m);e;e=snd_mixer_elem_next(e)){
            const char *name=snd_mixer_selem_get_name(e);
            if(name&&strstr(name,"A2DP")&&snd_mixer_selem_has_playback_switch(e))snd_mixer_selem_set_playback_switch_all(e,GetVolume()>0&&!GetSpeakerMute());
        }
    }
    snd_mixer_close(m);
}
void SetVolume(int value){if(value<0)value=0;if(value>20)value=20;if(api_volume)api_volume(value);bluetooth_switch();}
void SetRawVolume(int value){if(value<0)value=0;if(value>100)value=100;if(api_raw_volume)api_raw_volume(value);bluetooth_switch();}
