#include "nxt_personal.h"
#include "nxt_text.h"
#include "nxt_volume.h"
#include "nxt_karaoke.h"
#include "nxt_monitor.h"
#include "nxt_performance.h"
#include "nxt_ota.h"
#include "nxt_log.h"
#include "nxt_soundcloud.h"
#include "nxt_spotify.h"
/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt.h"
#include "nxt_theme.h"
#include "nxt_locale.h"
#include "nxt_controls.h"
#include "nxt_stock.h"
#include "api.h"
#include "settings.h"
#include "file_utils.h"
#include "player.h"
#include "radio.h"
#include "podcast.h"
#include "resume.h"
#include "background.h"
#include "browser.h"
#include "module_player.h"
#include "parson/parson.h"
#include "nxt_startup.h"
#include "nxt_write.h"
#include "nxt_cpu.h"
#include "nxt_audio_mode.h"
#include "nxt_translate.h"
#include "nxt_nxredux.h"
#include "toast.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
NxtConfig nxt;
static uint32_t timer_started,timer_duration;
static int clamp(double v,int low,int high) { return !isfinite(v)?low:v<low?low:v>high?high:(int)v; }
/* v1.9.7.4 action 22 was vocal reduction. Retired/invalid actions must not
 * clamp to the final valid action (Stop), including per-profile preferences. */
static int control_action(double value) {
    return isfinite(value)&&value>=NC_NONE&&value<NC_COUNT&&floor(value)==value?(int)value:NC_NONE;
}
static int readint(JSON_Object *o,const char *k,int fallback,int lo,int hi) {
    return clamp(json_object_has_value_of_type(o,k,JSONNumber)?json_object_get_number(o,k):fallback,lo,hi);
}
static float readfloat(JSON_Object *o,const char *k,float fallback,float lo,float hi) {
    double v=json_object_has_value_of_type(o,k,JSONNumber)?json_object_get_number(o,k):fallback;
    return !isfinite(v)?fallback:v<lo?lo:v>hi?hi:v;
}
void Nxt_init(void) {
    memset(&nxt,0,sizeof(nxt));
    nxt.performance_mode=NXT_PERFORMANCE_BALANCED;
    nxt.karaoke_estimate=1;nxt.lyric_rgb=16765286;nxt.lyric_unsung_rgb=15658734;nxt.lyric_translation_rgb=12444645;nxt.lyric_bold=0;nxt.lyric_outline=2;nxt.lyric_outline_rgb=1052688;nxt.lyric_shadow=1;nxt.song_title_rgb=16777215;nxt.song_artist_rgb=13165798;nxt.song_title_size=50;nxt.karaoke_auto=0;nxt.lyric_effect=1;nxt.auralis_reference_font=1;nxt.auralis_now_brightness=72;nxt.auralis_full_brightness=60;
    nxt.avatar_icon=-1;nxt.animations=1;nxt.cover_mode=1;nxt.spectrum_rgb=0x83ddb0;nxt.spectrum_rgb2=0x63b9ef;nxt.led_rgb2=0xffa4c9;snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"NXT");
    nxt.auto_bt=1;nxt.accent_rgb=0x83ddb0;nxt.wallpaper_dim=20;nxt.panel_opacity=35;
    nxt.language=0; nxt.eq_bands=10; nxt.font_size=100; nxt.lyric_size=36;
    nxt.recursive=1; nxt.unplug_pause=1; nxt.dsp=1;
#ifdef NXT_ROM_STOCK
    nxt.auto_dac=1;nxt.unplug_pause=0;
