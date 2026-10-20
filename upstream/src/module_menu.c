#include "nxt_personal.h"
#include "nxt_ota.h"
#include "nxt_soundcloud.h"
#include "nxt_spotify.h"
#include "module_menu.h"
#include "nxt.h"
#include "background.h"
#include "resume.h"
#include "module_player.h"
#include "nxt_skin.h"
#include "nxt_theme.h"
#include "api.h"
#include <stdio.h>
MenuSelection MenuModule_run(DisplayContext *display) {
 static int selected=0,column=0;int show=0;uint32_t redraw=0;Nxt_takeReturnHome();
 if(Background_getActive()==BG_NONE&&Player_getState()!=PLAYER_STATE_STOPPED)Background_setActive(BG_MUSIC);
 for(;;){
  ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(display);
  GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);int previous=selected;
  if(g.should_quit)return MENU_QUIT;
  if(!g.input_consumed&&!Nxt_lockInput()){
   if(NxtTheme_linearHome()&&(NxtPad_justRepeated(BTN_UP)||NxtPad_justRepeated(BTN_DOWN))){selected=(selected+(NxtPad_justRepeated(BTN_UP)?6:1))%7;}
   else if(NxtPad_justRepeated(BTN_UP)){if(selected==6)selected=4+column;else if(selected<2){column=selected;selected=6;}else selected-=2;}
   else if(NxtPad_justRepeated(BTN_DOWN)){if(selected==6)selected=column;else if(selected>=4){column=selected%2;selected=6;}else selected+=2;}
   else if(selected<6&&(NxtPad_justRepeated(BTN_LEFT)||NxtPad_justRepeated(BTN_RIGHT))){selected^=1;column=selected%2;}
   else if(NxtPad_justPressed(BTN_B))return MENU_QUIT;
   else if(NxtPad_justPressed(BTN_Y)){
    if(NxtSoundCloud_active()||NxtSpotify_active()||Background_getActive()!=BG_NONE||Player_getState()!=PLAYER_STATE_STOPPED)return MENU_NOW_PLAYING;
    if(Resume_isAvailable())return MENU_RESUME;
   }else if(NxtPad_justPressed(BTN_START)){if(Nxt_quickOptions(display)==MODULE_EXIT_QUIT)return MENU_QUIT;redraw=0;}
   else if(NxtPad_justPressed(BTN_A)){
    if(selected==0)return MENU_LIBRARY;if(selected==5)return MENU_SETTINGS;
    ModuleExitReason r=selected==1?Nxt_playlists(display):selected==2?Nxt_favorites(display):selected==3?Nxt_online(display):selected==4?NxtPersonal_run(display):Nxt_about(display);
    if(r==MODULE_EXIT_QUIT)return MENU_QUIT;Nxt_takeReturnHome();redraw=0;
   }
  }
  if(NxtSkin_shouldDraw(redraw,g.dirty||selected!=previous||PAD_anyJustPressed()||PAD_anyJustReleased())){NxtSkin_home(s,selected);ModuleCommon_markSurfaceDrawn();redraw=SDL_GetTicks();}
  ModuleCommon_frameEnd(s);
 }
}
