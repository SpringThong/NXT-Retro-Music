"""Production OTA initialization with each exact source from the final installer."""
from pathlib import Path
import json,os,subprocess,tempfile,zipfile,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',S/'include/mbedtls_lib',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
proof={}
for rom,label,prefix in [('stock','Stock','Apps/NXT_Retro_Music/'),('nextui','NextUI','Tools/tg5040/NXT Retro Music.pak/'),('nxredux','NXRedux','Tools/NXT Retro Music.pak/')]:
 with tempfile.TemporaryDirectory(prefix='nxt-v189-official-'+rom+'-') as td:
  w=Path(td);app=w/prefix;app.mkdir(parents=True);data=w/'data';data.mkdir();package=R/f'dist/{rom}-release/NXT_Retro_Music_v1.8.9_{label}.zip'
  with zipfile.ZipFile(package) as z:
   for n in ['config.json',f'ota-source-{rom}.url']:(app/n).write_bytes(z.read(prefix+n))
  expected=(app/f'ota-source-{rom}.url').read_text().strip()
  (data/'credentials.json').write_text('KEEP_CREDENTIALS');(data/'translation-service.json').write_text('{"provider":2,"groq_key":"FAKE_KEEP_GROQ","google_key":"FAKE_KEEP_GOOGLE"}')
  keep={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in data.iterdir()}
  driver=w/'driver.c';driver.write_text('''#include "nxt_ota.c"
#include <assert.h>
NxtConfig nxt;
const char*Nxt_text(const char*a,const char*b){return a;}
ToastToken Toast_show(const char*s,uint32_t ms){return 0;}
int NxtPersonal_route(const char*s,char*out,size_t n){return -1;}
void mbedtls_platform_zeroize(void*p,size_t n){volatile unsigned char*b=p;while(n--)*b++=0;}
static void saved(const char*url,const char*rom,const char*mode){JSON_Value*v=json_value_init_object();JSON_Object*o=json_value_get_object(v);json_object_set_string(o,"url",url);json_object_set_string(o,"rom",rom);if(mode)json_object_set_string(o,"mode",mode);assert(json_serialize_to_file(v,source_file)==JSONSuccess);json_value_free(v);}
int main(void){
 const char*expected=getenv("QA_EXPECTED");assert(expected&&*expected);
 NxtOTA_init();assert(*app&&!strcmp(source,expected));
 JSON_Value*v=json_parse_file(source_file);assert(!strcmp(json_object_get_string(json_value_get_object(v),"mode"),"official"));json_value_free(v);NxtOTA_quit();
 saved("https://example.com/old-v15.json",NXT_OTA_ROM,NULL);NxtOTA_init();assert(!strcmp(source,expected));NxtOTA_quit();
 saved("https://example.com/wrong-rom.json",!strcmp(NXT_OTA_ROM,"stock")?"nextui":"stock","custom");NxtOTA_init();assert(!strcmp(source,expected));
 assert(save_source("https://example.com/explicit-custom.json"));NxtOTA_quit();NxtOTA_init();assert(!strcmp(source,"https://example.com/explicit-custom.json"));
 assert(save_source_mode(expected,true));NxtOTA_quit();NxtOTA_init();assert(!strcmp(source,expected));
 char original[4096];snprintf(original,sizeof(original),"%s",expected);NxtOTA_quit();FILE*f=fopen(NXT_OTA_DEFAULT_FILE,"w");assert(f);fputs("https://example.com/future-official.json\\n",f);fclose(f);NxtOTA_init();assert(!strcmp(source,"https://example.com/future-official.json"));NxtOTA_quit();
 f=fopen(NXT_OTA_DEFAULT_FILE,"w");assert(f);fprintf(f,"%s\\n",original);fclose(f);NxtOTA_init();assert(!strcmp(source,expected));NxtOTA_quit();
 puts("PASS exact official source: fresh/legacy/wrong-ROM/custom/restore/future-link migration; no network or private-data changes");return 0;
}
''')
  b=w/'test';cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom.upper(),'-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(driver),str(S/'nxt_ota_package.c'),str(S/'nxt_drive_parser.c'),str(S/'nxt_log.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libzip.so.4','-lpthread','-lm','-o',str(b)]
  with (R/f'logs/v189-official-source-build-{rom}.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
  result=subprocess.run([str(b)],cwd=app,env={**os.environ,'QA_EXPECTED':expected,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=20)
  (R/f'logs/v189-official-source-{rom}.log').write_text(result.stdout+result.stderr);print(rom,result.stdout+result.stderr);result.check_returncode()
  for name,sha in keep.items():assert hashlib.sha256((data/name).read_bytes()).hexdigest()==sha
  proof[rom]={'exit':0,'actual_installer_source':expected,'fresh_and_legacy_source_migration':True,'custom_opt_in_and_restore':True,'future_official_change':True,'no_network_requests':True,'private_data_preserved':True,'ASan_UBSan':True}
(R/'logs/v189-official-source-test.json').write_text(json.dumps({'exit':0,'roms':proof},indent=2)+'\n')
