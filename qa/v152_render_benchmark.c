#define NXT_UI_FIXTURE_MAIN unused_v15_ui_main
#include "v15_ui_render.c"
#undef main
#include <time.h>
static unsigned text_renders;
SDL_Surface *__real_NxtText_render(TTF_Font *,const char *,SDL_Color);
SDL_Surface *__wrap_NxtText_render(TTF_Font *f,const char *s,SDL_Color c){text_renders++;return __real_NxtText_render(f,s,c);}
static double cpu_ms(void){struct timespec t;assert(!clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t));return t.tv_sec*1000.+t.tv_nsec/1000000.;}
static void draw_case(int i){
 if(i==0)NxtSkin_home(page,4);
 else{personal_header(page,i,true);if(i==1)stats_draw(page,1);else personal_cards(page,i,0);NxtSkin_footer(page,"L1/R1 Tab · D-pad Chọn · A Mở · B Về",false);}
}
int main(void){
 assert(!SDL_Init(0)&&!TTF_Init());nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.wallpaper_mode=2;nxt.wallpaper_dim=20;nxt.panel_opacity=35;nxt.accent_color=2;
 NxtPersonal_init();assert(db);assert(exec("INSERT INTO daily VALUES(0,date('now','localtime'),'Local',5400000,17);"));
 page=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);
 const char *names[]={"home","statistics","profile"};const int cases[]={0,1,3};
 for(int c=0;c<3;c++){
  for(int i=0;i<8;i++)draw_case(cases[c]);text_renders=0;double before=cpu_ms();
  for(int i=0;i<150;i++){ticks+=100;draw_case(cases[c]);}
  double elapsed=cpu_ms()-before;printf("%s %.6f %u\n",names[c],elapsed/150,text_renders);
 }
 NxtPersonal_quit();NxtSkin_release();SDL_FreeSurface(page);TTF_Quit();SDL_Quit();return 0;
}
