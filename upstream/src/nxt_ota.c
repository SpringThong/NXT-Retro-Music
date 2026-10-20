/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_ota.h"
#include "nxt_ota_package.h"
#include "nxt_drive_parser.h"
#include "nxt_skin.h"
#include "nxt_log.h"
#include "file_utils.h"
#include "keyboard.h"
#include "toast.h"
#include "include/parson/parson.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
enum{OTA_IDLE,OTA_CHECKING,OTA_AVAILABLE,OTA_CURRENT,OTA_DOWNLOADING,OTA_READY,OTA_ERROR,OTA_CANCELLED,OTA_DONE};
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_t worker;static bool joinable,running,initialized,installed;
static int cancel_flag;
#define cancel_requested __atomic_load_n(&cancel_flag,__ATOMIC_RELAXED)
static int state;static char message[300],source[2048],app[2048],source_file[2048],archive[2048],stage[2048],manifest_file[2048];
static NxtOTAManifest manifest;
static void status(int next,const char *text){pthread_mutex_lock(&lock);state=next;snprintf(message,sizeof(message),"%s",text);pthread_mutex_unlock(&lock);}
static char *read_file(const char *path,size_t limit){FILE *f=fopen(path,"rb");if(!f)return NULL;if(fseek(f,0,SEEK_END)){fclose(f);return NULL;}long len=ftell(f);if(len<0||(size_t)len>limit){fclose(f);return NULL;}rewind(f);char *text=malloc((size_t)len+1);if(text){if(fread(text,1,len,f)!=(size_t)len){free(text);text=NULL;}else text[len]=0;}fclose(f);return text;}
static bool fetch(const char *url,const char *target,uint64_t limit,char *error,size_t capacity){
    if(cancel_requested)return false;unlink(target);char bytes[32],cert[2200];snprintf(bytes,sizeof(bytes),"%llu",(unsigned long long)limit);snprintf(cert,sizeof(cert),"%s/res/cacert.pem",app);
    pid_t child=fork();if(child<0){snprintf(error,capacity,"Cannot start download");return false;}
    if(!child){int null=open("/dev/null",O_WRONLY);if(null>=0){dup2(null,STDOUT_FILENO);dup2(null,STDERR_FILENO);close(null);}const char *args[]={"curl","-sSL","--fail","--proto","=https","--proto-redir","=https","--cacert",cert,"--connect-timeout","15","--max-time","600","--retry","2","--speed-time","60","--speed-limit","128","--max-filesize",bytes,"-A","NXT-Retro-Music-OTA","-o",target,url,NULL};execvp("curl",(char *const *)args);_exit(127);}
    int result=0;pid_t done;while((done=waitpid(child,&result,WNOHANG))==0){if(cancel_requested){kill(child,SIGTERM);for(int i=0;i<10;i++){usleep(100000);done=waitpid(child,&result,WNOHANG);if(done!=0)break;}if(!done){kill(child,SIGKILL);waitpid(child,&result,0);}unlink(target);return false;}usleep(100000);}
    struct stat st;bool ok=done==child&&WIFEXITED(result)&&WEXITSTATUS(result)==0&&!stat(target,&st)&&st.st_size>0&&(uint64_t)st.st_size<=limit;
    if(!ok){unlink(target);snprintf(error,capacity,"Download failed; check Wi-Fi, sharing and quota");}return ok;
}
static bool html_file(const char *path){FILE *f=fopen(path,"rb");if(!f)return false;char bytes[513];size_t n=fread(bytes,1,512,f);fclose(f);bytes[n]=0;char *p=bytes;while(isspace((unsigned char)*p))p++;return !strncasecmp(p,"<!doctype",9)||!strncasecmp(p,"<html",5);}
static bool download(const char *url,const char *target,uint64_t limit,char *error,size_t capacity){
    NxtDriveEntry entry;char direct[2200];bool drive=NxtDrive_parseURL(url,&entry);if(drive){if(entry.folder||!NxtDrive_url(&entry,true,direct,sizeof(direct))){snprintf(error,capacity,"Use a public Drive file, not a folder");return false;}url=direct;}
    if(!fetch(url,target,limit,error,capacity))return false;
    if(html_file(target)){char *text=read_file(target,2*1024*1024);char confirmed[4096];bool confirmation=drive&&text&&NxtDrive_confirmation(text,&entry,confirmed,sizeof(confirmed));free(text);
        if(!confirmation){unlink(target);snprintf(error,capacity,"Drive returned a sign-in/quota/HTML page; make file public");return false;}NxtLog_step("ota","drive-confirmation",true,"Public download confirmation accepted");if(!fetch(confirmed,target,limit,error,capacity))return false;if(html_file(target)){unlink(target);snprintf(error,capacity,"Drive file unavailable or download quota reached");return false;}
    }return true;
}
static void *job(void *argument){
    int task=(int)(intptr_t)argument;char error[300]="",url[2048];pthread_mutex_lock(&lock);snprintf(url,sizeof(url),"%s",source);NxtOTAManifest update=manifest;pthread_mutex_unlock(&lock);bool ok=false;
    if(task==0){NxtLog_step("ota","check",true,"Checking public manifest");ok=download(url,manifest_file,128*1024,error,sizeof(error));char *text=ok?read_file(manifest_file,128*1024):NULL;if(ok)ok=text&&NxtOTA_parseManifest(text,&update,error,sizeof(error));free(text);
        if(ok){pthread_mutex_lock(&lock);manifest=update;pthread_mutex_unlock(&lock);int compare=NxtOTA_compareVersion(update.version,NXT_VERSION);if(compare>0)status(OTA_AVAILABLE,Nxt_text("New version available","Có phiên bản mới"));else status(OTA_CURRENT,Nxt_text("No newer version at this source","Nguồn này chưa có phiên bản mới hơn"));NxtLog_step("ota","manifest",true,"version=%s; newer=%d",update.version,compare>0);}
    }else{
        struct statvfs space;if(statvfs(app,&space)||(uint64_t)space.f_bavail*space.f_frsize<update.size*3+16*1024*1024){snprintf(error,sizeof(error),"Insufficient SD space for ZIP, staging and backup");ok=false;}
        else{NxtLog_step("ota","download",true,"Expected bytes=%llu",(unsigned long long)update.size);ok=download(update.url,archive,update.size,error,sizeof(error));if(ok&&!cancel_requested){status(OTA_DOWNLOADING,Nxt_text("Verifying and preparing update…","Đang kiểm tra và chuẩn bị cập nhật…"));char parent[2048];snprintf(parent,sizeof(parent),"%s",app);char *slash=strrchr(parent,'/');if(slash)*slash=0;char created[2200];snprintf(created,sizeof(created),"%s/.nxt-update-XXXXXX",parent);char *folder=mkdtemp(created);if(!folder){ok=false;snprintf(error,sizeof(error),"Cannot create staging directory");}else{pthread_mutex_lock(&lock);snprintf(stage,sizeof(stage),"%s",folder);pthread_mutex_unlock(&lock);ok=NxtOTA_verifyZip(archive,&update,folder,error,sizeof(error));}}}
        if(ok)status(OTA_READY,Nxt_text("Verified; ready to install","Đã kiểm tra; sẵn sàng cài đặt"));
    }
    if(cancel_requested){if(stage[0]){NxtOTA_removeTree(stage);stage[0]=0;}status(OTA_CANCELLED,Nxt_text("Cancelled; current app unchanged","Đã hủy; giữ app hiện tại"));NxtLog_step("ota","cancel",true,"Download worker stopped");}
    else if(!ok){status(OTA_ERROR,*error?error:Nxt_text("Update failed; export app log","Cập nhật lỗi; hãy xuất log app"));NxtLog_step("ota",task?"download-verify":"manifest",false,"%s",*error?error:"Cannot read manifest");}
    pthread_mutex_lock(&lock);running=false;pthread_mutex_unlock(&lock);return NULL;
}
static void reap(bool wait){
    pthread_mutex_lock(&lock);bool busy=running;pthread_mutex_unlock(&lock);if(joinable&&(wait||!busy)){pthread_join(worker,NULL);joinable=false;}
}
static bool start(int task){reap(false);pthread_mutex_lock(&lock);bool busy=running;pthread_mutex_unlock(&lock);if(busy||joinable)return false;if(task==0&&!NxtOTA_https(source)){status(OTA_ERROR,Nxt_text("Enter a public update.json link first","Nhập link update.json công khai trước"));return false;}
    if(task==0&&stage[0]){NxtOTA_removeTree(stage);stage[0]=0;}
    __atomic_store_n(&cancel_flag,0,__ATOMIC_RELAXED);status(task?OTA_DOWNLOADING:OTA_CHECKING,task?Nxt_text("Downloading update…","Đang tải bản cập nhật…"):Nxt_text("Checking update…","Đang kiểm tra cập nhật…"));pthread_mutex_lock(&lock);running=true;pthread_mutex_unlock(&lock);
    if(pthread_create(&worker,NULL,job,(void*)(intptr_t)task)){pthread_mutex_lock(&lock);running=false;pthread_mutex_unlock(&lock);status(OTA_ERROR,"Cannot start update worker");return false;}joinable=true;return true;
}
static bool save_source_mode(const char *url,bool official){
    if(!NxtOTA_https(url)){Toast_show(Nxt_text("Require public HTTPS file link","Cần link tệp HTTPS công khai"),2500);return false;}NxtDriveEntry entry;if(NxtDrive_parseURL(url,&entry)&&entry.folder){Toast_show(Nxt_text("Use update.json file link, not folder","Dùng link tệp update.json, không dùng thư mục"),2500);return false;}
    JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"url",url);json_object_set_string(o,"rom",NXT_OTA_ROM);json_object_set_string(o,"mode",official?"official":"custom");char temp[2100];snprintf(temp,sizeof(temp),"%s.tmp",source_file);bool ok=json_serialize_to_file_pretty(v,temp)==JSONSuccess&&!rename(temp,source_file);json_value_free(v);if(ok){chmod(source_file,0600);pthread_mutex_lock(&lock);snprintf(source,sizeof(source),"%s",url);memset(&manifest,0,sizeof(manifest));pthread_mutex_unlock(&lock);if(stage[0])NxtOTA_removeTree(stage);stage[0]=0;status(OTA_IDLE,Nxt_text("Source saved","Đã lưu nguồn cập nhật"));}NxtLog_step("ota","source-save",ok,"Public manifest source configured for %s",NXT_OTA_ROM);return ok;
}
static bool save_source(const char *url){return save_source_mode(url,false);}
static bool source_url(const char *url){NxtDriveEntry entry;return NxtOTA_https(url)&&(!NxtDrive_parseURL(url,&entry)||!entry.folder);}
static bool default_source(char *out,size_t size){char *text=read_file(NXT_OTA_DEFAULT_FILE,4096);if(!text)return false;text[strcspn(text,"\r\n")]=0;bool ok=source_url(text);if(ok)snprintf(out,size,"%s",text);free(text);return ok;}
static void enter_check(void){pthread_mutex_lock(&lock);bool check=source[0]&&!running&&state!=OTA_READY;pthread_mutex_unlock(&lock);if(check)start(0);}
void NxtOTA_init(void){
    if(initialized)return;initialized=true;installed=false;source[0]=0;memset(&manifest,0,sizeof(manifest));userdata_mkdir(NXT_OTA_DATA_DIR);userdata_snpath(NXT_OTA_DATA_DIR "/source.json",source_file,sizeof(source_file));userdata_snpath(NXT_OTA_DATA_DIR "/package.zip",archive,sizeof(archive));userdata_snpath(NXT_OTA_DATA_DIR "/update.json",manifest_file,sizeof(manifest_file));if(!getcwd(app,sizeof(app))){status(OTA_ERROR,"Cannot locate installed app");return;}const char *basename=strrchr(app,'/');if(!basename||strcmp(basename+1,NXT_OTA_APP_BASENAME)){app[0]=0;status(OTA_ERROR,"OTA requires the official app folder for this ROM");return;}
    char path[2200];snprintf(path,sizeof(path),"%s/config.json",app);JSON_Value *v=json_parse_file(path);JSON_Object *o=json_value_get_object(v);bool valid=o;const char *keys[]={"package","rom","device","version"},*values[]={NXT_OTA_PACKAGE,NXT_OTA_ROM,NXT_OTA_DEVICE,NXT_VERSION};for(int i=0;i<4&&valid;i++){const char *s=json_object_get_string(o,keys[i]);valid=s&&!strcmp(s,values[i]);}json_value_free(v);
    if(!valid){app[0]=0;status(OTA_ERROR,"OTA requires an installed app matching this ROM build");NxtLog_step("ota","target",false,"Current installed config does not match this build");return;}
    /* v1.8.9: unmarked legacy sources move to the ROM's official publisher
     * file. A later explicit custom source remains an opt-in override.
     * Official users always follow the bundled publisher link on upgrades. */
    char saved[2048]="",preset[2048]="";bool custom=false;
    v=json_parse_file(source_file);o=json_value_get_object(v);
    const char *s=o?json_object_get_string(o,"url"):NULL,*rom=o?json_object_get_string(o,"rom"):NULL;
    const char *mode=o?json_object_get_string(o,"mode"):NULL;
    if(s&&rom&&!strcmp(rom,NXT_OTA_ROM)&&source_url(s)){snprintf(saved,sizeof(saved),"%s",s);custom=mode&&!strcmp(mode,"custom");}
    json_value_free(v);
    if(custom)snprintf(source,sizeof(source),"%s",saved);
    else if(default_source(preset,sizeof(preset))){
        snprintf(source,sizeof(source),"%s",preset);
        /* Persistence failure must not make the old source win in memory. */
        if(!save_source_mode(preset,true))NxtLog_step("ota","source-migrate",false,"Official source active; cannot persist marker");
    }else if(*saved)snprintf(source,sizeof(source),"%s",saved);

    status(OTA_IDLE,source[0]?Nxt_text("Open Update to check automatically","Mở Cập nhật để tự kiểm tra"):Nxt_text("Publisher's update.json link is required","Cần link update.json của người phát hành"));NxtLog_step("ota","init",true,"ROM=%s; source configured=%d",NXT_OTA_ROM,source[0]!=0);
}
void NxtOTA_tick(void){reap(false);}
void NxtOTA_quit(void){__atomic_store_n(&cancel_flag,1,__ATOMIC_RELAXED);reap(true);if(stage[0]&&!installed)NxtOTA_removeTree(stage);stage[0]=0;initialized=false;}
ModuleExitReason NxtOTA_guide(DisplayContext *d){
    const char *rows[]={
        Nxt_text("Open About, Help & Update → Software update. The app checks this ROM's saved/publisher source automatically; it does not download or install without your choice.","Mở Thông tin, HDSD & cập nhật → Cập nhật phần mềm. App tự kiểm tra nguồn của ROM này; chọn Tải rồi Cài để thực hiện."),
        Nxt_text("The screen shows your version, the new version, download size and release notes. Choose Download, wait for verification, then choose Install. B returns while a download continues; open Update again to see progress. A on the first row cancels a running check/download.","Màn hình hiện bản hiện tại, bản mới, dung lượng và thay đổi. Chọn Tải, chờ kiểm tra rồi chọn Cài. B quay lại và tiếp tục tải nền; mở lại Cập nhật để xem tiến trình. A ở dòng đầu hủy bước kiểm tra/tải đang chạy."),
        Nxt_text("Install stops music and closes the app. Reopen NXT Retro Music. Your profiles, preferences, favorites, history and credentials stay outside the replaced app folder; a previous-app backup is saved.","Cài đặt sẽ dừng nhạc và đóng app. Mở lại NXT Retro Music. Hồ sơ, cài đặt, yêu thích, lịch sử và đăng nhập nằm ngoài thư mục app được thay; app cũng lưu bản cũ."),
        Nxt_text("Official Stock/NextUI sources are supplied and legacy sources migrate automatically. You may explicitly select a custom public update.json file or restore the official source. Sources and package checks stay separate per ROM.","Stock/NextUI có sẵn nguồn chính thức và tự chuyển nguồn cũ. Có thể chọn nguồn TỆP update.json riêng hoặc khôi phục nguồn chính thức. Nguồn và gói cài kiểm tra riêng theo ROM."),
        Nxt_text("For the publisher: upload the installer ZIP publicly. In the supplied update-v" NXT_VERSION ".json change download_url to that ZIP file link; save as update.json and make it public. Its version, SHA-256 and size match the supplied installer, not a source or OTA-tool ZIP.","Người phát hành: tải ZIP cài đặt lên Drive công khai. Trong update-v" NXT_VERSION ".json gửi kèm, đổi download_url thành link TỆP ZIP đó; lưu tên update.json và chia sẻ công khai. Version, SHA-256 và size khớp ZIP cài đặt, không dùng ZIP mã nguồn/công cụ OTA."),
        Nxt_text("For future releases, use Manage versions → Upload new version on the same update.json file to keep its ID/link. Update version, ZIP download_url, size and sha256 from the new installer. The Stock JSON must keep rom=stock; NextUI needs its own JSON file/link.","Các bản sau dùng Quản lý phiên bản → Tải phiên bản mới trên chính tệp update.json để giữ ID/link. Sửa version, download_url của ZIP, size và sha256 theo bản cài mới. JSON Stock giữ rom=stock; NextUI dùng tệp JSON/link riêng."),
        Nxt_text("The app verifies package, ROM, device, ZIP size, SHA-256, archive paths and ARM64 binaries. Use your trusted publisher's source; the manifest is unsigned. Keep power connected while installing. If interrupted, restore the previous-app backup beside the app. On sharing/quota/download errors, export NXT_App_Log.txt from About START.","App kiểm tra tên app, ROM, máy, dung lượng ZIP, SHA-256, đường dẫn và binary ARM64. Dùng nguồn nhà phát hành bạn tin cậy; manifest chưa có chữ ký. Giữ nguồn khi cài. Nếu gián đoạn, khôi phục thư mục sao lưu cạnh app. Khi lỗi chia sẻ/hạn mức/tải, START ở Thông tin xuất NXT_App_Log.txt.")
    };return Nxt_document(d,Nxt_text("OTA guide","Hướng dẫn cập nhật OTA"),rows,7);
}
static ModuleExitReason source_menu(DisplayContext *d){
    int selected=0;for(;;){char preset[2048];bool has_default=default_source(preset,sizeof(preset));
        const char *rows[]={Nxt_text("Enter update.json file link","Nhập link tệp update.json"),Nxt_text("Import link from .txt / .url","Đọc link từ tệp .txt / .url"),has_default?Nxt_text("Restore publisher's source","Khôi phục nguồn nhà phát hành"):Nxt_text("Publisher's source not supplied yet","Chưa được cung cấp nguồn nhà phát hành"),Nxt_text("OTA guide","Hướng dẫn OTA")};
        NxtPick p=Nxt_pick(d,Nxt_text("Update source · " NXT_OTA_ROM,"Nguồn cập nhật · " NXT_OTA_ROM),rows,4,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));
        if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action!=1)continue;bool changed=false;
        if(selected==0){char *url=Keyboard_openLatin(Nxt_text("Public update.json FILE link","Link TỆP update.json công khai"),2000);if(url){changed=save_source(url);free(url);}}
        else if(selected==1){char path[512];if(Nxt_chooseFile(d,".txt",".url",path)){char *text=read_file(path,4096);if(text){char *url=strstr(text,"https://");if(url){url[strcspn(url,"\r\n \t")]=0;changed=save_source(url);}else Toast_show(Nxt_text("No HTTPS link in this file","Không có link HTTPS trong tệp"),2500);free(text);}}}
        else if(selected==2){if(has_default)changed=save_source_mode(preset,true);else Toast_show(Nxt_text("Publisher's link is still required","Chưa có link cập nhật của người phát hành"),2200);}
        else if(selected==3&&NxtOTA_guide(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
        if(changed){enter_check();return MODULE_EXIT_TO_MENU;}
    }
}
ModuleExitReason NxtOTA_run(DisplayContext *d){
    if(!initialized)NxtOTA_init();if(!*app)return NxtOTA_guide(d);enter_check();int selected=0;
    for(;;){reap(false);pthread_mutex_lock(&lock);int current=state;bool busy=running,has_source=source[0]!=0;NxtOTAManifest update=manifest;char line[300];snprintf(line,sizeof(line),"%s",message);pthread_mutex_unlock(&lock);
        char primary[700],heading[140],notes[400];struct stat st;uint64_t loaded=!stat(archive,&st)&&st.st_size>0?(uint64_t)st.st_size:0;
        snprintf(heading,sizeof(heading),"OTA %s · %s%s%s",NXT_OTA_ROM,NXT_VERSION,*update.version?" → ":"",update.version);
        if(busy){if(current==OTA_DOWNLOADING&&update.size)snprintf(primary,sizeof(primary),"%s\n%s · %.1f / %.1f MiB",Nxt_text("Cancel download","Hủy tải cập nhật"),line,loaded/1048576.,update.size/1048576.);else snprintf(primary,sizeof(primary),"%s\n%s",Nxt_text("Cancel check","Hủy kiểm tra"),line);}
        else if(current==OTA_AVAILABLE)snprintf(primary,sizeof(primary),"%s %s\n%.1f MiB · %s",Nxt_text("Download version","Tải phiên bản"),update.version,update.size/1048576.,Nxt_text("Verify before installing","Kiểm tra trước khi cài"));
        else if(current==OTA_READY)snprintf(primary,sizeof(primary),"%s %s\n%s",Nxt_text("Install version","Cài phiên bản"),update.version,line);
        else snprintf(primary,sizeof(primary),"%s\n%s",has_source?Nxt_text("Check again","Kiểm tra lại"):Nxt_text("Set up update source","Thiết lập nguồn cập nhật"),line);
        snprintf(notes,sizeof(notes),"%s\n%s",Nxt_text("Release notes","Thay đổi bản mới"),*update.notes?Nxt_text("Open the full changelog","Mở nội dung thay đổi đầy đủ"):Nxt_text("Available after checking","Có sau khi kiểm tra"));
        const char *rows[]={primary,notes,Nxt_text("Update source\nPublisher link / enter / import","Nguồn cập nhật\nLink nhà phát hành / nhập / đọc tệp"),Nxt_text("OTA guide\nFor listeners and the publisher","Hướng dẫn OTA\nDành cho người nghe và người phát hành")};
        NxtPick p=Nxt_pickLive(d,heading,rows,4,&selected,Nxt_text("A Open · X Recheck · B Back","A Mở · X Kiểm tra lại · B Về"));if(p.action==2)return MODULE_EXIT_TO_MENU;if(p.action==9)return MODULE_EXIT_QUIT;if(p.action!=1&&p.action!=3)continue;
        pthread_mutex_lock(&lock);current=state;busy=running;update=manifest;pthread_mutex_unlock(&lock);
        if(p.action==3){if(busy)Toast_show(Nxt_text("Wait or cancel the current step","Chờ hoặc hủy bước đang chạy"),1800);else if(current==OTA_READY)Toast_show(Nxt_text("Verified package is ready; install or change source","Gói đã kiểm tra sẵn sàng; hãy cài hoặc đổi nguồn"),2500);else if(has_source)start(0);else if(source_menu(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;continue;}
        if(busy&&selected!=0&&selected!=3){Toast_show(Nxt_text("Wait for current step or cancel","Chờ bước hiện tại hoặc hủy"),2000);continue;}
        if(selected==0){
            if(busy)__atomic_store_n(&cancel_flag,1,__ATOMIC_RELAXED);
            else if(current==OTA_AVAILABLE)start(1);
            else if(current==OTA_READY){
                int choice=1;const char *options[]={Nxt_text("Install now · stop music and close app","Cài ngay · dừng nhạc và đóng app"),Nxt_text("Back","Quay lại")};
                NxtPick confirm=Nxt_pick(d,heading,options,2,&choice,Nxt_text("A Confirm · B Cancel","A Xác nhận · B Hủy"));if(confirm.action==9)return MODULE_EXIT_QUIT;
                if(confirm.action==1&&choice==0){Nxt_stop();Nxt_save();char error[300];if(NxtOTA_install(app,stage,error,sizeof(error))){installed=true;stage[0]=0;status(OTA_DONE,"Installed");ModuleCommon_signalStop();return MODULE_EXIT_QUIT;}status(OTA_ERROR,error);NxtLog_step("ota","install",false,"%s",error);}
            }else if(has_source)start(0);else if(source_menu(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
        }else if(selected==1){const char *paragraphs[]={*update.notes?update.notes:Nxt_text("No update notes yet","Chưa có nội dung thay đổi")};if(Nxt_document(d,Nxt_text("Release notes","Thay đổi bản mới"),paragraphs,1)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
        else if(selected==2){if(source_menu(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
        else if(selected==3&&NxtOTA_guide(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
    }
}
