"""Execute the production exit-decision function with deterministic UI/audio states."""
from pathlib import Path
import subprocess,tempfile
R=Path(__file__).resolve().parents[1]
s=(R/'upstream/src/nxt_session.c').read_text()
start=s.index('bool NxtSession_wantsBackground(')
end=s.index('bool NxtSession_background(',start)
production=s[start:end]
fixture=r'''
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
typedef int DisplayContext;
typedef enum {MODULE_EXIT_TO_MENU,MODULE_EXIT_QUIT} ModuleExitReason;
typedef struct {int action,index;} NxtPick;
struct {int background;} nxt;
static bool background_choice,sp,sc,stop,quit,home;
static int player,radio,action,choice,dialogs;
#define PLAYER_STATE_PLAYING 1
#define RADIO_STATE_PLAYING 1
bool NxtSpotify_isPlaying(void){return sp;}
bool NxtSoundCloud_isPlaying(void){return sc;}
int Player_getState(void){return player;}
int Radio_getState(void){return radio;}
void ModuleCommon_signalStop(void){stop=true;}
void ModuleCommon_requestQuit(void){quit=true;}
void Nxt_returnHome(void){home=true;}
const char *Nxt_text(const char *en,const char *vi){(void)vi;return en;}
NxtPick Nxt_pick(DisplayContext *d,const char *t,const char **rows,int count,int *selected,const char *footer){
 (void)d;(void)t;(void)rows;(void)footer;assert(count==2);dialogs++;*selected=choice;return (NxtPick){action,choice};
}
'''
checks=r'''
static void reset(void){background_choice=sp=sc=stop=quit=home=false;player=radio=action=choice=dialogs=0;nxt.background=0;}
int main(void){
 reset();nxt.background=1;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit&&!dialogs&&!background_choice);
 reset();player=1;action=1;choice=0;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(quit&&!stop&&NxtSession_wantsBackground()&&!nxt.background&&dialogs==1);
 reset();sp=true;action=1;choice=1;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit&&!background_choice);
 reset();sc=true;action=2;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit);
 reset();radio=1;action=3;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_TO_MENU);assert(home&&!quit&&!stop&&!background_choice);
 puts("PASS production B exit decisions: paused closes, playing background, close choice, B closes, X returns to menu; saved preference unchanged");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'exit.c';p.write_text(fixture+production+checks)
 exe=Path(d)/'exit';subprocess.run(['gcc','-std=gnu99','-O2','-Wall','-Wextra','-Werror',str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
