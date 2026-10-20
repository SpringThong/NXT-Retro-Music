/* SPDX-License-Identifier: GPL-3.0-only
 * Public Google Drive links. Audio streams through RAM into the shared
 * decoder/DSP/DAC engine; only saved links and names are persisted. */
#include "nxt_drive.h"
#include "nxt_drive_parser.h"
#include "nxt_sort.h"
#include "nxt_skin.h"
#include "nxt_decoder.h"
#include "nxt_text.h"
#include "keyboard.h"
#include "browser.h"
#include "file_utils.h"
#include "module_player.h"
#include "playlist_m3u.h"
#include "background.h"
#include "toast.h"
#include "parson/parson.h"
#include "api.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DRIVE_LINKS 64
#define DRIVE_ENTRIES 4096
typedef struct { char url[512],name[256]; } DriveLink;
static DriveLink links[DRIVE_LINKS];static int link_count;
static char data_dir[1024],streams_dir[1024],queues_dir[1024],links_file[1024];

static void paths(void) {
    if(*data_dir)return;
    userdata_snpath("drive",data_dir,sizeof(data_dir));mkdir(data_dir,0700);
    userdata_snpath("drive/streams",streams_dir,sizeof(streams_dir));mkdir(streams_dir,0700);
    userdata_snpath("drive/queues",queues_dir,sizeof(queues_dir));mkdir(queues_dir,0700);
    userdata_snpath("drive/links.json",links_file,sizeof(links_file));
}
bool NxtDrive_isStream(const char *path) {
    return path&&!strncmp(path,"nxtdrive://",11);
}
bool NxtDrive_isPath(const char *path) {
    return NxtDrive_isStream(path)||(path&&strstr(path,"/nxt-retro-music/drive/cache/")!=NULL);
}
/* Stable playlist URIs resolve to public links, never arbitrary filesystem or
 * network targets. The small JSON record survives restart/resume. */
bool NxtDrive_streamEntry(const char *path,NxtDriveEntry *out) {
    if(!NxtDrive_isStream(path)||!out)return false;
    const char *id=path+11,*end=strchr(id,'/');if(!end||strncmp(end,"/audio.",7))return false;
    size_t n=end-id;if(n<10||n>=128||!Browser_isAudioFile(end+1))return false;
    char url[512],record[1200],dir[1024];
    int size=snprintf(url,sizeof(url),"https://drive.google.com/file/d/%.*s/view",(int)n,id);
    NxtDriveEntry check;if(size<0||size>=(int)sizeof(url)||!NxtDrive_parseURL(url,&check))return false;
    userdata_snpath("drive/streams",dir,sizeof(dir));snprintf(record,sizeof(record),"%s/%s.json",dir,check.id);
    JSON_Value *v=json_parse_file(record);JSON_Object *o=json_value_get_object(v);
    const char *saved=json_object_get_string(o,"url"),*name=json_object_get_string(o,"name");
    bool ok=NxtDrive_parseURL(saved,out)&&!out->folder&&!strcmp(check.id,out->id)&&name&&Browser_isAudioFile(name);
    if(ok)NxtText_copy(out->name,name,sizeof(out->name));json_value_free(v);return ok;
}
static bool save_stream(const NxtDriveEntry *entry,char *uri,size_t capacity) {
    paths();char url[512],record[1200],tmp[1250];
    const char *extension=strrchr(entry->name,'.');
    if(entry->folder||!extension||!Browser_isAudioFile(entry->name)||!NxtDrive_url(entry,false,url,sizeof(url)))return false;
    int n=snprintf(uri,capacity,"nxtdrive://%s/audio%s",entry->id,extension);if(n<0||(size_t)n>=capacity)return false;
    snprintf(record,sizeof(record),"%s/%s.json",streams_dir,entry->id);snprintf(tmp,sizeof(tmp),"%s.new",record);
    JSON_Value *v=json_value_init_object();if(!v)return false;JSON_Object *o=json_value_get_object(v);
    json_object_set_string(o,"url",url);json_object_set_string(o,"name",entry->name);
    bool ok=json_serialize_to_file(v,tmp)==JSONSuccess;json_value_free(v);
    if(ok){chmod(tmp,0600);ok=rename(tmp,record)==0;}if(!ok)unlink(tmp);return ok;
}
void NxtDrive_metadata(const char *path,TrackInfo *info) {
    if(!NxtDrive_isPath(path))return;
    char name[256]="";NxtDriveEntry entry;
    if(NxtDrive_isStream(path)){if(NxtDrive_streamEntry(path,&entry))NxtText_copy(name,entry.name,sizeof(name));}
    else {char name_path[1100];snprintf(name_path,sizeof(name_path),"%s.name",path);FILE *f=fopen(name_path,"r");if(f){if(fgets(name,sizeof(name),f))name[strcspn(name,"\r\n")]=0;fclose(f);}}
    const char *base=strrchr(path,'/');base=base?base+1:path;
    if(*name&&(!*info->title||!strcmp(info->title,base))){char *dot=strrchr(name,'.');if(dot)*dot=0;NxtText_copy(info->title,name,sizeof(info->title));}
    if(!*info->album)snprintf(info->album,sizeof(info->album),"Google Drive");
}
/* Keep cancel, shortcuts, sleep and process signals responsive during open.
 * Hidden background sessions have no display surface to draw. */