#endif
    snprintf(nxt.momo,sizeof(nxt.momo),"0339838267");
    nxt.limiter=1; nxt.crossfade_ms=4000; nxt.background=1; nxt.led_strength=60; nxt.output=3; nxt.transition=1;
    for(int i=0;i<3;i++){nxt.volume[i]=8;nxt.volume_percent[i]=40;}
    for(int i=0;i<5;i++) nxt.shortcut[i]=i;
    NxtControl_defaults();nxt.led_rgb=0x9de4b6;nxt.led_mode=1;
    for(int i=0;i<NXT_PEQ_BANDS;i++) { nxt.peq[i].hz=80*powf(3,i); nxt.peq[i].q=1; }
    snprintf(nxt.music_root,sizeof(nxt.music_root),"%s",MUSIC_PATH);
    char path[1024]; userdata_snpath("nxt.json",path,sizeof(path));
    JSON_Value *v=json_parse_file(path); JSON_Object *o=json_value_get_object(v);
    if(o) {
        double perf=json_object_get_number(o,"performance_mode");
        if(json_object_has_value_of_type(o,"performance_mode",JSONNumber)&&isfinite(perf)&&floor(perf)==perf&&perf>=0&&perf<=2)nxt.performance_mode=(int)perf;
#define INT(field,lo,hi) nxt.field=readint(o,#field,nxt.field,lo,hi)
#define FLOAT(field,lo,hi) nxt.field=readfloat(o,#field,nxt.field,lo,hi)
        INT(bit_perfect,0,1);INT(dsd_output,0,2);INT(auto_play,0,1);nxt.audio_mode=readint(o,"audio_mode",nxt.bit_perfect?1:0,0,2);
        INT(translation_mode,0,2);INT(translation_source,0,14);
        INT(karaoke_auto,0,1);INT(karaoke_estimate,0,1);INT(lyric_rgb,0,16777215);INT(lyric_unsung_rgb,0,16777215);INT(lyric_translation_rgb,0,16777215);INT(lyric_bold,0,1);INT(lyric_outline,0,3);INT(lyric_outline_rgb,0,16777215);INT(lyric_shadow,0,1);INT(song_title_rgb,0,16777215);INT(song_artist_rgb,0,16777215);INT(song_title_size,38,60);INT(lyric_effect,0,9);INT(auralis_reference_font,0,1);INT(auralis_now_brightness,40,95);INT(auralis_full_brightness,30,90);INT(interface_style,0,NXT_THEME_COUNT-1);INT(cover_mode,0,1);INT(animations,0,1);INT(eq_preset,0,12);INT(spectrum_palette,0,20);INT(spectrum_mode,0,4);INT(spectrum_rgb,0,16777215);INT(spectrum_rgb2,0,16777215);INT(led_effect,0,5);INT(led_rgb2,0,16777215);INT(avatar_icon,-1,299);INT(avatar_mode,0,4);INT(avatar_emoji,0,179);INT(avatar_pixel,0,63);INT(birth_day,0,31);INT(birth_month,0,12);INT(genre,0,19);INT(mood,0,9);INT(country,0,15);
        INT(auto_bt,0,1);INT(accent_color,0,16);INT(accent_rgb,0,0xffffff);INT(wallpaper_mode,0,21);INT(wallpaper_dim,0,95);INT(panel_opacity,0,100);
        INT(language,0,13); INT(theme,0,2); INT(font_size,85,120); INT(font_index,0,NXT_FONT_COUNT-1); INT(lyric_size,24,48);
        INT(recursive,0,1);INT(auto_dac,0,1);INT(led_color,0,16);INT(led_rgb,0,0xffffff);INT(led_mode,0,1);INT(spotify_auth_source,0,2);
        INT(screen_off_controls,0,1); INT(lyric_offset,-30000,30000); INT(lyric_fullscreen,0,1); INT(dsp,0,1); INT(limiter,0,1);
        INT(repeat_mode,0,2);INT(shuffle,0,1);INT(replaygain,0,2); INT(transition,0,2); INT(crossfade_ms,500,12000); INT(led,0,1);
        INT(led_strength,0,100); INT(unplug_pause,0,1); INT(output,0,3); INT(background,0,1);
        nxt.eq_bands=readint(o,"eq_bands",10,5,31); if(nxt.eq_bands!=5&&nxt.eq_bands!=10&&nxt.eq_bands!=15&&nxt.eq_bands!=31) nxt.eq_bands=10;
        FLOAT(preamp,-24,12); FLOAT(bass,-12,12); FLOAT(treble,-12,12); FLOAT(balance,-1,1);
        FLOAT(width,0,2); if(!json_object_has_value(o,"width")) nxt.width=1;
        FLOAT(compressor,0,1); FLOAT(crossfeed,0,1);
#undef INT
#undef FLOAT
        const char *av=json_object_get_string(o,"avatar_text");if(av)snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"%s",av);av=json_object_get_string(o,"avatar_path");if(av)snprintf(nxt.avatar_path,sizeof(nxt.avatar_path),"%s",av);JSON_Array *mi=json_object_get_array(o,"menu_icons");for(int i=0;i<7;i++)nxt.menu_icons[i]=clamp(json_array_get_number(mi,i),0,480);
        const char *s=json_object_get_string(o,"music_root"); if(s&&s[0]=='/') snprintf(nxt.music_root,sizeof(nxt.music_root),"%s",s);
        s=json_object_get_string(o,"font_path"); if(s) snprintf(nxt.font_path,sizeof(nxt.font_path),"%s",s);
        s=json_object_get_string(o,"wallpaper_path");if(s)snprintf(nxt.wallpaper_path,sizeof(nxt.wallpaper_path),"%s",s);
        s=json_object_get_string(o,"momo"); if(s) snprintf(nxt.momo,sizeof(nxt.momo),"%s",s);
        JSON_Array *a=json_object_get_array(o,"eq"); for(int i=0;i<31;i++) { double f=json_array_get_number(a,i); nxt.eq[i]=isfinite(f)?fmax(-12,fmin(12,f)):0; }
        a=json_object_get_array(o,"volume"); for(int i=0;i<3;i++) if(json_array_get_count(a)>(size_t)i) nxt.volume[i]=clamp(json_array_get_number(a,i),0,20);
        a=json_object_get_array(o,"volume_percent");for(int i=0;i<3;i++)nxt.volume_percent[i]=json_array_get_count(a)>(size_t)i?clamp(json_array_get_number(a,i),0,100):nxt.volume[i]*5;
        a=json_object_get_array(o,"shortcut"); for(int i=0;i<5;i++) if(json_array_get_count(a)>(size_t)i) nxt.shortcut[i]=clamp(json_array_get_number(a,i),0,4);
        a=json_object_get_array(o,"keys");for(int i=0;i<16;i++)if(json_array_get_count(a)>(size_t)i){int key=json_array_get_number(a,i);for(int j=0;j<16;j++)if(key==Nxt_keyIds[j])nxt.keys[i]=key;}
        a=json_object_get_array(o,"playing_keys");for(int i=0;i<16;i++)if(json_array_get_count(a)>(size_t)i)nxt.playing_keys[i]=control_action(json_array_get_number(a,i));
        a=json_object_get_array(o,"peq"); for(int i=0;i<6;i++) { JSON_Object *p=json_array_get_object(a,i); if(p) { nxt.peq[i].hz=readfloat(p,"hz",100,20,20000); nxt.peq[i].gain=readfloat(p,"gain",0,-12,12); nxt.peq[i].q=readfloat(p,"q",1,.1,12); nxt.peq[i].enabled=readint(p,"enabled",0,0,1); } }
    } else nxt.width=1;
