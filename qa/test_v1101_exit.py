"""Run the real player-back and root-exit decisions with deterministic audio/UI stubs."""
from pathlib import Path
import subprocess
import tempfile

R = Path(__file__).resolve().parents[1]
session = (R / 'upstream/src/nxt_session.c').read_text()
start = session.index('bool NxtSession_wantsBackground(')
end = session.index('bool NxtSession_background(', start)
production = session[start:end]
fixture = r'''
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
#define PLAYER_STATE_PAUSED 2
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
checks = r'''
static void reset(void){background_choice=sp=sc=stop=quit=home=false;player=radio=action=choice=dialogs=0;nxt.background=0;}
int main(void){
 /* First B, on a fresh startup card or paused player: navigation only. */
 reset();assert(NxtSession_playerBack()==MODULE_EXIT_TO_MENU);assert(home&&!stop&&!quit&&!dialogs);
 reset();player=PLAYER_STATE_PAUSED;assert(NxtSession_playerBack()==MODULE_EXIT_TO_MENU);assert(home&&!stop&&!quit&&!dialogs&&player==PLAYER_STATE_PAUSED);
 /* Second B, at root, closes a paused app regardless of saved background. */
 home=false;nxt.background=1;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit&&!dialogs&&!background_choice);
 /* Local/Drive, Spotify, SoundCloud and radio keep playing through first B. */
 for(int source=0;source<4;source++){
  reset();player=source==0?1:0;sp=source==1;sc=source==2;radio=source==3?1:0;
  assert(NxtSession_playerBack()==MODULE_EXIT_TO_MENU);assert(home&&!stop&&!quit&&!dialogs);
  home=false;action=1;choice=0;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);
  assert(quit&&!stop&&NxtSession_wantsBackground()&&!nxt.background&&dialogs==1);
 }
 reset();assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit&&!dialogs);
 reset();player=1;action=1;choice=1;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit&&!background_choice&&dialogs==1);
 reset();sp=true;action=2;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_QUIT);assert(stop&&quit);
 reset();sc=true;action=3;assert(NxtSession_exitPrompt(0)==MODULE_EXIT_TO_MENU);assert(home&&!quit&&!stop&&!background_choice);
 puts("PASS v1.10.1 production B flow: player returns to main menu without pause/stop/dialog; root paused/stopped exits, playing asks background/exit; saved preference unchanged");
}
'''
with tempfile.TemporaryDirectory() as d:
    p = Path(d) / 'exit.c'
    p.write_text(fixture + production + checks)
    exe = Path(d) / 'exit'
    subprocess.run(['gcc', '-std=gnu99', '-O2', '-Wall', '-Wextra', '-Werror', str(p), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)

# Guard the route boundary: a player must never call the root exit dialog.
callers = []
for p in (R / 'upstream/src').glob('*.c'):
    if 'NxtSession_exitPrompt(' in p.read_text():
        callers.append(p.name)
assert sorted(callers) == ['module_menu.c', 'nxt_session.c'], callers
for name, count in [('nxt_startup.c', 1), ('nxt_spotify.c', 1), ('nxt_soundcloud.c', 1), ('module_player.c', 2)]:
    text = (R / 'upstream/src' / name).read_text()
    assert text.count('NxtSession_playerBack(') == count, name
print('PASS root-only exit prompt and all startup/local/playlist/Spotify/SoundCloud back routes')
