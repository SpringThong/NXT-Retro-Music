/* Production renderer/controller; isolated provider, audio and DAC fixtures. */
#define main unused_spotify_render_main
#define UiLayer_clear unused_layer_clear
#define UiLayer_blit unused_layer_blit
#include "spotify_render.c"
#undef main
#undef UiLayer_clear
#undef UiLayer_blit
#include "nxt_record.h"
#include "ui_music_playing.h"
#include <stdint.h>

const char *NxtPersonal_name(void){return "NXT Demo";}
const char *NxtPersonal_avatar(void){return "headphones";}
int NxtPersonal_id(void){return 0;}
void NxtLog_step(const char *category,const char *step,bool ok,const char *format,...){}
bool Nxt_save(void){return true;}

static SDL_Surface *layer_canvas;
static unsigned layer_paints;
void UiLayer_clear(UiLayer l){if(layer_canvas&&l==UI_LAYER_ANIMATION)SDL_FillRect(layer_canvas,NULL,0);}
void UiLayer_blit(SDL_Surface *s,int x,int y,UiLayer l){
    if(layer_canvas&&l==UI_LAYER_ANIMATION){SDL_BlitSurface(s,NULL,layer_canvas,&(SDL_Rect){x,y,0,0});layer_paints++;}
}
/* Include the production module unchanged so QA can inspect timing/cache state
 * without adding a test/debug interface to the release app. */
#define track_generation record_track_generation
#include "nxt_record.c"
#undef track_generation

