#include "parson/parson.h"
#include "nxt_text.h"
#include "nxt_write.h"
#include "nxt_startup.h"
#include <math.h>
/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_personal.h"
#include "nxt_custom.h"
#include "nxt_theme.h"
#include "nxt_appearance.h"
#include "nxt_playback.h"
#include "nxt_drive.h"
#include "nxt_skin.h"
#include "nxt_log.h"
#include "file_utils.h"
#include "keyboard.h"
#include "toast.h"
#include "ui_fonts.h"
#include "resume.h"
#include "radio.h"
#include "podcast.h"
#include "module_player.h"
#include "include/parson/parson.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
static sqlite3 *db;
static int listener,avatar,recording=1;
static char nickname[96]="Người nghe",current_path[512],current_title[256],current_artist[256],source[24],current_day[16],root[1200];
static unsigned generation;
static sqlite3_int64 event_id,total,pending,pending_plays;
static uint32_t sampled_at,flushed_at;
static int64_t sampled_audio;
static int last_position,duration;
static bool sampled_playing,qualified;
static const char *avatars[]={"headphones","disc-3","heart","music-2","gamepad-2","mic-2","radio","podcast","moon","sun-moon","book-open","zap","avatar-cat","avatar-dog","avatar-rocket","avatar-star","avatar-flower","avatar-leaf","avatar-mountain","avatar-waves","avatar-coffee","avatar-crown","avatar-sparkles","avatar-smile"};
static const char *avatar_en[]={"Headphones","Record","Heart","Music","Gamepad","Microphone","Radio","Podcast","Moon","Sun & moon","Book","Lightning","Cat","Dog","Rocket","Star","Flower","Leaf","Mountain","Waves","Coffee","Crown","Sparkles","Smile"};
static const char *avatar_vi[]={"Tai nghe","Đĩa nhạc","Trái tim","Nốt nhạc","Tay cầm","Micro","Radio","Podcast","Mặt trăng","Nhật nguyệt","Sách","Tia chớp","Mèo","Cún","Tên lửa","Ngôi sao","Bông hoa","Lá xanh","Núi","Sóng","Cà phê","Vương miện","Lấp lánh","Nụ cười"};
int NxtPersonal_id(void){return listener;}
const char *NxtPersonal_name(void){return nickname;}
const char *NxtPersonal_avatar(void){return nxt.avatar_mode?"personal-avatar":nxt.avatar_icon>=0?NxtCustom_iconName(nxt.avatar_icon):avatars[avatar>=0&&avatar<24?avatar:0];}
static bool exec(const char *sql){char *error=NULL;int result=sqlite3_exec(db,sql,NULL,NULL,&error);if(result!=SQLITE_OK)NxtLog_step("personal","database",false,"SQLite code %d",result);sqlite3_free(error);return result==SQLITE_OK;}
static void profile_path(int id,const char *file,char *out,size_t size){char rel[180];snprintf(rel,sizeof(rel),"listeners/%d",id);userdata_mkdir(rel);snprintf(rel,sizeof(rel),"listeners/%d/%s",id,file);userdata_snpath(rel,out,size);}
int NxtPersonal_route(const char *rel,char *out,size_t size){
    if(!db||listener==0||!rel)return -1;
    bool routed=!strncmp(rel,"playlists",9)&&(rel[9]==0||rel[9]=='/');
    routed|=!strncmp(rel,"presets",7)&&(rel[7]==0||rel[7]=='/');routed|=!strncmp(rel,"profiles",8)&&(rel[8]==0||rel[8]=='/');routed|=!strncmp(rel,"soundcloud",10)&&(rel[10]==0||rel[10]=='/');routed|=!strcmp(rel,"queue.m3u")||!strcmp(rel,"resume.cfg")||!strcmp(rel,"last-playback.json")||!strcmp(rel,"last-art.png");
    if(!routed)return -1;return snprintf(out,size,"%s/listeners/%d/%s",root,listener,rel);
}
static void load_listener(void){
    sqlite3_stmt *q=NULL;sqlite3_prepare_v2(db,"SELECT name,avatar,recording FROM listeners WHERE id=?",-1,&q,NULL);sqlite3_bind_int(q,1,listener);
    if(sqlite3_step(q)!=SQLITE_ROW){sqlite3_finalize(q);q=NULL;sqlite3_prepare_v2(db,"SELECT id,name,avatar,recording FROM listeners ORDER BY id LIMIT 1",-1,&q,NULL);
        if(sqlite3_step(q)==SQLITE_ROW){listener=sqlite3_column_int(q,0);snprintf(nickname,sizeof(nickname),"%s",sqlite3_column_text(q,1));avatar=sqlite3_column_int(q,2);recording=sqlite3_column_int(q,3);}}
    else{snprintf(nickname,sizeof(nickname),"%s",sqlite3_column_text(q,0));avatar=sqlite3_column_int(q,1);recording=sqlite3_column_int(q,2);}sqlite3_finalize(q);if(avatar<0||avatar>=24)avatar=0;
    char path[1400];profile_path(listener,"preferences.json",path,sizeof(path));if(listener!=0||access(path,R_OK)==0)Nxt_loadPreferences(path);
}
void NxtPersonal_init(void){
    if(db)return;userdata_mkdir("");userdata_snpath("",root,sizeof(root));char path[1400];userdata_snpath("listening.sqlite3",path,sizeof(path));
    if(sqlite3_open_v2(path,&db,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX,NULL)!=SQLITE_OK){NxtLog_step("personal","open",false,"Cannot open listening database");if(db)sqlite3_close(db);db=NULL;return;}
    chmod(path,0600);sqlite3_busy_timeout(db,1500);
    if(!exec("PRAGMA journal_mode=WAL;CREATE TABLE IF NOT EXISTS listeners(id INTEGER PRIMARY KEY,name TEXT NOT NULL,avatar INTEGER NOT NULL DEFAULT 0,recording INTEGER NOT NULL DEFAULT 1);CREATE TABLE IF NOT EXISTS app(key TEXT PRIMARY KEY,value INTEGER);CREATE TABLE IF NOT EXISTS history(id INTEGER PRIMARY KEY,listener INTEGER,path TEXT,title TEXT,artist TEXT,source TEXT,started INTEGER,last INTEGER,ms INTEGER DEFAULT 0,qualified INTEGER DEFAULT 0,position INTEGER DEFAULT 0);CREATE INDEX IF NOT EXISTS history_listener ON history(listener,last);CREATE INDEX IF NOT EXISTS history_path ON history(listener,path);CREATE TABLE IF NOT EXISTS daily(listener INTEGER,day TEXT,source TEXT,ms INTEGER DEFAULT 0,plays INTEGER DEFAULT 0,PRIMARY KEY(listener,day,source));")){sqlite3_close(db);db=NULL;return;}
    sqlite3_stmt *q=NULL;sqlite3_prepare_v2(db,"SELECT count(*) FROM listeners",-1,&q,NULL);bool empty=sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int(q,0)==0;sqlite3_finalize(q);q=NULL;
    if(empty&&!exec("INSERT INTO listeners(id,name,avatar) VALUES(0,'Listener',0),(1,'Guest',1);")){sqlite3_close(db);db=NULL;return;}
    sqlite3_prepare_v2(db,"SELECT value FROM app WHERE key='activeListener'",-1,&q,NULL);if(sqlite3_step(q)==SQLITE_ROW)listener=sqlite3_column_int(q,0);sqlite3_finalize(q);if(listener<0||listener>15)listener=0;load_listener();
    q=NULL;if(sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO app VALUES('activeListener',?)",-1,&q,NULL)==SQLITE_OK){sqlite3_bind_int(q,1,listener);sqlite3_step(q);}sqlite3_finalize(q);NxtLog_step("personal","init",true,"listener=%d",listener);
}
void NxtPersonal_savePreferences(void){
    if(!db)return;char from[1400],target[1400],temp[1420];userdata_snpath("nxt.json",from,sizeof(from));profile_path(listener,"preferences.json",target,sizeof(target));snprintf(temp,sizeof(temp),"%s.tmp",target);
    FILE *in=fopen(from,"rb"),*out=in?fopen(temp,"wb"):NULL;bool ok=in&&out;char buf[8192];size_t n;
    if(ok)while((n=fread(buf,1,sizeof(buf),in)))if(fwrite(buf,1,n,out)!=n){ok=false;break;}
    if(in){if(ferror(in))ok=false;fclose(in);}if(out){if(fflush(out)||fsync(fileno(out)))ok=false;if(fclose(out))ok=false;}
    if(ok)ok=!rename(temp,target);if(!ok){unlink(temp);NxtLog_step("personal","preferences-save",false,"listener=%d",listener);}else chmod(target,0600);
}
void NxtPersonal_preferencesPath(char *out,size_t size){if(db)profile_path(listener,"preferences.json",out,size);else if(size)out[0]=0;}
void NxtPersonal_flush(void){
    if(!db||!current_path[0]||(!pending&&!pending_plays))return;if(!exec("BEGIN IMMEDIATE"))return;bool ok=true;bool inserted=false;sqlite3_stmt *q=NULL;
    if(!event_id){ok=sqlite3_prepare_v2(db,"INSERT INTO history(listener,path,title,artist,source,started,last) VALUES(?,?,?,?,?,?,?)",-1,&q,NULL)==SQLITE_OK;
        sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,current_path,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,3,current_title,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,4,current_artist,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,5,source,-1,SQLITE_TRANSIENT);sqlite3_bind_int64(q,6,time(NULL));sqlite3_bind_int64(q,7,time(NULL));ok=ok&&sqlite3_step(q)==SQLITE_DONE;sqlite3_finalize(q);q=NULL;if(ok){event_id=sqlite3_last_insert_rowid(db);inserted=true;}}
    if(ok){ok=sqlite3_prepare_v2(db,"UPDATE history SET ms=ms+?,qualified=?,last=?,position=? WHERE id=?",-1,&q,NULL)==SQLITE_OK;sqlite3_bind_int64(q,1,pending);sqlite3_bind_int(q,2,qualified);sqlite3_bind_int64(q,3,time(NULL));sqlite3_bind_int(q,4,last_position);sqlite3_bind_int64(q,5,event_id);ok=ok&&sqlite3_step(q)==SQLITE_DONE;sqlite3_finalize(q);q=NULL;}
    if(ok){ok=sqlite3_prepare_v2(db,"INSERT OR IGNORE INTO daily(listener,day,source) VALUES(?,?,?)",-1,&q,NULL)==SQLITE_OK;sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,current_day,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,3,source,-1,SQLITE_TRANSIENT);ok=ok&&sqlite3_step(q)==SQLITE_DONE;sqlite3_finalize(q);q=NULL;}
    if(ok){ok=sqlite3_prepare_v2(db,"UPDATE daily SET ms=ms+?,plays=plays+? WHERE listener=? AND day=? AND source=?",-1,&q,NULL)==SQLITE_OK;sqlite3_bind_int64(q,1,pending);sqlite3_bind_int64(q,2,pending_plays);sqlite3_bind_int(q,3,listener);sqlite3_bind_text(q,4,current_day,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,5,source,-1,SQLITE_TRANSIENT);ok=ok&&sqlite3_step(q)==SQLITE_DONE;sqlite3_finalize(q);}
    if(ok&&exec("COMMIT")){pending=pending_plays=0;flushed_at=SDL_GetTicks();}else{exec("ROLLBACK");if(inserted)event_id=0;NxtLog_step("statistics","flush",false,"Batch retained for retry");}
}
void NxtPersonal_tick(void){
    if(!db)return;uint32_t now=SDL_GetTicks();if(sampled_at&&(uint32_t)(now-sampled_at)<500)return;
    int64_t audio=Player_getListeningTime();bool playing=recording&&!Radio_isActive()&&!Podcast_isActive()&&NxtPlayback_state()==PLAYER_STATE_PLAYING&&Player_outputReady();const char *path=NxtPlayback_path();if(!path)path="";unsigned gen=NxtPlayback_generation();
    time_t clock=time(NULL);struct tm tm;localtime_r(&clock,&tm);char day[16];strftime(day,sizeof(day),"%Y-%m-%d",&tm);
    bool changed=gen!=generation||strcmp(path,current_path);
    if(changed){NxtPersonal_flush();if(pending||pending_plays){sampled_at=now;sampled_audio=audio;sampled_playing=false;return;}
        event_id=total=0;qualified=false;generation=gen;snprintf(current_path,sizeof(current_path),"%s",path);const TrackInfo *info=NxtPlayback_info();snprintf(current_title,sizeof(current_title),"%s",info&&info->title[0]?info->title:path);snprintf(current_artist,sizeof(current_artist),"%s",info?info->artist:"");duration=NxtPlayback_duration();snprintf(source,sizeof(source),"%s",NxtSoundCloud_hasMetadata()?"SoundCloud":NxtSpotify_hasMetadata()?"Spotify":NxtDrive_isPath(path)?"Drive":"Local");snprintf(current_day,sizeof(current_day),"%s",day);
    }
    if(strcmp(day,current_day)){NxtPersonal_flush();if(!pending&&!pending_plays)snprintf(current_day,sizeof(current_day),"%s",day);}
    uint32_t elapsed=now-sampled_at;int64_t delta=audio-sampled_audio;
    if(!changed&&playing&&sampled_playing&&sampled_at&&elapsed<=3000&&delta>0&&current_path[0]){
        if(delta>elapsed)delta=elapsed;pending+=delta;total+=delta;int threshold=duration>0&&duration<60000?duration/2:30000;
        if(!qualified&&total>=threshold){qualified=true;pending_plays++;}
    }
    last_position=NxtPlayback_position();sampled_at=now;sampled_audio=audio;sampled_playing=playing;
    if(!playing||now-flushed_at>=10000||pending_plays)NxtPersonal_flush();
}
void NxtPersonal_quit(void){NxtPersonal_flush();if(db)sqlite3_close(db);db=NULL;listener=0;current_path[0]=0;generation=0;event_id=total=pending=pending_plays=0;sampled_at=0;sampled_playing=false;}
static bool switch_listener(int id){
    if(!db||id<0||id>15)return false;if(id==listener)return true;
    sqlite3_stmt *q=NULL;bool valid=sqlite3_prepare_v2(db,"SELECT 1 FROM listeners WHERE id=?",-1,&q,NULL)==SQLITE_OK;
    if(valid){sqlite3_bind_int(q,1,id);valid=sqlite3_step(q)==SQLITE_ROW;}sqlite3_finalize(q);if(!valid)return false;
    NxtPersonal_flush();if(pending||pending_plays||!Nxt_save())return false;NxtWrite_flush();
    bool ok=sqlite3_prepare_v2(db,"INSERT OR REPLACE INTO app VALUES('activeListener',?)",-1,&q,NULL)==SQLITE_OK;
    if(ok){sqlite3_bind_int(q,1,id);ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);if(!ok){NxtLog_step("personal","switch",false,"Active listener was not saved");return false;}
    Nxt_stop();PlayerModule_quit();Nxt_libraryQuit();listener=id;load_listener();
    current_path[0]=0;generation=0;event_id=total=pending=pending_plays=0;sampled_at=0;sampled_playing=false;NxtAppearance_release();Fonts_unload();Fonts_load();Resume_init();Nxt_save();NxtLog_step("personal","switch",ok,"listener=%d",listener);return ok;
}
/* Called only on explicit profile management, never in the playback/render tick.
 * Directory FDs and no-follow checks keep SD shortcuts outside this profile intact. */
