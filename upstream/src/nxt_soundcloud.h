/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_SOUNDCLOUD_H
#define NXT_SOUNDCLOUD_H
#include "nxt.h"
#include "player.h"
ModuleExitReason NxtSoundCloud_run(DisplayContext *);
ModuleExitReason NxtSoundCloud_nowPlaying(DisplayContext *);
ModuleExitReason NxtSoundCloud_queue(DisplayContext *);
void NxtSoundCloud_tick(void);
void NxtSoundCloud_quit(void);
bool NxtSoundCloud_active(void);
bool NxtSoundCloud_hasMetadata(void);
bool NxtSoundCloud_isPlaying(void);
int NxtSoundCloud_audioSamples(float *,int,int);
const TrackInfo *NxtSoundCloud_info(void);
const char *NxtSoundCloud_uri(void);
SDL_Surface *NxtSoundCloud_art(void);
int NxtSoundCloud_position(void);
unsigned NxtSoundCloud_generation(void);
void NxtSoundCloud_togglePause(void);
void NxtSoundCloud_seek(int);
void NxtSoundCloud_skip(int direction);
int NxtSoundCloud_queueCount(void);
int NxtSoundCloud_queueIndex(void);
bool NxtSoundCloud_isFavorite(void);
void NxtSoundCloud_favorite(void);
const char *NxtSoundCloud_lyricsStatus(void);
ModuleExitReason NxtSoundCloud_playSaved(DisplayContext *,const char *,const char *,const char *);
ModuleExitReason NxtSoundCloud_recommendations(DisplayContext *,bool);
#endif