#ifdef NXT_ROM_STOCK
    /* This release enables the automatic two-way handoff the user requested.
     * Save the marker so later explicit changes are respected. */
    if(!o||json_object_get_boolean(o,"stock_hotplug_fix1")!=1){nxt.auto_dac=1;nxt.unplug_pause=0;}
#endif
    bool valid_keys=true;for(int i=0;i<16;i++)for(int j=0;j<i;j++)if(nxt.keys[i]==nxt.keys[j])valid_keys=false;
    if(!valid_keys)for(int i=0;i<16;i++)nxt.keys[i]=Nxt_keyIds[i];
    if(o&&!json_object_get_array(o,"playing_keys"))NxtControl_migrate();
    if(o&&json_object_get_boolean(o,"wallpaper_visibility_fix1")!=1){if(nxt.wallpaper_dim>=65)nxt.wallpaper_dim=20;nxt.panel_opacity=35;}
 if(nxt.avatar_mode==4){nxt.avatar_mode=0;nxt.avatar_icon=23;}
    if(o&&json_object_get_boolean(o,"avatar_catalog_v18")!=1)for(int i=0;i<7;i++)if(nxt.menu_icons[i]>=65&&nxt.menu_icons[i]<=128)nxt.menu_icons[i]+=236;
    json_value_free(v);
    /* Fork updates belong to this product, never to the upstream self-updater. */
    Settings_setBool(&SETTING_AUTO_UPDATE,false);
    NxtPersonal_init();NxtOTA_init();NxtLog_step("app","startup",true,"Stock frontend initialized");
    NxtDSP_configure(&nxt,48000);
}
bool Nxt_save(void) {
    JSON_Value *v=json_value_init_object(); JSON_Object *o=json_value_get_object(v);
#ifdef NXT_ROM_STOCK
    json_object_set_boolean(o,"stock_hotplug_fix1",true);
#endif

#define SAVE(field) json_object_set_number(o,#field,nxt.field)
    SAVE(performance_mode);
    SAVE(bit_perfect);SAVE(dsd_output);SAVE(auto_play);SAVE(audio_mode);
    SAVE(karaoke_auto);SAVE(karaoke_estimate);SAVE(lyric_rgb);SAVE(lyric_unsung_rgb);SAVE(lyric_translation_rgb);SAVE(lyric_bold);SAVE(lyric_outline);SAVE(lyric_outline_rgb);SAVE(lyric_shadow);SAVE(song_title_rgb);SAVE(song_artist_rgb);SAVE(song_title_size);SAVE(lyric_effect);SAVE(auralis_reference_font);SAVE(auralis_now_brightness);SAVE(auralis_full_brightness);SAVE(interface_style);SAVE(cover_mode);SAVE(animations);SAVE(eq_preset);SAVE(spectrum_palette);SAVE(spectrum_mode);SAVE(spectrum_rgb);SAVE(spectrum_rgb2);SAVE(led_effect);SAVE(led_rgb2);SAVE(avatar_icon);SAVE(avatar_mode);SAVE(avatar_emoji);SAVE(avatar_pixel);SAVE(birth_day);SAVE(birth_month);SAVE(genre);SAVE(mood);SAVE(country);
    SAVE(auto_bt);SAVE(accent_color);SAVE(accent_rgb);SAVE(wallpaper_mode);SAVE(wallpaper_dim);SAVE(panel_opacity);
    json_object_set_boolean(o,"wallpaper_visibility_fix1",true);
    json_object_set_boolean(o,"avatar_catalog_v18",true);
    SAVE(translation_mode);SAVE(translation_source);
    SAVE(screen_off_controls); SAVE(language); SAVE(theme); SAVE(font_size); SAVE(font_index); SAVE(lyric_size); SAVE(lyric_offset); SAVE(lyric_fullscreen);
    SAVE(recursive);SAVE(auto_dac);SAVE(led_color);SAVE(led_rgb);SAVE(led_mode);SAVE(spotify_auth_source);
    SAVE(repeat_mode);SAVE(shuffle);SAVE(dsp); SAVE(eq_bands); SAVE(limiter); SAVE(replaygain); SAVE(transition); SAVE(crossfade_ms);
    SAVE(led); SAVE(led_strength); SAVE(unplug_pause); SAVE(output); SAVE(background);
    SAVE(preamp); SAVE(bass); SAVE(treble); SAVE(balance); SAVE(width); SAVE(compressor); SAVE(crossfeed);
#undef SAVE
    json_object_set_string(o,"avatar_text",nxt.avatar_text);json_object_set_string(o,"avatar_path",nxt.avatar_path);
    JSON_Value *mi=json_value_init_array();for(int i=0;i<7;i++)json_array_append_number(json_value_get_array(mi),nxt.menu_icons[i]);json_object_set_value(o,"menu_icons",mi);
    json_object_set_string(o,"wallpaper_path",nxt.wallpaper_path);
    json_object_set_string(o,"music_root",nxt.music_root); json_object_set_string(o,"font_path",nxt.font_path); json_object_set_string(o,"momo",nxt.momo);
    JSON_Value *a=json_value_init_array(); for(int i=0;i<31;i++) json_array_append_number(json_value_get_array(a),nxt.eq[i]); json_object_set_value(o,"eq",a);
    a=json_value_init_array(); for(int i=0;i<3;i++) json_array_append_number(json_value_get_array(a),nxt.volume[i]); json_object_set_value(o,"volume",a);
    a=json_value_init_array();for(int i=0;i<3;i++)json_array_append_number(json_value_get_array(a),nxt.volume_percent[i]);json_object_set_value(o,"volume_percent",a);
    a=json_value_init_array(); for(int i=0;i<5;i++) json_array_append_number(json_value_get_array(a),nxt.shortcut[i]); json_object_set_value(o,"shortcut",a);
    a=json_value_init_array();for(int i=0;i<16;i++)json_array_append_number(json_value_get_array(a),nxt.keys[i]);json_object_set_value(o,"keys",a);
    a=json_value_init_array();for(int i=0;i<16;i++)json_array_append_number(json_value_get_array(a),nxt.playing_keys[i]);json_object_set_value(o,"playing_keys",a);
    a=json_value_init_array(); for(int i=0;i<6;i++) { JSON_Value *f=json_value_init_object(); JSON_Object *p=json_value_get_object(f); json_object_set_number(p,"hz",nxt.peq[i].hz); json_object_set_number(p,"gain",nxt.peq[i].gain); json_object_set_number(p,"q",nxt.peq[i].q); json_object_set_number(p,"enabled",nxt.peq[i].enabled); json_array_append_value(json_value_get_array(a),f); } json_object_set_value(o,"peq",a);
    char path[1400];userdata_snpath("nxt.json",path,sizeof(path));char *json=json_serialize_to_string_pretty(v);
    bool ok=json&&NxtWrite_text(path,json,true);NxtPersonal_preferencesPath(path,sizeof(path));if(ok&&*path)ok=NxtWrite_text(path,json,true);
    json_free_serialized_string(json);json_value_free(v);if(!ok)NxtLog_step("config","save",false,"Config write queue unavailable");
    Nxt_profileSave(nxt.output);if(!nxt.bit_perfect)NxtDSP_configure(&nxt,Player_getOutputRate()>0?Player_getOutputRate():48000);Player_refreshOptions();return ok;
}
const char *Nxt_musicRoot(void) { return nxt.music_root[0]?nxt.music_root:SDCARD_PATH "/Music"; }
void Nxt_stop(void) {
    NxtStartup_save();NxtPersonal_flush();
    NxtSpotify_quit();
    NxtSoundCloud_quit();
    if(Radio_isActive()){Radio_stop();Background_setActive(BG_NONE);ModuleCommon_setAutosleepDisabled(false);return;}
    if(Podcast_isActive()||Background_getActive()==BG_PODCAST){Podcast_stop();Podcast_flushProgress();Background_setActive(BG_NONE);ModuleCommon_setAutosleepDisabled(false);return;}
    if(Player_getState()!=PLAYER_STATE_STOPPED){Resume_updatePosition(Player_getPosition());PlayerModule_markStopped();}Background_stopAll();Player_stop();
}
void Nxt_sleepTimer(int minutes){if(minutes>1440)minutes=1440;timer_started=SDL_GetTicks();timer_duration=minutes>0?(uint32_t)minutes*60000u:0;}
int Nxt_timerRemaining(void) { uint32_t elapsed=SDL_GetTicks()-timer_started;return elapsed>=timer_duration?0:(timer_duration-elapsed+999)/1000; }
void Nxt_tick(void) {
    NxtVolume_tick();
    NxtTranslate_tick();
    NxtKaraoke_tick();
    NxtSpotify_tick();
    NxtSoundCloud_tick();
    NxtSession_tick();
    NxtCPU_tick(NxtSession_isHidden());
    NxtAudio_tick();
    if(NxtWrite_failed())Toast_show(Nxt_text("Could not save to SD card; check free space","Không lưu được vào thẻ; kiểm tra dung lượng trống"),3500);
    Player_update();NxtStartup_tick();NxtNX_tick();NxtLibrary_tick();
    PlayerModule_syncQueue();
    NxtPersonal_tick();NxtMonitor_tick();NxtOTA_tick();
    if(timer_duration&&(uint32_t)(SDL_GetTicks()-timer_started)>=timer_duration){
        timer_duration=0;Nxt_stop();
        /* End the entire foreground/background process after saving, rather
         * than leaving a paused UI/decoder alive. The firmware then owns
         * idle screen-off and sleep under the user's system settings. */
        Nxt_save();ModuleCommon_signalStop();ModuleCommon_requestQuit();return;
    }
    // Screen-off frames still advance the queue and save resume position.
    if(PWR_getDisplayOff() && Background_getActive()==BG_NONE && !Radio_isActive() && !Podcast_isActive())
        PlayerModule_backgroundTick();
    NxtHardware_tick();
    Player_handleAudioSinkChange();
#ifdef NXT_ROM_STOCK
    /* Detached sessions have no SDL input loop. Keep DAC volume keys active. */
    if(NxtSession_isHidden()||NxtStock_panelActive()){
        USBHIDEvent event;
        while((event=Player_pollUSBHID())!=USB_HID_EVENT_NONE)ModuleCommon_handleHIDVolume(event);
    }
#endif
}
const char *Nxt_text(const char *en,const char *vi) { return nxt.language==1?vi:NxtLocale_text(en); }

