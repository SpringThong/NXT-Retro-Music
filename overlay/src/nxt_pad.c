/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt.h"
#include "api.h"
const int Nxt_keyIds[16]={BTN_UP,BTN_DOWN,BTN_LEFT,BTN_RIGHT,BTN_A,BTN_B,BTN_X,BTN_Y,BTN_L1,BTN_R1,BTN_L2,BTN_R2,BTN_L3,BTN_R3,BTN_SELECT,BTN_START};
const char *Nxt_keyNames[16]={"UP","DOWN","LEFT","RIGHT","A","B","X","Y","L1","R1","L2","R2","L3","R3","SELECT","START"};
int NxtPad_physical(int btn){for(int i=0;i<16;i++)if(btn==Nxt_keyIds[i])return nxt.keys[i]?nxt.keys[i]:btn;return btn;}
int NxtPad_directionMask(int key){
 switch(key){case BTN_UP:return key|BTN_ANALOG_UP;case BTN_DOWN:return key|BTN_ANALOG_DOWN;case BTN_LEFT:return key|BTN_ANALOG_LEFT;case BTN_RIGHT:return key|BTN_ANALOG_RIGHT;default:return key;}
}
static bool blocked(void){return Nxt_displayInputBlocked();}
int NxtPad_isPressed(int btn){return blocked()?0:PAD_isPressed(NxtPad_directionMask(NxtPad_physical(btn)));}
int NxtPad_justPressed(int btn){if(blocked())return 0;int key=NxtPad_physical(btn);return key==BTN_SELECT?(PAD_justReleased(key)&&!Nxt_selectLongUsed()):PAD_justPressed(NxtPad_directionMask(key));}
int NxtPad_justReleased(int btn){return blocked()?0:PAD_justReleased(NxtPad_directionMask(NxtPad_physical(btn)));}
int NxtPad_justRepeated(int btn){if(blocked())return 0;int key=NxtPad_physical(btn);return key==BTN_SELECT?NxtPad_justPressed(btn):PAD_justRepeated(NxtPad_directionMask(key));}
