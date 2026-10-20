static void keys(int held,int pressed,int released){down=held;just=pressed;up=released;repeat=pressed;}
int main(void){
 NxtControl_defaults();off=1;keys(BTN_A,BTN_A,0);assert(!NxtPad_justPressed(BTN_A)&&!NxtControl_pressed(NC_PLAY));
 nxt.screen_off_controls=1;assert(NxtPad_justPressed(BTN_A)&&NxtControl_pressed(NC_PLAY));
 keys(BTN_R1,BTN_R1,0);assert(NxtControl_pressed(NC_NEXT));keys(BTN_L1,BTN_L1,0);assert(NxtControl_pressed(NC_PREV));
 keys(BTN_RIGHT,BTN_RIGHT,0);assert(NxtControl_held(NC_SEEK_FORWARD));
 nxt.playing_keys[0]=NC_VOLUME_UP;keys(BTN_UP,BTN_UP,0);assert(NxtControl_aux()&&vol==86&&off);
 nxt.playing_keys[0]=NC_VOLUME_DOWN;assert(NxtControl_aux()&&vol==85&&off);
 consumed=1;assert(!NxtPad_justPressed(BTN_UP)&&!NxtControl_repeated(NC_VOLUME_DOWN));consumed=0;
 ticks=1000;keys(BTN_SELECT,BTN_SELECT,0);assert(!Nxt_lockInput());ticks+=901;assert(Nxt_lockInput()&&locked);assert(!NxtControl_repeated(NC_VOLUME_DOWN));
 keys(0,0,BTN_SELECT);assert(Nxt_lockInput());keys(BTN_SELECT,BTN_SELECT,0);ticks+=20;assert(Nxt_lockInput());ticks+=901;assert(!Nxt_lockInput()&&!locked);
 keys(0,0,BTN_SELECT);assert(!NxtControl_pressed(NC_EQ));assert(!Nxt_lockInput());
 keys(BTN_SELECT,BTN_SELECT,0);ticks+=10;assert(!Nxt_lockInput());keys(0,0,BTN_SELECT);assert(!Nxt_lockInput());assert(NxtControl_pressed(NC_EQ));
 keys(BTN_A,BTN_A,0);off=0;nxt.screen_off_controls=0;assert(NxtControl_pressed(NC_PLAY));
 puts("PASS production dark-screen pad/mapped play/skip/seek/volume routing, consumed wake, long SELECT lock/unlock and short EQ release.");
}
