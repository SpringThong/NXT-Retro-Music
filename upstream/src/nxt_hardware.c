#include "nxt_volume.h"
static int previous_bt;
static char previous_bt_route[1024];
#include "nxt_route.h"
#include "nxt_log.h"
#include "nxt_dac_name.h"
#include <sys/utsname.h>
#include "nxt_spotify.h"
#include "nxt_soundcloud.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt.h"
#include "nxt_bitperfect.h"
#include "nxt_audio_mode.h"
#include "nxt_led.h"
#include "player.h"
#include "debug.h"
#include "nxt_pcm.h"
#include "nxt_usb.h"
#include "file_utils.h"
#include "msettings.h"
#include "parson/parson.h"
#include "nxt_stock.h"
#include "nxt_nextui.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef NXT_LED_ROOT
#define NXT_LED_ROOT "/sys/class/led_anim"
#endif
#ifndef NXT_ASOUND_ROOT
#define NXT_ASOUND_ROOT "/proc/asound"
#endif
#ifndef NXT_SOUND_SYS_ROOT
#define NXT_SOUND_SYS_ROOT "/sys/class/sound"
#endif
static int usb_card=-1,last_sink=-1,last_jack=-1;
#ifdef NXT_ROM_STOCK
static bool usb_missing;
static uint32_t usb_missing_since;
#define NXT_DAC_DISCONNECT_MS 1500
#endif
#ifdef TEST_CONTROL
static int test_dac=-1;
#endif
static char product[256],manufacturer[256],vid[16],pid[16],output[512],details[2048],score_summary[128],pcm_info[160];
static char config_path[1024],saved_config[8192];static bool saved_exists,owns_config;
static uint32_t next_probe,next_led,next_led_check,led_retry_at;
static bool route_pending,last_route_ok=true,led_config_ok;
static unsigned attempted_rgb;static int attempted_enabled=-1;
#ifdef TEST_CONTROL
void NxtHardware_testDAC(int connected){test_dac=!!connected;next_probe=0;}
#endif
static char backup_path[1024],backup_flag[1024];
static const char *led_paths[]={NXT_LED_ROOT "/max_scale",NXT_LED_ROOT "/max_scale_lr",NXT_LED_ROOT "/max_scale_f1f2",NXT_LED_ROOT "/max_scale_rear"};
static int led_last_strength[4]={-1,-1,-1,-1};
static char led_saved[4][64];static bool led_owned[4];
static const char *led_groups[]={"m","lr","f1","f2","rear"};
static char rgb_saved[5][64],effect_saved[5][64];static bool rgb_owned[5],effect_owned[5];
static bool anim_owned;static char anim_saved[64];
static int last_led_mode=-1,last_led_effect=-1;static unsigned last_rgb=0xffffffff;static int last_enabled=-1;static char enable_saved[64];static bool enable_owned;
static int last_led_color=-1,last_user_rgb=-1,last_user_rgb2=-1;
bool NxtHardware_controlsLEDs(void){return nxt.led!=0;}
static bool readtext(const char *path,char *buf,size_t n){buf[0]=0;FILE *f=fopen(path,"r");if(!f)return false;size_t len=fread(buf,1,n-1,f);buf[len]=0;bool ok=!ferror(f);if(fclose(f))ok=false;if(!ok)return false;while(len&&strchr("\r\n \t",buf[len-1]))buf[--len]=0;return true;}
static bool write_text(const char *path,const char *value){FILE *f=fopen(path,"w");if(!f)return false;bool ok=fputs(value,f)>=0;return fclose(f)==0&&ok;}
static int led_static_effect(void){
#if defined(NXT_ROM_STOCK) || defined(NXT_ROM_NXREDUX)
    /* Brick Pro Stock reports 3=sniff, 4=static. Prefer the driver's table
     * over assuming that other firmware uses the same numbering. */
    char names[2048];
    if(readtext(NXT_LED_ROOT "/effect_names",names,sizeof(names))){
        for(const char *line=names;line;){
            unsigned id;char name[32];
            if(sscanf(line," %u - %31s",&id,name)==2&&id<=255&&!strcmp(name,"static"))return (int)id;
            const char *next=strchr(line,'\n');line=next?next+1:NULL;
        }
    }
    return 4;
#else
    return 3;
#endif
}
static bool write_led_number(const char *path,int number){
    char value[64];snprintf(value,sizeof(value),"%d\n",number);
    if(!write_text(path,value))return false;
#if defined(NXT_ROM_STOCK) || defined(NXT_ROM_NXREDUX)
    char *end;if(!readtext(path,value,sizeof(value)))return false;
    long observed=strtol(value,&end,10);return *value&&!*end&&observed==number;
#else
    return true;
#endif
}
static bool write_led_rgb(const char *path,unsigned rgb){
    char value[80];snprintf(value,sizeof(value),"%06X \n",rgb);
    if(!write_text(path,value))return false;
#if defined(NXT_ROM_STOCK) || defined(NXT_ROM_NXREDUX)
    unsigned observed;char extra;
    return readtext(path,value,sizeof(value))&&sscanf(value,"%x%c",&observed,&extra)==1&&observed==rgb;
#else
    return true;
#endif
}
static void identify(void) {
    usb_card=-1;product[0]=manufacturer[0]=vid[0]=pid[0]=0;
#ifdef TEST_CONTROL
    if(test_dac>=0){if(test_dac){usb_card=1;snprintf(product,sizeof(product),"QA USB DAC");}return;}
#endif
    for(int card=0;card<32;card++){char path[1024],buf[256];snprintf(path,sizeof(path),NXT_ASOUND_ROOT "/card%d/usbbus",card);if(!readtext(path,buf,sizeof(buf)))continue;usb_card=card;
        snprintf(path,sizeof(path),NXT_SOUND_SYS_ROOT "/card%d/device",card);char actual[4096];if(realpath(path,actual)){for(int up=0;up<12;up++){char item[4200];snprintf(item,sizeof(item),"%s/idVendor",actual);if(readtext(item,vid,sizeof(vid))){snprintf(item,sizeof(item),"%s/idProduct",actual);readtext(item,pid,sizeof(pid));snprintf(item,sizeof(item),"%s/product",actual);readtext(item,product,sizeof(product));snprintf(item,sizeof(item),"%s/manufacturer",actual);readtext(item,manufacturer,sizeof(manufacturer));break;}char *slash=strrchr(actual,'/');if(!slash||slash==actual)break;*slash=0;}}
        if(!*product){snprintf(path,sizeof(path),NXT_ASOUND_ROOT "/card%d/id",card);readtext(path,product,sizeof(product));}break;
    }
}
/* USB descriptor strings are firmware labels, not verified retail branding. */
static char dac_name[128],dac_identity_cache[600];
static void dac_label(bool refresh){
 char key[600];snprintf(key,sizeof(key),"%s|%s|%s|%s",vid,pid,manufacturer,product);
 if(!refresh&&!strcmp(key,dac_identity_cache))return;
 snprintf(dac_identity_cache,sizeof(dac_identity_cache),"%s",key);dac_name[0]=0;
 if(usb_card>=0){char path[1024];userdata_snpath("dac-names.json",path,sizeof(path));NxtDACIdentity i={vid,pid,manufacturer,product};NxtDACName_read(path,&i,dac_name,sizeof(dac_name));}
}
const char*NxtHardware_dacName(void){dac_label(false);return dac_name;}
bool NxtHardware_setDACName(const char*name){
 if(usb_card<0)return false;char path[1024];userdata_snpath("dac-names.json",path,sizeof(path));NxtDACIdentity i={vid,pid,manufacturer,product};
 if(!NxtDACName_save(path,&i,name))return false;dac_label(true);next_probe=0;return true;
}
/* Scores are review data, never inferred from USB capabilities or a shared chip ID. */
static void rating(char *out,size_t size) {
    snprintf(out,size,"%s",Nxt_text("No verified rating","Chưa có đánh giá"));
    JSON_Value *v=json_parse_file("./res/dac-ratings.json");JSON_Array *a=json_value_get_array(v);
    for(size_t i=0;i<json_array_get_count(a);i++){JSON_Object *r=json_array_get_object(a,i);const char *p=json_object_get_string(r,"product"),*m=json_object_get_string(r,"manufacturer"),*vendor=json_object_get_string(r,"vid"),*id=json_object_get_string(r,"pid"),*source=json_object_get_string(r,"source");double score=json_object_get_number(r,"score");
        if(p&&m&&vendor&&id&&source&&strlen(source)>8&&*manufacturer&&!strcmp(p,product)&&!strcmp(m,manufacturer)&&!strcmp(vendor,vid)&&!strcmp(id,pid)&&score>=0&&score<=10){snprintf(out,size,"%.1f/10 (%s) • %s",score,Nxt_text("reference","tham khảo"),source);break;}}
    json_value_free(v);
}
static bool route(int sink) {
#ifdef PLATFORM_DESKTOP
    (void)sink;return true;
#else
    if(!*config_path){
#ifdef NXT_ROM_STOCK
        const char *user_home=getenv("HOME");if(user_home)snprintf(config_path,sizeof(config_path),"%s/.asoundrc",user_home);
#else
        if(!NxtNextUI_audioConfig(config_path,sizeof(config_path)))return false;
#endif
    }
    if(!*config_path)return false;
    char text[2048];
    if(sink==1) {
#ifdef NXT_ROM_STOCK
        if(!NxtStock_bluetoothRoute(text,sizeof(text)))return false;
#else
        if(!NxtNextUI_bluetoothRoute(text,sizeof(text)))return false;
#endif
    }else{
    int card=sink==2?usb_card:0;if(card<0)return false;
    const char *format="S16_LE";char stream_path[1024],stream_formats[8192];snprintf(stream_path,sizeof(stream_path),NXT_ASOUND_ROOT "/card%d/stream0",card);if(sink==2&&readtext(stream_path,stream_formats,sizeof(stream_formats))&&strstr(stream_formats,"S32_LE"))format="S32_LE";
    int mix_key=17600+card, period_size=1024, hardware_rate=48000, buffer_size=8192;
    const char *mix_uid="true";
#ifdef NXT_ROM_STOCK
    /* Match /etc/asound.conf so stock audio and background music share dmix. */
    if(sink==0){mix_key=1111;period_size=2048;mix_uid="false";}
#endif
#ifdef NXT_ROM_NXREDUX
    /* Share the firmware's speaker/USB dmix rather than opening an exclusive
     * second mixer while games play. NX Redux publishes the negotiated DAC
     * rate in tmpfs; this is read only when the route is changed. */
    mix_uid="false";format="S16_LE";
    if(sink==0){mix_key=1111;period_size=2048;}
    else{
        mix_key=0x4e5858;buffer_size=4096;
#ifndef NXT_NXREDUX_SINK_STATE
#define NXT_NXREDUX_SINK_STATE "/tmp/nx_audio_sink"
#endif
        char state[1024];
        if(readtext(NXT_NXREDUX_SINK_STATE,state,sizeof(state))&&strstr(state,"sink=usb\n")){
            int published_card=-1,rate=0;char *c=strstr(state,"card="),*r=strstr(state,"rate=");
            if(c&&r&&sscanf(c,"card=%d",&published_card)==1&&sscanf(r,"rate=%d",&rate)==1&&published_card==card&&rate>=8000&&rate<=384000)hardware_rate=rate;
        }
    }
#endif
    snprintf(text,sizeof(text),"# NXT Retro Music shared PCM\n# Restored when the music session ends.\npcm.nxt_music_mix {\n type dmix\n ipc_key %d\n ipc_key_add_uid %s\n ipc_perm 0666\n slave { pcm \"hw:%d,0\" rate %d format %s channels 2 period_size %d buffer_size %d }\n bindings { 0 0 1 1 }\n}\npcm.nxt_music_output { type plug slave.pcm \"nxt_music_mix\" }\npcm.!default { type plug slave.pcm \"nxt_music_mix\" }\nctl.!default { type hw card %d }\n",mix_key,mix_uid,card,hardware_rate,format,period_size,buffer_size,card);
    }
    char old[8192];bool exists=readtext(config_path,old,sizeof(old));
#ifdef NXT_ROM_STOCK
    if(!*backup_path){userdata_snpath("asoundrc.before-nxt",backup_path,sizeof(backup_path));userdata_snpath("asoundrc.existed",backup_flag,sizeof(backup_flag));}
    if(!exists||!strstr(old,"# NXT Retro Music shared PCM")){saved_exists=exists;snprintf(saved_config,sizeof(saved_config),"%s",old);write_text(backup_path,old);write_text(backup_flag,exists?"1":"0");}
    else if(!owns_config){char flag[16];if(readtext(backup_flag,flag,sizeof(flag))){saved_exists=*flag=='1';readtext(backup_path,saved_config,sizeof(saved_config));}}
#endif
#ifdef NXT_ROM_NEXTUI
    /* Use the existing monitor's app-private config, never audiomon's file. */
    char wrapped[4096];if(!NxtNextUI_wrapRoute(text,wrapped,sizeof(wrapped)))return false;
    if(!exists||strcmp(old,wrapped)){
        char temp[1100];snprintf(temp,sizeof(temp),"%s.nxt-tmp",config_path);
        if(!write_text(temp,wrapped)||rename(temp,config_path)){unlink(temp);return false;}
    }
    if(setenv("ALSA_CONFIG_PATH",config_path,1))return false;
    owns_config=true;return true;
#else
    if(exists&&!strcmp(old,text)){owns_config=true;return true;}
    char temp[1100];snprintf(temp,sizeof(temp),"%s.nxt-tmp",config_path);
    if(!write_text(temp,text)||rename(temp,config_path)){unlink(temp);return false;}owns_config=true;return true;
#endif
#endif
}
bool NxtHardware_select(int choice) {
    if(choice<0||choice>3)return false;
    identify();if(choice==2&&usb_card<0)return false;
    int sink=choice==3?GetAudioSink():choice;
    if(choice==3&&sink==2&&usb_card<0)sink=0;
    if(sink<0||sink>2)return false;
    if(!route(sink)){route_pending=true;last_route_ok=false;NxtLog_step("audio","route-write",false,"sink=%d; retry pending",sink);return false;}
    route_pending=false;last_route_ok=true;
    if(last_sink>=0&&last_sink<3){nxt.volume[last_sink]=GetVolume();Nxt_profileSave(last_sink);}
    SetAudioSink(sink);SetVolume(nxt.volume[sink]);last_sink=sink;nxt.output=sink;Nxt_profileLoad(sink);NxtAudio_routeChanged();NxtAudio_tick();NxtBit_route(usb_card,sink==2);NxtVolume_restore();Player_requestOutputRefresh();return true;
}
void NxtHardware_tick(void) {
    NxtStock_tick();
    uint32_t now=SDL_GetTicks();
    int jack=NxtHardware_hasJack();
    if(now>=next_probe||jack!=last_jack){last_jack=jack;next_probe=now+500;int previous_usb=usb_card;
        bool usb_recovered=false;
#ifdef NXT_ROM_STOCK
        char old_product[256],old_manufacturer[256],old_vid[16],old_pid[16];
        memcpy(old_product,product,sizeof(product));memcpy(old_manufacturer,manufacturer,sizeof(manufacturer));
        memcpy(old_vid,vid,sizeof(vid));memcpy(old_pid,pid,sizeof(pid));
#endif
        identify();
#ifdef NXT_ROM_STOCK
        /* A transient USB reset must not change the selected output to the
         * speaker. Confirm removal; reopen the DAC after it comes back. */
        if(nxt.output==2&&last_sink==2&&previous_usb>=0&&usb_card<0){
            if(!usb_missing){usb_missing=true;usb_missing_since=now;LOG_info("DAC: waiting to confirm disconnect, card=%d\n",previous_usb);}
            if((uint32_t)(now-usb_missing_since)<NXT_DAC_DISCONNECT_MS){
                usb_card=previous_usb;memcpy(product,old_product,sizeof(product));memcpy(manufacturer,old_manufacturer,sizeof(manufacturer));
                memcpy(vid,old_vid,sizeof(vid));memcpy(pid,old_pid,sizeof(pid));
            }
        }else if(usb_missing){usb_recovered=usb_card>=0;usb_missing=false;if(usb_recovered)LOG_info("DAC: reconnected, keep output, card=%d\n",usb_card);}
#endif
        char bt_route[1024]="";bool bt=false;
#ifdef NXT_ROM_STOCK
        bt=NxtStock_bluetoothRoute(bt_route,sizeof(bt_route));
#else
        bt=NxtNextUI_bluetoothRoute(bt_route,sizeof(bt_route));
#endif
        bool bt_changed=bt!=previous_bt||strcmp(previous_bt_route,bt_route);if(bt_changed)NxtLog_step("audio","bluetooth-state",true,"ready=%d",bt);
        if(nxt.output==3)nxt.output=GetAudioSink();nxt.output=NxtRoute_choose(nxt.output,nxt.auto_dac||nxt.audio_mode==NXT_AUDIO_AUTO,nxt.auto_bt,usb_card>=0,previous_usb>=0,bt,previous_bt);int sink=nxt.output;
        previous_bt=bt;snprintf(previous_bt_route,sizeof(previous_bt_route),"%s",bt_route);
        bool route_change=last_sink<0||sink!=last_sink||GetAudioSink()!=sink||previous_usb!=usb_card||usb_recovered||(sink==1&&bt_changed)||route_pending;
        if(route_change){
            if(route(sink)){
                if(last_sink>=0&&last_sink<3){nxt.volume[last_sink]=GetVolume();Nxt_profileSave(last_sink);}
                bool removed=last_sink==2&&usb_card<0;
                NxtAudio_routeChanged();NxtAudio_tick();NxtBit_route(usb_card,sink==2);
                SetAudioSink(sink);SetVolume(nxt.volume[sink]);last_sink=sink;
                Nxt_profileLoad(sink);NxtVolume_restore();Player_requestOutputRefresh();
                if(removed&&nxt.unplug_pause)Player_pause();
                route_pending=false;last_route_ok=true;NxtLog_step("audio","route-applied",true,"sink=%d",sink);Nxt_save();
            }else{
                if(!route_pending)LOG_info("Audio: output configuration pending, sink=%d\n",sink);
                if(!route_pending)NxtLog_step("audio","route-write",false,"sink=%d; retry pending",sink);route_pending=true;last_route_ok=false;
            }
        }
#if !defined(PLATFORM_DESKTOP) && defined(NXT_ROM_STOCK)
        else if(sink!=1&&*config_path){char text[8192];bool found=readtext(config_path,text,sizeof(text));
            if(!found||!strstr(text,"# NXT Retro Music shared PCM")){
                last_route_ok=route(sink);route_pending=!last_route_ok;
                if(last_route_ok)Player_requestOutputRefresh();
            }
        }
#endif
        /* While a write is pending, the current volume still belongs to the
         * old output, not the newly detected DAC. */
        if(!route_pending&&sink>=0&&sink<3&&nxt.volume[sink]!=GetVolume()){nxt.volume[sink]=GetVolume();Nxt_save();}
        char score[600];rating(score,sizeof(score));snprintf(score_summary,sizeof(score_summary),"%s",score);char *source=strstr(score_summary," • ");if(source)*source=0;
        if(sink==2){
            NxtPCM pcm;bool confirmed=NxtPCM_read(NXT_ASOUND_ROOT,usb_card,&pcm);
            if(nxt.bit_perfect)snprintf(pcm_info,sizeof(pcm_info),"%s",NxtBit_status());else NxtPCM_label(confirmed?&pcm:NULL,nxt.language==1,pcm_info,sizeof(pcm_info));
        }else snprintf(pcm_info,sizeof(pcm_info),"%s: %d Hz / %d-bit",Nxt_text("App PCM","PCM ứng dụng"),Player_getOutputRate(),Player_getOutputBits());

        if(sink==2){dac_label(false);snprintf(output,sizeof(output),*dac_name?"DAC: %s":"USB DAC: %s",*dac_name?dac_name:*product?product:"USB Audio");snprintf(details,sizeof(details),"%s: %s / %s • USB %s:%s • %s • %s",Nxt_text("USB descriptor","Tên USB"),manufacturer,product,vid,pid,pcm_info,score);}else {snprintf(output,sizeof(output),"%s",sink==1?"Bluetooth":jack?Nxt_text("Headphones · 3.5 mm","Tai nghe · 3,5 mm"):Nxt_text("Speaker","Loa máy"));snprintf(details,sizeof(details),"%s • %s",output,Nxt_text("Source and output rates are shown separately","Thông số nguồn và đầu ra được hiển thị riêng"));}
    }
led_update:
    if(now>=next_led){
        next_led=now+50;
        float level=(Player_getState()==PLAYER_STATE_PLAYING||NxtSpotify_isPlaying()||NxtSoundCloud_isPlaying())?fminf(1,nxt.bit_perfect?NxtBit_peak():NxtDSP_peak()):0;
        int strength=nxt.led_strength;if(strength<0)strength=0;if(strength>100)strength=100;
        int v=(int)((nxt.led_mode?level:1)*strength*60/100);unsigned group_rgb[5],rgb=2166136261u;for(int i=0;i<5;i++){group_rgb[i]=NxtLED_groupRGB(i,now/125*125);rgb=(rgb^group_rgb[i])*16777619u;}if(!nxt.led_effect)rgb=NxtLED_rgb();
        bool apply=last_enabled!=nxt.led||last_rgb!=rgb||last_led_mode!=nxt.led_mode||last_led_effect!=nxt.led_effect||now>=next_led_check;
#if defined(NXT_ROM_STOCK) || defined(NXT_ROM_NXREDUX)
        /* Firmware can reclaim frame animation after its panel closes. */
        if(nxt.led){
            char state[64];if(!anim_owned)anim_owned=readtext(NXT_LED_ROOT "/anim_frames_enable",anim_saved,sizeof(anim_saved));
            if(anim_owned&&readtext(NXT_LED_ROOT "/anim_frames_enable",state,sizeof(state))&&atoi(state)!=0)apply=true;
        }
#endif
        if(led_retry_at&&(int32_t)(now-led_retry_at)<0&&attempted_enabled==nxt.led&&attempted_rgb==rgb)apply=false;
        if(apply){
            bool ok=true;int color_groups=0,static_effect=led_static_effect();attempted_enabled=nxt.led;attempted_rgb=rgb;
#if defined(NXT_ROM_STOCK) || defined(NXT_ROM_NXREDUX)
            if(nxt.led&&anim_owned)ok=write_led_number(NXT_LED_ROOT "/anim_frames_enable",0)&&ok;
#endif
            if(nxt.led){
                if(!enable_owned)enable_owned=readtext(NXT_LED_ROOT "/effect_enable",enable_saved,sizeof(enable_saved));
                /* effect_enable is applied after RGB/effect programming below. */
            }else if(enable_owned){if(write_text(NXT_LED_ROOT "/effect_enable",enable_saved))enable_owned=false;else ok=false;}
            for(int i=0;i<5;i++){
                char path[180],value[80];snprintf(path,sizeof(path),NXT_LED_ROOT "/effect_rgb_hex_%s",led_groups[i]);
                if(nxt.led){
                    if(!rgb_owned[i])rgb_owned[i]=readtext(path,rgb_saved[i],sizeof(rgb_saved[i]));
                    if(rgb_owned[i]){color_groups++;ok=write_led_rgb(path,group_rgb[i])&&ok;}
                }else if(rgb_owned[i]){snprintf(value,sizeof(value),"%s \n",rgb_saved[i]);if(write_text(path,value))rgb_owned[i]=false;else ok=false;}
                snprintf(path,sizeof(path),NXT_LED_ROOT "/effect_%s",led_groups[i]);
                if(nxt.led){if(!effect_owned[i])effect_owned[i]=readtext(path,effect_saved[i],sizeof(effect_saved[i]));if(effect_owned[i]){/* Reissue even when the ID is unchanged: this latches the new RGB. */ok=write_led_number(path,static_effect)&&ok;}}
                else if(effect_owned[i]){if(write_text(path,effect_saved[i]))effect_owned[i]=false;else ok=false;}
            }
            if(nxt.led&&enable_owned)ok=write_led_number(NXT_LED_ROOT "/effect_enable",1)&&ok;
#if defined(NXT_ROM_STOCK) || defined(NXT_ROM_NXREDUX)
            if(!nxt.led&&anim_owned){if(write_text(NXT_LED_ROOT "/anim_frames_enable",anim_saved))anim_owned=false;else ok=false;}
#endif
            /* Cache only successful writes and driver readbacks. A missing optional rear group is
             * allowed, but no supported RGB group is not a successful apply. */
            bool was_ok=led_config_ok;int old_enabled=last_enabled;
            led_config_ok=ok&&(!nxt.led||color_groups>0);
            static uint32_t next_led_log;
            /* Animation RGB changes can occur eight times a second. Log the
             * user's settings/recovery, not every animation step. */
            if(was_ok!=led_config_ok||old_enabled!=nxt.led||last_led_mode!=nxt.led_mode||last_led_effect!=nxt.led_effect||last_led_color!=nxt.led_color||last_user_rgb!=nxt.led_rgb||last_user_rgb2!=nxt.led_rgb2){
                if(led_config_ok||now>=next_led_log){NxtLog_step("led","apply",led_config_ok,"enabled=%d RGB=%06X static=%d groups=%d",nxt.led,NxtLED_rgb(),static_effect,color_groups);next_led_log=now+5000;}
            }
            if(led_config_ok){last_enabled=nxt.led;last_rgb=rgb;last_led_mode=nxt.led_mode;last_led_effect=nxt.led_effect;last_led_color=nxt.led_color;last_user_rgb=nxt.led_rgb;last_user_rgb2=nxt.led_rgb2;led_retry_at=0;}
            else led_retry_at=now+250;
            next_led_check=now+1000;
        }
        for(int i=0;i<4;i++){
            if(nxt.led){if(!led_owned[i])led_owned[i]=readtext(led_paths[i],led_saved[i],sizeof(led_saved[i]));
                if(led_owned[i]&&(led_last_strength[i]!=v||apply)){char value[32];snprintf(value,sizeof(value),"%d\n",v);if(write_text(led_paths[i],value))led_last_strength[i]=v;else led_last_strength[i]=-1;}}
            else if(led_owned[i]&&write_text(led_paths[i],led_saved[i])){led_owned[i]=false;led_last_strength[i]=-1;}
        }
    }
}
void NxtHardware_release(void) {
    if(enable_owned){write_text(NXT_LED_ROOT "/effect_enable",enable_saved);enable_owned=false;}last_enabled=-1;last_rgb=0xffffffff;led_config_ok=false;led_retry_at=next_led_check=0;
    for(int i=0;i<5;i++){char path[180];snprintf(path,sizeof(path),NXT_LED_ROOT "/effect_rgb_hex_%s",led_groups[i]);if(rgb_owned[i]){char value[80];snprintf(value,sizeof(value),"%s \n",rgb_saved[i]);write_text(path,value);}snprintf(path,sizeof(path),NXT_LED_ROOT "/effect_%s",led_groups[i]);if(effect_owned[i])write_text(path,effect_saved[i]);rgb_owned[i]=effect_owned[i]=false;}
    for(int i=0;i<4;i++)if(led_owned[i]){write_text(led_paths[i],led_saved[i]);led_owned[i]=false;}
    if(anim_owned){write_text(NXT_LED_ROOT "/anim_frames_enable",anim_saved);anim_owned=false;}
#ifndef PLATFORM_DESKTOP
    char text[8192];if(owns_config&&readtext(config_path,text,sizeof(text))&&strstr(text,"# NXT Retro Music shared PCM")){if(saved_exists)write_text(config_path,saved_config);else unlink(config_path);if(*backup_path)unlink(backup_path);if(*backup_flag)unlink(backup_flag);}owns_config=false;
#endif
}
static void diagnostic_attr(FILE *f,const char *attr){
    char path[180],value[80];snprintf(path,sizeof(path),NXT_LED_ROOT "/%s",attr);
    fprintf(f,"LED %s: %s\n",attr,readtext(path,value,sizeof(value))?value:"unavailable");
}
bool NxtHardware_writeDiagnostic(void){
    char temp[]="NXT_Device_Log.txt.tmp.XXXXXX";int fd=mkstemp(temp);if(fd<0)return false;
    FILE *f=fdopen(fd,"w");if(!f){close(fd);unlink(temp);return false;}
    fprintf(f,"NXT Retro Music %s / %s\n",NXT_VERSION,NXT_BUILD_REVISION);
    fprintf(f,"Uptime ms: %u\nAudio selected: %d; requested: %d; USB card: %d\n",SDL_GetTicks(),GetAudioSink(),nxt.output,usb_card);
    fprintf(f,"Auto DAC: %d; pause on unplug: %d; volume: %d\n",nxt.auto_dac,nxt.unplug_pause,GetVolume());
    fprintf(f,"Route write OK: %d; pending: %d; PCM ready: %d; retry pending: %d\n",last_route_ok,route_pending,Player_outputReady(),Player_outputRetryPending());
    fprintf(f,"Bluetooth active in player: %d; last PCM error: %s\n",Player_isBluetoothActive(),*Player_lastOutputError()?Player_lastOutputError():"none");
    fprintf(f,"ALSA private config selected: %d; named app PCM: %d\n",getenv("ALSA_CONFIG_PATH")&&!strcmp(getenv("ALSA_CONFIG_PATH"),config_path),getenv("AUDIODEV")&&!strcmp(getenv("AUDIODEV"),"nxt_music_output"));
    fprintf(f,"Output PCM: %d Hz / %d-bit\n",Player_getOutputRate(),Player_getOutputBits());
    struct utsname kernel;if(!uname(&kernel))fprintf(f,"Kernel: %s %s %s\n",kernel.sysname,kernel.release,kernel.machine);
    fprintf(f,"DAC user label: %s\nUSB descriptor manufacturer: %s\nUSB descriptor product: %s\nUSB VID:PID: %s:%s\n",NxtHardware_dacName(),manufacturer,product,vid,pid);
    fprintf(f,"DSD setting: %d (0 Native, 1 DoP, 2 explicit Native/DoP); %s\nDSD detail: %s\n",nxt.dsd_output,NxtBit_status(),NxtBit_error());
    if(NxtBit_isDSD()){TrackInfo source;char path[512];NxtBit_snapshot(&source,path,sizeof(path));fprintf(f,"DSD source: %s; rate=%d; channels=%d; duration_ms=%d\n",source.codec,source.sample_rate,source.channels,source.duration_ms);}
    if(usb_card>=0){
        char path[1200],text[16384];const char*attrs[]={"id","usbid","usbbus"};
        for(int a=0;a<3;a++){snprintf(path,sizeof(path),NXT_ASOUND_ROOT "/card%d/%s",usb_card,attrs[a]);if(readtext(path,text,sizeof(text)))fprintf(f,"%s:\n%s\n",path,text);}
        for(int dev=0;dev<8;dev++){
            snprintf(path,sizeof(path),NXT_ASOUND_ROOT "/card%d/stream%d",usb_card,dev);if(readtext(path,text,sizeof(text)))fprintf(f,"%s:\n%s\n",path,text);
            snprintf(path,sizeof(path),NXT_ASOUND_ROOT "/card%d/pcm%dp/sub0/hw_params",usb_card,dev);if(readtext(path,text,sizeof(text)))fprintf(f,"%s:\n%s\n",path,text);
        }
    }

    char config[8192];bool found=*config_path&&readtext(config_path,config,sizeof(config));
    fprintf(f,"ALSA config present: %d; NXT shared PCM: %d\n",found,found&&strstr(config,"# NXT Retro Music shared PCM")!=NULL);
    fprintf(f,"LED requested: enabled=%d RGB=%06X brightness=%d mode=%d; last write OK=%d\n",nxt.led,NxtLED_rgb(),nxt.led_strength,nxt.led_mode,led_config_ok);
    fprintf(f,"LED static effect selected: %d\n",led_static_effect());
    char effects[2048];fprintf(f,"LED effect_names:\n%s\n",readtext(NXT_LED_ROOT "/effect_names",effects,sizeof(effects))?effects:"unavailable (Stock fallback: 4)");
    diagnostic_attr(f,"anim_frames_enable");diagnostic_attr(f,"effect_enable");
    for(int i=0;i<5;i++){char attr[80];snprintf(attr,sizeof(attr),"effect_rgb_hex_%s",led_groups[i]);diagnostic_attr(f,attr);snprintf(attr,sizeof(attr),"effect_%s",led_groups[i]);diagnostic_attr(f,attr);}
    for(int i=0;i<4;i++){const char *name=strrchr(led_paths[i],'/');diagnostic_attr(f,name?name+1:led_paths[i]);}
    fprintf(f,"Spotify login source: %s\n",nxt.spotify_auth_source==1?"PC":nxt.spotify_auth_source==2?"QR":"Auto");
    bool ok=!ferror(f);if(fclose(f))ok=false;
    if(ok)ok=!rename(temp,"NXT_Device_Log.txt");if(!ok)unlink(temp);return ok;
}
const char *NxtHardware_output(void){return output;}
const char *NxtHardware_details(void){return details;}

const char *NxtHardware_score(void){return score_summary;}
const char *NxtHardware_pcm(void){return pcm_info;}
bool NxtHardware_hasJack(void){return InitializedSettings()&&GetJack()!=0;}
bool NxtHardware_hasDAC(void){return usb_card>=0;}

/* Explicit settings actions only: opening the app does not switch the port. */
const char *NxtHardware_bottomStatus(void){
    int role=NxtUSB_role("/sys/devices/platform/soc/usbc0");
    return role==1?Nxt_text("DAC mode","Chế độ DAC"):role==2?Nxt_text("Charging mode","Chế độ sạc"):Nxt_text("Unavailable","Chưa xác định");
}
int NxtHardware_bottomUSB(bool host){
    const char *base="/sys/devices/platform/soc/usbc0";
    int before=NxtUSB_role(base);
    /* Switching to device can disconnect the active bottom DAC. */
    if(!host&&before!=2&&Player_isUSBDACActive())Player_pause();
    int result=NxtUSB_switch(base,host);
    LOG_info("Bottom USB: request=%s before=%d result=%d\n",host?"DAC":"charging",before,result);
    next_probe=0;
    return result;
}

int NxtHardware_dacCard(void){return usb_card;}
