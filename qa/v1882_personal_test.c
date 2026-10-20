/* The real SQLite sampler/profile routing; audio/UI boundaries are fixtures. */
#include "nxt_personal.c"
#define play_rows library_play_rows
#include "nxt_library.c"
#undef play_rows
#include <assert.h>
NxtConfig nxt;
static uint32_t now=1000;static int64_t audio_ms;static PlayerState playback=PLAYER_STATE_PLAYING;static bool ready=true;
static TrackInfo info={.title="Bài thử",.artist="Nghệ sĩ",.duration_ms=200000};static char file[512]="/Music/01.mp3";static unsigned gen=1;
Uint32 SDL_GetTicks(void){return now;}
int64_t Player_getListeningTime(void){return audio_ms;}
bool Player_outputReady(void){return ready;}
PlayerState Player_getState(void){return playback;}
const TrackInfo *Player_getTrackInfo(void){return &info;}
const char *Player_getCurrentFile(void){return file;}
int Player_getPosition(void){return last_position;}
int Player_getDuration(void){return info.duration_ms;}
unsigned Player_getTrackGeneration(void){return gen;}
#include "soundcloud_stub.inc"
bool NxtSpotify_hasMetadata(void){return false;}
bool NxtSpotify_active(void){return false;}
bool NxtSpotify_isPlaying(void){return false;}
const TrackInfo *NxtSpotify_info(void){return &info;}
const char *NxtSpotify_uri(void){return "";}
int NxtSpotify_position(void){return 0;}
unsigned NxtSpotify_generation(void){return 0;}
bool NxtDrive_isPath(const char *path){return !strncmp(path,"gdrive:",7);}
bool Radio_isActive(void){return false;}
bool Podcast_isActive(void){return false;}
const char *Nxt_text(const char *a,const char *b){return b;}
void NxtLog_step(const char *c,const char *s,bool ok,const char *fmt,...){}
void Nxt_loadPreferences(const char *path){assert(strstr(path,"preferences.json"));}
static bool save_fails;static int startup_quits;
bool NxtStartup_preview(void){return false;}
const TrackInfo *NxtStartup_info(void){return &info;}
const char *NxtStartup_path(void){return "";}
int NxtStartup_position(void){return 0;}
void NxtStartup_quit(void){startup_quits++;}
char *Keyboard_open(const char *p,size_t n){return strdup("New listener");}
static int module_quits;
void Nxt_stop(void){NxtPersonal_flush();}
void PlayerModule_quit(void){module_quits++;}
void NxtAppearance_release(void){}
void Fonts_load(void){}void Fonts_unload(void){}void Resume_init(void){}
void LOG_note(int level,const char *f,...){}
char *NxtText_searchKey(const char *s){return strdup(s?s:"");}
int NxtSort_compare(const char *a,const char *b){return strcmp(a,b);}
ToastToken Toast_show(const char *s,uint32_t ms){return 0;}
bool Nxt_save(void){if(save_fails)return false;char path[1400];userdata_snpath("nxt.json",path,sizeof(path));FILE *f=fopen(path,"w");assert(f);fputs("{\"theme\":1}",f);fclose(f);NxtPersonal_savePreferences();return true;}
static long long scalar(const char *sql){sqlite3_stmt *q=NULL;assert(sqlite3_prepare_v2(db,sql,-1,&q,NULL)==SQLITE_OK);assert(sqlite3_step(q)==SQLITE_ROW);long long value=sqlite3_column_int64(q,0);sqlite3_finalize(q);return value;}
static void sample(int ms,int produced){now+=ms;audio_ms+=produced;NxtPersonal_tick();}
static int queue_calls,provider_calls,resume_calls;
static bool can_resume;
static ResumeState resume_state;
AudioFormat Player_detectFormat(const char *path){return strstr(path,".wav")?AUDIO_FORMAT_WAV:AUDIO_FORMAT_MP3;}
bool Resume_isAvailable(void){return can_resume;}
const ResumeState *Resume_getState(void){return &resume_state;}
ModuleExitReason PlayerModule_runResume(DisplayContext *d,const ResumeState *state){assert(state==&resume_state);resume_calls++;return MODULE_EXIT_TO_MENU;}
void PlayerModule_setResumePlaylistPath(const char *path){assert(!*path);}
ModuleExitReason PlayerModule_runWithPlaylist(DisplayContext *d,PlaylistTrack *tracks,int count,int selected){
 assert(count==3&&selected==1);assert(!strcmp(tracks[0].path,"/Music/one.mp3"));
 assert(!strcmp(tracks[1].path,"nxtdrive://file-two/audio.wav"));assert(!strcmp(tracks[2].path,"/Music/three.mp3"));
 queue_calls++;return MODULE_EXIT_TO_MENU;
}
ModuleExitReason NxtSpotify_playSaved(DisplayContext *d,const char *path,const char *title,const char *artist){assert(!strncmp(path,"spotify:track:",14));provider_calls++;return MODULE_EXIT_TO_MENU;}
ModuleExitReason NxtSoundCloud_playSaved(DisplayContext *d,const char *path,const char *title,const char *artist){assert(strstr(path,"soundcloud.com/"));provider_calls++;return MODULE_EXIT_TO_MENU;}
static void mixed_queue(void){
 HistoryRow rows[5]={
  {.path="/Music/one.mp3",.title="One",.source="Local"},
  {.path="spotify:track:1234567890123456789012",.title="Spotify",.source="Spotify"},
  {.path="nxtdrive://file-two/audio.wav",.title="Two",.source="Drive"},
  {.path="https://soundcloud.com/test/track",.title="Cloud",.source="SoundCloud"},
  {.path="/Music/three.mp3",.title="Three",.source="Local"},
 };
 assert(play_rows(NULL,rows,5,2,false)==MODULE_EXIT_TO_MENU&&queue_calls==1);
 assert(play_rows(NULL,rows,5,1,false)==MODULE_EXIT_TO_MENU&&provider_calls==1);
 assert(play_rows(NULL,rows,5,3,false)==MODULE_EXIT_TO_MENU&&provider_calls==2);
 can_resume=true;strcpy(resume_state.track_path,rows[2].path);
 assert(play_rows(NULL,rows,5,2,true)==MODULE_EXIT_TO_MENU&&resume_calls==1&&queue_calls==1);
 assert(play_rows(NULL,rows,5,-1,false)==MODULE_EXIT_TO_MENU&&queue_calls==1);
 assert(play_rows(NULL,rows,5,5,false)==MODULE_EXIT_TO_MENU&&queue_calls==1);
 puts("PASS mixed-source history selects the correct provider, keeps only Local/Drive in the decoder queue, preserves selected index and resume, and rejects invalid selection.");
}
static void fixture_file(const char *relative,const char *text){char path[1500];userdata_snpath(relative,path,sizeof(path));char parent[1500];snprintf(parent,sizeof(parent),"%s",path);char *slash=strrchr(parent,'/');assert(slash);*slash=0;assert(mkdir_p(parent));FILE *f=fopen(path,"w");assert(f);fputs(text,f);fclose(f);}
static bool fixture_exists(const char *relative){char path[1500];userdata_snpath(relative,path,sizeof(path));return access(path,F_OK)==0;}
static void deletion_checks(void){
 userdata_mkdir("");NxtPersonal_init();assert(db&&listener==0);
 assert(exec("INSERT INTO listeners(id,name) VALUES(2,'Delete me'),(3,'Keep me');INSERT INTO history(listener,path,title,artist,source,started,last,ms) VALUES(2,'/Music/keep.flac','Keep song','Artist','Local',1,1,1000);INSERT INTO daily VALUES(2,'2026-01-01','Local',1000,1);"));
 fixture_file("credentials.json","SHARED_SPOTIFY");fixture_file("translation-service.json","SHARED_KEY");fixture_file("translation-cache/keep.json","SHARED_CACHE");fixture_file("Music/keep.flac","MUSIC_KEEP");
 fixture_file("listeners/0/preferences.json","ZERO_KEEP");fixture_file("listeners/1/private.txt","ONE_PRIVATE");fixture_file("listeners/2/nested/private.txt","TWO_PRIVATE");fixture_file("listeners/3/private.txt","THREE_KEEP");
 char linkpath[1500],target[1500];userdata_snpath("listeners/2/shortcut",linkpath,sizeof(linkpath));userdata_snpath("Music",target,sizeof(target));assert(!symlink(target,linkpath));
 Nxt_favorite("/Music/zero.flac");assert(switch_listener(2));Nxt_favorite("/Music/two.flac");assert(switch_listener(1));Nxt_favorite("/Music/one.flac");
 assert(exec("CREATE TRIGGER keep_profile BEFORE DELETE ON listeners WHEN OLD.id=2 BEGIN SELECT RAISE(ABORT,'fixture refusal'); END;"));assert(!delete_listener(2));assert(scalar("SELECT count(*) FROM listeners WHERE id=2")==1&&scalar("SELECT count(*) FROM history WHERE listener=2")==1);assert(fixture_exists("listeners/2/nested/private.txt"));assert(switch_listener(2)&&Nxt_isFavorite("/Music/two.flac"));assert(switch_listener(1));assert(exec("DROP TRIGGER keep_profile;"));
 char queued[1500];userdata_snpath("listeners/2/preferences.json",queued,sizeof(queued));assert(NxtWrite_text(queued,"QUEUED_PRIVATE",true));assert(delete_listener(2));assert(listener==1&&scalar("SELECT count(*) FROM listeners WHERE id=2")==0&&scalar("SELECT count(*) FROM history WHERE listener=2")==0&&scalar("SELECT count(*) FROM daily WHERE listener=2")==0);assert(!fixture_exists("listeners/2")&&fixture_exists("Music/keep.flac"));
 save_fails=true;assert(!delete_listener(1)&&listener==1&&scalar("SELECT count(*) FROM listeners WHERE id=1")==1);save_fails=false;
 assert(delete_listener(1)&&listener==0&&startup_quits==1);assert(!fixture_exists("listeners/1")&&Nxt_isFavorite("/Music/zero.flac"));assert(scalar("SELECT value FROM app WHERE key='activeListener'")==0);
 fixture_file("playlists/zero.m3u","/Music/keep.flac");assert(delete_listener(0)&&listener==3&&startup_quits==2);assert(!fixture_exists("listeners/0")&&!fixture_exists("playlists"));assert(fixture_exists("listeners/3/private.txt"));
 assert(!delete_listener(3)&&listener==3&&scalar("SELECT count(*) FROM listeners")==1);assert(!delete_listener(-1)&&!delete_listener(16)&&!delete_listener(2));
 Nxt_libraryQuit();NxtPersonal_quit();NxtPersonal_init();assert(listener==3&&scalar("SELECT count(*) FROM listeners")==1&&scalar("SELECT count(*) FROM listeners WHERE id IN(0,1,2)")==0);
 assert(fixture_exists("credentials.json")&&fixture_exists("translation-service.json")&&fixture_exists("translation-cache/keep.json")&&fixture_exists("Music/keep.flac"));
 assert(create_listener()&&listener==1);assert(!Nxt_isFavorite("/Music/one.flac")&&!Nxt_isFavorite("/Music/two.flac")&&scalar("SELECT count(*) FROM history WHERE listener=1")==0);assert(!fixture_exists("listeners/1/private.txt"));
 Nxt_libraryQuit();NxtPersonal_quit();NxtWrite_quit();puts("PASS confirmed deletion backend: active/inactive, final listener guard, favorite/history cleanup, transaction rollback, failed-switch refusal, queued writer drain, no-follow shortcut cleanup, music/shared credentials preserved, no resurrection and clean reused ID.");
}
int main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--delete")){deletion_checks();return 0;}

 if(argc>1&&!strcmp(argv[1],"--mixed-queue")){mixed_queue();return 0;}
 userdata_mkdir("");NxtPersonal_init();assert(db&&NxtPersonal_id()==0);NxtPersonal_tick();for(int i=0;i<60;i++)sample(500,500);NxtPersonal_flush();assert(scalar("SELECT sum(ms) FROM daily WHERE listener=0")==30000);assert(scalar("SELECT sum(plays) FROM daily WHERE listener=0")==1);
 playback=PLAYER_STATE_PAUSED;sample(500,0);sample(5000,0);playback=PLAYER_STATE_PLAYING;sample(500,0);last_position=120000;sample(500,0);sample(500,0);NxtPersonal_flush();assert(scalar("SELECT sum(ms) FROM daily WHERE listener=0")==30000);
 ready=false;sample(500,500);ready=true;sample(500,500);sample(500,500);NxtPersonal_flush();assert(scalar("SELECT sum(ms) FROM daily WHERE listener=0")==30500);
 strcpy(file,"/Music/short.mp3");gen++;info.duration_ms=10000;sample(500,0);for(int i=0;i<10;i++)sample(500,500);NxtPersonal_flush();assert(scalar("SELECT sum(plays) FROM daily WHERE listener=0")==2);
 gen++;sample(500,0);for(int i=0;i<10;i++)sample(500,500);NxtPersonal_flush();assert(scalar("SELECT sum(plays) FROM daily WHERE listener=0")==3);
 assert(switch_listener(1));assert(module_quits==1);char path[1400];int length=userdata_snpath("playlists/test.m3u",path,sizeof(path));assert(strstr(path,"/listeners/1/playlists/test.m3u"));char *dynamic=userdata_path("playlists/test.m3u");assert(dynamic&&strlen(dynamic)==(size_t)length&&!strcmp(dynamic,path));free(dynamic);
 userdata_snpath("spotify/credentials.json",path,sizeof(path));assert(!strstr(path,"/listeners/"));userdata_snpath("soundcloud/bookmarks.json",path,sizeof(path));assert(strstr(path,"/listeners/1/soundcloud/"));
 Nxt_favorite("/Music/01.mp3");assert(Nxt_isFavorite("/Music/01.mp3"));assert(switch_listener(0));assert(!Nxt_isFavorite("/Music/01.mp3"));Nxt_favorite("/Music/default.mp3");assert(Nxt_isFavorite("/Music/default.mp3"));assert(switch_listener(1));assert(Nxt_isFavorite("/Music/01.mp3")&&!Nxt_isFavorite("/Music/default.mp3"));
 strcpy(file,"gdrive:file-id");gen++;info.duration_ms=200000;sample(500,0);for(int i=0;i<60;i++)sample(500,500);NxtPersonal_flush();assert(scalar("SELECT sum(ms) FROM daily WHERE listener=1 AND source='Drive'")==30000);
 recording=0;sample(500,500);for(int i=0;i<60;i++)sample(500,500);NxtPersonal_flush();assert(scalar("SELECT sum(ms) FROM daily WHERE listener=1")==30000);
 Nxt_libraryQuit();NxtPersonal_quit();NxtPersonal_init();assert(NxtPersonal_id()==1);assert(scalar("SELECT sum(ms) FROM daily WHERE listener=0")==40500);NxtPersonal_quit();
 puts("PASS actual profile persistence, shared Spotify path, private SoundCloud/playlist path, dynamic path allocation, per-profile favorites, PCM-time stats, paused/seek/underrun/output recovery, short-track/repeat qualification and recording off.");return 0;
}