static uint64_t pixels(SDL_Surface *s){
    uint64_t h=1469598103934665603ULL;
    for(int y=0;y<s->h;y++){Uint32 *row=(Uint32 *)((char *)s->pixels+y*s->pitch);
        for(int x=0;x<s->w;x++){h^=row[x];h*=1099511628211ULL;}}
    return h;
}
static void snapshot(SDL_Surface *page,const char *directory,const char *name){
    SDL_Surface *out=SDL_ConvertSurfaceFormat(page,SDL_PIXELFORMAT_ARGB8888,0);assert(out);
    SDL_BlitSurface(layer_canvas,NULL,out,NULL);char path[1300];snprintf(path,sizeof(path),"%s/%s.bmp",directory,name);
    assert(!SDL_SaveBMP(out,path));SDL_FreeSurface(out);
}
static void draw_page(SDL_Surface *page){
    MusicPlaying_frame(true);NxtSkin_playing(page,queue_index+1,2,true,0);MusicPlaying_paintLayers();
}
int main(int argc,char **argv){
    assert(argc==4&&!SDL_Init(0)&&!TTF_Init());
    nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.output=2;
    nxt.wallpaper_mode=4;nxt.wallpaper_dim=20;nxt.panel_opacity=35;
    snprintf(nxt.wallpaper_path,sizeof(nxt.wallpaper_path),"%s",argv[3]);
    const int defaults[16]={0,0,11,12,1,2,3,4,5,6,7,8,0,0,9,10};memcpy(nxt.playing_keys,defaults,sizeof(defaults));paths();connected=true;child=123;queue=calloc(2,sizeof(*queue));assert(queue);queue_capacity=queue_count=2;
    strcpy(queue[0].title,"Giai điệu bình minh");strcpy(queue[0].artist,"NXT Demo");
    strcpy(queue[0].album,"Bản thử giao diện đĩa than");strcpy(queue[0].uri,"spotify:track:0123456789012345678901");queue[0].duration=226000;
    queue[1]=queue[0];strcpy(queue[1].title,"Bài tiếp theo");strcpy(queue[1].uri,"spotify:track:0123456789012345678902");
    select_track(0);event("{\"event\":\"playing\",\"position_ms\":37000}");
    assert(Lyrics_loadTextFile(argv[2],duration_ms));cover=IMG_Load(argv[3]);assert(cover);
    SDL_Surface *page=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);
    layer_canvas=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);assert(layer_canvas);
    SDL_SetSurfaceBlendMode(layer_canvas,SDL_BLENDMODE_BLEND);
    Spectrum_init();while(Spectrum_isEnabled())Spectrum_cycleNext();
    draw_page(page);uint64_t first=pixels(animation);SDL_Surface *first_cache=animation,*first_label=label;
    snapshot(page,argv[1],"vinyl-wallpaper-dark-1024");
    assert(phase==0&&arm==1);unsigned painted=layer_paints;
    ticks+=49;MusicPlaying_frame(false);assert(layer_paints==painted);
    ticks++;MusicPlaying_frame(false);assert(layer_paints>painted&&phase>1.49f&&phase<1.51f&&pixels(animation)!=first);
    /* Turning still works when the spectrum is disabled/uninitialized. */
    Spectrum_cycleNext();
    for(int i=0;i<150;i++){
        if(i==60)event("{\"event\":\"paused\",\"position_ms\":40000}");
        if(i==90)event("{\"event\":\"playing\",\"position_ms\":40000}");
        ticks+=50;draw_page(page);
        assert(animation==first_cache&&label==first_label);
        if(i==65){assert(arm==0);snapshot(page,argv[1],"vinyl-paused-1024");}
        if(i==100){assert(arm==1);snapshot(page,argv[1],"vinyl-playing-1024");}
        if(i%2==0){char name[64];snprintf(name,sizeof(name),"frame-%03d",i/2);snapshot(page,argv[1],name);}
    }
    event("{\"event\":\"paused\",\"position_ms\":43000}");
    for(int i=0;i<6;i++){ticks+=50;MusicPlaying_frame(false);}
    float stopped_phase=phase;uint64_t stopped_pixels=pixels(animation);assert(arm==0);
    ticks+=1000;assert(!NxtRecord_needsRefresh(false));MusicPlaying_frame(false);
    assert(phase==stopped_phase&&pixels(animation)==stopped_pixels);
    /* Neither sleep nor another page should keep the animation busy. */
    fixture_display_off=true;int hidden_paints=layer_paints;ticks+=50000;assert(!MusicPlaying_frame(false)&&layer_paints==hidden_paints);assert(!NxtRecord_needsRefresh(true));NxtRecord_paint(true);assert(phase==stopped_phase);
    fixture_display_off=false;fixture_hidden=true;assert(!MusicPlaying_frame(false)&&layer_paints==hidden_paints);assert(!NxtRecord_needsRefresh(true));fixture_hidden=false;
    NxtSkin_home(page,3);painted=layer_paints;ticks+=50;assert(!NxtRecord_needsRefresh(true));NxtRecord_paint(true);assert(layer_paints==painted);
    event("{\"event\":\"playing\",\"position_ms\":43000}");draw_page(page);assert(phase==stopped_phase);
    ticks+=50;MusicPlaying_frame(false);assert(phase>stopped_phase);
    /* A different track starts from zero, even with the same artwork address. */
    SDL_Surface *art=cover;cover=NULL;select_track(1);cover=art;event("{\"event\":\"playing\",\"position_ms\":0}");draw_page(page);assert(phase==0);
    ticks+=50;MusicPlaying_frame(false);float before_cover=phase;
    cover=NULL;draw_page(page);assert(phase==before_cover);snapshot(page,argv[1],"vinyl-no-cover-1024");
    cover=art;draw_page(page);assert(phase==before_cover&&cached_art==art);
    /* Portrait/square artwork crops safely; theme and display resize rebuild. */
    SDL_Surface *portrait=SDL_CreateRGBSurfaceWithFormat(0,37,121,32,SDL_PIXELFORMAT_ARGB8888);assert(portrait);
    SDL_FillRect(portrait,NULL,SDL_MapRGB(portrait->format,154,72,64));cover=portrait;draw_page(page);assert(cached_art==portrait);
    cover=art;SDL_FreeSurface(portrait);nxt.theme=2;draw_page(page);snapshot(page,argv[1],"vinyl-light-1024");
    SDL_FreeSurface(page);SDL_FreeSurface(layer_canvas);
    page=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_ARGB8888);
    layer_canvas=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_ARGB8888);assert(page&&layer_canvas);
    SDL_SetSurfaceBlendMode(layer_canvas,SDL_BLENDMODE_BLEND);nxt.theme=1;draw_page(page);assert(animation->w==151);
    snapshot(page,argv[1],"vinyl-playing-640");
    /* The same screen follows the local/Drive decoder's playback state. */
    track_loaded=playing=false;child=-1;current.uri[0]=0;fixture_state=PLAYER_STATE_PLAYING;fixture_info=track_info;
    strcpy(fixture_info.title,"Nhạc từ Google Drive");strcpy(fixture_path,"nxtdrive://1RecordFixture01/audio.flac");fixture_art=art;fixture_generation=42;
    draw_page(page);assert(phase==0&&was_playing);ticks+=50;MusicPlaying_frame(false);assert(phase>0);
    snapshot(page,argv[1],"vinyl-drive-640");fixture_state=PLAYER_STATE_PAUSED;
    for(int i=0;i<6;i++){ticks+=50;MusicPlaying_frame(false);}assert(arm==0&&!NxtRecord_needsRefresh(false));
    strcpy(fixture_path,"/Music/NXT Demo.flac");fixture_generation++;fixture_state=PLAYER_STATE_PLAYING;
    draw_page(page);assert(phase==0&&was_playing);snapshot(page,argv[1],"vinyl-local-640");
    /* SDL's 32-bit clock wraps after ~49 days without jumping the platter. */
    ticks=UINT32_MAX-20;draw_page(page);float before_wrap=phase;ticks+=50;MusicPlaying_frame(false);
    assert(fabsf(phase-before_wrap-1.5f)<.01f);
    MusicPlaying_leave();assert(!NxtRecord_needsRefresh(true));Spectrum_quit();NxtSkin_release();assert(!animation&&!label&&!platter&&!circle_mask);
    SDL_FreeSurface(art);cover=NULL;fixture_art=NULL;SDL_FreeSurface(page);SDL_FreeSurface(layer_canvas);layer_canvas=NULL;
    TTF_Quit();SDL_Quit();
    puts("PASS production vinyl + now-playing frame loop: rotates without spectrum, 20fps gate, pause/resume and arm, no sleep/menu animation or wake jump, track/cover change, portrait art, two themes/sizes, Spotify/local/Drive state, timer wrap, bounded reusable caches and release. Audio/DAC/providers are fixtures.");return 0;
}
