#!/usr/bin/env python3
"""Actual OTA worker/curl child and Drive confirmation with deterministic fixtures."""
from pathlib import Path
import hashlib,json,os,re,subprocess,tempfile,zipfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
VERSION=re.search(r'#define NXT_VERSION "([^"]+)"',(S/'nxt.h').read_text())[1]
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',S/'include/mbedtls_lib',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-ota-flow-') as td:
    w=Path(td);app=w/'Apps/NXT_Retro_Music';app.mkdir(parents=True);(app/'res').mkdir();(app/'res/cacert.pem').write_text('fixture');data=w/'data';data.mkdir();bin_=w/'bin';bin_.mkdir()
    config={'package':'vn.nguyenxuanthong.nxtretromusic','rom':'stock','device':'trimui-brick-pro','version':VERSION,'launch':'launch.sh'};(app/'config.json').write_text(json.dumps(config));(app/'old.txt').write_text('old version');(data/'credentials.json').write_text('PRIVATE_FIXTURE_KEEP_ME')
    (app/'ota-source-stock.url').write_text('https://drive.google.com/file/d/PUBLIC_MANIFEST_ID/view\n');(app/'ota-source-nextui.url').write_text('https://drive.google.com/file/d/DO_NOT_USE_NEXTUI/view\n')
    (data/'ota/stock').mkdir(parents=True);(data/'ota/stock/source.json').write_text(json.dumps({'rom':'nextui','url':'https://drive.google.com/file/d/DO_NOT_USE_NEXTUI/view'}));(data/'ota/source.json').write_text(json.dumps({'rom':'nextui','url':'https://drive.google.com/file/d/DO_NOT_USE_NEXTUI/view'}))
    elf=bytearray(64);elf[:6]=b'\x7fELF\2\1';elf[18]=183;package=w/'package.zip';future={**config,'version':'1.6'}
    with zipfile.ZipFile(package,'w',zipfile.ZIP_DEFLATED) as z:
        prefix='Apps/NXT_Retro_Music/';z.writestr(prefix+'config.json',json.dumps(future));z.writestr(prefix+'launch.sh','#!/bin/sh\nexit 0\n')
        for name in ['musicplayer.elf','nxt-spotify-backend','nxt-soundcloud-backend']:z.writestr(prefix+'bin/tg5040/'+name,bytes(elf))
    manifest={'schema':1,**{k:future[k] for k in ('package','rom','device','version')},'download_url':'https://drive.google.com/file/d/PUBLIC_PACKAGE_ID/view','sha256':hashlib.sha256(package.read_bytes()).hexdigest(),'size':package.stat().st_size,'changelog':'Phiên bản fixture 1.6'};(w/'manifest.json').write_text(json.dumps(manifest))
    (bin_/'curl').write_text('''#!/usr/bin/env python3
import sys,os,time,shutil
from pathlib import Path
from urllib.parse import urlsplit,parse_qs
url=sys.argv[-1];out=Path(sys.argv[sys.argv.index('-o')+1]);q=parse_qs(urlsplit(url).query);id=q.get('id',[''])[0];w=Path(os.environ['QA_OTA_DATA'])
if id=='PUBLIC_MANIFEST_ID':shutil.copyfile(w/'manifest.json',out)
elif id=='PUBLIC_BAD_MANIFEST':out.write_text('<!doctype html><html>sign in</html>')
elif id=='PUBLIC_SLOW_MANIFEST':time.sleep(60)
elif id=='PUBLIC_PACKAGE_ID' and 'uuid' not in q:out.write_text('<!doctype html><html><form action="https://drive.usercontent.google.com/download"><input name="id" value="PUBLIC_PACKAGE_ID"><input name="export" value="download"><input name="confirm" value="t"><input name="uuid" value="fixture-uuid"></form></html>')
elif id=='PUBLIC_PACKAGE_ID':shutil.copyfile(w/'package.zip',out)
else:sys.exit(22)
''');(bin_/'curl').chmod(0o755)
    driver=w/'driver.c';driver.write_text('''#include "nxt_ota.c"
#include <assert.h>
NxtConfig nxt;
ToastToken Toast_show(const char*s,uint32_t ms){return 0;}
const char *Nxt_text(const char *a,const char *b){return a;}
int NxtPersonal_route(const char*s,char*out,size_t n){return -1;}
void mbedtls_platform_zeroize(void*p,size_t n){volatile unsigned char*b=p;while(n--)*b++=0;}
static int ui_mode,ui_calls;
NxtPick Nxt_pickLive(DisplayContext*d,const char*t,const char**rows,int count,int*selected,const char*hint){
 assert(count==4&&strstr(t,"stock")&&strstr(t,NXT_VERSION));assert(++ui_calls<1500);
 if(ui_mode==0)return (NxtPick){2,0};
 if(ui_mode==1){if(state==OTA_AVAILABLE){*selected=0;return (NxtPick){1,0};}if(state==OTA_DOWNLOADING||state==OTA_READY)return (NxtPick){2,0};usleep(10000);return (NxtPick){7,0};}
 assert(ui_mode==2&&state==OTA_READY&&!running&&*stage);return (NxtPick){2,0};
}
NxtPick Nxt_pick(DisplayContext*d,const char*t,const char**rows,int count,int*selected,const char*hint){return (NxtPick){2,*selected};}
ModuleExitReason Nxt_document(DisplayContext*d,const char*t,const char*const*rows,int n){return MODULE_EXIT_TO_MENU;}
char *Keyboard_openLatin(const char*t,size_t max){return NULL;}
bool Nxt_chooseFile(DisplayContext*d,const char*a,const char*b,char*out){return false;}
void Nxt_stop(void){}bool Nxt_save(void){return true;}void ModuleCommon_signalStop(void){}
static void wait_job(void){for(int i=0;i<150;i++){NxtOTA_tick();pthread_mutex_lock(&lock);bool busy=running;pthread_mutex_unlock(&lock);if(!busy){reap(true);return;}usleep(100000);}assert(!"worker timeout");}
int main(int argc,char**argv){NxtOTA_init();assert(*app);assert(strstr(source,"PUBLIC_MANIFEST_ID")&&!strstr(source,"NEXTUI")&&strstr(source_file,"ota/stock/source.json"));assert(!save_source("https://drive.google.com/drive/folders/PUBLIC_FOLDER"));
 ui_mode=0;ui_calls=0;assert(NxtOTA_run(NULL)==MODULE_EXIT_TO_MENU);wait_job();assert(state==OTA_AVAILABLE&&manifest.size>0&&!strcmp(manifest.version,"1.6"));
 ui_mode=1;ui_calls=0;assert(NxtOTA_run(NULL)==MODULE_EXIT_TO_MENU);wait_job();assert(state==OTA_READY&&*stage);char staged_before[2048];snprintf(staged_before,sizeof(staged_before),"%s",stage);ui_mode=2;ui_calls=0;assert(NxtOTA_run(NULL)==MODULE_EXIT_TO_MENU);assert(!strcmp(stage,staged_before)&&state==OTA_READY&&!running&&!joinable);
 assert(save_source("https://drive.google.com/file/d/PUBLIC_MANIFEST_ID/view"));assert(state==OTA_IDLE&&!*stage);assert(start(0));wait_job();assert(start(1));wait_job();if(state!=OTA_READY)fprintf(stderr,"OTA failed: %s\\n",message);assert(state==OTA_READY&&*stage);char error[300];assert(NxtOTA_install(app,stage,error,sizeof(error)));installed=true;stage[0]=0;assert(!access("config.json",F_OK));NxtOTA_quit();
 /* Reopen the old-version test fixture only to test errors/cancellation. */
 assert(!chdir(app));FILE*f=fopen("config.json","w");assert(f);fputs("{\\"package\\":\\"vn.nguyenxuanthong.nxtretromusic\\",\\"rom\\":\\"stock\\",\\"device\\":\\"trimui-brick-pro\\",\\"version\\":\\"__CURRENT_VERSION__\\"}",f);fclose(f);NxtOTA_init();
 assert(save_source("https://drive.google.com/file/d/PUBLIC_BAD_MANIFEST/view"));assert(start(0));wait_job();assert(state==OTA_ERROR);
 assert(save_source("https://drive.google.com/file/d/PUBLIC_SLOW_MANIFEST/view"));assert(start(0));usleep(300000);NxtOTA_quit();assert(!running&&!joinable&&state==OTA_CANCELLED);int result;assert(waitpid(-1,&result,WNOHANG)==-1&&errno==ECHILD);puts("PASS four-row OTA entry auto-check; source/ROM separation and folder rejection; background download; ready-stage preserved on reentry; real OTA worker: manifest → Drive confirmation → download/hash/stage/install/backup; HTML error; cancellation and child reaping.");return 0;}
'''.replace('__CURRENT_VERSION__',VERSION))
    binary=w/'flow';cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(driver),str(S/'nxt_ota_package.c'),str(S/'nxt_drive_parser.c'),str(S/'nxt_log.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),'-Wl,--gc-sections','-l:libzip.so.4','-lpthread','-lm','-o',str(binary)]
    with (R/'logs/v15-ota-flow-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
    result=subprocess.run([str(binary)],cwd=app,env={**os.environ,'PATH':str(bin_)+':'+os.environ['PATH'],'QA_OTA_DATA':str(w),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=30)
    (R/'logs/v15-ota-flow-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
    log=(data/'app-log.txt').read_text();assert 'https://' not in log and 'PRIVATE_FIXTURE_KEEP_ME' not in log;assert (data/'credentials.json').read_text()=='PRIVATE_FIXTURE_KEEP_ME';assert (w/'Apps/.NXT_Retro_Music.previous/old.txt').read_text()=='old version'
(R/'logs/v15-ota-flow-test.json').write_text(json.dumps({'actual_worker':{'exit':0,'compact_ui_rows':4,'entry_auto_check':True,'stock_nextui_default_and_override_separated':True,'ready_stage_preserved_on_reentry':True,'back_keeps_download_running':True,'drive_confirmation':True,'hash_staging_install':True,'html_error':True,'cancel_and_reap':True,'private_data_unchanged':True,'no_urls_tokens_in_log':True,'asan_ubsan':True,'network':'deterministic curl child fixtures; no live publisher URL'}},indent=2)+'\n')
