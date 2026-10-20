from pathlib import Path
import subprocess,tempfile,json
r=Path(__file__).resolve().parents[1];s=(r/'upstream/src/nxt_music_playing.c').read_text();a=s.index('bool MusicPlaying_frame(');b=s.index('\nvoid MusicPlaying_invalidate',a);function=s[a:b]
fixture=r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef uint32_t Uint32;
#define PLAYER_STATE_PLAYING 1
#define NXT_THEME_AURALIS 50
static struct{int animations,interface_style,lyric_effect;}nxt={1,50,1};
static uint32_t ticks,last_draw,last_spectrum,cue_began;static bool spectrum_clock_set,off,hidden,word_motion,cue_motion,record_motion;static int state=1,updates,paints,last_cue=-2,current;static unsigned revision=1,last_lyrics,last_translation,translation;
int PWR_getDisplayOff(void){return off;}bool NxtSession_isHidden(void){return hidden;}uint32_t SDL_GetTicks(void){return ticks;}int NxtPlayback_state(void){return state;}int NxtPlayback_position(void){return ticks;}
bool NxtRecord_needsRefresh(bool playing){return record_motion&&playing;}bool Spectrum_needsRefresh(void){return true;}void Spectrum_update(void){updates++;}void MusicPlaying_paintLayers(void){paints++;}
uint32_t NxtPerformance_spectrumMS(void){return 50;}uint32_t NxtPerformance_frameMS(bool playing,bool mechanical,int style){return !playing?1000:mechanical&&nxt.animations?16:250;}
int Lyrics_currentIndex(int pos){return current;}unsigned Lyrics_revision(void){return revision;}unsigned NxtTranslate_revision(void){return translation;}bool Lyrics_wordMotion(int i,int pos){return word_motion;}bool NxtAuralis_motion(void){return cue_motion;}
'''+function+r'''
int main(void){
 int draws=0;word_motion=true;for(ticks=16;ticks<=1000;ticks+=16)draws+=MusicPlaying_frame(false);assert(draws>=60&&draws<=63);assert(updates<=20&&updates>=15);
 word_motion=false;draws=0;for(ticks=1024;ticks<2024;ticks+=16)draws+=MusicPlaying_frame(false);assert(draws>=3&&draws<=5);
 state=0;draws=0;for(ticks=2032;ticks<4032;ticks+=16)draws+=MusicPlaying_frame(false);assert(draws<=2);
 off=true;int old=updates;for(int i=0;i<100;i++){ticks+=16;assert(!MusicPlaying_frame(false));}assert(updates==old);off=false;hidden=true;ticks+=10000;assert(!MusicPlaying_frame(false)&&updates==old);hidden=false;
 state=1;cue_motion=true;ticks+=16;assert(MusicPlaying_frame(false));cue_motion=false;ticks+=16;current++;assert(MusicPlaying_frame(false));ticks+=16;translation++;assert(MusicPlaying_frame(false));
 nxt.interface_style=1;last_draw=ticks;ticks+=16;assert(MusicPlaying_frame(false));nxt.animations=0;last_draw=ticks;ticks+=16;assert(!MusicPlaying_frame(false));ticks=UINT32_MAX-20;last_draw=ticks;ticks+=300;assert(MusicPlaying_frame(false));
 puts("PASS actual production player frame function: 60-Hz-target timed motion, bounded static/paused redraw, separate FFT cap, immediate cue/translation changes, no hidden/off work and tick wrap.");}
'''
with tempfile.TemporaryDirectory(prefix='nxt195-budget-') as td:
 c=Path(td)/'test.c';exe=Path(td)/'test';c.write_text(fixture);subprocess.run(['gcc','-O1','-g','-fsanitize=undefined',str(c),'-o',str(exe)],check=True);p=subprocess.run([str(exe)],capture_output=True,text=True,check=True);(r/'logs/v1971-frame-budget-test.log').write_text(p.stdout+p.stderr);print(p.stdout)
(r/'logs/v1971-frame-budget.json').write_text(json.dumps({'exit':0,'production_frame_function':True,'timed_motion_target_Hz':60,'idle_and_paused_bounded':True,'FFT_separately_capped':True,'hidden_off_no_visual_work':True,'tick_wrap':True,'physical_FPS_or_temperature_measured':False},indent=2)+'\n')
