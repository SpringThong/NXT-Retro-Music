/* SPDX-License-Identifier: GPL-3.0-only
 * Profile-local startup card. No login, questionnaire or network request until
 * the listener plays a saved online track. Saves are atomic and bounded. */
#include "nxt_startup.h"
#include "nxt_write.h"
#include "nxt_playback.h"
#include "nxt_personal.h"
#include "nxt_controls.h"
#include "nxt_skin.h"
#include "nxt_decoder.h"
#include "nxt_drive.h"
#include "nxt_bitperfect.h"
#include "module_player.h"
#include "background.h"
#include "resume.h"
#include "radio.h"
#include "podcast.h"
#include "file_utils.h"
#include "album_art.h"
#include "toast.h"
#include "api.h"
#include "parson/parson.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
static TrackInfo saved;
static char saved_path[512],source[24],written_path[512],pending_source[24],pending_path[512];
static int position,profile=-1,written_position=-1,pending_position;
static bool preview,art_written;
static SDL_Surface *cover;
static uint32_t last_save,pending_until;
static PlayerState last_state=PLAYER_STATE_STOPPED;
bool NxtStartup_preview(void){return preview;}
const TrackInfo *NxtStartup_info(void){return &saved;}
const char *NxtStartup_path(void){return saved_path;}
int NxtStartup_position(void){return position;}
SDL_Surface *NxtStartup_art(void){return cover;}
void NxtStartup_quit(void){NxtSkin_forgetArt();if(cover)SDL_FreeSurface(cover);cover=NULL;preview=false;}
static int number(JSON_Object *o,const char *name){double n=json_object_get_number(o,name);return n>=0&&n<=INT_MAX?(int)n:0;}
static void text(JSON_Object *o,const char *name,char *out,size_t size){const char *s=json_object_get_string(o,name);snprintf(out,size,"%s",s?s:"");}
static void read_card(void){
    NxtWrite_flush();
    NxtStartup_quit();memset(&saved,0,sizeof(saved));saved_path[0]=source[0]=0;position=0;
    char path[1400];userdata_snpath("last-playback.json",path,sizeof(path));
    JSON_Value *v=json_parse_file(path);JSON_Object *o=json_value_get_object(v);
    if(o){text(o,"path",saved_path,sizeof(saved_path));text(o,"source",source,sizeof(source));
        text(o,"title",saved.title,sizeof(saved.title));text(o,"artist",saved.artist,sizeof(saved.artist));text(o,"album",saved.album,sizeof(saved.album));text(o,"codec",saved.codec,sizeof(saved.codec));
        position=number(o,"position_ms");saved.duration_ms=number(o,"duration_ms");saved.sample_rate=number(o,"sample_rate");saved.channels=number(o,"channels");saved.bit_depth=number(o,"bit_depth");saved.bitrate=number(o,"bitrate");}
    json_value_free(v);
    if(!*saved_path){const ResumeState *r=Resume_getState();if(r){snprintf(saved_path,sizeof(saved_path),"%s",r->track_path);snprintf(saved.title,sizeof(saved.title),"%s",r->track_name);position=r->position_ms;snprintf(source,sizeof(source),"Local");}}
    if(saved.duration_ms>0&&position>=saved.duration_ms)position=0;
    userdata_snpath("last-art.png",path,sizeof(path));if(*saved_path)cover=IMG_Load(path);
    profile=NxtPersonal_id();written_position=position;snprintf(written_path,sizeof(written_path),"%s",saved_path);art_written=cover!=NULL;preview=true;
}
void NxtStartup_save(void){
    if(preview||Radio_isActive()||Podcast_isActive()||Player_resume()||(nxt.bit_perfect&&!NxtBit_ready()))return;
    if(*pending_source&&(int32_t)(SDL_GetTicks()-pending_until)<0)return;
    const char *path=NxtPlayback_path();const TrackInfo *info=NxtPlayback_info();
    if(!path||!*path||!info||!*info->title||NxtPlayback_state()==PLAYER_STATE_STOPPED)return;
    if(profile!=NxtPersonal_id()){profile=NxtPersonal_id();written_path[0]=0;written_position=-1;art_written=false;}
    int ms=NxtPlayback_position();bool changed=strcmp(path,written_path)!=0;if(changed)art_written=false;
    SDL_Surface *art=NxtPlayback_art();if(!art&&!NxtSpotify_hasMetadata()&&!NxtSoundCloud_hasMetadata())art=album_art_get();
    if(art&&!art_written){char file[1400];userdata_snpath("last-art.png",file,sizeof(file));art_written=NxtWrite_image(file,art);}
    if(!changed&&ms==written_position)return;
    JSON_Value *v=json_value_init_object();JSON_Object *o=json_value_get_object(v);
    json_object_set_string(o,"path",path);json_object_set_string(o,"source",NxtSoundCloud_hasMetadata()?"SoundCloud":NxtSpotify_hasMetadata()?"Spotify":NxtDrive_isStream(path)?"Drive":"Local");
    json_object_set_string(o,"title",info->title);json_object_set_string(o,"artist",info->artist);json_object_set_string(o,"album",info->album);json_object_set_string(o,"codec",info->codec);
    json_object_set_number(o,"position_ms",ms);json_object_set_number(o,"duration_ms",info->duration_ms);json_object_set_number(o,"sample_rate",info->sample_rate);json_object_set_number(o,"bit_depth",info->bit_depth);json_object_set_number(o,"channels",info->channels);json_object_set_number(o,"bitrate",info->bitrate);
    char file[1400];userdata_snpath("last-playback.json",file,sizeof(file));
    char *json=json_serialize_to_string(v);bool ok=json&&NxtWrite_text(file,json,true);json_free_serialized_string(json);json_value_free(v);if(!ok)return;
    if(changed&&!art_written){userdata_snpath("last-art.png",file,sizeof(file));NxtWrite_remove(file);}
    snprintf(written_path,sizeof(written_path),"%s",path);written_position=ms;
}
void NxtStartup_tick(void){uint32_t now=SDL_GetTicks();PlayerState state=NxtPlayback_state();if(state!=last_state||(uint32_t)(now-last_save)>=10000){NxtStartup_save();last_save=now;last_state=state;}}
int NxtStartup_takeSeek(const char *provider,const char *path){if(!strcmp(pending_source,provider)){bool valid=path&&!strcmp(path,pending_path)&&(int32_t)(SDL_GetTicks()-pending_until)<0;int ms=pending_position;pending_source[0]=0;return valid?ms:-1;}return -1;}
static ModuleExitReason resume_local(DisplayContext *d,bool play){
    const ResumeState *r=Resume_getState();ResumeState fallback={0};
    if(!r||strcmp(r->track_path,saved_path)){fallback.type=RESUME_TYPE_FILES;snprintf(fallback.track_path,sizeof(fallback.track_path),"%s",saved_path);snprintf(fallback.track_name,sizeof(fallback.track_name),"%s",saved.title);snprintf(fallback.folder_path,sizeof(fallback.folder_path),"%s",saved_path);char *slash=strrchr(fallback.folder_path,'/');if(slash)*slash=0;fallback.position_ms=position;r=&fallback;}
    ResumeState snapshot=*r;snapshot.position_ms=position;preview=false;
    ModuleExitReason result=PlayerModule_runResumeMode(d,&snapshot,play);
    if(result!=MODULE_EXIT_QUIT&&!PlayerModule_isActive())preview=true;
    return result;
}
ModuleExitReason NxtStartup_run(DisplayContext *d){
    read_card();bool local=*saved_path&&strcmp(source,"Spotify")&&strcmp(source,"SoundCloud");
    if(local&&(NxtDrive_isStream(saved_path)||access(saved_path,R_OK)==0)){
        ModuleExitReason r=resume_local(d,nxt.auto_play);if(r==MODULE_EXIT_QUIT||!preview)return r;
    }
    if(local&&!NxtDrive_isStream(saved_path)&&access(saved_path,R_OK))Toast_show(Nxt_text("Saved file is missing; B opens the menu","Không tìm thấy file đã lưu; B về menu"),4000);
    int show=0;bool dirty=true,auto_try=nxt.auto_play&&*saved_path;uint32_t drawn=0;
    for(;;){
        ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);
        if(g.should_quit||ModuleCommon_stopSignalled())return MODULE_EXIT_QUIT;
        if(!g.input_consumed&&!Nxt_lockInput()){
            if(NxtControl_pressed(NC_MENU)||NxtPad_justPressed(BTN_B)){return NxtSession_playerBack();}
            if(NxtControl_pressed(NC_EQ)){if(Nxt_dspScreen(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;dirty=true;}
            else if(NxtControl_pressed(NC_QUICK)){if(Nxt_quickOptions(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;dirty=true;}
            else if(NxtControl_pressed(NC_LYRICS)){if(Nxt_lyricsScreen(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;dirty=true;}
            else if(NxtControl_pressed(NC_QUEUE)){Toast_show(Nxt_text("Press A to restore the saved queue","Nhấn A để khôi phục danh sách đã lưu"),2000);dirty=true;}
            else if(auto_try||NxtControl_pressed(NC_PLAY)||NxtPad_justPressed(BTN_A)){
                auto_try=false;if(!*saved_path){preview=false;return Nxt_library(d);}
                if(local){if(!NxtDrive_isStream(saved_path)&&access(saved_path,R_OK)){Toast_show(Nxt_text("Saved file is missing. B opens the menu to choose another track.","Không tìm thấy file đã lưu. B về menu để chọn bài khác."),3500);dirty=true;}else{ModuleExitReason r=resume_local(d,true);if(r==MODULE_EXIT_QUIT||!preview)return r;Toast_show(nxt.bit_perfect?NxtBit_error():Nxt_text("Cannot open saved track","Không mở được bài đã lưu"),3500);dirty=true;}}
                else if(nxt.bit_perfect){Toast_show(Nxt_text("Choose Normal mode for this streaming source","Chọn chế độ Thường cho nguồn streaming này"),3000);dirty=true;}
                else{
                    preview=false;snprintf(pending_source,sizeof(pending_source),"%s",source);snprintf(pending_path,sizeof(pending_path),"%s",saved_path);pending_position=position;pending_until=SDL_GetTicks()+60000;
                    ModuleExitReason r=!strcmp(source,"Spotify")?NxtSpotify_playSaved(d,saved_path,saved.title,saved.artist):NxtSoundCloud_playSaved(d,saved_path,saved.title,saved.artist);
                    pending_source[0]=0;return r;
                }
            }
        }
        if(NxtSkin_shouldDraw(drawn,dirty||g.dirty)){
            if(*saved_path){Nxt_renderPlaying(s,show,1,1,nxt.shuffle,nxt.repeat_mode);}
            else{NxtSkin_begin(s,"Now Playing",NxtPersonal_name(),false);int u=s->w;NxtSkin_icon(s,"music-2",u*45/100,s->h/3,u/10,NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,28,Nxt_text("Your music starts here","Bắt đầu nghe nhạc tại đây"),u/10,s->h*57/100,u*8/10,NxtSkin_color(NXT_INK));NxtSkin_text(s,21,Nxt_text("A opens the music library","A mở thư viện nhạc"),u/10,s->h*66/100,u*8/10,NxtSkin_color(NXT_MUTED));NxtSkin_footer(s,Nxt_text("A Library · SELECT EQ · START Options · B Menu","A Thư viện · SELECT EQ · START Tùy chọn · B Menu"),false);}
            ModuleCommon_markSurfaceDrawn();dirty=false;drawn=SDL_GetTicks();
        }
        int changed=dirty;ModuleCommon_PWR_update(&changed,&show);dirty|=changed!=0;ModuleCommon_frameEnd(s);
    }
}
