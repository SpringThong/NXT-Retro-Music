#!/usr/bin/env python3
"""Production Drive fetch/cache/list/browser logic with deterministic curl/UI fixtures.
The decoder and player are narrow fixtures; this does not claim real audio/network.
"""
from pathlib import Path
import subprocess,tempfile,os,json
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';render=R/'.cache/render-sp'
incs=(render/'includes.txt').read_text().splitlines()+['-I'+str(S),'-I'+str(S/'include'),'-I'+str(R/'upstream/NextUI/workspace/all/common'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings')]
with tempfile.TemporaryDirectory(prefix='nxt-drive-io-') as directory:
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
if 'embeddedfolderview' in url:
 out.write_text('<title>Thư mục - Google Drive</title><div class="flip-entries"><a href="https://drive.google.com/file/d/1File00000002/view">10 Nhạc.wav</a><a href="https://drive.google.com/file/d/1File00000003/view">2 Nhạc.wav</a><a href="https://drive.google.com/file/d/1Text00000004/view">note.txt</a><a href="https://drive.google.com/drive/folders/1Folder0000005">A Folder</a></div>')
elif '/file/d/' in url:out.write_text('<title>2 Nhạc.wav - Google Drive</title>')
else:out.write_bytes(b'RIFF'+b'fixture audio'*16)
''');curl.chmod(0o755)
 source=w/'drive_harness.c';source.write_text(r'''
#include "nxt_drive.c"
#include "browser.h"
#include <assert.h>
NxtConfig nxt;int is_brick=0,is_brickpro=1;
static int frames,plays;static char current[512];
void LOG_note(int level,const char *f,...){}
const char *Nxt_text(const char *a,const char *b){return b;}
int userdata_snpath(const char *r,char *out,size_t n){return snprintf(out,n,"%s/%s",getenv("QA_DATA"),r);}
void ModuleCommon_frameBegin(void){frames++;}void ModuleCommon_frameEnd(SDL_Surface *s){}void ModuleCommon_markSurfaceDrawn(void){}void Background_tick(void){}
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface *s,int *v,HelpId h){return(GlobalInputResult){0};}
int NxtPad_justPressed(int b){return b==BTN_B&&!strcmp(getenv("QA_TRANSFER"),"cancel")&&frames>4;}
SDL_Surface *DisplayHelper_getSurface(const DisplayContext *d){static SDL_Surface s={.w=1024,.h=768};return &s;}
Uint32 SDL_GetTicks(void){return frames*110;}
void NxtSkin_begin(SDL_Surface *s,const char *t,const char *d,bool mini){}
void NxtSkin_text(SDL_Surface *s,int p,const char *t,int x,int y,int w,SDL_Color c){}
void NxtSkin_footer(SDL_Surface *s,const char *t,bool p){}
SDL_Color NxtSkin_color(NxtColor r){return(SDL_Color){0};}
ToastToken Toast_show(const char *s,uint32_t t){return 0;}
const char *Player_getCurrentFile(void){return current;}
void PlayerModule_setResumePlaylistPath(const char *p){}
AudioFormat Player_detectFormat(const char *p){const char *ext=strrchr(p,'.');return ext&&!strcasecmp(ext,".wav")?AUDIO_FORMAT_WAV:AUDIO_FORMAT_UNKNOWN;}
bool NxtDecoder_probe(const char *p,TrackInfo *i){FILE *f=fopen(p,"rb");char bytes[4];bool ok=f&&fread(bytes,1,4,f)==4&&!memcmp(bytes,"RIFF",4);if(f)fclose(f);memset(i,0,sizeof(*i));return ok;}
ModuleExitReason PlayerModule_runWithPlaylist(DisplayContext *d,PlaylistTrack *t,int n,int sel){assert(n==1&&sel==0);assert(!strcmp(t[0].name,"2 Nhạc.wav"));assert(NxtDrive_isPath(t[0].path));snprintf(current,sizeof(current),"%s",t[0].path);plays++;return MODULE_EXIT_TO_MENU;}
int main(int argc,char **argv){setenv("QA_TRANSFER","ok",1);paths();assert(add_link("https://drive.google.com/drive/folders/1Root00000001"));assert(add_link("https://drive.google.com/drive/folders/1Root00000001"));assert(link_count==1);link_count=0;load_links();assert(link_count==1);
 NxtDriveEntry root,entries[8];char title[256];assert(NxtDrive_parseURL(links[0].url,&root));assert(list_folder((DisplayContext*)1,&root,entries,title)==3);assert(entries[0].folder&&!strcmp(entries[1].name,"2 Nhạc.wav")&&!strcmp(entries[2].name,"10 Nhạc.wav"));assert(!strcmp(title,"Thư mục"));
 NxtDriveEntry file=entries[1];assert(file_name((DisplayContext*)1,&file)==0);assert(play((DisplayContext*)1,&file)==MODULE_EXIT_TO_MENU&&plays==1);struct stat st;assert(!stat(current,&st)&&st.st_size>0);
 TrackInfo info={0};NxtDrive_metadata(current,&info);assert(!strcmp(info.title,"2 Nhạc")&&!strcmp(info.album,"Google Drive"));
 setenv("QA_TRANSFER","fail",1);assert(play((DisplayContext*)1,&file)==MODULE_EXIT_TO_MENU&&plays==2);/* cache hit even offline */
 char partial[1200];snprintf(partial,sizeof(partial),"%s/cancel.part",data_dir);frames=0;setenv("QA_TRANSFER","cancel",1);assert(fetch((DisplayContext*)1,"https://drive.google.com/file/d/1Cancel0000001",partial,"Cancel",true)==-2);assert(access(partial,F_OK));
 setenv("QA_TRANSFER","fail",1);assert(fetch((DisplayContext*)1,"https://drive.google.com/file/d/1Fail000000001",partial,"Fail",true)==-1);assert(access(partial,F_OK));int status;assert(waitpid(-1,&status,WNOHANG)<0&&errno==ECHILD);
 trim_cache(0,NULL);assert(!access(current,F_OK));current[0]=0;trim_cache(0,NULL);assert(cache_total()==0);
 /* Real local Browser ties each naturally sorted label to the right path. */
 char music[1200];snprintf(music,sizeof(music),"%s/Music",getenv("QA_DATA"));mkdir(music,0700);const char *names[]={"10 Nhạc","2 Nhạc","A Nhạc","Đường","z","1.wav","10.wav","2.wav"};
 for(int i=0;i<8;i++){char p[1400];snprintf(p,sizeof(p),"%s/%s",music,names[i]);if(i<5)mkdir(p,0700);else{FILE *f=fopen(p,"w");assert(f);fclose(f);}}
 BrowserContext b={0};Browser_loadDirectory(&b,music,music);const char *order[]={"2 Nhạc","10 Nhạc","A Nhạc","Đường","z","1.wav","2.wav","10.wav"};assert(b.entry_count==9);
 for(int i=0;i<8;i++){assert(!strcmp(b.entries[i].name,order[i]));const char *base=strrchr(b.entries[i].path,'/');assert(base&&!strcmp(base+1,order[i]));}assert(b.entries[8].is_play_all);Browser_freeEntries(&b);
 puts("PASS production Drive: persistent links/dedup, public folder parsing/filter/sort, names and metadata, selected playback/cache hit, B cancel/reap/cleanup, HTTP failure cleanup, active-cache protection; production local folder labels match sorted file paths. Curl/decoder/player/UI are fixtures.");return 0;}
''')
 cmd=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-ffunction-sections','-fdata-sections',*incs,str(source),str(S/'nxt_drive_parser.c'),str(S/'nxt_sort.c'),str(S/'nxt_text.c'),str(S/'browser.c'),str(S/'include/parson/parson.c'),str(render/'ttf.o'),str(render/'utf8proc.o'),'-Wl,--gc-sections','-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-lm','-o',str(w/'drive')]
 subprocess.run(cmd,check=True)
 subprocess.run([str(w/'drive')],env={**os.environ,'QA_DATA':str(data),'PATH':str(w)+':'+os.environ['PATH']},timeout=15,check=True)
(R/'logs/stock-drive-io-test.json').write_text(json.dumps({'drive_io':{'exit':0,'curl_decoder_player_fixtures':True},'local_folder_mapping':{'exit':0}},indent=2)+'\n')
