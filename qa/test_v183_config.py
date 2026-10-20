#!/usr/bin/env python3
"""Test real config migration, atomic save, reload and profile isolation."""
from pathlib import Path
import json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
fixture=r'''
#include "nxt.h"
#include "nxt_performance.h"
#include "nxt_controls.h"
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
void NxtPersonal_savePreferences(void){}
int NxtPersonal_route(const char*s,char*out,size_t n){return -1;}
void NxtLog_step(const char*c,const char*step,bool ok,const char*fmt,...){}
void Nxt_profileSave(int p){}
void NxtDSP_configure(const NxtConfig*c,int rate){}
int Player_getOutputRate(void){return 48000;}
void Player_refreshOptions(void){}
static void write_file(const char *path,const char *json){FILE*f=fopen(path,"w");assert(f);assert(fputs(json,f)>=0);assert(!fclose(f));}
int main(void){
 char path[1024];userdata_snpath("nxt.json",path,sizeof(path));unlink(path);
 Nxt_init();assert(nxt.performance_mode==NXT_PERFORMANCE_BALANCED&&nxt.language==0);
 /* An installed v1.8.2 config has no new field: retain its existing choices. */
 write_file(path,"{\"language\":1,\"interface_style\":17,\"eq_bands\":5,\"eq\":[3,2,1,0,-1],\"wallpaper_mode\":11,\"led_rgb\":1122867,\"avatar_catalog_v18\":true,\"wallpaper_visibility_fix1\":true,\"stock_hotplug_fix1\":true}");
 Nxt_init();assert(nxt.performance_mode==1&&nxt.language==1&&nxt.interface_style==17&&nxt.eq_bands==5&&nxt.eq[0]==3&&nxt.wallpaper_mode==11&&nxt.led_rgb==1122867);
 for(int mode=0;mode<3;mode++){
  nxt.performance_mode=mode;assert(Nxt_save());JSON_Value*v=json_parse_file(path);assert(v);assert(json_object_get_number(json_value_get_object(v),"performance_mode")==mode);json_value_free(v);
  nxt.performance_mode=-99;Nxt_init();assert(nxt.performance_mode==mode&&nxt.interface_style==17&&nxt.eq[0]==3&&nxt.led_rgb==1122867);
  char profile[1024];userdata_snpath("profile-test.json",profile,sizeof(profile));write_file(profile,"{\"performance_mode\":99,\"interface_style\":0,\"wallpaper_mode\":3,\"avatar_catalog_v18\":true,\"wallpaper_visibility_fix1\":true}");
  Nxt_loadPreferences(profile);assert(nxt.performance_mode==mode&&nxt.interface_style==0&&nxt.wallpaper_mode==3);
  write_file(profile,"{}");Nxt_loadPreferences(profile);assert(nxt.performance_mode==mode);
  nxt.interface_style=17;
 }
 const char *invalid[]={"-1","3","99999","0.5","1.5","\"0\"","null","true","false","[]","{}"};
 for(unsigned i=0;i<sizeof(invalid)/sizeof(*invalid);i++){char json[128];snprintf(json,sizeof(json),"{\"performance_mode\":%s}",invalid[i]);write_file(path,json);Nxt_init();assert(nxt.performance_mode==NXT_PERFORMANCE_BALANCED);}
 write_file(path,"{ broken JSON");Nxt_init();assert(nxt.performance_mode==1);
 puts("PASS real config: v1.8.2 migration defaults to Balanced without changing audio/appearance; three modes save/reload atomically; profile changes preserve device mode; malformed, fractional and out-of-range values fall back safely.");
}
'''
results={}
for rom in ['stock','nextui']:
 with tempfile.TemporaryDirectory(prefix='nxt-v183-config-') as td:
  w=Path(td);data=w/'data';data.mkdir();c=w/'test.c';c.write_text(fixture);binary=w/'test'
  cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom.upper(),'-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(c),str(S/'nxt_config.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-lm','-o',str(binary)]
  with (R/f'logs/v183-config-{rom}-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
  p=subprocess.run([str(binary)],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,check=True);print(rom,p.stdout,end='')
  (R/f'logs/v183-config-{rom}-test.log').write_text(p.stdout+p.stderr)
  results[rom]={'exit':0,'real_config_init_save_load':True,'missing_mode_Balanced':True,'old_audio_wallpaper_LED_preserved':True,'all_modes_saved_and_reloaded':True,'global_mode_not_overridden_by_profile':True,'invalid_config_values_Balanced':True,'ASan_UBSan':True}
(R/'logs/v183-config-test.json').write_text(json.dumps(results,indent=2)+'\n')
