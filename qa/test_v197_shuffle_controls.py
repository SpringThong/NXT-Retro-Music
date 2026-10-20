from pathlib import Path
import subprocess,tempfile,json,os
r=Path(__file__).resolve().parents[1];src=r/'upstream/src'
def function(file,signature):
 s=(src/file).read_text();a=s.index(signature);brace=s.index('{',a);depth=1;i=brace+1
 while depth:
  depth+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
common=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static bool shuffle_enabled,playlist_active,initialized=true;
static struct{int track_count,index;}playlist={5,2};
static struct{int selected,entry_count;struct{int is_dir;char path[20];}entries[6];}browser;
static int loaded=-1,stopped;
int Browser_countAudioFiles(void*b){int n=0;for(int i=0;i<browser.entry_count;i++)n+=!browser.entries[i].is_dir;return n;}
bool try_load_and_play(const char*p){loaded=atoi(p);return true;}
void Player_stop(void){stopped++;}void playlist_try_play(int i){playlist.index=i;loaded=i;}
int Playlist_shuffle(void*p){if(!playlist.track_count)return -1;int n=rand()%(playlist.track_count-1);return n>=playlist.index?n+1:n;}
int Playlist_next(void*p){return ++playlist.index<playlist.track_count?playlist.index:-1;}int Playlist_prev(void*p){return --playlist.index;}
void Playlist_setCurrentIndex(void*p,int n){playlist.index=n;}
'''
fixture=common+'\n'+function('module_player.c','static bool browser_pick_random(')+'\n'+function('module_player.c','void PlayerModule_nextTrack(')+'\n'+function('module_player.c','void PlayerModule_prevTrack(')+r'''
int main(void){srand(42);browser.entry_count=6;browser.selected=2;for(int i=0;i<6;i++){browser.entries[i].is_dir=i==1||i==4;snprintf(browser.entries[i].path,20,"%d",i);}shuffle_enabled=true;
 for(int i=0;i<200;i++){int old=browser.selected;if(i&1)PlayerModule_nextTrack();else PlayerModule_prevTrack();assert(loaded!=old&&!browser.entries[loaded].is_dir&&browser.selected==loaded);}
 shuffle_enabled=false;browser.selected=2;PlayerModule_nextTrack();assert(loaded==3);PlayerModule_prevTrack();assert(loaded==2);playlist_active=true;shuffle_enabled=true;
 for(int i=0;i<200;i++){int old=playlist.index;if(i&1)PlayerModule_nextTrack();else PlayerModule_prevTrack();assert(loaded!=old&&loaded>=0&&loaded<5);}
 shuffle_enabled=false;playlist.index=2;PlayerModule_nextTrack();assert(loaded==3);PlayerModule_prevTrack();assert(loaded==2);
 puts("PASS actual local playlist/browser manual previous/next, random excludes current and folders; sequential when disabled.");}
'''
q=r/'.cache/render-sp';incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(src)]
geom='''#include <SDL2/SDL.h>\n#include <assert.h>\n#include <stdbool.h>\n#include <stdio.h>\nstatic SDL_Rect buttons[20];static int n;\nstatic int u(SDL_Surface*s,int x){return x*s->w/1024;}\nstatic SDL_Color rgb(unsigned c){return(SDL_Color){c>>16,(c>>8)&255,c&255,255};}\nvoid NxtSkin_rect(SDL_Surface*s,SDL_Rect r,SDL_Color c,int radius){if(n%2==1)buttons[n/2]=r;n++;}\nvoid NxtSkin_icon(SDL_Surface*s,const char*icon,int x,int y,int edge,SDL_Color c){SDL_Rect r=buttons[(n-1)/2];assert(abs(2*x+edge-(2*r.x+r.w))<=1);assert(abs(2*y+edge-(2*r.y+r.h))<=1);}\n'''+function('nxt_auralis.c','static void button(')+'''
int main(void){for(int size=640;size<=1024;size+=384){SDL_Surface s={.w=size};n=0;button(&s,384,650,48,48,"skip-back",false,false);button(&s,481,643,62,62,"play",true,true);button(&s,592,650,48,48,"skip-forward",false,false);int a=buttons[0].x+buttons[0].w/2,b=buttons[1].x+buttons[1].w/2,c=buttons[2].x+buttons[2].w/2;assert(b==size/2&&abs((b-a)-(c-b))<=1);for(int i=0;i<3;i++)assert(buttons[i].y+buttons[i].h/2==u(&s,674));int xs[]={224,282,706,764};for(int i=0;i<4;i++)button(&s,xs[i],656,36,36,"aux",false,false);for(int i=3;i<7;i++)assert(buttons[i].y+buttons[i].h/2==u(&s,674));assert(abs((2*buttons[3].x+buttons[3].w)+(2*buttons[6].x+buttons[6].w)-2*size)<=2);assert(abs((2*buttons[4].x+buttons[4].w)+(2*buttons[5].x+buttons[5].w)-2*size)<=2);}
puts("PASS production Auralis button geometry and icon centres at 640/1024.");}
'''
with tempfile.TemporaryDirectory() as td:
 for name,code in [('local',fixture),('geometry',geom)]:
  c=Path(td)/(name+'.c');exe=Path(td)/name;c.write_text(code);subprocess.run(['gcc','-O1','-g','-fsanitize=undefined',*incs,str(c),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
 for name in ['nxt_spotify.c','nxt_soundcloud.c']:
  code='''#include <stdbool.h>\n#include <stdlib.h>\n#include <assert.h>\n#include <stdio.h>\nstatic struct{int shuffle,repeat_mode;}nxt;static int queue_count=8,queue_index=3;static bool queue_more;\n'''+function(name,'static int adjacent(')+'''\nint main(void){nxt.shuffle=1;for(int i=0;i<500;i++){int x=adjacent(i&1?1:-1);assert(x>=0&&x<8&&x!=queue_index);}nxt.shuffle=0;assert(adjacent(-1)==2&&adjacent(1)==4);puts("PASS production online previous/next respects shuffle in both directions.");}\n'''
  c=Path(td)/'online.c';exe=Path(td)/'online';c.write_text(code);subprocess.run(['gcc','-O1','-fsanitize=undefined',str(c),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
(r/'logs/v197-shuffle-controls.json').write_text(json.dumps({'exit':0,'actual_production_functions':True,'local_browser_playlist_online_previous_next':True,'Auralis_controls_centred_at':[640,1024],'physical_buttons_tested':False},indent=2)+'\n')