static bool remove_profile_tree(int parent,const char *name,int depth){
    struct stat st;if(fstatat(parent,name,&st,AT_SYMLINK_NOFOLLOW))return errno==ENOENT;
    if(!S_ISDIR(st.st_mode))return unlinkat(parent,name,0)==0;
    if(depth>32)return false;int fd=openat(parent,name,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);if(fd<0)return false;
    DIR *dir=fdopendir(fd);if(!dir){close(fd);return false;}bool ok=true;struct dirent *entry;
    while((entry=readdir(dir)))if(strcmp(entry->d_name,".")&&strcmp(entry->d_name,".."))if(!remove_profile_tree(dirfd(dir),entry->d_name,depth+1))ok=false;
    closedir(dir);if(ok)ok=unlinkat(parent,name,AT_REMOVEDIR)==0;return ok;
}
static bool remove_listener_files(int id){
    if(id<0||id>15)return false;int base=open(root,O_RDONLY|O_DIRECTORY|O_CLOEXEC);if(base<0)return false;bool ok=true;
    int parent=openat(base,"listeners",O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    if(parent>=0){char name[24];snprintf(name,sizeof(name),"%d",id);ok=remove_profile_tree(parent,name,0);close(parent);}else if(errno!=ENOENT)ok=false;
    if(id==0){const char *legacy[]={"playlists","presets","profiles","soundcloud","queue.m3u","resume.cfg","last-playback.json","last-art.png"};
        for(unsigned i=0;i<sizeof(legacy)/sizeof(*legacy);i++)if(!remove_profile_tree(base,legacy[i],0))ok=false;}
    close(base);return ok;
}
static bool delete_listener(int id){
    if(!db||id<0||id>15)return false;sqlite3_stmt *q=NULL;int remaining=-1;
    if(sqlite3_prepare_v2(db,"SELECT id FROM listeners WHERE id<>? ORDER BY id LIMIT 1",-1,&q,NULL)==SQLITE_OK){sqlite3_bind_int(q,1,id);if(sqlite3_step(q)==SQLITE_ROW)remaining=sqlite3_column_int(q,0);}sqlite3_finalize(q);q=NULL;
    if(remaining<0){Toast_show(Nxt_text("Keep at least one listener","Cần giữ ít nhất một hồ sơ"),2500);return false;}
    bool exists=sqlite3_prepare_v2(db,"SELECT 1 FROM listeners WHERE id=?",-1,&q,NULL)==SQLITE_OK;if(exists){sqlite3_bind_int(q,1,id);exists=sqlite3_step(q)==SQLITE_ROW;}sqlite3_finalize(q);q=NULL;if(!exists)return false;
    if(id==listener){if(!switch_listener(remaining)){Toast_show(Nxt_text("Cannot switch listener; profile kept","Không đổi được người nghe; giữ lại hồ sơ"),2500);return false;}NxtStartup_quit();}
    NxtWrite_flush();Nxt_libraryQuit();
    /* Favorites live in the shared catalog; delete only this listener's rows
     * within the same SQLite transaction as their history/profile record. */
    char path[1400];userdata_snpath("library.sqlite3",path,sizeof(path));struct stat st;bool attached=false,ok=true;
    if(!lstat(path,&st)){
        ok=S_ISREG(st.st_mode)&&sqlite3_prepare_v2(db,"ATTACH DATABASE ? AS profile_catalog",-1,&q,NULL)==SQLITE_OK;
        if(ok){sqlite3_bind_text(q,1,path,-1,SQLITE_TRANSIENT);ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);q=NULL;attached=ok;
    }else ok=errno==ENOENT;
    if(ok)ok=exec("BEGIN IMMEDIATE");bool transaction=ok;
    if(ok&&attached){
        const char *tables[]={"listener_favorites","favorites"};
        for(int i=0;ok&&i<2;i++){
            if(i==1&&id!=0)continue;
            ok=sqlite3_prepare_v2(db,"SELECT 1 FROM profile_catalog.sqlite_master WHERE type='table' AND name=?",-1,&q,NULL)==SQLITE_OK;bool present=false;
            if(ok){sqlite3_bind_text(q,1,tables[i],-1,SQLITE_STATIC);int rc=sqlite3_step(q);ok=rc==SQLITE_ROW||rc==SQLITE_DONE;present=rc==SQLITE_ROW;}sqlite3_finalize(q);q=NULL;
            if(ok&&present){ok=sqlite3_prepare_v2(db,i==0?"DELETE FROM profile_catalog.listener_favorites WHERE listener=?":"DELETE FROM profile_catalog.favorites",-1,&q,NULL)==SQLITE_OK;
                if(ok){if(!i)sqlite3_bind_int(q,1,id);ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);q=NULL;}
        }
    }
    const char *sql[]={"DELETE FROM history WHERE listener=?","DELETE FROM daily WHERE listener=?","DELETE FROM listeners WHERE id=?"};
    for(unsigned i=0;ok&&i<sizeof(sql)/sizeof(*sql);i++){ok=sqlite3_prepare_v2(db,sql[i],-1,&q,NULL)==SQLITE_OK;if(ok){sqlite3_bind_int(q,1,id);ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);q=NULL;}
    if(ok)ok=exec("COMMIT");if(!ok&&transaction)exec("ROLLBACK");if(attached)exec("DETACH DATABASE profile_catalog");
    if(!ok){NxtLog_step("personal","delete",false,"listener=%d kept",id);Toast_show(Nxt_text("Cannot delete profile; try again","Không xóa được hồ sơ; hãy thử lại"),2500);return false;}
    bool cleaned=remove_listener_files(id);NxtLog_step("personal","delete",cleaned,"listener=%d removed, files=%d",id,cleaned);
    Toast_show(cleaned?Nxt_text("Listener deleted","Đã xóa hồ sơ người nghe"):Nxt_text("Profile deleted; some private files could not be removed","Đã xóa hồ sơ; một số tệp riêng chưa xóa được"),3000);return true;
}
static ModuleExitReason confirm_delete_listener(DisplayContext *d,int id,const char *name){
    char title[180];snprintf(title,sizeof(title),"%s %s?",Nxt_text("Delete","Xóa"),name?name:"");
    const char *rows[]={Nxt_text("Cancel","Hủy"),Nxt_text("Delete profile, history & favorites","Xóa hồ sơ, lịch sử và yêu thích")};int selected=0;
    NxtPick p=Nxt_pick(d,title,rows,2,&selected,Nxt_text("Music files stay · Cannot undo","Giữ tệp nhạc · Không thể hoàn tác"));
    if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==1&&selected==1)delete_listener(id);return MODULE_EXIT_TO_MENU;
}
static char *listener_name(void){
    char *name=Keyboard_open(Nxt_text("Your nickname","Biệt danh của bạn"),90);if(!name)return NULL;char *begin=name;while(*begin==' '||*begin=='\t')begin++;size_t len=strlen(begin);while(len&&(begin[len-1]==' '||begin[len-1]=='\t'))begin[--len]=0;
    if(!len||len>=sizeof(nickname)){Toast_show(Nxt_text("Use a shorter, nonempty nickname","Nhập biệt danh ngắn hơn và không để trống"),2500);free(name);return NULL;}memmove(name,begin,len+1);return name;
}
static bool edit_listener(void){
    sqlite3_stmt *q=NULL;bool ok=sqlite3_prepare_v2(db,"UPDATE listeners SET name=?,avatar=?,recording=? WHERE id=?",-1,&q,NULL)==SQLITE_OK;
    if(ok){sqlite3_bind_text(q,1,nickname,-1,SQLITE_TRANSIENT);sqlite3_bind_int(q,2,avatar);sqlite3_bind_int(q,3,recording);sqlite3_bind_int(q,4,listener);ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);NxtLog_step("personal","edit",ok,"listener=%d",listener);if(!ok)Toast_show(Nxt_text("Cannot save profile; export app log","Không lưu được hồ sơ; hãy xuất log app"),2500);return ok;
}
static int avatar_gallery(DisplayContext *d){
    int selected=avatar,show=0;uint32_t drawn=0;for(;;){int previous=selected;ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);if(g.should_quit)return -2;
        if(!g.input_consumed&&!Nxt_lockInput()){if(NxtPad_justPressed(BTN_B))return -1;if(NxtPad_justPressed(BTN_A))return selected;if(NxtPad_justRepeated(BTN_LEFT))selected=(selected+23)%24;if(NxtPad_justRepeated(BTN_RIGHT))selected=(selected+1)%24;if(NxtPad_justRepeated(BTN_UP))selected=(selected+18)%24;if(NxtPad_justRepeated(BTN_DOWN))selected=(selected+6)%24;}
        if(NxtSkin_shouldDraw(drawn,g.dirty||selected!=previous)){NxtSkin_begin(screen,Nxt_text("24 profile icons","24 biểu tượng hồ sơ"),nickname,false);int u=screen->w;
        for(int i=0;i<24;i++){SDL_Rect r={(32+(i%6)*162)*u/1024,(154+(i/6)*120)*u/1024,150*u/1024,106*u/1024};SDL_Color ink=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK);if(i==selected)NxtSkin_rect(screen,r,NxtSkin_color(NXT_ACCENT),10*u/1024);else NxtSkin_panel(screen,r,NxtSkin_color(NXT_PANEL),10*u/1024);NxtSkin_icon(screen,avatars[i],r.x+53*u/1024,r.y+10*u/1024,44*u/1024,ink);NxtSkin_text(screen,16,Nxt_text(avatar_en[i],avatar_vi[i]),r.x+8*u/1024,r.y+65*u/1024,r.w-16*u/1024,ink);}
        NxtSkin_footer(screen,Nxt_text("D-pad Browse · A Apply · B Cancel","D-pad Chọn · A Áp dụng · B Hủy"),false);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(screen);
    }
}
static ModuleExitReason profile_edit(DisplayContext *d){
 int selected=0;for(;;){char lines[7][210];const char *rows[7];snprintf(lines[0],210,"%s\t%s",Nxt_text("Nickname","Biệt danh"),nickname);snprintf(lines[1],210,"%s\t%s",Nxt_text("Avatar","Avatar"),Nxt_text("Edit","Chỉnh sửa"));snprintf(lines[2],210,"%s\t%s",Nxt_text("Record listening","Ghi lịch sử nghe"),recording?Nxt_text("On","Bật"):Nxt_text("Off","Tắt"));snprintf(lines[3],210,"%s\t%02d/%02d",Nxt_text("Birthday","Sinh nhật"),nxt.birth_day,nxt.birth_month);snprintf(lines[4],210,"%s\t%s",Nxt_text("Favorite genre","Thể loại yêu thích"),NxtCustom_genre(nxt.genre));snprintf(lines[5],210,"%s\t%s",Nxt_text("Mood","Tâm trạng"),NxtCustom_mood(nxt.mood));snprintf(lines[6],210,"%s",Nxt_text("Delete listener","Xóa hồ sơ người nghe"));for(int i=0;i<7;i++)rows[i]=lines[i];NxtPick pick=Nxt_pick(d,Nxt_text("Edit profile","Chỉnh hồ sơ"),rows,7,&selected,Nxt_text("A Edit · B Back","A Sửa · B Về"));if(pick.action==2||pick.action==9)return pick.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(pick.action!=1)continue;
 if(selected==6)return confirm_delete_listener(d,listener,nickname);
 if(selected==0){char *name=listener_name();if(!name)continue;snprintf(nickname,sizeof(nickname),"%s",name);free(name);if(!edit_listener())load_listener();}
 else if(selected==1){if(NxtCustom_avatarEdit(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
 else if(selected==2){NxtPersonal_flush();recording=!recording;sampled_playing=false;if(!edit_listener())load_listener();}
 else if(selected==3){char *t=Keyboard_openLatin(Nxt_text("Birthday DD/MM · 00/00 clears","Sinh nhật DD/MM · 00/00 để xóa"),5);if(t){int day,month;char tail;bool ok=sscanf(t,"%d/%d%c",&day,&month,&tail)==2;int limits[]={0,31,29,31,30,31,30,31,31,30,31,30,31};ok=ok&&((day==0&&month==0)||(month>=1&&month<=12&&day>=1&&day<=limits[month]));if(ok){nxt.birth_day=day;nxt.birth_month=month;}else Toast_show(Nxt_text("Use a valid DD/MM","Nhập ngày hợp lệ DD/MM"),2000);free(t);}}
 else if(selected==4||selected==5){const char *options[20];int n=selected==4?20:10,choice=selected==4?nxt.genre:nxt.mood;for(int i=0;i<n;i++)options[i]=selected==4?NxtCustom_genre(i):NxtCustom_mood(i);NxtPick p=Nxt_pick(d,selected==4?Nxt_text("Favorite genre","Thể loại yêu thích"):Nxt_text("Mood","Tâm trạng"),options,n,&choice,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==1){if(selected==4)nxt.genre=choice;else nxt.mood=choice;}}
 Nxt_save();
 }
}
static bool create_listener(void){
    int id=-1;sqlite3_stmt *q=NULL;
    /* Naming the original profile retains pre-profile favorites and playlists. */
    sqlite3_prepare_v2(db,"SELECT name FROM listeners WHERE id=0",-1,&q,NULL);if(sqlite3_step(q)==SQLITE_ROW&&!strcmp((const char*)sqlite3_column_text(q,0),"Người nghe"))id=0;sqlite3_finalize(q);
    for(int i=1;id<0&&i<16;i++){sqlite3_prepare_v2(db,"SELECT 1 FROM listeners WHERE id=?",-1,&q,NULL);sqlite3_bind_int(q,1,i);bool exists=sqlite3_step(q)==SQLITE_ROW;sqlite3_finalize(q);if(!exists)id=i;}
    if(id<0){Toast_show(Nxt_text("Maximum 16 listeners","Tối đa 16 hồ sơ"),2000);return false;}
    if(id>0){NxtWrite_flush();if(!remove_listener_files(id)){Toast_show(Nxt_text("Cannot clear old profile files; check SD card","Không dọn được tệp hồ sơ cũ; kiểm tra thẻ nhớ"),3000);return false;}}char *name=listener_name();if(!name)return false;
    bool ok=sqlite3_prepare_v2(db,id==0?"UPDATE listeners SET name=? WHERE id=?":"INSERT INTO listeners(name,id) VALUES(?,?)",-1,&q,NULL)==SQLITE_OK;
    if(ok){sqlite3_bind_text(q,1,name,-1,SQLITE_TRANSIENT);sqlite3_bind_int(q,2,id);ok=sqlite3_step(q)==SQLITE_DONE;}sqlite3_finalize(q);free(name);
    if(ok){if(id==listener){load_listener();Nxt_save();}else ok=switch_listener(id);}NxtLog_step("personal","create",ok,"listener=%d",id);return ok;
}
#include "nxt_personal_v18.inc"
static ModuleExitReason choose_listener(DisplayContext *d,bool login){
    int selected=0,show=0,count=0;char names[16][96];int ids[16],images[16];ProfileAvatar profile_avatars[16];bool reload=true;uint32_t drawn=0;for(;;){int previous=selected;if(reload){count=0;sqlite3_stmt *q=NULL;sqlite3_prepare_v2(db,"SELECT id,name,avatar FROM listeners ORDER BY id",-1,&q,NULL);
        while(count<16&&sqlite3_step(q)==SQLITE_ROW){ids[count]=sqlite3_column_int(q,0);snprintf(names[count],sizeof(names[count]),"%s",sqlite3_column_text(q,1));images[count]=sqlite3_column_int(q,2);if(images[count]<0||images[count]>=24)images[count]=0;read_profile_avatar(ids[count],images[count],&profile_avatars[count]);count++;}sqlite3_finalize(q);if(selected>count)selected=count;reload=false;drawn=0;}
        ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);if(g.should_quit)return MODULE_EXIT_QUIT;
        if(!g.input_consumed&&!Nxt_lockInput()){
            if(NxtPad_justPressed(BTN_B))return login?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
            if(NxtPad_justRepeated(BTN_LEFT))selected=(selected+count)%(count+1);if(NxtPad_justRepeated(BTN_RIGHT))selected=(selected+1)%(count+1);if(NxtPad_justRepeated(BTN_UP))selected=(selected+4*(count+1)-4)%(count+1);if(selected<0)selected=0;if(NxtPad_justRepeated(BTN_DOWN))selected=(selected+4)%(count+1);
            if(NxtPad_justPressed(BTN_X)&&selected<count){if(confirm_delete_listener(d,ids[selected],names[selected])==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;reload=true;drawn=0;}
            else if(NxtPad_justPressed(BTN_A)){bool ok=selected==count||(ids[selected]==0&&!strcmp(names[selected],"Người nghe"))?create_listener():switch_listener(ids[selected]);if(ok)return MODULE_EXIT_TO_MENU;reload=true;drawn=0;}
        }
        if(NxtSkin_shouldDraw(drawn,g.dirty||selected!=previous)){NxtSkin_begin(screen,Nxt_text("Choose your listener","Chọn người nghe"),NULL,false);int u=screen->w,first=selected/12*12;NxtSkin_text(screen,21,Nxt_text("Create a local profile to keep your own music, history and appearance.","Tạo hồ sơ trên máy để lưu nhạc, lịch sử và giao diện riêng."),32*u/1024,149*u/1024,960*u/1024,NxtSkin_color(NXT_INK));
        for(int i=first;i<=count&&i<first+12;i++){int col=(i-first)%4,row=(i-first)/4;SDL_Rect r={(32+col*244)*u/1024,(204+row*139)*u/1024,230*u/1024,124*u/1024};SDL_Color ink=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK);if(i==selected)NxtSkin_rect(screen,r,NxtSkin_color(NXT_ACCENT),12*u/1024);else NxtSkin_panel(screen,r,NxtSkin_color(NXT_PANEL),12*u/1024);if(i==count)NxtSkin_icon(screen,"plus",r.x+91*u/1024,r.y+13*u/1024,44*u/1024,ink);else{ProfileAvatar *a=&profile_avatars[i];NxtCustom_drawAvatar(screen,a->mode,a->icon,a->emoji,a->text,a->path,r.x+91*u/1024,r.y+13*u/1024,44*u/1024,ink);}profile_name(screen,i==count?Nxt_text("Create listener","Tạo người nghe"):names[i],r,ink);}
        NxtSkin_footer(screen,Nxt_text("D-pad Select · A Continue · X Delete · B Back","D-pad Chọn · A Tiếp tục · X Xóa · B Về"),false);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(screen);
    }
}
ModuleExitReason NxtPersonal_login(DisplayContext *d){
    if(!db){const char *rows[]={Nxt_text("Cannot open profiles. Check SD card space and restart the app.","Không mở được hồ sơ. Kiểm tra dung lượng thẻ rồi mở lại app.")};Nxt_document(d,Nxt_text("Profile error","Lỗi hồ sơ"),rows,1);return MODULE_EXIT_QUIT;}return choose_listener(d,true);
}
typedef struct{char path[512],title[256],artist[256],label[650],source[24];int position;time_t last;sqlite3_int64 listened;} HistoryRow;
static ModuleExitReason play_rows(DisplayContext *d,HistoryRow *rows,int count,int selected,bool resume){
    if(!rows||count<=0||selected<0||selected>=count)return MODULE_EXIT_TO_MENU;
    HistoryRow *r=&rows[selected];if(!strcmp(r->source,"Spotify"))return NxtSpotify_playSaved(d,r->path,r->title,r->artist);
    if(!strcmp(r->source,"SoundCloud"))return NxtSoundCloud_playSaved(d,r->path,r->title,r->artist);
    if(resume&&Resume_isAvailable()&&!strcmp(Resume_getState()->track_path,r->path))return PlayerModule_runResume(d,Resume_getState());
    /* The local decoder supports files and Drive streams. Online providers
     * own their own playback queues and must not enter that decoder. */
    PlaylistTrack *list=calloc(count,sizeof(*list));if(!list)return MODULE_EXIT_TO_MENU;int length=0,index=-1;
    for(int i=0;i<count;i++){
        if(strcmp(rows[i].source,"Local")&&strcmp(rows[i].source,"Drive"))continue;
        if(i==selected)index=length;PlaylistTrack *track=&list[length++];
        snprintf(track->path,sizeof(track->path),"%s",rows[i].path);snprintf(track->name,sizeof(track->name),"%s",rows[i].title);track->format=Player_detectFormat(track->path);
    }
    ModuleExitReason result=MODULE_EXIT_TO_MENU;if(index>=0){PlayerModule_setResumePlaylistPath("");result=PlayerModule_runWithPlaylist(d,list,length,index);}free(list);return result;
}
static ModuleExitReason history_browser(DisplayContext *d,int mode){
    NxtPersonal_flush();int selected=0,offset=0;
    for(;;){HistoryRow *data=calloc(100,sizeof(*data));const char **rows=calloc(101,sizeof(*rows));if(!data||!rows){free(data);free(rows);return MODULE_EXIT_TO_MENU;}
        const char *sql=mode==0?"SELECT path,title,artist,source,position FROM history WHERE listener=? AND ms>0 ORDER BY last DESC LIMIT 100 OFFSET ?":mode==1?"SELECT path,title,artist,source,position FROM history WHERE listener=? AND ms>0 GROUP BY path ORDER BY sum(ms) DESC LIMIT 100 OFFSET ?":mode==2?"SELECT path,title,artist,source,position FROM history WHERE listener=? AND ms>0 GROUP BY path HAVING max(last)<strftime('%s','now','-30 days') ORDER BY max(last) ASC LIMIT 100 OFFSET ?":"SELECT path,title,artist,source,position FROM history WHERE listener=? AND ms>0 ORDER BY last DESC LIMIT 100 OFFSET ?";
        sqlite3_stmt *q=NULL;sqlite3_prepare_v2(db,sql,-1,&q,NULL);sqlite3_bind_int(q,1,listener);sqlite3_bind_int(q,2,offset);int count=0;while(count<100&&sqlite3_step(q)==SQLITE_ROW){HistoryRow *r=&data[count];snprintf(r->path,sizeof(r->path),"%s",sqlite3_column_text(q,0));snprintf(r->title,sizeof(r->title),"%s",sqlite3_column_text(q,1));snprintf(r->artist,sizeof(r->artist),"%s",sqlite3_column_text(q,2));snprintf(r->source,sizeof(r->source),"%s",sqlite3_column_text(q,3));r->position=sqlite3_column_int(q,4);snprintf(r->label,sizeof(r->label),"%s\n%s · %s",r->title,r->artist,r->source);rows[count++]=r->label;}sqlite3_finalize(q);
        if(count==100)rows[count]=Nxt_text("Load older tracks…","Xem các bài cũ hơn…");const char *title=mode==0?Nxt_text("Continue listening","Nghe tiếp"):mode==1?Nxt_text("Familiar mix","Những bài quen thuộc"):mode==2?Nxt_text("Long unheard","Lâu chưa nghe"):Nxt_text("Listening history","Lịch sử nghe");
        if(!count){const char *doc[]={Nxt_text("Listening history is saved for this profile after you listen. Switch profiles to see their own history. Radio and podcasts are not counted here.","Lịch sử được lưu riêng theo hồ sơ khi nghe nhạc. Hãy nghe một vài bài để có gợi ý. Radio và podcast không tính trong mục này.")};ModuleExitReason result=Nxt_document(d,title,doc,1);free(rows);free(data);return result;}
        NxtPick p=Nxt_pick(d,title,rows,count+(count==100),&selected,Nxt_text("A Play · X Favorite · L1 Newer · B Back","A Phát · X Yêu thích · L1 Mới hơn · B Về"));ModuleExitReason result=MODULE_EXIT_TO_MENU;
        if(p.action==1){if(selected==count){offset+=100;selected=0;}else result=play_rows(d,data,count,selected,mode==0);}
        if(p.action==3&&selected<count&&!strcmp(data[selected].source,"Local"))Nxt_favorite(data[selected].path);
        if(p.action==11&&offset){offset-=100;selected=0;}free(rows);free(data);if(p.action==9||result==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;if(p.action==2||Nxt_takeReturnHome())return MODULE_EXIT_TO_MENU;
    }
}
static ModuleExitReason unheard(DisplayContext *d){
    char path[1400];userdata_snpath("library.sqlite3",path,sizeof(path));sqlite3 *library=NULL;sqlite3_stmt *q=NULL;HistoryRow *data=calloc(500,sizeof(*data));const char **rows=calloc(500,sizeof(*rows));int count=0;
    if(data&&rows&&sqlite3_open_v2(path,&library,SQLITE_OPEN_READONLY,NULL)==SQLITE_OK&&sqlite3_prepare_v2(library,"SELECT path,title,artist FROM tracks ORDER BY title",-1,&q,NULL)==SQLITE_OK){while(count<500&&sqlite3_step(q)==SQLITE_ROW){const char *file=(const char*)sqlite3_column_text(q,0);sqlite3_stmt *heard=NULL;sqlite3_prepare_v2(db,"SELECT 1 FROM history WHERE listener=? AND path=? AND ms>0 LIMIT 1",-1,&heard,NULL);sqlite3_bind_int(heard,1,listener);sqlite3_bind_text(heard,2,file,-1,SQLITE_TRANSIENT);bool found=sqlite3_step(heard)==SQLITE_ROW;sqlite3_finalize(heard);if(found)continue;HistoryRow *r=&data[count];snprintf(r->path,sizeof(r->path),"%s",file);snprintf(r->title,sizeof(r->title),"%s",sqlite3_column_text(q,1));snprintf(r->artist,sizeof(r->artist),"%s",sqlite3_column_text(q,2));snprintf(r->source,sizeof(r->source),"Local");snprintf(r->label,sizeof(r->label),"%s\n%s",r->title,r->artist);rows[count++]=r->label;}}
    sqlite3_finalize(q);if(library)sqlite3_close(library);ModuleExitReason result=MODULE_EXIT_TO_MENU;int selected=0;
    if(count){NxtPick p=Nxt_pick(d,Nxt_text("Not heard in this profile","Chưa nghe trong hồ sơ này"),rows,count,&selected,Nxt_text("A Play · B Back","A Phát · B Về"));if(p.action==1)result=play_rows(d,data,count,selected,false);if(p.action==9)result=MODULE_EXIT_QUIT;}
    else{const char *doc[]={Nxt_text("Open Library first to index local music. This list shows up to 500 local tracks you have not played in this profile.","Mở Thư viện trước để quét nhạc. Mục này chọn tối đa 500 bài trong thẻ nhớ mà hồ sơ hiện tại chưa nghe.")};result=Nxt_document(d,Nxt_text("Not heard yet","Chưa nghe"),doc,1);}free(data);free(rows);return result;
}
static ModuleExitReason for_you_action(DisplayContext *d,int selected){switch(selected){case 0:case 1:return history_browser(d,selected);case 2:return unheard(d);case 3:return history_browser(d,2);case 4:return NxtSpotify_recommendations(d,false);case 5:return NxtSoundCloud_recommendations(d,false);case 6:return NxtSpotify_recommendations(d,true);default:return NxtSoundCloud_recommendations(d,true);}}
ModuleExitReason NxtPersonal_forYou(DisplayContext *d){int selected=0;for(;;){const char *rows[]={Nxt_text("Continue listening","Nghe tiếp"),Nxt_text("Familiar mix","Những bài quen thuộc"),Nxt_text("Not heard yet · local music","Chưa nghe · nhạc trong thẻ"),Nxt_text("Long unheard · 30 days","Lâu chưa nghe · 30 ngày"),Nxt_text("Spotify suggested tracks","Bài hát Spotify gợi ý"),Nxt_text("SoundCloud suggested tracks","Bài hát SoundCloud gợi ý"),Nxt_text("Spotify playable playlists","Playlist Spotify nghe được"),Nxt_text("SoundCloud playable playlists","Playlist SoundCloud nghe được")};char title[240];snprintf(title,sizeof(title),"%s · %s · %s",NxtCustom_mood(nxt.mood),NxtCustom_genre(nxt.genre),NxtPersonal_country(nxt.country));NxtPick p=Nxt_pick(d,title,rows,8,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action==1){ModuleExitReason r=for_you_action(d,selected);if(r==MODULE_EXIT_QUIT||Nxt_takeReturnHome())return r;}}}
static sqlite3_int64 statistic(const char *sql,const char *since){sqlite3_stmt *q=NULL;sqlite3_int64 result=0;sqlite3_prepare_v2(db,sql,-1,&q,NULL);sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,since,-1,SQLITE_TRANSIENT);if(sqlite3_step(q)==SQLITE_ROW)result=sqlite3_column_int64(q,0);sqlite3_finalize(q);return result;}
static ModuleExitReason detail_stats(DisplayContext *d,const char *since){
    char text[80][650];const char *rows[80];int n=0;sqlite3_stmt *q=NULL;snprintf(text[n],650,"%s",Nxt_text("DAILY LISTENING","NGHE THEO NGÀY"));rows[n]=text[n];n++;
    const char *queries[]={"SELECT day,sum(ms),sum(plays) FROM daily WHERE listener=? AND day>=? GROUP BY day ORDER BY day DESC LIMIT 31","SELECT artist,sum(ms),sum(qualified) FROM history WHERE listener=? AND date(last,'unixepoch','localtime')>=? AND artist<>'' GROUP BY artist ORDER BY sum(ms) DESC LIMIT 10","SELECT title,sum(ms),sum(qualified) FROM history WHERE listener=? AND date(last,'unixepoch','localtime')>=? GROUP BY path ORDER BY sum(ms) DESC LIMIT 10"};
    for(int i=0;i<3;i++){if(i){snprintf(text[n],650,"%s",i==1?Nxt_text("TOP ARTISTS","NGHỆ SĨ NGHE NHIỀU"):Nxt_text("TOP TRACKS","BÀI NGHE NHIỀU"));rows[n]=text[n];n++;}sqlite3_prepare_v2(db,queries[i],-1,&q,NULL);sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,since,-1,SQLITE_TRANSIENT);while(n<79&&sqlite3_step(q)==SQLITE_ROW){snprintf(text[n],650,"%s · %lld %s · %lld %s",sqlite3_column_text(q,0),(long long)sqlite3_column_int64(q,1)/60000,Nxt_text("min","phút"),(long long)sqlite3_column_int64(q,2),Nxt_text("plays","lượt"));rows[n]=text[n];n++;}sqlite3_finalize(q);q=NULL;}
    return Nxt_document(d,Nxt_text("Listening details","Chi tiết nghe nhạc"),rows,n);
}
static const char *personal_tabs[4];
static void personal_header(SDL_Surface *screen,int tab,bool mini){
    personal_tabs[0]=Nxt_text("For you","Dành cho bạn");personal_tabs[1]=Nxt_text("Statistics","Thống kê");personal_tabs[2]=Nxt_text("History","Lịch sử");personal_tabs[3]=Nxt_text("Profile","Hồ sơ");
    NxtSkin_begin(screen,Nxt_text("Personal","Cá nhân"),nickname,mini);NxtSkin_tabs(screen,personal_tabs,4,tab,145*screen->w/1024);
}
typedef struct {int id,period,days,changes;char today[16];uint32_t at;sqlite3_int64 ms,plays,artists,bars[31],sources[4];char since[16],labels[31][16],top_artist[180],top_track[240];} StatsSnapshot;
static StatsSnapshot stats={.id=-1,.period=-1};
static void stats_load(int period){
    uint32_t now=SDL_GetTicks();time_t stamp=time(NULL);struct tm tm;localtime_r(&stamp,&tm);char day[16];strftime(day,sizeof(day),"%Y-%m-%d",&tm);int changes=sqlite3_total_changes(db);
    if(stats.id==listener&&stats.period==period&&stats.changes==changes&&!strcmp(stats.today,day))return;
    memset(&stats,0,sizeof(stats));stats.id=listener;stats.period=period;stats.at=now;stats.changes=changes;snprintf(stats.today,sizeof(stats.today),"%s",day);int today=tm.tm_mday;
    if(period==1)tm.tm_mday-=6;if(period==2)tm.tm_mday=1;mktime(&tm);strftime(stats.since,sizeof(stats.since),"%Y-%m-%d",&tm);
    stats.ms=statistic("SELECT coalesce(sum(ms),0) FROM daily WHERE listener=? AND day>=?",stats.since);stats.plays=statistic("SELECT coalesce(sum(plays),0) FROM daily WHERE listener=? AND day>=?",stats.since);stats.artists=statistic("SELECT count(DISTINCT artist) FROM history WHERE listener=? AND date(last,'unixepoch','localtime')>=? AND ms>0 AND artist<>''",stats.since);
    const char *sources[]={"Local","Spotify","SoundCloud","Drive"};sqlite3_stmt *q=NULL;
    for(int i=0;i<4;i++){sqlite3_prepare_v2(db,"SELECT coalesce(sum(ms),0) FROM daily WHERE listener=? AND day>=? AND source=?",-1,&q,NULL);sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,stats.since,-1,SQLITE_TRANSIENT);sqlite3_bind_text(q,3,sources[i],-1,SQLITE_STATIC);if(sqlite3_step(q)==SQLITE_ROW)stats.sources[i]=sqlite3_column_int64(q,0);sqlite3_finalize(q);}
    localtime_r(&stamp,&tm);tm.tm_mday=period==2?1:tm.tm_mday-6;mktime(&tm);stats.days=period==2?today:7;if(stats.days>31)stats.days=31;
    for(int i=0;i<stats.days;i++){strftime(stats.labels[i],sizeof(stats.labels[i]),"%Y-%m-%d",&tm);sqlite3_prepare_v2(db,"SELECT coalesce(sum(ms),0) FROM daily WHERE listener=? AND day=?",-1,&q,NULL);sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,stats.labels[i],-1,SQLITE_TRANSIENT);if(sqlite3_step(q)==SQLITE_ROW)stats.bars[i]=sqlite3_column_int64(q,0);sqlite3_finalize(q);tm.tm_mday++;mktime(&tm);}
    const char *queries[]={"SELECT artist FROM history WHERE listener=? AND date(last,'unixepoch','localtime')>=? AND ms>0 AND artist<>'' GROUP BY artist ORDER BY sum(ms) DESC LIMIT 1","SELECT title FROM history WHERE listener=? AND date(last,'unixepoch','localtime')>=? AND ms>0 GROUP BY path ORDER BY sum(ms) DESC LIMIT 1"};
    for(int i=0;i<2;i++){sqlite3_prepare_v2(db,queries[i],-1,&q,NULL);sqlite3_bind_int(q,1,listener);sqlite3_bind_text(q,2,stats.since,-1,SQLITE_TRANSIENT);if(sqlite3_step(q)==SQLITE_ROW)snprintf(i?stats.top_track:stats.top_artist,i?sizeof(stats.top_track):sizeof(stats.top_artist),"%s",sqlite3_column_text(q,0));sqlite3_finalize(q);}
}
static void stats_draw(SDL_Surface *screen,int period){
    stats_load(period);int u=screen->w;const char *ranges[]={Nxt_text("Today","Hôm nay"),Nxt_text("7 days","7 ngày"),Nxt_text("This month","Tháng này")};char line[320];NxtSkin_text(screen,20,ranges[period],32*u/1024,204*u/1024,450*u/1024,NxtSkin_color(NXT_ACCENT));NxtSkin_text(screen,17,Nxt_text("← / → change period","← / → đổi thời gian"),632*u/1024,206*u/1024,360*u/1024,NxtSkin_color(NXT_MUTED));
    const char *names[]={Nxt_text("Listening time","Thời gian nghe"),Nxt_text("Plays","Lượt nghe"),Nxt_text("Artists","Nghệ sĩ")},*symbols[]={"headphones","music-2","mic-2"};
    for(int i=0;i<3;i++){SDL_Rect r={(32+i*325)*u/1024,242*u/1024,310*u/1024,86*u/1024};NxtSkin_panel(screen,r,NxtSkin_color(NXT_PANEL),12*u/1024);NxtSkin_icon(screen,symbols[i],r.x+16*u/1024,r.y+18*u/1024,26*u/1024,NxtSkin_color(NXT_ACCENT));NxtSkin_text(screen,17,names[i],r.x+57*u/1024,r.y+9*u/1024,r.w-66*u/1024,NxtSkin_color(NXT_MUTED));if(i==0)snprintf(line,sizeof(line),"%lldh %02lldm",(long long)stats.ms/3600000,(long long)(stats.ms/60000)%60);else snprintf(line,sizeof(line),"%lld",(long long)(i==1?stats.plays:stats.artists));NxtSkin_text(screen,28,line,r.x+57*u/1024,r.y+35*u/1024,r.w-66*u/1024,NxtSkin_color(NXT_INK));}
    SDL_Rect chart={32*u/1024,341*u/1024,554*u/1024,199*u/1024};NxtSkin_panel(screen,chart,NxtSkin_color(NXT_PANEL),12*u/1024);NxtSkin_text(screen,19,period==2?Nxt_text("Daily listening · this month","Nghe theo ngày · tháng này"):Nxt_text("Daily listening · last 7 days","Nghe theo ngày · 7 ngày gần nhất"),50*u/1024,351*u/1024,510*u/1024,NxtSkin_color(NXT_INK));
    sqlite3_int64 maximum=0;for(int i=0;i<stats.days;i++)if(stats.bars[i]>maximum)maximum=stats.bars[i];if(!maximum)maximum=60000;int left=55*u/1024,bottom=500*u/1024,width=506*u/1024,step=width/stats.days,gap=(stats.days>14?3:10)*u/1024;if(gap<1)gap=1;
    for(int i=0;i<stats.days;i++){int height=(int)(stats.bars[i]*(105*u/1024)/maximum);if(height<1)height=1;NxtSkin_rect(screen,(SDL_Rect){left+i*step,bottom-height,step-gap>0?step-gap:1,height},NxtSkin_color(NXT_ACCENT),3*u/1024);if(stats.days<=7||i%7==0||i==stats.days-1){snprintf(line,sizeof(line),"%d",atoi(stats.labels[i]+8));NxtSkin_text(screen,15,line,left+i*step,bottom+7*u/1024,step*2,NxtSkin_color(NXT_MUTED));}}
    const char *source_names[]={Nxt_text("SD music","Nhạc thẻ"),"Spotify","SoundCloud","Drive"};unsigned colors[]={0x83ddb0,0x72db94,0xf6b16e,0x97bdf4};NxtSkin_panel(screen,(SDL_Rect){603*u/1024,341*u/1024,389*u/1024,199*u/1024},NxtSkin_color(NXT_PANEL),12*u/1024);
    for(int i=0;i<4;i++){int y=(351+i*46)*u/1024;snprintf(line,sizeof(line),"%s · %lld %s",source_names[i],(long long)stats.sources[i]/60000,Nxt_text("min","phút"));NxtSkin_text(screen,17,line,621*u/1024,y,349*u/1024,NxtSkin_color(NXT_INK));NxtSkin_rect(screen,(SDL_Rect){621*u/1024,y+30*u/1024,348*u/1024,5*u/1024},NxtSkin_color(NXT_LINE),2);if(stats.ms){int w=(int)((348*u/1024)*stats.sources[i]/stats.ms);SDL_Color c={colors[i]>>16,(colors[i]>>8)&255,colors[i]&255,255};NxtSkin_rect(screen,(SDL_Rect){621*u/1024,y+30*u/1024,w,5*u/1024},c,2);}}
    snprintf(line,sizeof(line),"%s: %s",Nxt_text("Top artist","Nghe nhiều"),stats.top_artist[0]?stats.top_artist:Nxt_text("No listening data yet","Chưa có dữ liệu nghe"));NxtSkin_text(screen,19,line,32*u/1024,555*u/1024,960*u/1024,NxtSkin_color(NXT_INK));
}
ModuleExitReason NxtPersonal_statistics(DisplayContext *d){
    if(!db)return MODULE_EXIT_TO_MENU;NxtPersonal_flush();int period=0,show=0;uint32_t drawn=0;
    for(;;){ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);if(g.should_quit)return MODULE_EXIT_QUIT;bool dirty=g.dirty;
        if(!g.input_consumed&&!Nxt_lockInput()){if(NxtPad_justPressed(BTN_B))return MODULE_EXIT_TO_MENU;if(NxtPad_justPressed(BTN_LEFT)||NxtPad_justPressed(BTN_L1)){period=(period+2)%3;dirty=true;}if(NxtPad_justPressed(BTN_RIGHT)||NxtPad_justPressed(BTN_R1)){period=(period+1)%3;dirty=true;}if(NxtPad_justPressed(BTN_Y)){stats_load(period);if(detail_stats(d,stats.since)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;dirty=true;}}
        if(NxtSkin_shouldDraw(drawn,dirty)){NxtPersonal_flush();personal_header(screen,1,false);stats_draw(screen,period);NxtSkin_footer(screen,Nxt_text("←/→ Period · Y Details · B Back","←/→ Thời gian · Y Chi tiết · B Về"),false);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();}ModuleCommon_frameEnd(screen);
    }
}
static void personal_cards(SDL_Surface *screen,int tab,int selected){
    int u=screen->w;const char *labels[8],*sub[8],*icons[8];int top=tab==0?250:354,height=tab==0?75:103;int count=tab==0?8:4;
    if(tab==0){labels[0]=Nxt_text("Continue listening","Nghe tiếp");labels[1]=Nxt_text("Familiar mix","Những bài quen thuộc");labels[2]=Nxt_text("Not heard yet","Khám phá nhạc chưa nghe");labels[3]=Nxt_text("Long unheard","Lâu chưa nghe");sub[0]=Nxt_text("Return to your recent tracks","Trở lại những bài gần đây");sub[1]=Nxt_text("Music you spend time with","Các bài bạn thường nghe");sub[2]=Nxt_text("Find something new in your library","Chọn bài mới trong thư viện");sub[3]=Nxt_text("Rediscover tracks after 30 days","Gặp lại giai điệu sau 30 ngày");icons[0]="play";icons[1]="heart";icons[2]="avatar-sparkles";icons[3]="disc-3";
        labels[4]="Spotify · Tracks";labels[5]="SoundCloud · Tracks";labels[6]="Spotify · Playlists";labels[7]="SoundCloud · Playlists";sub[4]=sub[5]=Nxt_text("Suggested from your preferences","Gợi ý từ sở thích của bạn");sub[6]=Nxt_text("NXT Mix + verified account playlists","NXT Mix + playlist đọc được");sub[7]=Nxt_text("Eligible full-length public tracks","Bài công khai có quyền phát đầy đủ");icons[4]=icons[6]="disc-3";icons[5]=icons[7]="avatar-waves";
        time_t birth_now=time(NULL);struct tm birth_tm;localtime_r(&birth_now,&birth_tm);bool birthday=nxt.birth_day==birth_tm.tm_mday&&nxt.birth_month==birth_tm.tm_mon+1;NxtSkin_text(screen,26,birthday?Nxt_text("Happy birthday! Your music is ready.","Chúc mừng sinh nhật! Nhạc đã sẵn sàng."):Nxt_text("Your music, your space","Nhạc của bạn, góc riêng của bạn"),32*u/1024,207*u/1024,960*u/1024,NxtSkin_color(NXT_INK));
    }else{labels[0]=Nxt_text("Edit profile","Sửa hồ sơ");labels[1]=Nxt_text("Switch listener","Đổi người nghe");labels[2]=Nxt_text("Add listener","Thêm người nghe");labels[3]=Nxt_text("Personal appearance","Giao diện cá nhân");sub[0]=Nxt_text("Nickname · avatar · birthday","Biệt danh · avatar · sinh nhật");sub[1]=Nxt_text("Each listener has their own space","Mỗi người có dữ liệu riêng");sub[2]=Nxt_text("Create a new local profile","Tạo hồ sơ mới trên máy");sub[3]=Nxt_text("Themes · original options · LED · spectrum","Giao diện · tùy chọn gốc · LED · phổ nhạc");icons[0]="pencil";icons[1]="headphones";icons[2]="plus";icons[3]="sun-moon";
        NxtSkin_panel(screen,(SDL_Rect){32*u/1024,207*u/1024,960*u/1024,130*u/1024},NxtSkin_color(NXT_PANEL),14*u/1024);NxtSkin_icon(screen,NxtPersonal_avatar(),58*u/1024,231*u/1024,74*u/1024,NxtSkin_color(NXT_ACCENT));NxtSkin_text(screen,32,nickname,161*u/1024,222*u/1024,797*u/1024,NxtSkin_color(NXT_INK));NxtSkin_text(screen,19,recording?Nxt_text("Listening history on · local profile","Đang ghi lịch sử nghe · hồ sơ trên máy"):Nxt_text("Listening history off · local profile","Đã tắt ghi lịch sử · hồ sơ trên máy"),161*u/1024,278*u/1024,797*u/1024,NxtSkin_color(NXT_MUTED));
    }
    for(int i=0;i<count;i++){int x=32+(i%2)*488,y=top+(i/2)*(height+10);SDL_Rect r={x*u/1024,y*u/1024,474*u/1024,height*u/1024};SDL_Color ink=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK),muted=i==selected?ink:NxtSkin_color(NXT_MUTED);if(i==selected)NxtSkin_rect(screen,r,NxtSkin_color(NXT_ACCENT),13*u/1024);else NxtSkin_panel(screen,r,NxtSkin_color(NXT_PANEL),13*u/1024);NxtSkin_icon(screen,icons[i],r.x+18*u/1024,r.y+18*u/1024,29*u/1024,ink);NxtSkin_text(screen,23,labels[i],r.x+63*u/1024,r.y+13*u/1024,r.w-79*u/1024,ink);NxtSkin_text(screen,17,sub[i],r.x+18*u/1024,r.y+(tab==0?43:60)*u/1024,r.w-36*u/1024,muted);}
}
static int recent_load(HistoryRow *rows,int offset){
    NxtPersonal_flush();sqlite3_stmt *q=NULL;int n=0;if(sqlite3_prepare_v2(db,"SELECT path,title,artist,source,position,last,ms FROM history WHERE listener=? AND ms>0 ORDER BY last DESC,id DESC LIMIT 100 OFFSET ?",-1,&q,NULL)==SQLITE_OK){sqlite3_bind_int(q,1,listener);sqlite3_bind_int(q,2,offset);while(n<100&&sqlite3_step(q)==SQLITE_ROW){HistoryRow *r=&rows[n++];memset(r,0,sizeof(*r));snprintf(r->path,sizeof(r->path),"%s",sqlite3_column_text(q,0));snprintf(r->title,sizeof(r->title),"%s",sqlite3_column_text(q,1));snprintf(r->artist,sizeof(r->artist),"%s",sqlite3_column_text(q,2));snprintf(r->source,sizeof(r->source),"%s",sqlite3_column_text(q,3));r->position=sqlite3_column_int(q,4);r->last=sqlite3_column_int64(q,5);r->listened=sqlite3_column_int64(q,6);}}sqlite3_finalize(q);return n;
}
static void recent_draw(SDL_Surface *screen,HistoryRow *rows,int count,int selected){
    int u=screen->w,first=selected>1?selected-1:0;if(first>count-4)first=count-4;if(first<0)first=0;
    if(!count){NxtSkin_icon(screen,"disc-3",450*u/1024,275*u/1024,124*u/1024,NxtSkin_color(NXT_ACCENT));NxtSkin_text(screen,27,Nxt_text("Your listening journey starts here","Hành trình nghe nhạc bắt đầu ở đây"),128*u/1024,425*u/1024,768*u/1024,NxtSkin_color(NXT_INK));NxtSkin_text(screen,19,Nxt_text("Play a track to build your own history.","Phát vài bài để tạo lịch sử nghe riêng của bạn."),128*u/1024,482*u/1024,768*u/1024,NxtSkin_color(NXT_MUTED));return;}
    int total=count+(count==100);
    for(int i=first;i<first+4&&i<total;i++){SDL_Rect r={32*u/1024,(211+(i-first)*90)*u/1024,960*u/1024,82*u/1024};SDL_Color ink=NxtSkin_color(i==selected?NXT_ACCENT_INK:NXT_INK);if(i==selected)NxtSkin_rect(screen,r,NxtSkin_color(NXT_ACCENT),11*u/1024);else NxtSkin_panel(screen,r,NxtSkin_color(NXT_PANEL),11*u/1024);
        if(i==count){NxtSkin_text(screen,23,Nxt_text("Load older history…","Xem lịch sử cũ hơn…"),r.x+20*u/1024,r.y+24*u/1024,r.w-40*u/1024,ink);continue;}
        HistoryRow *entry=&rows[i];const char *symbol=!strcmp(entry->source,"Spotify")?"disc-3":!strcmp(entry->source,"SoundCloud")?"avatar-waves":!strcmp(entry->source,"Drive")?"folder":"music-2";
        NxtSkin_icon(screen,symbol,r.x+18*u/1024,r.y+24*u/1024,34*u/1024,ink);NxtSkin_text(screen,23,entry->title,r.x+72*u/1024,r.y+7*u/1024,r.w-280*u/1024,ink);char label[400];snprintf(label,sizeof(label),"%s · %s",entry->artist,entry->source);NxtSkin_text(screen,17,label,r.x+72*u/1024,r.y+44*u/1024,r.w-280*u/1024,ink);
        struct tm tm;localtime_r(&entry->last,&tm);strftime(label,sizeof(label),"%d/%m · %H:%M",&tm);NxtSkin_text(screen,16,label,r.x+r.w-190*u/1024,r.y+12*u/1024,175*u/1024,ink);snprintf(label,sizeof(label),"%lld:%02lld %s",(long long)entry->listened/60000,(long long)(entry->listened/1000)%60,Nxt_text("heard","đã nghe"));NxtSkin_text(screen,15,label,r.x+r.w-190*u/1024,r.y+43*u/1024,175*u/1024,ink);
    }
}
ModuleExitReason NxtPersonal_run(DisplayContext *d){
    if(!db)return NxtPersonal_login(d);HistoryRow *recent=calloc(100,sizeof(*recent));if(!recent)return MODULE_EXIT_TO_MENU;int tab=0,selection[4]={0},period=0,offset=0,count=0,show=0,oldtab=-1;int viewed_listener=listener;uint32_t drawn=0,loaded=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;
    for(;;){ModuleCommon_frameBegin();SDL_Surface *screen=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(screen,&show,HELP_NONE);if(g.should_quit){result=MODULE_EXIT_QUIT;break;}bool dirty=g.dirty;ModuleExitReason action=MODULE_EXIT_TO_MENU;
        if(!g.input_consumed&&!Nxt_lockInput()){
            if(NxtPad_justPressed(BTN_B))break;
            if(NxtPad_justPressed(BTN_L1)){tab=(tab+3)%4;dirty=true;}if(NxtPad_justPressed(BTN_R1)){tab=(tab+1)%4;dirty=true;}
            if(tab==1){if(NxtPad_justPressed(BTN_LEFT)){period=(period+2)%3;dirty=true;}if(NxtPad_justPressed(BTN_RIGHT)){period=(period+1)%3;dirty=true;}if(NxtPad_justPressed(BTN_Y)){stats_load(period);action=detail_stats(d,stats.since);dirty=true;}}
            else if(tab==0||tab==3){if(tab==3&&NxtPad_justPressed(BTN_X)){action=confirm_delete_listener(d,listener,nickname);oldtab=-1;dirty=true;}int *selected=&selection[tab];if(NxtPad_justRepeated(BTN_UP)||NxtPad_justRepeated(BTN_DOWN)){*selected=(*selected+(NxtPad_justRepeated(BTN_UP)?(tab==0?6:2):2))%(tab==0?8:4);dirty=true;}if(NxtPad_justRepeated(BTN_LEFT)||NxtPad_justRepeated(BTN_RIGHT)){*selected^=1;dirty=true;}
                if(NxtPad_justPressed(BTN_A)){if(tab==0)action=for_you_action(d,*selected);else if(*selected==0)action=profile_edit(d);else if(*selected==1)action=choose_listener(d,false);else if(*selected==2)create_listener();else action=NxtAppearance_run(d);oldtab=-1;dirty=true;}}
            else if(tab==2){int total=count+(count==100);if(NxtPad_justRepeated(BTN_UP)&&total){selection[2]=(selection[2]+total-1)%total;dirty=true;}if(NxtPad_justRepeated(BTN_DOWN)&&total){selection[2]=(selection[2]+1)%total;dirty=true;}if(NxtPad_justPressed(BTN_L2)&&offset){offset-=100;loaded=0;selection[2]=0;dirty=true;}if(NxtPad_justPressed(BTN_R2)&&count==100){offset+=100;loaded=0;selection[2]=0;dirty=true;}if(NxtPad_justPressed(BTN_A)&&total){if(selection[2]==count){offset+=100;loaded=0;selection[2]=0;}else action=play_rows(d,recent,count,selection[2],true);dirty=true;}if(NxtPad_justPressed(BTN_X)&&selection[2]<count&&!strcmp(recent[selection[2]].source,"Local")){Nxt_favorite(recent[selection[2]].path);dirty=true;}}
        }
        if(action==MODULE_EXIT_QUIT){result=action;break;}if(Nxt_takeReturnHome())break;
        if(viewed_listener!=listener){viewed_listener=listener;offset=count=0;loaded=drawn=0;oldtab=-1;stats.id=-1;memset(selection,0,sizeof(selection));dirty=true;}
        if(tab==2&&!PWR_getDisplayOff()&&!NxtSession_isHidden()&&(oldtab!=tab||!loaded||SDL_GetTicks()-loaded>=5000)){count=recent_load(recent,offset);int total=count+(count==100);if(selection[2]>=total)selection[2]=total?total-1:0;loaded=SDL_GetTicks();dirty=true;}
        if(NxtSkin_shouldDraw(drawn,dirty||oldtab!=tab)){NxtPersonal_flush();personal_header(screen,tab,true);if(tab==1)stats_draw(screen,period);else if(tab==2)recent_draw(screen,recent,count,selection[2]);else personal_cards(screen,tab,selection[tab]);
            const char *hint=tab==1?Nxt_text("L1/R1 Tabs · ←/→ Period · Y Details · B Back","L1/R1 Tab · ←/→ Thời gian · Y Chi tiết · B Về"):tab==2?Nxt_text("L1/R1 Tabs · A Play · X Favorite · L2/R2 Pages · B Back","L1/R1 Tab · A Phát · X Thích · L2/R2 Trang · B Về"):tab==3?Nxt_text("L1/R1 Tabs · A Open · X Delete profile · B Back","L1/R1 Tab · A Mở · X Xóa hồ sơ · B Về"):Nxt_text("L1/R1 Tabs · D-pad Select · A Open · B Back","L1/R1 Tab · D-pad Chọn · A Mở · B Về");NxtSkin_footer(screen,hint,false);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();oldtab=tab;
        }ModuleCommon_frameEnd(screen);
    }
    free(recent);return result;
}
