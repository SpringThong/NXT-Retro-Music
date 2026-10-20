#!/usr/bin/env python3
"""Test real config migration, atomic save, reload and profile isolation."""
from pathlib import Path
import json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'.cache/stock-build/headers/zip/lib',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
fixture=r'''
#include "nxt.h"
#include "nxt_performance.h"
#include "nxt_write.h"
#include "nxt_controls.h"
#include "nxt_text.h"
#include "settings.h"
#include "file_utils.h"
#include "parson/parson.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
const int Nxt_keyIds[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
void NxtControl_defaults(void){for(int i=0;i<16;i++){nxt.keys[i]=Nxt_keyIds[i];nxt.playing_keys[i]=i;}}
void NxtControl_migrate(void){NxtControl_defaults();}
const BoolSetting SETTING_AUTO_UPDATE={0};
void Settings_setBool(const BoolSetting*k,bool value){assert(!value);}
void NxtPersonal_init(void){}
void NxtOTA_init(void){}
void NxtKaraoke_tick(void){}
void NxtPersonal_savePreferences(void){}
void NxtPersonal_preferencesPath(char*p,size_t n){if(n)*p=0;}
int NxtPersonal_route(const char*s,char*out,size_t n){return -1;}
void NxtLog_step(const char*c,const char*step,bool ok,const char*fmt,...){}
void Nxt_profileSave(int p){}
void NxtDSP_configure(const NxtConfig*c,int rate){}
int Player_getOutputRate(void){return 48000;}
void Player_refreshOptions(void){}
static void write_file(const char *path,const char *json){FILE*f=fopen(path,"w");assert(f);assert(fputs(json,f)>=0);assert(!fclose(f));}
int main(void){
 char path[1024];userdata_snpath("nxt.json",path,sizeof(path));unlink(path);
 Nxt_init();assert(nxt.performance_mode==NXT_PERFORMANCE_BALANCED&&nxt.language==0&&nxt.bit_perfect==0&&nxt.auto_play==0);

 /* v1.9.7.4's retired vocal action must never clamp to Stop (21). */
 char migrated[4096];
 for(int slot=0;slot<16;slot++){
  JSON_Value*v=json_value_init_object();JSON_Object*o=json_value_get_object(v);JSON_Value*a=json_value_init_array();
  for(int j=0;j<16;j++)json_array_append_number(json_value_get_array(a),j==slot?22:NC_PLAY);
  json_object_set_value(o,"playing_keys",a);json_object_set_boolean(o,"vocal_keymap_v1974",true);
  assert(json_serialize_to_file(v,path)==JSONSuccess);json_value_free(v);Nxt_init();
  for(int j=0;j<16;j++)assert(nxt.playing_keys[j]==(j==slot?NC_NONE:NC_PLAY));
  assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.playing_keys[slot]==NC_NONE);
  userdata_snpath("retired-key-profile.json",migrated,sizeof(migrated));
  write_file(migrated,"{\"playing_keys\":[22,21,1,2,3,4,5,6,7,8,9,10,22,13,14,15]}");Nxt_loadPreferences(migrated);
  assert(nxt.playing_keys[0]==NC_NONE&&nxt.playing_keys[12]==NC_NONE&&nxt.playing_keys[1]==NC_STOP&&nxt.playing_keys[2]==NC_PLAY);
 }
 for(int action=NC_NONE;action<NC_COUNT;action++){
  snprintf(migrated,sizeof(migrated),"{\"playing_keys\":[%d]}",action);write_file(path,migrated);Nxt_init();assert(nxt.playing_keys[0]==action);
 }
 write_file(path,"{\"playing_keys\":[-1,22,999,1.5]}");Nxt_init();
 for(int j=0;j<4;j++)assert(nxt.playing_keys[j]==NC_NONE);

 /* An installed v1.8.2 config has no new field: retain its existing choices. */
 write_file(path,"{\"language\":1,\"interface_style\":17,\"eq_bands\":5,\"eq\":[3,2,1,0,-1],\"wallpaper_mode\":11,\"led_rgb\":1122867,\"avatar_catalog_v18\":true,\"wallpaper_visibility_fix1\":true,\"stock_hotplug_fix1\":true}");
 Nxt_init();assert(nxt.performance_mode==1&&nxt.language==1&&nxt.interface_style==17&&nxt.eq_bands==5&&nxt.eq[0]==3&&nxt.wallpaper_mode==11&&nxt.led_rgb==1122867);
 for(int mode=0;mode<3;mode++){
  nxt.performance_mode=mode;nxt.audio_mode=mode;nxt.bit_perfect=mode%2;nxt.auto_play=mode%2;assert(Nxt_save());NxtWrite_flush();JSON_Value*v=json_parse_file(path);assert(v);assert(json_object_get_number(json_value_get_object(v),"performance_mode")==mode);json_value_free(v);
  nxt.performance_mode=-99;Nxt_init();assert(nxt.audio_mode==mode&&nxt.performance_mode==mode&&nxt.interface_style==17&&nxt.eq[0]==3&&nxt.led_rgb==1122867&&nxt.bit_perfect==mode%2&&nxt.auto_play==mode%2);
  char profile[1024];userdata_snpath("profile-test.json",profile,sizeof(profile));write_file(profile,"{\"performance_mode\":99,\"interface_style\":0,\"wallpaper_mode\":3,\"avatar_catalog_v18\":true,\"wallpaper_visibility_fix1\":true}");
  Nxt_loadPreferences(profile);assert(nxt.performance_mode==mode&&nxt.interface_style==0&&nxt.wallpaper_mode==3&&nxt.bit_perfect==mode%2&&nxt.auto_play==mode%2);
  write_file(profile,"{}");Nxt_loadPreferences(profile);assert(nxt.performance_mode==mode);
  nxt.interface_style=17;
 }
 const char *invalid[]={"-1","3","99999","0.5","1.5","\"0\"","null","true","false","[]","{}"};
 for(unsigned i=0;i<sizeof(invalid)/sizeof(*invalid);i++){char json[128];snprintf(json,sizeof(json),"{\"performance_mode\":%s}",invalid[i]);write_file(path,json);Nxt_init();assert(nxt.performance_mode==NXT_PERFORMANCE_BALANCED);}
 for(int style=0;style<51;style++){nxt.interface_style=style;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.interface_style==style);}
 char pref[1024];userdata_snpath("profile-theme-test.json",pref,sizeof(pref));
 write_file(pref,"{\"interface_style\":49}");Nxt_loadPreferences(pref);assert(nxt.interface_style==49);
 write_file(path,"{\"interface_style\":51}");Nxt_init();assert(nxt.interface_style==50);
 write_file(path,"{\"interface_style\":-1}");Nxt_init();assert(nxt.interface_style==0);
 write_file(path,"{ broken JSON");Nxt_init();assert(nxt.performance_mode==1);
 write_file(path,"{\"volume\":[17,8,4]}");Nxt_init();assert(nxt.volume_percent[0]==85&&nxt.volume_percent[1]==40&&nxt.volume_percent[2]==20);
 nxt.volume_percent[0]=86;nxt.volume_percent[1]=47;nxt.volume_percent[2]=23;nxt.font_index=16;nxt.interface_style=50;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.volume_percent[0]==86&&nxt.volume_percent[1]==47&&nxt.volume_percent[2]==23&&nxt.font_index==16&&nxt.interface_style==50);
 write_file(pref,"{\"font_index\":16,\"interface_style\":50,\"volume_percent\":[0,0,0]}");Nxt_loadPreferences(pref);assert(nxt.font_index==16&&nxt.interface_style==50&&nxt.volume_percent[0]==86);
 for(int font=0;font<NXT_FONT_COUNT;font++){nxt.font_index=font;nxt.auralis_now_brightness=91;nxt.auralis_full_brightness=43;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.font_index==font&&nxt.auralis_now_brightness==91&&nxt.auralis_full_brightness==43);}
 for(int font=0;font<NXT_FONT_COUNT;font++){char text[64];snprintf(text,sizeof(text),"{\"font_index\":%d}",font);write_file(pref,text);Nxt_loadPreferences(pref);assert(nxt.font_index==font);}
 write_file(pref,"{\"font_index\":999}");Nxt_loadPreferences(pref);assert(nxt.font_index==59);
 write_file(pref,"{\"font_index\":-5}");Nxt_loadPreferences(pref);assert(nxt.font_index==0);
 write_file(pref,"{\"auralis_now_brightness\":83,\"auralis_full_brightness\":58,\"font_index\":19}");Nxt_loadPreferences(pref);assert(nxt.auralis_now_brightness==83&&nxt.auralis_full_brightness==58&&nxt.font_index==19);
 write_file(pref,"{}");Nxt_loadPreferences(pref);assert(nxt.auralis_now_brightness==72&&nxt.auralis_full_brightness==60);
 write_file(pref,"{\"auralis_now_brightness\":-3,\"auralis_full_brightness\":999}");Nxt_loadPreferences(pref);assert(nxt.auralis_now_brightness==40&&nxt.auralis_full_brightness==90);

 assert(nxt.screen_off_controls==0);nxt.screen_off_controls=1;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.screen_off_controls==1);
 write_file(pref,"{\"screen_off_controls\":0}");Nxt_loadPreferences(pref);assert(nxt.screen_off_controls==1);
 nxt.screen_off_controls=0;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.screen_off_controls==0);
 nxt.lyric_rgb=0x123456;nxt.lyric_unsung_rgb=0xabcdef;nxt.lyric_translation_rgb=0x778899;nxt.lyric_bold=1;nxt.lyric_outline=3;nxt.lyric_outline_rgb=0x234567;nxt.lyric_shadow=0;nxt.song_title_rgb=0xff0033;nxt.song_artist_rgb=0x00eeff;nxt.song_title_size=58;nxt.karaoke_estimate=0;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.lyric_rgb==0x123456&&nxt.lyric_unsung_rgb==0xabcdef&&nxt.lyric_translation_rgb==0x778899&&nxt.lyric_bold==1&&nxt.lyric_outline==3&&nxt.lyric_outline_rgb==0x234567&&nxt.lyric_shadow==0&&nxt.song_title_rgb==0xff0033&&nxt.song_artist_rgb==0x00eeff&&nxt.song_title_size==58&&!nxt.karaoke_estimate);
 write_file(pref,"{}");Nxt_loadPreferences(pref);assert(nxt.lyric_rgb==0xffd166&&nxt.lyric_outline==2&&nxt.karaoke_estimate==1&&nxt.song_title_size==50);
 for(int fx=0;fx<10;fx++){nxt.lyric_effect=fx;nxt.auralis_reference_font=fx%2;nxt.karaoke_auto=fx%2;assert(Nxt_save());NxtWrite_flush();Nxt_init();assert(nxt.lyric_effect==fx&&nxt.auralis_reference_font==fx%2&&nxt.karaoke_auto==fx%2);}
 write_file(pref,"{}");Nxt_loadPreferences(pref);assert(nxt.lyric_effect==1&&nxt.auralis_reference_font==1&&nxt.karaoke_auto==0);
 NxtWrite_quit();
 puts("PASS fine volume legacy migration, exact levels save/reload, MiSans/Auralis profile choices and independent device volumes.");
 puts("PASS real config: v1.8.2 migration defaults to Balanced without changing audio/appearance; three modes save/reload atomically; profile changes preserve device mode; malformed, fractional and out-of-range values fall back safely.");
}
'''
results={}
for rom in ['stock','nextui','nxredux']:
 with tempfile.TemporaryDirectory(prefix='nxt-v1975-config-') as td:
  w=Path(td);data=w/'data';data.mkdir();c=w/'test.c';c.write_text(fixture);binary=w/'test'
  cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom.upper(),'-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(c),str(S/'nxt_config.c'),str(S/'nxt_write.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-lSDL2_image','-lpthread','-lm','-o',str(binary)]
  with (R/f'logs/v1975-config-{rom}-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
  p=subprocess.run([str(binary)],env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True);print(rom,p.stdout+p.stderr,end='');p.check_returncode()
  (R/f'logs/v1975-config-{rom}-test.log').write_text(p.stdout+p.stderr)
  results[rom]={'exit':0,'real_config_init_save_load':True,'missing_mode_Balanced':True,'old_audio_wallpaper_LED_preserved':True,'all_modes_saved_and_reloaded':True,'global_mode_not_overridden_by_profile':True,'invalid_config_values_Balanced':True,'ASan_UBSan':True,'all_60_font_indices_saved_reloaded':True,'independent_per_listener_cover_brightness_defaults_bounds_and_restore':True,'all_50_themes_saved_loaded':True,'last_theme_profile_loaded':True,'invalid_theme_safe':True}
(R/'logs/v1975-config-test.json').write_text(json.dumps(results,indent=2)+'\n')