void Nxt_loadPreferences(const char *path){
 JSON_Value *v=json_parse_file(path);JSON_Object *o=json_value_get_object(v);
 nxt.karaoke_estimate=1;nxt.lyric_rgb=16765286;nxt.lyric_unsung_rgb=15658734;nxt.lyric_translation_rgb=12444645;nxt.lyric_bold=0;nxt.lyric_outline=2;nxt.lyric_outline_rgb=1052688;nxt.lyric_shadow=1;nxt.song_title_rgb=16777215;nxt.song_artist_rgb=13165798;nxt.song_title_size=50;nxt.karaoke_auto=0;nxt.lyric_effect=1;nxt.auralis_reference_font=1;nxt.auralis_now_brightness=72;nxt.auralis_full_brightness=60;
 nxt.avatar_icon=-1;nxt.avatar_mode=0;nxt.avatar_emoji=nxt.avatar_pixel=nxt.birth_day=nxt.birth_month=nxt.genre=nxt.mood=nxt.country=0;memset(nxt.menu_icons,0,sizeof(nxt.menu_icons));snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"NXT");nxt.avatar_path[0]=0;
 if(!o){json_value_free(v);return;}
nxt.translation_mode=0;nxt.translation_source=0;
nxt.interface_style=0;nxt.animations=1;nxt.cover_mode=1;nxt.spectrum_palette=nxt.spectrum_mode=nxt.eq_preset=0;nxt.spectrum_rgb=0x83ddb0;nxt.spectrum_rgb2=0x63b9ef;nxt.led_effect=0;nxt.led_rgb2=0xffa4c9;
#define PREF(field,lo,hi) nxt.field=readint(o,#field,nxt.field,lo,hi)
 PREF(translation_mode,0,2);PREF(translation_source,0,14);
 PREF(karaoke_auto,0,1);PREF(karaoke_estimate,0,1);PREF(lyric_rgb,0,16777215);PREF(lyric_unsung_rgb,0,16777215);PREF(lyric_translation_rgb,0,16777215);PREF(lyric_bold,0,1);PREF(lyric_outline,0,3);PREF(lyric_outline_rgb,0,16777215);PREF(lyric_shadow,0,1);PREF(song_title_rgb,0,16777215);PREF(song_artist_rgb,0,16777215);PREF(song_title_size,38,60);PREF(lyric_effect,0,9);PREF(auralis_reference_font,0,1);PREF(auralis_now_brightness,40,95);PREF(auralis_full_brightness,30,90);PREF(interface_style,0,NXT_THEME_COUNT-1);PREF(cover_mode,0,1);PREF(animations,0,1);PREF(eq_preset,0,12);PREF(spectrum_palette,0,20);PREF(spectrum_mode,0,4);PREF(spectrum_rgb,0,16777215);PREF(spectrum_rgb2,0,16777215);PREF(led_effect,0,5);PREF(led_rgb2,0,16777215);PREF(avatar_icon,-1,299);PREF(avatar_mode,0,4);PREF(avatar_emoji,0,179);PREF(avatar_pixel,0,63);PREF(birth_day,0,31);PREF(birth_month,0,12);PREF(genre,0,19);PREF(mood,0,9);PREF(country,0,15);
 PREF(language,0,13);PREF(theme,0,2);PREF(accent_color,0,16);PREF(accent_rgb,0,0xffffff);PREF(wallpaper_mode,0,21);PREF(wallpaper_dim,0,95);PREF(panel_opacity,0,100);
 PREF(font_size,85,120);PREF(font_index,0,NXT_FONT_COUNT-1);PREF(lyric_size,24,48);PREF(lyric_offset,-30000,30000);PREF(lyric_fullscreen,0,1);
 PREF(dsp,0,1);PREF(limiter,0,1);PREF(replaygain,0,2);PREF(transition,0,2);PREF(crossfade_ms,500,12000);PREF(repeat_mode,0,2);PREF(shuffle,0,1);
 PREF(eq_bands,5,31);if(nxt.eq_bands!=5&&nxt.eq_bands!=10&&nxt.eq_bands!=15&&nxt.eq_bands!=31)nxt.eq_bands=10;
