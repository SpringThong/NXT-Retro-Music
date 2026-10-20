#!/usr/bin/env python3
"""Test the actual v1.8 ZIP via production OTA worker, with a v1.7.1 installed-version fixture."""
from pathlib import Path
import hashlib,json,os,subprocess,tempfile,shutil
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';config=json.loads((R/'SOURCE.json').read_text());ROM=os.environ.get('NXT_TEST_ROM') or ('stock' if config['build_define']=='NXT_ROM_STOCK' else 'nextui');LABEL='Stock' if ROM=='stock' else 'NextUI';OTHER='NextUI' if ROM=='stock' else 'Stock'
release=R/'dist'/f'OTA_Google_Drive_v1.8_{LABEL}'
package=R/f'dist/{ROM}-release/NXT_Retro_Music_v1.8_{LABEL}.zip'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',S/'include/mbedtls_lib',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-v18-ota-') as td:
 w=Path(td);app=w/('Apps/NXT_Retro_Music' if ROM=='stock' else 'Tools/tg5040/NXT Retro Music.pak');app.mkdir(parents=True);(app/'res').mkdir();shutil.copy2(R/'upstream/res/cacert.pem',app/'res/cacert.pem');data=w/'data';data.mkdir();bin_=w/'bin';bin_.mkdir()
 old={'package':'vn.nguyenxuanthong.nxtretromusic','rom':ROM,'device':'trimui-brick-pro','version':'1.7.1','launch':'launch.sh'};(app/'config.json').write_text(json.dumps(old));(app/'old-version.txt').write_text('1.7.1')
 (data/'credentials.json').write_text('PRIVATE_TEST_KEEP');(data/'listeners/0').mkdir(parents=True);(data/'listeners/0/appearance.json').write_text('{"theme":1,"panel_opacity":25}');(data/'wallpapers/files').mkdir(parents=True);(data/'wallpapers/files/keep.png').write_bytes(b'OFFLINE_WALLPAPER_KEEP')
 keep={str(p.relative_to(data)):hashlib.sha256(p.read_bytes()).hexdigest() for p in data.rglob('*') if p.is_file()}
 (app/f'ota-source-{ROM}.url').write_text('https://drive.google.com/file/d/PUBLIC_MANIFEST_ID/view\n');(app/f'ota-source-{OTHER.lower()}.url').write_text('https://drive.google.com/file/d/WRONG_ROM_ID/view\n')
 manifest=json.loads((release/'update.json').read_text());assert manifest['rom']==ROM and manifest['size']==package.stat().st_size and manifest['sha256']==hashlib.sha256(package.read_bytes()).hexdigest();manifest['download_url']='https://drive.google.com/file/d/PUBLIC_PACKAGE_ID/view';(w/'manifest.json').write_text(json.dumps(manifest));other=json.loads((release/'update.json').read_text());other['rom']=OTHER.lower();(w/'other.json').write_text(json.dumps(other))
 (bin_/'curl').write_text('''#!/usr/bin/env python3
import sys,os,time,shutil
from pathlib import Path
from urllib.parse import urlsplit,parse_qs
q=parse_qs(urlsplit(sys.argv[-1]).query);id=q.get('id',[''])[0];out=Path(sys.argv[sys.argv.index('-o')+1]);w=Path(os.environ['QA_OTA_DATA'])
if id=='PUBLIC_MANIFEST_ID':shutil.copyfile(w/'manifest.json',out)
elif id=='PUBLIC_OTHER_MANIFEST':shutil.copyfile(w/'other.json',out)
elif id=='PUBLIC_SLOW_MANIFEST':time.sleep(30)
elif id=='PUBLIC_BAD_MANIFEST':out.write_text('<!doctype html><html>sign in</html>')
elif id=='PUBLIC_PACKAGE_ID' and 'uuid' not in q:out.write_text('<!doctype html><html><form action="https://drive.usercontent.google.com/download"><input name="id" value="PUBLIC_PACKAGE_ID"><input name="export" value="download"><input name="confirm" value="t"><input name="uuid" value="fixture-uuid"></form></html>')
elif id=='PUBLIC_PACKAGE_ID':shutil.copyfile(os.environ['QA_OTA_PACKAGE'],out)
else:sys.exit(22)
''');(bin_/'curl').chmod(0o755)
 driver=w/'driver.c';driver.write_text('''#include "nxt.h"
/* Only this test fixture emulates the installed v1.7.1 version. The release binary remains v1.8. */
#undef NXT_VERSION
#define NXT_VERSION "1.7.1"
#include "nxt_ota.c"
#include <assert.h>
NxtConfig nxt;
ToastToken Toast_show(const char*s,uint32_t ms){return 0;}
const char *Nxt_text(const char*a,const char*b){return a;}
int NxtPersonal_route(const char*s,char*out,size_t n){return -1;}
void mbedtls_platform_zeroize(void*p,size_t n){volatile unsigned char*b=p;while(n--)*b++=0;}
NxtPick Nxt_pickLive(DisplayContext*d,const char*t,const char**rows,int n,int*sel,const char*hint){assert(n==4&&strstr(t,NXT_OTA_ROM));return(NxtPick){2,0};}
NxtPick Nxt_pick(DisplayContext*d,const char*t,const char**rows,int n,int*sel,const char*hint){return(NxtPick){2,0};}
ModuleExitReason Nxt_document(DisplayContext*d,const char*t,const char*const*rows,int n){return MODULE_EXIT_TO_MENU;}
char *Keyboard_openLatin(const char*t,size_t n){return NULL;}
bool Nxt_chooseFile(DisplayContext*d,const char*a,const char*b,char*out){return false;}
void Nxt_stop(void){}bool Nxt_save(void){return true;}void ModuleCommon_signalStop(void){}
static void done(void){for(int i=0;i<900;i++){NxtOTA_tick();pthread_mutex_lock(&lock);bool b=running;pthread_mutex_unlock(&lock);if(!b){reap(true);return;}usleep(50000);}assert(!"OTA timeout");}
int main(void){
 NxtOTA_init();assert(*app&&strstr(source,"PUBLIC_MANIFEST_ID")&&!strstr(source,"WRONG_ROM"));assert(!save_source("https://drive.google.com/drive/folders/PUBLIC_FOLDER"));assert(NxtOTA_compareVersion("1.8","1.7.1")>0);
 assert(NxtOTA_run(NULL)==MODULE_EXIT_TO_MENU);done();assert(state==OTA_AVAILABLE&&!strcmp(manifest.version,"1.8"));
 char error[400];NxtOTAManifest wrong=manifest;wrong.size++;assert(!NxtOTA_verifyZip(getenv("QA_OTA_PACKAGE"),&wrong,"invalid-stage",error,sizeof(error)));wrong=manifest;wrong.sha256[0]=wrong.sha256[0]=='0'?'1':'0';assert(!NxtOTA_verifyZip(getenv("QA_OTA_PACKAGE"),&wrong,"invalid-stage",error,sizeof(error)));
 assert(start(1));done();if(state!=OTA_READY)fprintf(stderr,"%s\\n",message);assert(state==OTA_READY&&*stage);char before[2048];snprintf(before,sizeof(before),"%s",stage);assert(NxtOTA_run(NULL)==MODULE_EXIT_TO_MENU&&state==OTA_READY&&!strcmp(before,stage));
 assert(NxtOTA_install(app,stage,error,sizeof(error)));installed=true;stage[0]=0;NxtOTA_quit();char path[4096];snprintf(path,sizeof(path),"%s/config.json",app);JSON_Value*v=json_parse_file(path);assert(!strcmp(json_object_get_string(json_value_get_object(v),"version"),"1.8"));json_value_free(v);snprintf(path,sizeof(path),"%s/res/icons/images.png",app);assert(!access(path,F_OK));snprintf(path,sizeof(path),"%s/res/fonts/notosans.ttf",app);assert(!access(path,F_OK));snprintf(path,sizeof(path),"%s/bin/tg5040/musicplayer.elf",app);assert(!access(path,X_OK));snprintf(path,sizeof(path),"%s/bin/usb-transfer.sh",app);assert(!access(path,X_OK));
 /* Reopen a v1.7.1 config only to exercise cancellation/error paths of the unchanged worker. */
 assert(!chdir(app));FILE*f=fopen("config.json","w");assert(f);fputs("{\\"package\\":\\"vn.nguyenxuanthong.nxtretromusic\\",\\"rom\\":\\"__ROM__\\",\\"device\\":\\"trimui-brick-pro\\",\\"version\\":\\"1.7.1\\"}",f);fclose(f);NxtOTA_init();
 assert(save_source("https://drive.google.com/file/d/PUBLIC_OTHER_MANIFEST/view"));assert(start(0));done();assert(state==OTA_ERROR);assert(save_source("https://drive.google.com/file/d/PUBLIC_BAD_MANIFEST/view"));assert(start(0));done();assert(state==OTA_ERROR);
 assert(save_source("https://drive.google.com/file/d/PUBLIC_SLOW_MANIFEST/view"));assert(start(0));usleep(300000);NxtOTA_quit();assert(!running&&!joinable&&state==OTA_CANCELLED);int child;assert(waitpid(-1,&child,WNOHANG)==-1&&errno==ECHILD);
 puts("PASS actual v1.8 ZIP: auto-check from v1.7.1 fixture, Drive confirmation, size/SHA rejection, download/stage/install, preserved ready state, old-app backup, other-ROM/HTML rejection, cancellation/reaping; ASan/UBSan.");return 0;}
'''.replace('__ROM__',ROM))
 binary=w/'flow';cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+ROM.upper(),'-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(driver),str(S/'nxt_ota_package.c'),str(S/'nxt_drive_parser.c'),str(S/'nxt_log.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libzip.so.4','-lpthread','-lm','-o',str(binary)]
 with (R/f'logs/v18-ota-qa-build-{ROM}.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 result=subprocess.run([str(binary)],cwd=app,env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'QA_OTA_DATA':str(w),'QA_OTA_PACKAGE':str(package),'PATH':str(bin_)+':'+os.environ['PATH'],'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=90)
 (R/f'logs/v18-ota-qa-{ROM}.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 for path,sha in keep.items():assert hashlib.sha256((data/path).read_bytes()).hexdigest()==sha,path
 backup=app.parent/('.NXT_Retro_Music.previous' if ROM=='stock' else '.NXT_Retro_Music.previous.pak');assert (backup/'old-version.txt').read_text()=='1.7.1'
 log=(data/'app-log.txt').read_text();assert 'https://' not in log and 'PRIVATE_TEST_KEEP' not in log
(R/f'logs/v18-ota-qa-{ROM}.json').write_text(json.dumps({'version':'1.8','rom':ROM,'exit':0,'installer_sha256':hashlib.sha256(package.read_bytes()).hexdigest(),'OTA_source_sha256':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_ota.c','nxt_ota_package.c','nxt_ota_package.h']},'actual_v18_full_installer':True,'baseline_installed_version_fixture':'1.7.1','production_worker_and_installer':True,'drive_download_confirmation_fixture':True,'sha_size_rom_checks':True,'userdata_and_wallpaper_preserved':True,'previous_app_backup':True,'asan_ubsan':True,'publisher_live_url_tested':False,'hardware_tested':False},indent=2)+'\n')
