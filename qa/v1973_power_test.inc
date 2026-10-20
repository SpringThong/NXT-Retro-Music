static void frame(int held,int pressed,int released){down=held;just=pressed;up=released;ticks+=20;PWR_displayFrameBegin();PWR_update(NULL,NULL,NULL,NULL);just=up=0;}
static void idle(void){frame(0,0,0);frame(0,0,0);}
int main(void){
 pwr.can_sleep=1;PWR_setDisplayOnlySleep(1);PWR_setScreenOffControls(0);
 PWR_setDisplayOff(1);assert(!lit);frame(BTN_A,BTN_A,0);assert(lit&&!PWR_getDisplayOff()&&PWR_displayInputConsumed());
 frame(0,0,BTN_A);assert(PWR_displayInputConsumed());idle();assert(!PWR_displayInputConsumed());
 PWR_setScreenOffControls(1);PWR_setDisplayOff(1);idle();assert(PWR_getDisplayOff()&&!PWR_displayInputConsumed()&&!lit);
 int before=backlight_calls;
 const int keys[]={BTN_A,BTN_B,BTN_X,BTN_Y,BTN_UP,BTN_DOWN,BTN_LEFT,BTN_RIGHT,BTN_L1,BTN_R1,BTN_L2,BTN_R2,BTN_SELECT,BTN_START,BTN_PLUS,BTN_MINUS};
 for(unsigned i=0;i<sizeof(keys)/sizeof(*keys);i++){frame(keys[i],keys[i],0);assert(PWR_getDisplayOff()&&!lit&&!PWR_displayInputConsumed());frame(keys[i],0,0);frame(0,0,keys[i]);}
 assert(backlight_calls==before&&!sleep_calls&&!poweroff_calls);
 /* Auto-timeout still blanks without invoking suspend or muting audio. */
 PWR_setDisplayOff(0);idle();ticks+=61000;frame(0,0,0);assert(PWR_getDisplayOff()&&!lit);idle();
 /* Power is consumed; a press/release wakes exactly once. */
 before=backlight_calls;frame(BTN_POWER,BTN_POWER,0);assert(PWR_getDisplayOff()&&PWR_displayInputConsumed());frame(0,0,BTN_SLEEP);assert(!PWR_getDisplayOff()&&lit&&PWR_displayInputConsumed());assert(backlight_calls==before+1);idle();assert(!PWR_displayInputConsumed());
 frame(BTN_POWER,BTN_POWER,0);frame(0,0,BTN_SLEEP);assert(PWR_getDisplayOff()&&!lit);idle();assert(!PWR_displayInputConsumed());
 /* Disabling restores legacy wake and consumes that gesture. */
 PWR_setScreenOffControls(0);frame(BTN_R1,BTN_R1,0);assert(lit&&PWR_displayInputConsumed());idle();
 PWR_setScreenOffControls(1);PWR_setDisplayOff(1);PWR_setDisplayOnlySleep(0);assert(lit&&!PWR_getDisplayOff());
 assert(!sleep_calls&&!poweroff_calls);
 puts("PASS actual PWR functions: all mapped keys stay dark, default any-key wake, release guards, Power toggle, auto blank, cleanup, no suspend/audio callbacks.");
}