#undef PREF
#define F(field,lo,hi) nxt.field=readfloat(o,#field,nxt.field,lo,hi)
 F(preamp,-24,12);F(bass,-12,12);F(treble,-12,12);F(balance,-1,1);F(width,0,2);F(compressor,0,1);F(crossfeed,0,1);
#undef F
 const char *av=json_object_get_string(o,"avatar_text");snprintf(nxt.avatar_text,sizeof(nxt.avatar_text),"%s",av?av:"NXT");av=json_object_get_string(o,"avatar_path");snprintf(nxt.avatar_path,sizeof(nxt.avatar_path),"%s",av?av:"");JSON_Array *mi=json_object_get_array(o,"menu_icons");for(int i=0;i<7;i++)nxt.menu_icons[i]=clamp(json_array_get_number(mi,i),0,480);
 const char *s=json_object_get_string(o,"wallpaper_path");snprintf(nxt.wallpaper_path,sizeof(nxt.wallpaper_path),"%s",s?s:"");s=json_object_get_string(o,"font_path");snprintf(nxt.font_path,sizeof(nxt.font_path),"%s",s?s:"");
 JSON_Array *a=json_object_get_array(o,"eq");for(int i=0;i<31;i++)if(json_array_get_count(a)>(size_t)i){double x=json_array_get_number(a,i);nxt.eq[i]=isfinite(x)?fmax(-12,fmin(12,x)):0;}
 a=json_object_get_array(o,"peq");for(int i=0;i<6;i++){JSON_Object *p=json_array_get_object(a,i);if(p){nxt.peq[i].hz=readfloat(p,"hz",100,20,20000);nxt.peq[i].gain=readfloat(p,"gain",0,-12,12);nxt.peq[i].q=readfloat(p,"q",1,.1,12);nxt.peq[i].enabled=readint(p,"enabled",0,0,1);}}
 a=json_object_get_array(o,"keys");for(int i=0;i<16;i++)if(json_array_get_count(a)>(size_t)i){int k=json_array_get_number(a,i);for(int j=0;j<16;j++)if(k==Nxt_keyIds[j])nxt.keys[i]=k;}
 bool valid=true;for(int i=0;i<16;i++)for(int j=0;j<i;j++)if(nxt.keys[i]==nxt.keys[j])valid=false;if(!valid)for(int i=0;i<16;i++)nxt.keys[i]=Nxt_keyIds[i];
 a=json_object_get_array(o,"playing_keys");for(int i=0;i<16;i++)if(json_array_get_count(a)>(size_t)i)nxt.playing_keys[i]=control_action(json_array_get_number(a,i));
 if(json_object_get_boolean(o,"wallpaper_visibility_fix1")!=1){if(nxt.wallpaper_dim>=65)nxt.wallpaper_dim=20;nxt.panel_opacity=35;}
 if(o&&json_object_get_boolean(o,"avatar_catalog_v18")!=1)for(int i=0;i<7;i++)if(nxt.menu_icons[i]>=65&&nxt.menu_icons[i]<=128)nxt.menu_icons[i]+=236;
 if(nxt.avatar_mode==4){nxt.avatar_mode=0;nxt.avatar_icon=23;}
 json_value_free(v);
}
