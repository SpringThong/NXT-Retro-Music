/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "ui_music_playing.h"
#include "nxt.h"
#include "module_player.h"
#include "player.h"
#include "spectrum.h"
#include "ui_layers.h"
#include "ui_theme.h"
#include "nxt_record.h"
#include "nxt_playback.h"
#include "nxt_performance.h"
#include "nxt_auralis.h"
#include "nxt_theme.h"
#include "lyrics.h"
#include "nxt_translate.h"
#include "api.h"
static uint32_t last_draw,last_spectrum;
static bool spectrum_clock_set;
static Uint32 cue_began;static int last_cue=-2;static unsigned last_lyrics,last_translation;
void render_playing(SDL_Surface *s,int show,BrowserContext *browser,bool shuffle,bool repeat,int track,int total){(void)repeat;if(total<=0){total=Browser_countAudioFiles(browser);track=Browser_getCurrentTrackNumber(browser);}Nxt_renderPlaying(s,show,track,total,shuffle,PlayerModule_repeatMode());}
void MusicPlaying_paintLayers(void){UiLayer_clear(UI_LAYER_ANIMATION);if(Spectrum_isShowing())Spectrum_paint(UI_LAYER_ANIMATION);NxtRecord_paint(nxt.animations&&NxtPlayback_state()==PLAYER_STATE_PLAYING);}
bool MusicPlaying_frame(bool renders){
    if(PWR_getDisplayOff()||NxtSession_isHidden())return false;
    uint32_t now=SDL_GetTicks();bool repaint=NxtRecord_needsRefresh(nxt.animations&&NxtPlayback_state()==PLAYER_STATE_PLAYING);
    /* Spectrum FFT/paint has its own mode-dependent budget (at most 30 Hz).
     * Button polling remains at
     * the display loop rate; hidden and screen-off frames do no visual work. */
    if(Spectrum_needsRefresh()&&(!spectrum_clock_set||(uint32_t)(now-last_spectrum)>=NxtPerformance_spectrumMS())){
        Spectrum_update();last_spectrum=now;spectrum_clock_set=true;repaint=true;
    }
    if(repaint&&!renders)MusicPlaying_paintLayers();
    int style=nxt.interface_style;
    bool mechanical=nxt.animations&&(style==1||style==3||style==4||style==5||style==6||style==16||style==32);
    /* Reels/meters use the selected visual budget; other pages need only progress/lyrics.
     * Vinyl and spectrum retain their independent animation layers. */
    int current=Lyrics_currentIndex(NxtPlayback_position());unsigned lyric_rev=Lyrics_revision(),translated=NxtTranslate_revision();
    bool lyrics_changed=current!=last_cue||lyric_rev!=last_lyrics||translated!=last_translation;
    if(current!=last_cue)cue_began=now;
    last_cue=current;last_lyrics=lyric_rev;last_translation=translated;
    bool word_motion=nxt.animations&&NxtPlayback_state()==PLAYER_STATE_PLAYING&&Lyrics_wordMotion(current,NxtPlayback_position());
    bool cue_motion=nxt.animations&&nxt.lyric_effect&&((Uint32)(now-cue_began)<360||(style==NXT_THEME_AURALIS&&NxtAuralis_motion()));
    uint32_t interval=NxtPerformance_frameMS(NxtPlayback_state()==PLAYER_STATE_PLAYING,mechanical,style);
    if(word_motion||cue_motion)interval=16;
    if(lyrics_changed||!last_draw||now-last_draw>=interval){last_draw=now;return true;}return false;
}
void MusicPlaying_invalidate(void){last_draw=0;spectrum_clock_set=false;}
void MusicPlaying_leave(void){NxtRecord_hide();UiLayer_clear(UI_LAYER_ANIMATION);UiLayer_clear(UI_LAYER_STATUS);UiLayer_clear(UI_LAYER_SELECTION);}
int MusicPlaying_lyricRows(int height,int row,int gap){return row>0&&height>=row?(height+gap)/(row+gap):0;}
void MusicPlaying_drawLyricRow(SDL_Surface *s,TTF_Font *f,const char *text,SDL_Color color,int x,int y,int w){if(!text||!*text||w<=0)return;SDL_Surface *t=TTF_RenderUTF8_Blended(f,text,color);if(t){SDL_Rect src={0,0,t->w>w?w:t->w,t->h};SDL_BlitSurface(t,&src,s,&(SDL_Rect){x,y,0,0});SDL_FreeSurface(t);}}
