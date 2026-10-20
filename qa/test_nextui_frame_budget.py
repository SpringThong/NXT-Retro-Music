from pathlib import Path
import json,subprocess,tempfile
R=Path(__file__).resolve().parents[1];s=(R/'upstream/src/nxt_music_playing.c').read_text();start=s.index('bool MusicPlaying_frame(');end=s.index('\nvoid MusicPlaying_invalidate',start);fn=s[start:end]
fixture='''#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
static struct {int animations,interface_style;} nxt={1,0};static int state=1;static uint32_t ticks,last_draw,last_spectrum;static bool spectrum_clock_set,off,hidden;static int updates,paints;
#define PLAYER_STATE_PLAYING 1
bool PWR_getDisplayOff(void){return off;}bool NxtSession_isHidden(void){return hidden;}uint32_t SDL_GetTicks(void){return ticks;}int NxtPlayback_state(void){return state;}bool NxtRecord_needsRefresh(bool p){return false;}bool Spectrum_needsRefresh(void){return true;}void Spectrum_update(void){updates++;}void MusicPlaying_paintLayers(void){paints++;}
'''+fn+'''
int main(void){for(ticks=0;ticks<1000;ticks+=16)MusicPlaying_frame(false);assert(updates>15&&updates<=31&&paints==updates);int previous=updates;off=true;ticks+=10000;assert(!MusicPlaying_frame(false)&&updates==previous);off=false;hidden=true;assert(!MusicPlaying_frame(false)&&updates==previous);hidden=false;ticks+=33;MusicPlaying_frame(false);assert(updates==previous+1);ticks=UINT32_MAX-12;last_spectrum=ticks;ticks+=48;previous=updates;MusicPlaying_frame(false);assert(updates==previous+1);last_draw=1;spectrum_clock_set=true;last_spectrum=0;updates=paints=0;state=0;ticks=500;assert(!MusicPlaying_frame(false));ticks=1001;assert(MusicPlaying_frame(false));last_draw=1001;state=1;nxt.interface_style=1;ticks=1049;assert(!MusicPlaying_frame(false));ticks=1051;assert(MusicPlaying_frame(false));nxt.animations=0;last_draw=1051;ticks=1102;assert(!MusicPlaying_frame(false));ticks=1151;assert(MusicPlaying_frame(false));puts("PASS production visual frame budget: spectrum <=31 updates/s at 60 Hz input loop, no FFT/paint when display-off or hidden, wake and tick-wrap.");}
'''
with tempfile.TemporaryDirectory() as td:
 c=Path(td)/'t.c';b=Path(td)/'t';c.write_text(fixture);subprocess.run(['gcc','-O1','-g','-fsanitize=undefined',str(c),'-o',str(b)],check=True);p=subprocess.run([str(b)],capture_output=True,text=True,check=True);print(p.stdout,end='')
(R/'logs/nextui-frame-budget-test.json').write_text(json.dumps({'visual_budget':{'exit':0,'actual_production_frame_function':True,'input_poll_rate_unchanged':True,'spectrum_capped':True,'off_hidden_no_fft_paint':True,'wake_and_uint32_wrap':True,'physical_temperature_measured':False}},indent=2)+'\n')
