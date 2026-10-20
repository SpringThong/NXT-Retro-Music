/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_SPOTIFY_H
#define NXT_SPOTIFY_H
#include "nxt.h"
#include "player.h"
ModuleExitReason NxtSpotify_run(DisplayContext *);
ModuleExitReason NxtSpotify_nowPlaying(DisplayContext *);
void NxtSpotify_tick(void);
void NxtSpotify_quit(void);
bool NxtSpotify_active(void);
bool NxtSpotify_hasMetadata(void);
bool NxtSpotify_isPlaying(void);
int NxtSpotify_audioSamples(float *out,int frames,int rate);
const TrackInfo *NxtSpotify_info(void);
const char *NxtSpotify_uri(void);
SDL_Surface *NxtSpotify_art(void);
int NxtSpotify_position(void);
unsigned NxtSpotify_generation(void);
void NxtSpotify_togglePause(void);
void NxtSpotify_seek(int ms);
void NxtSpotify_skip(int direction);
int NxtSpotify_queueCount(void);
int NxtSpotify_queueIndex(void);
bool NxtSpotify_isFavorite(void);
void NxtSpotify_favorite(void);
const char *NxtSpotify_lyricsStatus(void);
ModuleExitReason NxtSpotify_queue(DisplayContext *);
ModuleExitReason NxtSpotify_playSaved(DisplayContext *,const char *,const char *,const char *);
ModuleExitReason NxtSpotify_recommendations(DisplayContext *,bool);
#endif
