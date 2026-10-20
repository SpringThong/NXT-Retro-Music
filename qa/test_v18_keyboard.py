#!/usr/bin/env python3
"""Real keyboard frame loop, ABXY/shoulder shortcuts and software rendering."""
from pathlib import Path
import os,subprocess,tempfile,json
from PIL import Image
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';render=R/'.cache/render-sp'
incs=(render/'includes.txt').read_text().splitlines()+['-I'+str(R/'.host-tools/usr/include'),'-I'+str(S),'-I'+str(S/'include'),'-I'+str(R/'upstream/NextUI/workspace/all/common'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings')]
with tempfile.TemporaryDirectory(prefix='nxt-keyboard-') as directory:
 w=Path(directory)
 source=w/'keyboard_harness.c';source.write_text(r'''
#include "keyboard.c"
#include "nxt_text.h"
#define UIKeyboard_render ProductionRender
#include "ui_keyboard.c"
#undef UIKeyboard_render
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
NxtConfig nxt;
unsigned NxtAppearance_accent(void){return nxt.accent_rgb;}
void NxtAppearance_release(void){}
int is_brick=0,is_brickpro=1;
static SDL_Surface *canvas;static int keys[4096],count,index_,pressed,previous;
static KeyboardUiState last;
const char *Nxt_text(const char *a,const char *b){return nxt.language?b:a;}
TTF_Font *Fonts_open(int px){return NxtText_open("./res/fonts/notosans.ttf",px);}
SDL_Surface *Nxt_render(TTF_Font *f,const char *t,SDL_Color c){return NxtText_render(f,t,c);}
void LOG_note(int level,const char *fmt,...){}
uint32_t CFG_getColor(int role){return role==COLOR_MAIN?0x94debaff:role==COLOR_ACCENT2?0x19362aff:role==COLOR_LIST_TEXT_SELECTED?0x102218ff:0xe4f1e8ff;}
SDL_Color uintToColour(uint32_t c){return(SDL_Color){c>>24,c>>16,c>>8,c};}
void GFX_clear(SDL_Surface *s){SDL_FillRect(s,NULL,SDL_MapRGB(s->format,8,24,17));}
void render_screen_header(SDL_Surface *s,const char *title,int show){TTF_Font *f=Fonts_open(26);SDL_Surface *t=NxtText_render(f,title,Theme_getColor(THEME_ROLE_PRIMARY,false));if(t){SDL_BlitSurface(t,NULL,s,&(SDL_Rect){20,16,0,0});SDL_FreeSurface(t);}NxtText_close(f);}
void render_rounded_rect_bg(SDL_Surface *s,int x,int y,int w,int h,uint32_t c){SDL_FillRect(s,&(SDL_Rect){x,y,w,h},c);}
DisplayContext *DisplayHelper_current(void){return(DisplayContext*)1;}
SDL_Surface *DisplayHelper_getSurface(const DisplayContext *d){return canvas;}
int DisplayHelper_addRecreatedCallback(DisplayRecreatedCallback c){return 0;}
void DisplayHelper_removeRecreatedCallback(DisplayRecreatedCallback c){}
bool ScreenTitle_start(bool defer){return false;}
void ScreenTitle_frameEnd(int *dirty,bool allowed){}
void ModuleCommon_frameBegin(void){assert(index_<count);previous=pressed;pressed=keys[index_++];}
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface *s,int *show,HelpId id){return(GlobalInputResult){0};}
void ModuleCommon_frameEnd(SDL_Surface *s){}
void ModuleCommon_markSurfaceDrawn(void){}
int PWR_getDisplayOff(void){return 0;}int PWR_displayInputConsumed(void){return 0;}
const char *NxtLocale_text(const char*s){return s;}
bool NxtTheme_color(NxtColor c,SDL_Color*out){return false;}
static const char *candidate_want;
NxtPick Nxt_pick(DisplayContext*d,const char*t,const char**items,int n,int*selected,const char*h){for(int i=0;i<n;i++)if(candidate_want&&!strcmp(items[i],candidate_want)){*selected=i;return(NxtPick){1,i};}assert(!candidate_want);return(NxtPick){1,0};}
bool Nxt_selectLongUsed(void){return false;}
int PAD_isPressed(int b){return pressed&b;}
int PAD_justPressed(int b){return (pressed&~previous)&b;}
int PAD_justReleased(int b){return(previous&~pressed)&b;}
int PAD_justRepeated(int b){return PAD_justPressed(b);}
Uint32 SDL_GetTicks(void){return index_*16;}
void UIKeyboard_render(SDL_Surface *s,const KeyboardUiState *ui){
 assert(ui->layout&&ui->layout->name&&ui->layout->name[0]);
 assert(!strcmp(ui->lang_label,ui->layout->name));
 last=*ui;ProductionRender(s,ui);
}
static bool glyph(void *p,const char *c){return true;}
static bool ascii_glyph(void *p,const char *c){return (unsigned char)*c<128;}
static void load(const int *sequence,size_t n){memcpy(keys,sequence,n*sizeof(int));count=n;index_=pressed=previous=0;}
static void frame(int button){assert(count<sizeof(keys)/sizeof(keys[0]));keys[count++]=button;}
static void tap(int button){frame(button);frame(0);}
/* Generate pad events, then pass them through Keyboard_open's actual frame loop.
 * Routing only finds a path to an ANSI slot; it never writes the input text. */
static void route(int *row,int *col,char ansi){
 const Keyboard *k=KeyboardMap_get();const KeyboardGeometry *g=k->geometry;
 int start=*row*16+*col,target=-1,queue[80],prev[80],button[80],head=0,tail=0;
 for(int i=0;i<80;i++)prev[i]=-1;
 for(int r=0;r<g->rows;r++)for(int c=0;g->keys[r][c];c++)
  if(g->keys[r][c]->action==KEY_TEXT&&g->keys[r][c]->ansi==ansi)target=r*16+c;
 assert(target>=0);queue[tail++]=start;prev[start]=start;
 const int dirs[]={BTN_UP,BTN_DOWN,BTN_LEFT,BTN_RIGHT};
 while(head<tail&&prev[target]<0){
  int current=queue[head++];
  for(int i=0;i<4;i++){
   KeyboardState cursor={.keyboard=k,.geometry=g,.current_layout=k->layouts[0],.row=current/16,.col=current%16};
   update_current_key(&cursor);
   if(i<2)move_vertical(&cursor,i?1:-1);else move_horizontal(&cursor,i==2?-1:1);
   int next=cursor.row*16+cursor.col;if(prev[next]>=0)continue;
   prev[next]=current;button[next]=dirs[i];queue[tail++]=next;
  }
 }
 assert(prev[target]>=0);int path[80],n=0;
 for(int at=target;at!=start;at=prev[at])path[n++]=button[at];
 while(n)tap(path[--n]);*row=target/16;*col=target%16;
}
static void phrase(const char *raw){
 count=index_=pressed=previous=0;frame(0);int row=2,col=1;
 for(const unsigned char *c=(const unsigned char*)raw;*c;c++){
  if(*c==' '){tap(BTN_X);continue;}
  char slot=(char)tolower(*c);bool shift=isupper(*c);
  if(*c==':'){slot=';';shift=true;}else if(*c=='?'){slot='/';shift=true;}
  route(&row,&col,slot);if(shift)tap(BTN_L1);tap(BTN_A);
 }
 tap(BTN_SELECT);
}
static void check_maps(void){
 const Keyboard *k=KeyboardMap_get();assert(k->layout_count==15);
 assert(!strcmp(k->layouts[0]->name,"TELEX")&&!strcmp(k->layouts[1]->name,"LAT"));
 for(int i=0;i<2;i++)for(int r=0;r<k->geometry->rows;r++)for(int c=0;k->geometry->keys[r][c];c++){
  const Key *key=k->geometry->keys[r][c];if(key->action!=KEY_TEXT||!isalpha((unsigned char)key->ansi))continue;
  const char *plain=KeyboardMap_char(k->layouts[i],key,false,0),*shift=KeyboardMap_char(k->layouts[i],key,true,0);
  assert(plain[0]==key->ansi&&!plain[1]);assert(shift[0]==toupper((unsigned char)key->ansi)&&!shift[1]);
 }
}
int main(int argc,char **argv){assert(!SDL_Init(0)&&!TTF_Init());assert(KeyboardMap_prepare(glyph,NULL));nxt.language=1;
 check_maps();assert(KeyboardMap_prepare(ascii_glyph,NULL));check_maps();assert(KeyboardMap_prepare(glyph,NULL));check_maps();
 canvas=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);assert(canvas);
 const int seq[]={0,BTN_A,0,BTN_A,0,BTN_X,0,BTN_Y,0,BTN_R1,0,BTN_A,0,BTN_R1,0,BTN_B,0,BTN_L1,0,BTN_A,0,BTN_SELECT,0};
 load(seq,sizeof(seq)/sizeof(int));char *text=Keyboard_open("Tìm bài hát",100);assert(text&&!strcmp(text,"â A"));free(text);
 const int latin[]={0,BTN_A,0,BTN_A,0,BTN_SELECT,0};load(latin,sizeof(latin)/sizeof(int));text=Keyboard_openLatin("Google Drive",100);assert(text&&!strcmp(text,"aa"));assert(!strcmp(last.layout->name,"LAT"));free(text);
 const char *raws[]={"tieengs Vieetj","nguyeenx xuaan thoong","Tooi yeeu Vieetj Nam","aa aw ee oo ow uw dd","as af ar ax aj","TIEENGS VIEETJ"};
 const char *want[]={"tiếng Việt","nguyễn xuân thông","Tôi yêu Việt Nam","â ă ê ô ơ ư đ","á à ả ã ạ","TIẾNG VIỆT"};
 for(int i=0;i<sizeof(raws)/sizeof(raws[0]);i++){phrase(raws[i]);text=Keyboard_open("Tìm bài hát",500);assert(text&&!strcmp(text,want[i]));assert(!strcmp(last.layout->name,"TELEX"));free(text);}
 phrase("https://drive.google.com/file/d/AbCd/view?usp=sharing");text=Keyboard_openLatin("Google Drive",500);assert(text&&!strcmp(text,"https://drive.google.com/file/d/AbCd/view?usp=sharing"));free(text);
 nxt.language=0;load(latin,sizeof(latin)/sizeof(int));text=Keyboard_open("Search",100);assert(text&&!strcmp(text,"aa"));free(text);nxt.language=1;

 const char *layout_names[]={"LAT","TELEX","Français","Русский","한국어","日本語","ไทย","拼音","拼音繁體","Deutsch","Español","Português","Italiano","Indonesia"};
 for(int lang=0;lang<14;lang++){nxt.language=lang;const int seq[]={0,BTN_A,0,BTN_SELECT,0};load(seq,5);text=Keyboard_open("Search",500);assert(text&&!strcmp(last.layout->name,layout_names[lang]));free(text);}
 const char *raw_ime[]={"gksrmf","nihon","nihao","diannao"},*ime_want[]={"한글","にほん","你好","電腦"};int lang_ime[]={4,5,7,8};
 for(int i=0;i<4;i++){nxt.language=lang_ime[i];phrase(raw_ime[i]);if(i>=2){count-=2;tap(BTN_X);tap(BTN_SELECT);candidate_want=ime_want[i];}text=Keyboard_open("Search",500);assert(text&&!strcmp(text,ime_want[i]));free(text);candidate_want=NULL;}
 nxt.language=5;phrase("nihon");count-=2;tap(BTN_X);tap(BTN_SELECT);candidate_want="日本";text=Keyboard_open("Search",500);assert(text&&!strcmp(text,"日本"));free(text);candidate_want=NULL;nxt.language=1;
 const int cancel[]={0,BTN_B};load(cancel,2);assert(!Keyboard_open("Tìm",100));
 const Keyboard *k=KeyboardMap_get();KeyboardUiState ui={.title="Tìm bài hát",.text="Nguyễn Xuân Thông · tiếng Việt",.layout=k->layouts[0],.lang_label="TELEX",.row=2,.col=1};
 for(int size=0;size<2;size++){if(size){SDL_FreeSurface(canvas);UIKeyboard_quit();canvas=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_ARGB8888);}for(int mode=1;mode<=2;mode++){nxt.theme=mode;nxt.accent_rgb=0x83ddb0;ProductionRender(canvas,&ui);char path[1200];snprintf(path,sizeof(path),"%s/keyboard-%s-%d.bmp",argv[1],mode==1?"dark":"light",canvas->w);assert(!SDL_SaveBMP(canvas,path));}}
 UIKeyboard_quit();KeyboardMap_quit();SDL_FreeSurface(canvas);TTF_Quit();SDL_Quit();puts("PASS production keyboard: Vietnamese Telex phrases through actual D-pad/A/B/X/Y frame loop; 14 matching default layouts; Hangul, Romaji, kanji and Pinyin through pad input; active-mode label; glyph filtering; Latin URLs; Shift/Caps/SELECT; rendered at 1024 and 640.");return 0;}
''')
 cmd=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections',*incs,'-include',str(S/'nxt_i18n.h'),str(source),str(S/'nxt_pad.c'),str(S/'nxt_telex.c'),str(S/'nxt_ime.c'),str(S/'keyboard_map.c'),str(S/'utf8.c'),str(S/'nxt_text.c'),str(S/'ui_theme_utils.c'),str(S/'ui_theme.c'),str(S/'nxt_skin.c'),str(render/'ttf.o'),str(render/'utf8proc.o'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0','-l:libsqlite3.so.0','-lm','-o',str(w/'keyboard')]
 subprocess.run(cmd,check=True)
 out=R/'qa/screens-v1.8-keyboard';out.mkdir(exist_ok=True)
 subprocess.run([str(w/'keyboard'),str(out)],cwd=render/'stage',timeout=90,check=True,env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')})
 for p in out.glob('keyboard*.bmp'):Image.open(p).save(p.with_suffix('.png'));p.unlink()
(R/'logs/v18-keyboard-test.json').write_text(json.dumps({'keyboard':{'exit':0,'production_frame_loop':True,'shortcuts':'ABXY L1 R1 SELECT','render_sizes':[1024,640],'matching_system_languages':14,'IME_pad_composition':True,'active_mode_label':True,'vietnamese_pad_phrases':6,'real_map_with_glyph_filter':True,'latin_url_preserved':True}},indent=2)+'\n')
