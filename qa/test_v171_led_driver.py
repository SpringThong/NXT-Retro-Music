#!/usr/bin/env python3
"""Exercise production stock DAC volume/routing; no physical-device claim."""
from pathlib import Path
import json
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "upstream/src"
SDK = ROOT / ".nxt-sdk/tg5040/compiler"
SYSROOT = SDK / "aarch64-nextui-linux-gnu/libc"
SETTINGS = ROOT / "upstream/NextUI/workspace/tg5040/libmsettings"
COMMON = ROOT / "upstream/NextUI/workspace/all/common"
PLATFORM = ROOT / "upstream/NextUI/workspace/tg5040/platform"
checks = {}

def run(name, command, **kwargs):
    result = subprocess.run([str(v) for v in command], check=False, text=True,
                            capture_output=True, timeout=40, **kwargs)
    if result.returncode:
        raise RuntimeError(name + "\n" + result.stdout + result.stderr)
    checks[name] = {"exit": result.returncode, "stdout": result.stdout}
    print(result.stdout.strip(), flush=True)

with tempfile.TemporaryDirectory(prefix="nxt-stock-dac-") as directory:
    work = Path(directory)
    inc = work / "include"
    inc.mkdir()
    for name in ("SDL2", "tinyalsa", "GLES2", "EGL", "KHR"):
        (inc / name).symlink_to(SYSROOT / "usr/include" / name, target_is_directory=True)
    (inc / "samplerate.h").symlink_to(ROOT / ".cache/stock-build/headers/samplerate/samplerate.h")
    flags = ["gcc", "-O2", "-D_GNU_SOURCE", "-DNXT_ROM_STOCK", "-DUSE_SDL2",
             "-ffunction-sections", "-fdata-sections", "-Werror=implicit-function-declaration",
             "-I" + str(inc), "-I" + str(SRC), "-I" + str(SRC / "include"),
             "-I" + str(COMMON), "-I" + str(PLATFORM), "-I" + str(SETTINGS)]

    volume = work / "volume.c"
    volume.write_text(r'''
#include <assert.h>
#include <stdarg.h>
static void qa_sync(void) {}
#define sync qa_sync
#include "msettings.c"
#undef sync
#include "nxt_player_core.c"
#include "nxt_config.c"
void NxtMonitor_tick(void){}
static int mixer_calls, opens, resets, fail_opens;
static bool device_open;static Uint32 qa_ticks=100;
int snd_config_update_free_global(void){assert(!device_open);resets++;return 0;}
const BoolSetting SETTING_AUTO_UPDATE={0};
void Settings_setBool(const BoolSetting *key,bool value) {}
int userdata_snpath(const char *rel,char *out,size_t size){return snprintf(out,size,"%s/%s",getenv("QA_DATA"),rel);}
void Nxt_profileSave(int sink) {}
static bool hidden;
void LOG_note(int level, const char *format, ...) {}
void NxtPersonal_init(void){}void NxtPersonal_tick(void){}void NxtPersonal_flush(void){}void NxtPersonal_savePreferences(void){}
void NxtOTA_init(void){}void NxtOTA_tick(void){}
void NxtLog_step(const char *c,const char *s,bool ok,const char *fmt,...){}

struct mixer *mixer_open(unsigned int card) { mixer_calls++; return NULL; }
void mixer_close(struct mixer *m) {}
unsigned int mixer_get_num_ctls(const struct mixer *m) { return 0; }
struct mixer_ctl *mixer_get_ctl(struct mixer *m, unsigned int id) { return NULL; }
struct mixer_ctl *mixer_get_ctl_by_name(struct mixer *m, const char *name) { return NULL; }
const char *mixer_ctl_get_name(const struct mixer_ctl *c) { return NULL; }
enum mixer_ctl_type mixer_ctl_get_type(const struct mixer_ctl *c) { return MIXER_CTL_TYPE_INT; }
int mixer_ctl_get_range_min(const struct mixer_ctl *c) { return 0; }
int mixer_ctl_get_range_max(const struct mixer_ctl *c) { return 100; }
unsigned int mixer_ctl_get_num_values(const struct mixer_ctl *c) { return 0; }
int mixer_ctl_set_value(struct mixer_ctl *c, unsigned int id, int value) { return 0; }
int mixer_ctl_set_percent(struct mixer_ctl *c, unsigned int id, int value) { return 0; }
bool Radio_isActive(void) { return true; }
int Radio_getAudioSamples(int16_t *pcm, int count) { for(int i=0;i<count;i++)pcm[i]=16384; return count; }
int NxtSoundCloud_audioSamples(float *pcm, int count, int rate) { return 0; }
int NxtSpotify_audioSamples(float *pcm, int count, int rate) { return 0; }
void NxtDSP_process(float *pcm, int count, float gain) {}
void NxtDecoder_cancelStreams(void) {}
void NxtDSP_configure(const NxtConfig *config, int rate) {}
void NxtSpotify_tick(void) {}
void NxtSoundCloud_tick(void) {}
void NxtSpotify_quit(void) {}
void NxtSoundCloud_quit(void) {}
void NxtSession_tick(void) {}
bool NxtSession_isHidden(void) { return hidden; }
bool NxtStock_panelActive(void) { return false; }
void NxtHardware_tick(void) {}
void PlayerModule_syncQueue(void) {}
void PlayerModule_backgroundTick(void) {}
void PlayerModule_markStopped(void) {}
int PWR_getDisplayOff(void) { return 0; }
BackgroundPlayerType Background_getActive(void) { return BG_NONE; }
void Background_setActive(BackgroundPlayerType type) {}
void Background_stopAll(void) {}
bool Background_isPlaying(void) { return true; }
void PWR_enableAutosleep(void) {}
void PWR_disableAutosleep(void) {}
bool Podcast_isActive(void) { return false; }
void Podcast_stop(void) {}
void Podcast_flushProgress(void) {}
void Radio_stop(void) {}
void Resume_updatePosition(int position) {}
SDL_Surface *NxtDecoder_art(const char *path) { return NULL; }
char *NxtDecoder_lyrics(const char *path) { return NULL; }
void SDL_FreeSurface(SDL_Surface *surface) {}
int src_reset(SRC_STATE *state) { return 0; }
int src_process(SRC_STATE *state, SRC_DATA *data) { return -1; }
int SDL_setenv(const char *name, const char *value, int overwrite) { return setenv(name,value,overwrite); }
SDL_AudioDeviceID SDL_OpenAudioDevice(const char *name,int capture,const SDL_AudioSpec *want,SDL_AudioSpec *have,int changes) {
    assert(!name && !capture); assert(!strcmp(getenv("AUDIODEV"),"nxt_music_output"));
    *have=*want; opens++;assert(resets>0);if(fail_opens>0){fail_opens--;return 0;}device_open=true;return 2;
}
void SDL_PauseAudioDevice(SDL_AudioDeviceID device,int pause) {}
void SDL_CloseAudioDevice(SDL_AudioDeviceID device) {assert(device_open);device_open=false;}
Uint32 SDL_GetTicks(void) { return qa_ticks; }
const char *SDL_GetError(void) { return "QA"; }
static void expect_gain(float wanted) {
    float pcm[32]; output_audio(NULL,(Uint8*)pcm,sizeof(pcm));
    for(int i=0;i<32;i++)assert(fabsf(pcm[i]-wanted)<.00001f);
}
int main(int argc,char **argv) {
    static Settings current;
    settings=&current; current.audiosink=AUDIO_SINK_USBDAC;
    snprintf(SettingsPath,sizeof(SettingsPath),"%s/volume.bin",argv[1]);
    e.volume=1; e.rate=e.radio_rate=48000; e.format=AUDIO_F32SYS; e.sink=2;
    SetVolume(8); expect_gain(.2f);
    for(int i=0;i<100;i++) {
        assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP));
        assert(GetVolume()==9 && GetAudioSink()==2); expect_gain(.225f);
        assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_DOWN));
        assert(GetVolume()==8 && GetAudioSink()==2); expect_gain(.2f);
    }
    SetVolume(20); ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP);
    assert(GetVolume()==20); expect_gain(.5f);
    SetVolume(0); ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_DOWN);
    assert(GetVolume()==0); expect_gain(0);
    assert(!ModuleCommon_handleHIDVolume(USB_HID_EVENT_NEXT_TRACK));
    SetRawVolume(-10); assert(GetUSBDACSoftwareVolume()==0);
    SetRawVolume(110); assert(GetUSBDACSoftwareVolume()==100);
    assert(mixer_calls==0 && opens==0);
    assert(!open_audio() && opens==1);
    int remote[2];assert(!pipe(remote));usb_hid_fd=remote[0];assert(!fcntl(usb_hid_fd,F_SETFL,O_NONBLOCK));
    SetVolume(8);hidden=true;
    struct input_event_raw event={.type=EV_KEY,.code=KEY_VOLUMEUP,.value=1};
    assert(write(remote[1],&event,sizeof(event))==sizeof(event));Nxt_tick();
    assert(GetVolume()==9 && GetAudioSink()==2 && opens==1);expect_gain(.225f);
    hidden=false;event.code=KEY_VOLUMEDOWN;
    assert(write(remote[1],&event,sizeof(event))==sizeof(event));Nxt_tick();assert(GetVolume()==9);
    assert(ModuleCommon_handleHIDVolume(Player_pollUSBHID()));assert(GetVolume()==8);
    Player_quitUSBHID();close(remote[1]);
    current.audiosink=AUDIO_SINK_DEFAULT; e.sink=0;
    SetVolume(10); assert(mixer_calls==1); expect_gain(.5f);
    e.state=PLAYER_STATE_PLAYING;current.audiosink=AUDIO_SINK_USBDAC;Player_requestOutputRefresh();Player_handleAudioSinkChange();assert(e.device&&e.sink==2&&e.state==PLAYER_STATE_PLAYING&&!sink_changed);
    current.audiosink=AUDIO_SINK_DEFAULT;Player_requestOutputRefresh();Player_handleAudioSinkChange();assert(e.device&&e.sink==0&&e.state==PLAYER_STATE_PLAYING&&!sink_changed);
    current.audiosink=AUDIO_SINK_USBDAC;fail_opens=2;Player_requestOutputRefresh();Player_handleAudioSinkChange();assert(!e.device&&sink_changed);int before=opens;
    qa_ticks+=200;Player_handleAudioSinkChange();assert(opens==before&&!e.device);
    qa_ticks+=101;Player_handleAudioSinkChange();assert(e.device&&e.sink==2&&!sink_changed&&e.state==PLAYER_STATE_PLAYING);
    assert(resets==5);
    /* A driver that becomes ready after the old five-second limit must still
     * recover, with slow polling rather than a reopen on every frame. */
    fail_opens=64;Player_requestOutputRefresh();Player_handleAudioSinkChange();assert(!Player_outputReady()&&Player_outputRetryPending());
    qa_ticks+=6000;Player_handleAudioSinkChange();assert(!e.device&&sink_changed);before=opens;
    qa_ticks+=1000;Player_handleAudioSinkChange();assert(opens==before);
    fail_opens=0;qa_ticks+=1001;Player_handleAudioSinkChange();assert(Player_outputReady()&&!Player_outputRetryPending()&&e.state==PLAYER_STATE_PLAYING);
    puts("PASS production output controller: closes SDL before ALSA reset, speaker/DAC handoff retains playback, transient USB open failure retries and recovers, slow DAC starts after five seconds recover with bounded polling.");
    setenv("QA_DATA",argv[1],1);char config_path[1024];userdata_snpath("nxt.json",config_path,sizeof(config_path));FILE *config=fopen(config_path,"w");assert(config);
    fputs("{\"auto_dac\":0,\"unplug_pause\":1,\"theme\":2,\"font_size\":110,\"playing_keys\":[0,0,0,0,21,0,0,0,0,0,0,0,0,0,0,0]}",config);fclose(config);
    Nxt_init();assert(nxt.auto_dac==1&&nxt.unplug_pause==0&&nxt.theme==2&&nxt.font_size==110&&nxt.playing_keys[4]==21);assert(Nxt_save());
    JSON_Value *saved=json_parse_file(config_path);assert(json_object_get_boolean(json_value_get_object(saved),"stock_hotplug_fix1")==1);json_value_free(saved);
    nxt.auto_dac=0;nxt.unplug_pause=1;nxt.spotify_auth_source=1;assert(Nxt_save());Nxt_init();assert(!nxt.auto_dac&&nxt.unplug_pause==1&&nxt.playing_keys[4]==21&&nxt.spotify_auth_source==1);
    puts("PASS one-time hotplug defaults migration retains theme/font/key mappings; later manual output preferences persist.");
    puts("PASS DAC volume: 200 HID presses, foreground/background evdev input, bounds/mute, PCM gain changes, unchanged sink/device, zero DAC mixer writes, speaker path retained.");
    return 0;
}
''')
    run("compile_volume", flags + [volume, SRC / "module_common.c", SRC / "nxt_controls.c", SRC / "nxt_pad.c", SRC / "include/parson/parson.c",
        "-Wl,--gc-sections", "-lm", "-lpthread", "-o", work / "volume"])
    run("hid_pcm_volume", [work / "volume", work])

    asound = work / "asound"
    for card in (1, 3):
        path = asound / ("card" + str(card))
        path.mkdir(parents=True)
        (path / "id").write_text("BlackPearlQA\n")
        (path / "stream0").write_text("Playback:\n Format: S32_LE\n")
    (asound / "card1/usbbus").write_text("003/002\n")
    led=work/'led';led.mkdir()
    for name in ['max_scale','max_scale_lr','max_scale_f1f2','max_scale_rear']:(led/name).write_text('20')
    for name in ['m','lr','f1','f2','rear']:
        (led/('effect_rgb_hex_'+name)).write_text('88FF88 ')
        (led/('effect_'+name)).write_text('1')
    (led/'effect_enable').write_text('1')
    (led/'effect_names').write_text((ROOT/'qa/fixtures/brick-pro-stock-led-effects.txt').read_text())
    (led/'anim_frames_enable').write_text('1')
    hardware = work / "hardware.c"
    hardware.write_text(r'''
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
static FILE *effect_write,*enable_write;
static int effect_group=-1,effect_latches,disable_writes;
static unsigned latched_rgb[5];
static FILE *qa_fopen(const char *path,const char *mode){
 FILE *f=fopen(path,mode);if(!f||!strchr(mode,'w'))return f;
 static const char *groups[]={"m","lr","f1","f2","rear"};
 for(int i=0;i<5;i++){char expected[1024];snprintf(expected,sizeof(expected),NXT_LED_ROOT "/effect_%s",groups[i]);if(!strcmp(path,expected)){effect_write=f;effect_group=i;}}
 if(!strcmp(path,NXT_LED_ROOT "/effect_enable"))enable_write=f;
 return f;
}
static int qa_fclose(FILE *f){
 int group=f==effect_write?effect_group:-1;int enable=f==enable_write;
 int result=fclose(f);
 if(group>=0){effect_write=NULL;if(!result){static const char *groups[]={"m","lr","f1","f2","rear"};char path[1024];snprintf(path,sizeof(path),NXT_LED_ROOT "/effect_rgb_hex_%s",groups[group]);FILE *rgb=fopen(path,"r");if(rgb){fscanf(rgb,"%x",&latched_rgb[group]);fclose(rgb);}effect_latches++;}}
 if(enable){enable_write=NULL;FILE *value=fopen(NXT_LED_ROOT "/effect_enable","r");int state=1;if(value){fscanf(value,"%d",&state);fclose(value);}if(!state)disable_writes++;}
 return result;
}
#define fopen qa_fopen
#define fclose qa_fclose
#include "nxt_hardware.c"
#undef fopen
#undef fclose
#include <assert.h>
#include <stdarg.h>
NxtConfig nxt;
bool PLAT_btIsConnected(void){return false;}
void NxtNextUI_quit(void){}
bool NxtNextUI_audioConfig(char *out,size_t n){if(n)out[0]=0;return false;}
bool NxtNextUI_bluetoothRoute(char *out,size_t n){if(n)out[0]=0;return false;}
bool NxtNextUI_wrapRoute(const char *body,char *out,size_t n){return snprintf(out,n,"%s",body)>=0;}

static Uint32 clock_ms;
static float peak;
static int sink=2,volume=8,refreshes,pauses;
static char workspace[1024];
void LOG_note(int level,const char *format,...) {}
void NxtPersonal_init(void){}void NxtPersonal_tick(void){}void NxtPersonal_flush(void){}void NxtPersonal_savePreferences(void){}
void NxtOTA_init(void){}void NxtOTA_tick(void){}
void NxtLog_step(const char *c,const char *s,bool ok,const char *fmt,...){}

Uint32 SDL_GetTicks(void) { return clock_ms; }
int InitializedSettings(void) { return 1; }
int GetJack(void) { return 0; }
int GetAudioSink(void) { return sink; }
void SetAudioSink(int value) { sink=value; }
int GetVolume(void) { return volume; }
void SetVolume(int value) { volume=value; }
void NxtStock_tick(void) {}
static bool bt_ready;static int bt_peer;
bool NxtStock_bluetoothRoute(char *text,unsigned long size) { if(!bt_ready)return false;snprintf(text,size,"# BT fixture peer %d\n",bt_peer);return true; }
void Player_requestOutputRefresh(void) { refreshes++; }
void Player_pause(void) { pauses++; }
PlayerState Player_getState(void) { return PLAYER_STATE_PLAYING; }
int Player_getOutputRate(void) { return 48000; }
int Player_getOutputBits(void) { return 32; }
bool Player_outputReady(void) { return true; }
bool Player_outputRetryPending(void) { return false; }
bool NxtSpotify_isPlaying(void) { return false; }
bool NxtSoundCloud_isPlaying(void) { return false; }
float NxtDSP_peak(void) { return peak; }
void Nxt_profileSave(int output) {}
void Nxt_profileLoad(int output) {}
bool Nxt_save(void) { return true; }
const char *Nxt_text(const char *en,const char *vi) { return en; }
int userdata_snpath(const char *file,char *out,size_t size) { return snprintf(out,size,"%s/%s",workspace,file); }
static void card_file(int card,bool present) {
    char path[1024];snprintf(path,sizeof(path),NXT_ASOUND_ROOT "/card%d/usbbus",card);
    if(present){FILE *f=fopen(path,"w");assert(f);fputs("003/002\n",f);fclose(f);}else unlink(path);
}
static void expect_route(int card) {
    char data[8192],wanted[80];assert(readtext(config_path,data,sizeof(data)));
    snprintf(wanted,sizeof(wanted),"hw:%d,0",card);assert(strstr(data,wanted));
    assert(strstr(data,"pcm.nxt_music_output"));
}
static void tick(Uint32 time) { clock_ms=time;NxtHardware_tick(); }
int main(int argc,char **argv) {
    snprintf(workspace,sizeof(workspace),"%s",argv[1]);
    snprintf(config_path,sizeof(config_path),"%s/.asoundrc",workspace);
    nxt.output=2;nxt.unplug_pause=1;for(int i=0;i<3;i++)nxt.volume[i]=8;
    tick(100);expect_route(1);assert(sink==2);refreshes=0;
    for(int i=0;i<40;i++){ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP);ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_DOWN);}
    assert(sink==2 && refreshes==0);expect_route(1);
    card_file(1,false);tick(700);assert(sink==2 && nxt.output==2 && refreshes==0);
    ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP);assert(sink==2);
    card_file(1,true);tick(1200);assert(sink==2 && nxt.output==2 && refreshes==1 && pauses==0);expect_route(1);
    card_file(1,false);tick(1800);tick(2300);tick(2800);assert(sink==2);
    tick(3300);assert(sink==0 && nxt.output==0 && pauses==1);expect_route(0);
    nxt.auto_dac=1;card_file(3,true);tick(3800);assert(sink==2 && nxt.output==2);expect_route(3);
    assert(NxtHardware_select(0));nxt.output=0;tick(4300);assert(sink==0);expect_route(0);
    /* Fail the route rename after detecting a new card, then repair the
     * filesystem without another USB event. Selection must recover. */
    char actual[1024];snprintf(actual,sizeof(actual),"%s",config_path);snprintf(config_path,sizeof(config_path),"%s/blocked-route",workspace);assert(!mkdir(config_path,0700));
    next_probe=0;nxt.output=2;tick(4350);assert(route_pending&&!last_route_ok&&sink==0);
    assert(!rmdir(config_path));snprintf(config_path,sizeof(config_path),"%s",actual);tick(4850);assert(!route_pending&&last_route_ok&&sink==2);expect_route(3);
    nxt.led=1;nxt.led_mode=0;nxt.led_color=16;nxt.led_rgb=0x123456;nxt.led_strength=100;
    tick(4900);char raw[80];assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw)));assert(!strcmp(raw,"123456"));FILE *rgb=fopen(NXT_LED_ROOT "/effect_rgb_hex_lr","r");assert(rgb);char bytes[9]={0};assert(fread(bytes,1,9,rgb)==8&&!strcmp(bytes,"123456 \n"));fclose(rgb);
    assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));assert(latched_rgb[1]==0x123456&&disable_writes==0);int latches_before=effect_latches;
    assert(readtext(NXT_LED_ROOT "/max_scale",raw,sizeof(raw))&&!strcmp(raw,"60"));
    assert(readtext(NXT_LED_ROOT "/effect_enable",raw,sizeof(raw))&&!strcmp(raw,"1"));
    assert(readtext(NXT_LED_ROOT "/anim_frames_enable",raw,sizeof(raw))&&!strcmp(raw,"0"));
    /* Simulate Stock re-enabling its animation (e.g. after the Home panel). */
    assert(write_text(NXT_LED_ROOT "/anim_frames_enable","1"));
    nxt.led_rgb=0xabcdef;tick(5000);assert(latched_rgb[1]==0xabcdef&&effect_latches>latches_before&&disable_writes==0);assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_rear",raw,sizeof(raw))&&!strcmp(raw,"ABCDEF"));
    /* Stock overwrites RGB/effect while its animation remains disabled. */
    assert(write_text(NXT_LED_ROOT "/effect_rgb_hex_lr","000000 "));assert(write_text(NXT_LED_ROOT "/effect_lr","0"));
    tick(6100);assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"ABCDEF"));assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));
    /* A failed RGB write must not be cached as success. */
    assert(!unlink(NXT_LED_ROOT "/effect_rgb_hex_lr"));assert(!mkdir(NXT_LED_ROOT "/effect_rgb_hex_lr",0700));
    nxt.led_rgb=0x654321;tick(6200);assert(!led_config_ok&&last_rgb!=0x654321);
    assert(!rmdir(NXT_LED_ROOT "/effect_rgb_hex_lr"));assert(write_text(NXT_LED_ROOT "/effect_rgb_hex_lr","000000 "));
    tick(6500);assert(led_config_ok&&last_rgb==0x654321);assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"654321"));
    char oldcwd[1024];assert(getcwd(oldcwd,sizeof(oldcwd)));assert(!chdir(workspace));assert(NxtHardware_writeDiagnostic());
    char diagnostic[8192];assert(readtext("NXT_Device_Log.txt",diagnostic,sizeof(diagnostic)));assert(strstr(diagnostic,"Demo Layouts & Live LED")&&strstr(diagnostic,"LED static effect selected: 4")&&strstr(diagnostic,"4 - static")&&strstr(diagnostic,"RGB=654321")&&strstr(diagnostic,"LED effect_rgb_hex_lr: 654321"));assert(!strstr(diagnostic,"access_token")&&!strstr(diagnostic,"refresh_token"));assert(!chdir(oldcwd));
    /* The kernel can accept a write but expose no matching state. A /dev/null
     * endpoint reproduces that without requiring physical LED hardware. */
    assert(!unlink(NXT_LED_ROOT "/effect_rgb_hex_lr"));assert(!symlink("/dev/null",NXT_LED_ROOT "/effect_rgb_hex_lr"));
    nxt.led_rgb=0x222333;tick(6600);assert(!led_config_ok&&last_rgb==0x654321);
    assert(!unlink(NXT_LED_ROOT "/effect_rgb_hex_lr"));assert(write_text(NXT_LED_ROOT "/effect_rgb_hex_lr","88FF88 \n"));
    tick(6900);assert(led_config_ok&&last_rgb==0x222333);
    /* Another firmware's table must select its advertised static ID. */
    assert(write_text(NXT_LED_ROOT "/effect_names","TRIMUI LED Animation Effect names:\n3 - sniff\n4 - blink1\n8 - static\n"));
    tick(8000);assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"8"));
    assert(!unlink(NXT_LED_ROOT "/effect_names"));tick(9100);
    assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));
    assert(write_text(NXT_LED_ROOT "/effect_names","TRIMUI LED Animation Effect names:\n3 - sniff\n4 - static\n"));
    /* All preset colors, including the exact red request from the device log,
     * use static driver output; music changes brightness, not the selected RGB. */
    for(int i=0;i<16;i++){
        nxt.led_color=i;tick(9200+i*50);unsigned expected=NxtLED_rgb(),observed=0;
        assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&sscanf(raw,"%x",&observed)==1&&observed==expected);assert(latched_rgb[1]==expected&&disable_writes==0);
        assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));
    }
    latches_before=effect_latches;nxt.led_mode=2;tick(10000);assert(effect_latches>latches_before&&disable_writes==0);
    nxt.led_color=5;nxt.led_mode=1;nxt.led_strength=60;peak=.5f;tick(10100);
    assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"FF4040"));
    assert(readtext(NXT_LED_ROOT "/max_scale",raw,sizeof(raw))&&!strcmp(raw,"18"));
    peak=0;tick(10150);assert(readtext(NXT_LED_ROOT "/max_scale",raw,sizeof(raw))&&!strcmp(raw,"0"));
    assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"FF4040"));
    nxt.led_mode=0;nxt.led_color=16;nxt.led_rgb=0xff0000;nxt.led_rgb2=0x0000ff;nxt.led_effect=1;tick(10200);
    assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"FF0000"));
    assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_rear",raw,sizeof(raw))&&!strcmp(raw,"0000FF"));
    nxt.led_effect=2;tick(10250);assert(disable_writes==0);char rainbow_before[80];assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",rainbow_before,sizeof(rainbow_before)));tick(10400);assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&strcmp(raw,rainbow_before));nxt.led_effect=0;assert(effect_latches>20&&disable_writes==0);puts("PASS live LED driver latch: color and mode/effect writes commit new RGB while enabled; no disable/on cycle during 16 colors, gradients, mode changes and animation.");
    NxtHardware_release();
    assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"88FF88"));
    assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"1"));
    assert(readtext(NXT_LED_ROOT "/max_scale",raw,sizeof(raw))&&!strcmp(raw,"20"));
    assert(readtext(NXT_LED_ROOT "/effect_enable",raw,sizeof(raw))&&!strcmp(raw,"1"));
    assert(readtext(NXT_LED_ROOT "/anim_frames_enable",raw,sizeof(raw))&&!strcmp(raw,"1"));
    /* The supplied log has animation already off; native linear/green must
     * still be replaced and its original values restored when control ends. */
    assert(write_text(NXT_LED_ROOT "/anim_frames_enable","0"));
    nxt.led_mode=0;nxt.led_color=16;nxt.led_rgb=0x123456;tick(10500);
    assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));
    assert(write_text(NXT_LED_ROOT "/effect_rgb_hex_lr","88FF88 "));assert(write_text(NXT_LED_ROOT "/effect_lr","1"));
    tick(11550);assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"123456"));
    assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"4"));
    nxt.led=0;tick(11600);
    assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"88FF88"));
    assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"1"));
    assert(readtext(NXT_LED_ROOT "/anim_frames_enable",raw,sizeof(raw))&&!strcmp(raw,"0"));
    assert(readtext(NXT_LED_ROOT "/max_scale",raw,sizeof(raw))&&!strcmp(raw,"20"));
    bt_ready=true;nxt.auto_bt=1;tick(14000);assert(sink==1&&nxt.output==1);int before=refreshes;
    bt_peer=1;tick(14500);assert(sink==1&&refreshes==before+1);
    bt_ready=false;tick(15000);assert(sink==2&&nxt.output==2);
    card_file(3,false);tick(15500);tick(17500);assert(sink==0);
    bt_ready=true;tick(18000);assert(sink==1);
    card_file(3,true);tick(18500);assert(sink==2);
    card_file(3,false);tick(19000);tick(21000);assert(sink==1);
    bt_ready=false;tick(21500);assert(sink==0);
    nxt.auto_bt=0;bt_ready=true;tick(22000);assert(sink==0);bt_ready=false;tick(22500);
    nxt.auto_bt=1;bt_ready=true;tick(23000);assert(sink==1);
    assert(NxtHardware_select(0));nxt.output=0;tick(23500);assert(sink==0);
    puts("PASS production Bluetooth ready/disconnect, peer change, DAC priority/fallback, auto off and stable manual output.");
    puts("PASS Stock LED: native animation disabled/reclaimed/restored, driver-table static=4 (not sniff=3), table-aware/fallback IDs, verified RGB readback, all 16 presets/reactive brightness, trailing-space/newline RGB, enable/60-scale and manual color; RGB retry/reclaim and private device snapshot; DAC route: failed write recovers without new hotplug; volume preserves selected PCM; short disconnect/reconnect retains DAC; sustained unplug follows setting; new ALSA card and explicit speaker selection work.");
    return 0;
}
''')
    run("compile_route", flags + ['-DNXT_ASOUND_ROOT="' + str(asound) + '"',
        '-DNXT_SOUND_SYS_ROOT="' + str(work / "sys") + '"', '-DNXT_LED_ROOT="'+str(led)+'"', hardware,
        SRC / "nxt_route.c", SRC / "module_common.c", SRC / "nxt_pcm.c", SRC / "nxt_led.c", SRC / "include/parson/parson.c",
        "-Wl,--gc-sections", "-lm", "-o", work / "hardware"])
    run("dac_route", [work / "hardware", work])

    # Exercise the same production live-latch path under the NextUI define.
    for name in ['max_scale','max_scale_lr','max_scale_f1f2','max_scale_rear']:(led/name).write_text('20')
    for name in ['m','lr','f1','f2','rear']:
        (led/('effect_rgb_hex_'+name)).write_text('88FF88 ')
        (led/('effect_'+name)).write_text('1')
    (led/'effect_enable').write_text('1');(led/'anim_frames_enable').write_text('1')
    prefix=hardware.read_text().split('int main(int argc,char **argv)')[0].replace('void NxtStock_tick(void) {}','')
    next_main=r"""
int main(int argc,char **argv){
 snprintf(workspace,sizeof(workspace),"%s",argv[1]);snprintf(config_path,sizeof(config_path),"%s/.asoundrc",workspace);
 nxt.output=0;nxt.led=1;nxt.led_color=16;nxt.led_rgb=0x123456;nxt.led_strength=100;
 tick(100);assert(latched_rgb[1]==0x123456&&disable_writes==0);int before=effect_latches;
 nxt.led_rgb=0xabcdef;tick(150);assert(latched_rgb[1]==0xabcdef&&effect_latches>before&&disable_writes==0);
 before=effect_latches;nxt.led_mode=1;tick(200);assert(effect_latches>before&&disable_writes==0);
 nxt.led_mode=0;nxt.led_effect=1;nxt.led_rgb=0xff0000;nxt.led_rgb2=0x0000ff;tick(250);assert(latched_rgb[1]==0xff0000&&latched_rgb[4]==0x0000ff&&disable_writes==0);
 char raw[80];assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"3"));
 before=effect_latches;nxt.led_effect=2;tick(300);tick(450);assert(effect_latches>before&&disable_writes==0);
 NxtHardware_release();assert(readtext(NXT_LED_ROOT "/effect_lr",raw,sizeof(raw))&&!strcmp(raw,"1"));assert(readtext(NXT_LED_ROOT "/effect_rgb_hex_lr",raw,sizeof(raw))&&!strcmp(raw,"88FF88"));
 puts("PASS NextUI production LED: live color/mode/effect latches without disable, two group colors, animated changes, NextUI static ID and original driver state restored.");return 0;
}
"""
    nxt_hardware=work/'nextui-hardware.c';nxt_hardware.write_text(prefix+next_main)
    next_flags=['-DNXT_ROM_NEXTUI' if f=='-DNXT_ROM_STOCK' else f for f in flags]
    run('compile_nextui_led',next_flags+['-DNXT_ASOUND_ROOT="'+str(asound)+'"','-DNXT_SOUND_SYS_ROOT="'+str(work/'sys')+'"','-DNXT_LED_ROOT="'+str(led)+'"',nxt_hardware,SRC/'nxt_route.c',SRC/'module_common.c',SRC/'nxt_pcm.c',SRC/'nxt_led.c',SRC/'include/parson/parson.c','-Wl,--gc-sections','-lm','-o',work/'nextui-hardware'])
    run('nextui_live_led',[work/'nextui-hardware',work])



(ROOT / "logs/v171-led-driver-test.json").write_text(json.dumps({"exit":0,"native_driver_checks":checks,"effect_write_latches_actual_RGB":True,"no_disable_cycle_for_config_changes":True,"multicolor_group_rgb":True,"restores_original_driver_state":True,"physical_device_tested":False},indent=2)+"\n")
print("PASS native Stock driver fixture: audio routing/volume, gradients per LED group, animated rainbow changes, readback/retry and restored original state.")
