/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_BITPERFECT_H
#define NXT_BITPERFECT_H
#include "player.h"
int NxtBit_init(void);
void NxtBit_quit(void);
int NxtBit_load(const char *,int position,PlayerState state);
int NxtBit_loadStatus(void);
int NxtBit_waitLoaded(void);
void NxtBit_route(int card,bool usb_selected);
void NxtBit_play(void);
void NxtBit_pause(void);
void NxtBit_stop(void);
void NxtBit_seek(int ms);
bool NxtBit_seeking(void);
PlayerState NxtBit_state(void);
int NxtBit_position(void);
int64_t NxtBit_listened(void);
bool NxtBit_ready(void);
int NxtBit_rate(void);
int NxtBit_bits(void);
int NxtBit_channels(void);
int NxtBit_vis(int16_t *,int);
float NxtBit_peak(void);
unsigned NxtBit_snapshot(TrackInfo *,char *,size_t);
const char *NxtBit_error(void);
const char *NxtBit_status(void);
void NxtBit_setDSDMode(int);
bool NxtBit_isDSD(void);
#endif
