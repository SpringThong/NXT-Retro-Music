#!/usr/bin/env python3
"""Production Drive public link/stream/UI/browser logic with deterministic curl/UI fixtures.
The decoder and player are narrow fixtures; this does not claim real audio/network.
"""
from pathlib import Path
import subprocess,tempfile,os,json
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';render=R/'.cache/render-sp'
incs=(render/'includes.txt').read_text().splitlines()+['-I'+str(S),'-I'+str(S/'include'),'-I'+str(R/'upstream/NextUI/workspace/all/common'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings')]
with tempfile.TemporaryDirectory(prefix='nxt-nextui-drive-ui-') as directory:
 w=Path(directory);data=w/'nxt-retro-music';data.mkdir()
 curl=w/'curl';curl.write_text('''#!/usr/bin/env python3
import sys,os,time
from pathlib import Path
a=sys.argv[1:];assert a[a.index('--proto')+1]=='=https';assert a[a.index('--proto-redir')+1]=='=https';assert '--max-filesize' in a and '--max-time' in a
out=Path(a[a.index('-o')+1]);url=a[-1];mode=os.getenv('QA_TRANSFER','ok')
if mode=='fail':out.write_bytes(b'incomplete');sys.exit(22)
if mode=='cancel':
 out.write_bytes(b'partial')
 while True:time.sleep(.03)
if 'embeddedfolderview' in url and os.getenv('QA_AUDIO_ONLY'):
 out.write_text('<title>Thư mục - Google Drive</title><div class="flip-entries"><a href="https://drive.google.com/file/d/1File00000002/view">10 Nhạc.wav</a><a href="https://drive.google.com/file/d/1File00000003/view">2 Nhạc.wav</a><a href="https://drive.google.com/file/d/1File00000005/view">1 Nhạc.wav</a></div>')
elif 'embeddedfolderview' in url and os.getenv('QA_ONLY_FOLDERS'):
 out.write_text('<title>Thư mục - Google Drive</title><div class="flip-entries"><a href="https://drive.google.com/drive/folders/1Folder0000005">A Folder</a></div>')
elif 'embeddedfolderview' in url:
 out.write_text('<title>Thư mục - Google Drive</title><div class="flip-entries"><a href="https://drive.google.com/file/d/1File00000002/view">10 Nhạc.wav</a><a href="https://drive.google.com/file/d/1File00000003/view">2 Nhạc.wav</a><a href="https://drive.google.com/file/d/1Text00000004/view">note.txt</a><a href="https://drive.google.com/drive/folders/1Folder0000005">A Folder</a></div>')
elif '/file/d/' in url:out.write_text('<title>2 Nhạc.wav - Google Drive</title>')
else:out.write_bytes(b'RIFF'+b'fixture audio'*16)
''');curl.chmod(0o755)
 source=w/'drive_harness.c';source.write_text(r'''
#include "nxt_drive.c"
#include "browser.h"
#include <assert.h>
NxtConfig nxt;int is_brick=0,is_brickpro=1;
static int frames,plays,loads,stops,loadmode,status_polls;
static int expected_tracks=1,expected_start=0,pick_index,pick_count,expected_rows=4;
static int pick_actions[12],pick_selections[12];static bool hidden,queue_cancel,queue_quit;
static char current[512],resume_path[512],last_toast[256];
void LOG_note(int level,const char *f,...){}
const char *Nxt_text(const char *a,const char *b){return b;}
int userdata_snpath(const char *r,char *out,size_t n){return snprintf(out,n,"%s/%s",getenv("QA_DATA"),r);}
bool NxtSession_isHidden(void){return hidden;}
void Nxt_tick(void){frames++;}
bool ModuleCommon_stopSignalled(void){return hidden&&frames>4;}
int Player_beginLoad(const char *path){loads++;status_polls=0;return 0;}
int Player_loadStatus(void){status_polls++;return loadmode==2?-1:loadmode==0&&status_polls>6?0:1;}
void Player_stop(void){stops++;}
DisplayContext *DisplayHelper_current(void){return(DisplayContext*)1;}
void ModuleCommon_frameBegin(void){frames++;}void ModuleCommon_frameEnd(SDL_Surface *s){}void ModuleCommon_markSurfaceDrawn(void){}void Background_tick(void){}
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface *s,int *v,HelpId h){return(GlobalInputResult){.should_quit=queue_quit};}
int NxtPad_justPressed(int b){return b==BTN_B&&(queue_cancel||(!strcmp(getenv("QA_TRANSFER"),"cancel")&&frames>4));}
SDL_Surface *DisplayHelper_getSurface(const DisplayContext *d){static SDL_Surface s={.w=1024,.h=768};return &s;}
Uint32 SDL_GetTicks(void){return frames*110;}
void NxtSkin_begin(SDL_Surface *s,const char *t,const char *d,bool mini){}
void NxtSkin_text(SDL_Surface *s,int p,const char *t,int x,int y,int w,SDL_Color c){}
void NxtSkin_footer(SDL_Surface *s,const char *t,bool p){}
SDL_Color NxtSkin_color(NxtColor r){return(SDL_Color){0};}
ToastToken Toast_show(const char *s,uint32_t t){snprintf(last_toast,sizeof(last_toast),"%s",s);return 0;}
const char *Player_getCurrentFile(void){return current;}
void PlayerModule_setResumePlaylistPath(const char *p){snprintf(resume_path,sizeof(resume_path),"%s",p);}
bool Nxt_takeReturnHome(void){return false;}
NxtPick Nxt_pickLive(DisplayContext *d,const char *title,const char **rows,int n,int *sel,const char *hint){
 assert(pick_index<pick_count);assert(n==expected_rows);
 if(n==4&&getenv("QA_AUDIO_ONLY")){assert(strstr(rows[0],"Nghe cả thư mục")&&strstr(rows[0],"3 bài"));assert(strstr(rows[1],"1 Nhạc")&&strstr(rows[2],"2 Nhạc")&&strstr(rows[3],"10 Nhạc"));}
 else if(n==4){assert(strstr(rows[0],"Nghe cả thư mục")&&strstr(rows[0],"2 bài"));assert(strstr(rows[1],"A Folder")&&strstr(rows[2],"2 Nhạc")&&strstr(rows[3],"10 Nhạc"));}
 else{assert(n==1&&strstr(rows[0],"A Folder")&&!strstr(rows[0],"Nghe cả thư mục"));}
 *sel=pick_selections[pick_index];return(NxtPick){.action=pick_actions[pick_index++],.index=*sel};
}
AudioFormat Player_detectFormat(const char *p){const char *ext=strrchr(p,'.');return ext&&!strcasecmp(ext,".wav")?AUDIO_FORMAT_WAV:AUDIO_FORMAT_UNKNOWN;}
bool NxtDecoder_probe(const char *p,TrackInfo *i){FILE *f=fopen(p,"rb");char bytes[4];bool ok=f&&fread(bytes,1,4,f)==4&&!memcmp(bytes,"RIFF",4);if(f)fclose(f);memset(i,0,sizeof(*i));return ok;}
ModuleExitReason PlayerModule_runWithPlaylist(DisplayContext *d,PlaylistTrack *t,int n,int sel){
 assert(n==expected_tracks&&sel==expected_start);
 if(n==3){assert(!strcmp(t[0].name,"1 Nhạc.wav")&&!strcmp(t[1].name,"2 Nhạc.wav")&&!strcmp(t[2].name,"10 Nhạc.wav"));}
 else{assert(!strcmp(t[0].name,"2 Nhạc.wav"));if(n==2)assert(!strcmp(t[1].name,"10 Nhạc.wav"));}
 PlaylistTrack saved[8];int count=0;assert(*resume_path&&!M3U_loadTracks(resume_path,saved,8,&count)&&count==n);
 for(int i=0;i<n;i++){assert(NxtDrive_isStream(t[i].path));assert(!strcmp(saved[i].path,t[i].path));}
 snprintf(current,sizeof(current),"%s",t[sel].path);plays++;return MODULE_EXIT_TO_MENU;
}
int main(int argc,char **argv){setenv("QA_TRANSFER","ok",1);paths();assert(add_link("https://drive.google.com/drive/folders/1Root00000001"));assert(add_link("https://drive.google.com/drive/folders/1Root00000001"));assert(link_count==1);link_count=0;load_links();assert(link_count==1);
 NxtDriveEntry root,entries[8];char title[256];assert(NxtDrive_parseURL(links[0].url,&root));assert(list_folder((DisplayContext*)1,&root,entries,title)==3);assert(entries[0].folder&&!strcmp(entries[1].name,"2 Nhạc.wav")&&!strcmp(entries[2].name,"10 Nhạc.wav"));assert(!strcmp(title,"Thư mục"));
 NxtDriveEntry file=entries[1];assert(file_name((DisplayContext*)1,&file)==0);assert(play((DisplayContext*)1,&file)==MODULE_EXIT_TO_MENU&&plays==1);struct stat st;assert(NxtDrive_isStream(current)&&stat(current,&st));NxtDriveEntry resolved;assert(NxtDrive_streamEntry(current,&resolved)&&!strcmp(resolved.name,file.name));
 TrackInfo info={0};NxtDrive_metadata(current,&info);assert(!strcmp(info.title,"2 Nhạc")&&!strcmp(info.album,"Google Drive"));
 setenv("QA_TRANSFER","fail",1);assert(play((DisplayContext*)1,&file)==MODULE_EXIT_TO_MENU&&plays==2);/* saved link only, no audio cache */
 char partial[1200];snprintf(partial,sizeof(partial),"%s/cancel.part",data_dir);frames=0;setenv("QA_TRANSFER","cancel",1);assert(fetch((DisplayContext*)1,"https://drive.google.com/file/d/1Cancel0000001",partial,"Cancel")==-2);assert(access(partial,F_OK));
 setenv("QA_TRANSFER","fail",1);assert(fetch((DisplayContext*)1,"https://drive.google.com/file/d/1Fail000000001",partial,"Fail")==-1);assert(access(partial,F_OK));int status;assert(waitpid(-1,&status,WNOHANG)<0&&errno==ECHILD);
 setenv("QA_TRANSFER","ok",1);frames=0;loadmode=0;assert(NxtDrive_preparePlayback(current)&&loads==1&&frames>=6);
 frames=0;loadmode=1;setenv("QA_TRANSFER","cancel",1);assert(!NxtDrive_preparePlayback(current)&&stops==1);
 loadmode=2;assert(!NxtDrive_preparePlayback(current)&&stops==2);
 frames=0;loadmode=1;hidden=true;assert(!NxtDrive_preparePlayback(current)&&stops==3);hidden=false;
 char queue[1200];snprintf(queue,sizeof(queue),"%s/queue.m3u",getenv("QA_DATA"));FILE *q=fopen(queue,"w");assert(q);fprintf(q,"#EXTM3U\n%s\nnxtdrive://MissingRecord00/audio.wav\n",current);fclose(q);
 PlaylistTrack saved[4];int saved_count=0;assert(M3U_loadTracks(queue,saved,4,&saved_count)==0&&saved_count==1);assert(!strcmp(saved[0].path,current)&&!strcmp(saved[0].name,"2 Nhạc.wav"));
 /* The actual folder menu sends every audio row to playback, both for Play
  * All and an individual song. Folder rows keep their original navigation. */
 setenv("QA_TRANSFER","ok",1);expected_tracks=2;expected_start=0;pick_index=0;pick_count=2;pick_actions[0]=1;pick_selections[0]=0;pick_actions[1]=2;pick_selections[1]=0;
 assert(folder_browser((DisplayContext*)1,root,links[0].url)==MODULE_EXIT_TO_MENU&&pick_index==2&&plays==3);
 expected_start=1;pick_index=0;pick_actions[0]=1;pick_selections[0]=3;
 assert(folder_browser((DisplayContext*)1,root,links[0].url)==MODULE_EXIT_TO_MENU&&pick_index==2&&plays==4);
 assert(!strcmp(resume_path+strlen(resume_path)-strlen("1Root00000001.m3u"),"1Root00000001.m3u"));
 assert(!M3U_loadTracks(resume_path,saved,4,&saved_count)&&saved_count==2);
 assert(!strcmp(saved[0].name,"2 Nhạc.wav")&&!strcmp(saved[1].name,"10 Nhạc.wav"));
 pick_index=0;pick_count=4;pick_actions[0]=1;pick_selections[0]=1;pick_actions[1]=2;pick_selections[1]=0;pick_actions[2]=4;pick_selections[2]=3;pick_actions[3]=2;pick_selections[3]=0;
 assert(folder_browser((DisplayContext*)1,root,links[0].url)==MODULE_EXIT_TO_MENU&&pick_index==4&&plays==4);
 queue_cancel=true;assert(play_entries((DisplayContext*)1,&root,entries,3,-1)==MODULE_EXIT_TO_MENU&&plays==4);queue_cancel=false;
 queue_quit=true;assert(play_entries((DisplayContext*)1,&root,entries,3,-1)==MODULE_EXIT_QUIT&&plays==4);queue_quit=false;
 char bad_queue[1200];snprintf(bad_queue,sizeof(bad_queue),"%s/1Root00000001.m3u.new",queues_dir);assert(!mkdir(bad_queue,0700));
 assert(play_entries((DisplayContext*)1,&root,entries,3,-1)==MODULE_EXIT_TO_MENU&&plays==4&&strstr(last_toast,"hàng đợi"));assert(!rmdir(bad_queue));
 assert(play_entries((DisplayContext*)1,&root,entries,DRIVE_ENTRIES+1,-1)==MODULE_EXIT_TO_MENU&&plays==4);
 /* A directory-only result has no Play All row and never starts an empty queue. */
 setenv("QA_ONLY_FOLDERS","1",1);expected_rows=1;pick_index=0;pick_count=1;pick_actions[0]=2;pick_selections[0]=0;
 assert(folder_browser((DisplayContext*)1,root,links[0].url)==MODULE_EXIT_TO_MENU&&plays==4);unsetenv("QA_ONLY_FOLDERS");expected_rows=4;
 /* Flat three-song folders also map Play All and individual selection
  * correctly when there is no folder row before the songs. */
 setenv("QA_AUDIO_ONLY","1",1);expected_tracks=3;expected_start=0;pick_index=0;pick_count=2;pick_actions[0]=1;pick_selections[0]=0;pick_actions[1]=2;pick_selections[1]=0;
 assert(folder_browser((DisplayContext*)1,root,links[0].url)==MODULE_EXIT_TO_MENU&&plays==5);
 expected_start=2;pick_index=0;pick_selections[0]=3;
 assert(folder_browser((DisplayContext*)1,root,links[0].url)==MODULE_EXIT_TO_MENU&&plays==6);
 assert(!M3U_loadTracks(resume_path,saved,4,&saved_count)&&saved_count==3);unsetenv("QA_AUDIO_ONLY");
 /* Real local Browser ties each naturally sorted label to the right path. */
 char music[1200];snprintf(music,sizeof(music),"%s/Music",getenv("QA_DATA"));mkdir(music,0700);const char *names[]={"10 Nhạc","2 Nhạc","A Nhạc","Đường","z","1.wav","10.wav","2.wav"};
 for(int i=0;i<8;i++){char p[1400];snprintf(p,sizeof(p),"%s/%s",music,names[i]);if(i<5)mkdir(p,0700);else{FILE *f=fopen(p,"w");assert(f);fclose(f);}}
 BrowserContext b={0};Browser_loadDirectory(&b,music,music);const char *order[]={"2 Nhạc","10 Nhạc","A Nhạc","Đường","z","1.wav","2.wav","10.wav"};assert(b.entry_count==9);
 for(int i=0;i<8;i++){assert(!strcmp(b.entries[i].name,order[i]));const char *base=strrchr(b.entries[i].path,'/');assert(base&&!strcmp(base+1,order[i]));}assert(b.entries[8].is_play_all);Browser_freeEntries(&b);
 puts("PASS production Drive: Play All and individual songs queue the whole naturally sorted folder; selected song index; persisted M3U with every streaming URI; folder navigation/refresh; queue B/Home cancel, write failure and empty queues; links/stream metadata/HTTP cleanup; local folder order. Curl/decoder/player/UI are fixtures.");return 0;}
''')
 cmd=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-ffunction-sections','-fdata-sections',*incs,str(source),str(S/'nxt_drive_parser.c'),str(S/'nxt_sort.c'),str(S/'nxt_text.c'),str(S/'browser.c'),str(S/'playlist_m3u.c'),str(S/'include/parson/parson.c'),str(render/'ttf.o'),str(render/'utf8proc.o'),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-lm','-o',str(w/'drive')]
 subprocess.run(cmd,check=True)
 subprocess.run([str(w/'drive')],env={**os.environ,'QA_DATA':str(data),'PATH':str(w)+':'+os.environ['PATH']},timeout=15,check=True)
(R/'logs/stock-drive-ui-test.json').write_text(json.dumps({'drive_io':{'exit':0,'curl_decoder_player_fixtures':True},'folder_queue':{'exit':0,'play_all_and_selected_song':True,'natural_order_and_correct_start':True,'persistent_stream_uri_m3u':True,'folder_navigation_refresh_and_empty_audio':True,'cancel_exit_write_failure':True},'local_folder_mapping':{'exit':0}},indent=2)+'\n')