bool NxtDrive_preparePlayback(const char *path) {
    if(Player_beginLoad(path)<0)return false;
    int show=0,status;Uint32 drawn=0;
    while((status=Player_loadStatus())==1){
        if(NxtSession_isHidden()){
            Nxt_tick();if(ModuleCommon_stopSignalled()){Player_stop();return false;}usleep(10000);continue;
        }
        DisplayContext *display=DisplayHelper_current();
        if(!display){Player_stop();return false;}
        ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(display);
        GlobalInputResult input=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);
        bool cancel=input.should_quit||ModuleCommon_stopSignalled()||(!input.input_consumed&&NxtPad_justPressed(BTN_B));
        if(!cancel&&(SDL_GetTicks()-drawn>100||input.dirty)){
            NxtDriveEntry entry;NxtSkin_begin(screen,"Google Drive",NULL,true);
            NxtSkin_text(screen,30,NxtDrive_streamEntry(path,&entry)?entry.name:"Google Drive",screen->w/24,screen->h/3,screen->w*11/12,NxtSkin_color(NXT_INK));
            NxtSkin_text(screen,24,Nxt_text("Opening online audio…","Đang mở nhạc online…"),screen->w/24,screen->h/2,screen->w*11/12,NxtSkin_color(NXT_ACCENT));
            NxtSkin_footer(screen,Nxt_text("B Cancel · SELECT + direction adjusts levels","B Hủy · SELECT + hướng chỉnh âm lượng/độ sáng"),false);
            ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();
        }
        ModuleCommon_frameEnd(screen);
        if(cancel){Player_stop();return false;}usleep(10000);
    }
    if(status<0){Player_stop();Toast_show(Nxt_text("Drive cannot stream; check Wi-Fi, public sharing and quota","Không phát được Drive; kiểm tra Wi-Fi, chia sẻ công khai và hạn mức"),3500);return false;}
    return true;
}
static void load_links(void) {
    paths();link_count=0;JSON_Value *v=json_parse_file(links_file);JSON_Array *a=json_value_get_array(v);
    for(size_t i=0;a&&i<json_array_get_count(a)&&link_count<DRIVE_LINKS;i++){
        JSON_Object *o=json_array_get_object(a,i);const char *url=json_object_get_string(o,"url"),*name=json_object_get_string(o,"name");NxtDriveEntry e;
        if(NxtDrive_parseURL(url,&e)){
            snprintf(links[link_count].url,512,"%s",url);NxtText_copy(links[link_count].name,name?name:"Google Drive",256);link_count++;
        }
    }json_value_free(v);
}
static bool save_links(void) {
    JSON_Value *v=json_value_init_array();if(!v)return false;JSON_Array *a=json_value_get_array(v);
    for(int i=0;i<link_count;i++){
        JSON_Value *item=json_value_init_object();if(!item){json_value_free(v);return false;}JSON_Object *o=json_value_get_object(item);
        json_object_set_string(o,"url",links[i].url);json_object_set_string(o,"name",links[i].name);json_array_append_value(a,item);
    }
    char tmp[1100];snprintf(tmp,sizeof(tmp),"%s.new",links_file);
    bool ok=json_serialize_to_file_pretty(v,tmp)==JSONSuccess;json_value_free(v);
    if(ok)ok=rename(tmp,links_file)==0;if(!ok)unlink(tmp);return ok;
}
static bool add_link(const char *url) {
    NxtDriveEntry entry;char canonical[512];
    if(!NxtDrive_parseURL(url,&entry)||!NxtDrive_url(&entry,false,canonical,sizeof(canonical)))return false;
    for(int i=0;i<link_count;i++)if(!strcmp(canonical,links[i].url))return true;
    if(link_count==DRIVE_LINKS)return false;
    snprintf(links[link_count].url,512,"%s",canonical);snprintf(links[link_count].name,256,"Google Drive · %s",Nxt_text(entry.folder?"Folder":"File",entry.folder?"Thư mục":"Tệp"));
    link_count++;if(!save_links()){link_count--;return false;}return true;
}
static void end_transfer(pid_t child) {
    kill(child,SIGTERM);int status;
    for(int i=0;i<20;i++){pid_t p=waitpid(child,&status,WNOHANG);if(p==child||(p<0&&errno!=EINTR))return;usleep(10000);}
    kill(child,SIGKILL);while(waitpid(child,&status,0)<0&&errno==EINTR){}
}
/* Small folder/name pages are read with a cancellable frame loop. */
static int fetch(DisplayContext *display,const char *url,const char *target,const char *label) {
    unlink(target);pid_t child=fork();
    if(child<0)return -1;
    if(!child){
        const char *args[]={"curl","-sSL","--fail","--proto","=https","--proto-redir","=https",
            "--cacert","./res/cacert.pem","--connect-timeout","15","--max-time","45",
            "--speed-time","30","--speed-limit","512","--max-filesize","8388608",
            "-A","NXT-Retro-Music","-o",target,url,NULL};
        execvp("curl",(char *const *)args);_exit(127);
    }
    int show=0;Uint32 drawn=0;bool done=false,cancel=false,quit=false;int status=0;
    while(!done){
        pid_t result=waitpid(child,&status,WNOHANG);if(result==child){done=true;break;}if(result<0&&errno!=EINTR){done=true;status=-1;break;}
        ModuleCommon_frameBegin();Background_tick();SDL_Surface *screen=DisplayHelper_getSurface(display);
        GlobalInputResult input=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);
        if(input.should_quit){cancel=quit=true;}
        else if(!input.input_consumed&&NxtPad_justPressed(BTN_B))cancel=true;
        struct stat st={0};stat(target,&st);struct statvfs space;
        if(st.st_size>(8L*1024*1024)||
           (!statvfs(data_dir,&space)&&(uint64_t)space.f_bavail*space.f_frsize<1024*1024))cancel=true;
        if(cancel){end_transfer(child);unlink(target);ModuleCommon_frameEnd(screen);return quit?-3:-2;}
        if(SDL_GetTicks()-drawn>100||input.dirty){
            char detail[160];snprintf(detail,sizeof(detail),"%s · %.1f MB",Nxt_text("Loading","Đang tải"),st.st_size/1048576.0);
            NxtSkin_begin(screen,"Google Drive",NULL,true);
            NxtSkin_text(screen,30,label,screen->w/24,screen->h/3,screen->w*11/12,NxtSkin_color(NXT_INK));
            NxtSkin_text(screen,24,detail,screen->w/24,screen->h/2,screen->w*11/12,NxtSkin_color(NXT_ACCENT));
            NxtSkin_footer(screen,Nxt_text("B Cancel","B Hủy"),false);
            ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();
        }
        ModuleCommon_frameEnd(screen);usleep(10000);
    }
    struct stat st;
    if(status==-1||!WIFEXITED(status)||WEXITSTATUS(status)||stat(target,&st)||!st.st_size){unlink(target);return -1;}
    return 0;
}
static char *read_text(const char *path,size_t maximum) {
    struct stat st;if(stat(path,&st)||st.st_size<1||(uint64_t)st.st_size>maximum)return NULL;
    FILE *f=fopen(path,"rb");if(!f)return NULL;char *text=malloc(st.st_size+1);
    if(text){size_t n=fread(text,1,st.st_size,f);text[n]=0;if(n!=(size_t)st.st_size){free(text);text=NULL;}}fclose(f);return text;
}
static int entry_order(const void *a,const void *b){
    const NxtDriveEntry *x=a,*y=b;if(x->folder!=y->folder)return x->folder?-1:1;
    return NxtSort_compare(x->name,y->name);
}
static int list_folder(DisplayContext *display,const NxtDriveEntry *folder,NxtDriveEntry *entries,char title[256]) {
    char url[512],page[1100];snprintf(url,sizeof(url),"https://drive.google.com/embeddedfolderview?id=%s%s%s",folder->id,*folder->resource_key?"&resourcekey=":"",folder->resource_key);
    snprintf(page,sizeof(page),"%s/list.html.part",data_dir);int result=fetch(display,url,page,Nxt_text("Reading folder…","Đang đọc thư mục…"));
    if(result)return result== -2 ? -10 : result;
    char *html=read_text(page,8*1024*1024);unlink(page);if(!html)return -1;
    int n=NxtDrive_parseFolder(html,entries,DRIVE_ENTRIES,title,256);free(html);if(n==-2)return -4;if(n<0)return -1;
    int used=0;for(int i=0;i<n;i++)if(entries[i].folder||Browser_isAudioFile(entries[i].name))entries[used++]=entries[i];
    qsort(entries,used,sizeof(*entries),entry_order);return used;
}
static int file_name(DisplayContext *display,NxtDriveEntry *entry) {
    char url[512],page[1100];if(!NxtDrive_url(entry,false,url,sizeof(url)))return -1;
    snprintf(page,sizeof(page),"%s/file.html.part",data_dir);int result=fetch(display,url,page,Nxt_text("Reading audio file…","Đang đọc tệp nhạc…"));
    if(result)return result;
    char *html=read_text(page,8*1024*1024);unlink(page);bool ok=html&&NxtDrive_fileName(html,entry->name,sizeof(entry->name));free(html);
    return ok&&Browser_isAudioFile(entry->name)?0:-1;
}
/* Folder entries are already naturally sorted. Only audio enters the queue;
 * folders remain navigation rows. The selected entry keeps its queue index. */
