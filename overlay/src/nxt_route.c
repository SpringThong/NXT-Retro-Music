/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_route.h"
int NxtRoute_choose(int current,int auto_dac,int auto_bt,int usb,int previous_usb,int bt,int previous_bt){
    if(current<0||current>2)current=0;
    if(auto_bt&&bt&&!previous_bt)current=1;
    if(auto_dac&&usb&&!previous_usb)current=2;
    if(current==2&&!usb)current=bt&&auto_bt?1:0;
    if(current==1&&!bt)current=usb&&auto_dac?2:0;
    return current;
}