static ModuleExitReason play_entries(DisplayContext *display,const NxtDriveEntry *queue,
                                    const NxtDriveEntry *entries,int count,int selected_entry) {
    if(!queue||!entries||count<=0||count>DRIVE_ENTRIES||selected_entry>=count)return MODULE_EXIT_TO_MENU;
    int total=0,start=selected_entry<0?0:-1;
    for(int i=0;i<count;i++)if(!entries[i].folder&&Browser_isAudioFile(entries[i].name)){
        if(i==selected_entry)start=total;
        total++;
    }
    if(!total||start<0||total>PLAYLIST_MAX_TRACKS)return MODULE_EXIT_TO_MENU;
    PlaylistTrack *tracks=calloc(total,sizeof(*tracks));
    if(!tracks){Toast_show(Nxt_text("Not enough memory for the queue","Không đủ bộ nhớ cho hàng đợi"),2500);return MODULE_EXIT_TO_MENU;}

    ModuleExitReason result=MODULE_EXIT_TO_MENU;int used=0,show=0;
    for(int i=0;i<count;i++){
        if(entries[i].folder||!Browser_isAudioFile(entries[i].name))continue;
        if(used%16==0){
            ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(display);
            GlobalInputResult global=ModuleCommon_handleGlobalInput(screen,&show,HELP_BROWSER);
            if(global.should_quit){ModuleCommon_frameEnd(screen);result=MODULE_EXIT_QUIT;goto done;}
            if(!global.input_consumed&&NxtPad_justPressed(BTN_B)){ModuleCommon_frameEnd(screen);goto done;}
            if(!global.input_consumed){
                char progress[128];snprintf(progress,sizeof(progress),Nxt_text("Preparing queue · %d / %d tracks","Đang tạo hàng đợi · %d / %d bài"),used,total);
                NxtSkin_begin(screen,"Google Drive",progress,false);
                NxtSkin_footer(screen,Nxt_text("B Cancel","B Hủy"),false);
                ModuleCommon_markSurfaceDrawn();
            }
            ModuleCommon_frameEnd(screen);
        }
        PlaylistTrack *track=&tracks[used];
        if(!save_stream(&entries[i],track->path,sizeof(track->path))){
            Toast_show(Nxt_text("Cannot save Drive queue links","Không lưu được link trong hàng đợi Drive"),2500);goto done;
        }
        NxtText_copy(track->name,entries[i].name,sizeof(track->name));
        track->format=Player_detectFormat(entries[i].name);used++;
    }

    /* Save URI text, never audio, before playback so even immediate exit can
     * resume the whole folder. One file per folder/file keeps other queues. */
    char url[512],path[512],temporary[520];paths();
    if(!NxtDrive_url(queue,false,url,sizeof(url))||
       snprintf(path,sizeof(path),"%s/%s.m3u",queues_dir,queue->id)>=(int)sizeof(path))goto done;
    snprintf(temporary,sizeof(temporary),"%s.new",path);
    FILE *file=fopen(temporary,"w");bool ok=file!=NULL;
    if(file){
        if(fputs("#EXTM3U\n",file)==EOF)ok=false;
        for(int i=0;i<used&&ok;i++)if(fprintf(file,"%s\n",tracks[i].path)<0)ok=false;
        if(fclose(file))ok=false;
        if(ok){chmod(temporary,0600);ok=rename(temporary,path)==0;}
    }
    if(!ok){unlink(temporary);Toast_show(Nxt_text("Cannot save Drive queue","Không lưu được hàng đợi Drive"),2500);goto done;}
    PlayerModule_setResumePlaylistPath(path);
    result=PlayerModule_runWithPlaylist(display,tracks,used,start);
done:
    free(tracks);return result;
}
static ModuleExitReason play(DisplayContext *display,const NxtDriveEntry *entry) {
    return play_entries(display,entry,entry,1,0);
}
static ModuleExitReason folder_browser(DisplayContext *display,NxtDriveEntry root,const char *root_url) {
    NxtDriveEntry history[32];int selections[32]={0},depth=0;history[0]=root;char title[256];
    NxtDriveEntry *entries=calloc(DRIVE_ENTRIES,sizeof(*entries));if(!entries)return MODULE_EXIT_TO_MENU;
    for(;;){
        int count=list_folder(display,&history[depth],entries,title);
        if(count<0){if(count!=-10&&count!=-3)Toast_show(count==-4?Nxt_text("Folder exceeds 4096 entries","Thư mục vượt quá 4096 mục"):Nxt_text("Cannot read Drive folder; check public sharing and Wi-Fi","Không đọc được Drive; kiểm tra chia sẻ công khai và Wi-Fi"),3500);free(entries);return count==-3?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}
        if(depth==0){for(int i=0;i<link_count;i++)if(!strcmp(links[i].url,root_url)){NxtText_copy(links[i].name,title,256);save_links();break;}}
        int audio_count=0;for(int i=0;i<count;i++)if(!entries[i].folder)audio_count++;
        int offset=audio_count?1:0,row_count=count+offset;
        const char **labels=calloc(row_count?row_count:1,sizeof(*labels));char (*rows)[300]=calloc(row_count?row_count:1,300);
        if(!labels||!rows){free(labels);free(rows);free(entries);return MODULE_EXIT_TO_MENU;}
        if(offset){snprintf(rows[0],300,Nxt_text("Listen to the whole folder\n%d tracks · Online music","Nghe cả thư mục\n%d bài · Nhạc online"),audio_count);labels[0]=rows[0];}
        for(int i=0;i<count;i++){int row=i+offset;snprintf(rows[row],300,"%s%s",entries[i].name,entries[i].folder?Nxt_text("\nFolder","\nThư mục"):"");labels[row]=rows[row];}
        if(selections[depth]<0||selections[depth]>=row_count)selections[depth]=0;
        for(;;){
            NxtPick choice=Nxt_pickLive(display,title,labels,row_count,&selections[depth],Nxt_text("A Listen online/Open · Y Refresh · B Back","A Nghe online/Mở · Y Làm mới · B Quay lại"));
            if(choice.action==9){free(labels);free(rows);free(entries);return MODULE_EXIT_QUIT;}
            if(choice.action==2){if(!depth){free(labels);free(rows);free(entries);return MODULE_EXIT_TO_MENU;}depth--;break;}
            if(choice.action==4)break;
            int selected=selections[depth];if(choice.action==1&&selected>=0&&selected<row_count){
                ModuleExitReason result=MODULE_EXIT_TO_MENU;
                if(offset&&selected==0)result=play_entries(display,&history[depth],entries,count,-1);
                else{int entry_index=selected-offset;NxtDriveEntry e=entries[entry_index];
                    if(e.folder){if(depth==31)Toast_show(Nxt_text("Folder nesting limit","Thư mục con quá sâu"),2000);else{history[++depth]=e;selections[depth]=0;break;}}
                    else result=play_entries(display,&history[depth],entries,count,entry_index);
                }
                if(result==MODULE_EXIT_QUIT||Nxt_takeReturnHome()){free(labels);free(rows);free(entries);return result;}
            }
        }free(labels);free(rows);
    }
}
static void import_links(DisplayContext *display) {
    char path[512];if(!Nxt_chooseFile(display,".txt",".url",path))return;char *text=read_text(path,64*1024);if(!text){Toast_show(Nxt_text("Cannot read link file","Không đọc được tệp link"),2000);return;}
    int before=link_count;char *save=NULL;
    for(char *line=strtok_r(text,"\r\n",&save);line;line=strtok_r(NULL,"\r\n",&save)){
        while(isspace((unsigned char)*line))line++;if(!strncmp(line,"URL=",4))line+=4;size_t n=strlen(line);while(n&&isspace((unsigned char)line[n-1]))line[--n]=0;add_link(line);
    }
    free(text);char message[100];snprintf(message,sizeof(message),Nxt_text("Added %d Drive links","Đã thêm %d link Drive"),link_count-before);Toast_show(message,2500);
}
ModuleExitReason NxtDrive_run(DisplayContext *display) {
    load_links();int selected=0;NxtSkinSource previous=NxtSkin_setSource(NXT_SOURCE_DRIVE);
    ModuleExitReason result=MODULE_EXIT_TO_MENU;
    for(;;){
        const char *rows[DRIVE_LINKS+2];char labels[DRIVE_LINKS][320];
        rows[0]=Nxt_text("Add Google Drive link\nPublic folder or audio file","Thêm link Google Drive\nThư mục hoặc tệp nhạc chia sẻ công khai");
        rows[1]=Nxt_text("Import links from .txt / .url\nOne link per line on the SD card","Nhập link từ .txt / .url\nMỗi dòng một link trên thẻ nhớ");
        for(int i=0;i<link_count;i++){snprintf(labels[i],sizeof(labels[i]),"%s\nGoogle Drive",links[i].name);rows[i+2]=labels[i];}
        NxtPick choice=Nxt_pickLive(display,"Google Drive",rows,link_count+2,&selected,Nxt_text("A Open · X Remove link · B Back","A Mở · X Xóa link · B Quay lại"));
        if(choice.action==9){result=MODULE_EXIT_QUIT;break;}if(choice.action==2)break;
        if(choice.action==3&&selected>=2&&selected<link_count+2){
            int i=selected-2;memmove(links+i,links+i+1,(link_count-i-1)*sizeof(*links));link_count--;save_links();if(selected>=link_count+2)selected=link_count+1;continue;
        }
        if(choice.action!=1)continue;
        if(selected==0){char *url=Keyboard_openLatin(Nxt_text("Google Drive public link","Link Google Drive công khai"),511);if(url){if(!add_link(url))Toast_show(Nxt_text("Invalid link, list full or cannot save","Link không hợp lệ, danh sách đầy hoặc không lưu được"),3000);free(url);}continue;}
        if(selected==1){import_links(display);continue;}
        int i=selected-2;if(i<0||i>=link_count)continue;NxtDriveEntry e;char url[512];snprintf(url,sizeof(url),"%s",links[i].url);if(!NxtDrive_parseURL(url,&e))continue;
        if(e.folder)result=folder_browser(display,e,url);
        else{int r=file_name(display,&e);if(!r){NxtText_copy(links[i].name,e.name,256);save_links();result=play(display,&e);}else if(r==-3)result=MODULE_EXIT_QUIT;else if(r!=-2)Toast_show(Nxt_text("Cannot read public audio file; check sharing","Không đọc được tệp nhạc; kiểm tra chia sẻ công khai"),3000);}
        if(result==MODULE_EXIT_QUIT||Nxt_takeReturnHome())break;
    }
    NxtSkin_setSource(previous);return result;
}
