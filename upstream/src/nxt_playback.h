/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_PLAYBACK_H
#define NXT_PLAYBACK_H
#include "player.h"
#include "nxt_spotify.h"
#include "nxt_soundcloud.h"
#include "nxt_startup.h"
/* One source of display/control data; the local decoder keeps its own state. */
static inline const TrackInfo *NxtPlayback_info(void){return NxtStartup_preview()?NxtStartup_info():NxtSoundCloud_hasMetadata()?NxtSoundCloud_info():NxtSpotify_hasMetadata()?NxtSpotify_info():Player_getTrackInfo();}
static inline PlayerState NxtPlayback_state(void){return NxtStartup_preview()?PLAYER_STATE_PAUSED:NxtSoundCloud_hasMetadata()?(NxtSoundCloud_active()?(NxtSoundCloud_isPlaying()?PLAYER_STATE_PLAYING:PLAYER_STATE_PAUSED):PLAYER_STATE_STOPPED):NxtSpotify_hasMetadata()?(NxtSpotify_active()?(NxtSpotify_isPlaying()?PLAYER_STATE_PLAYING:PLAYER_STATE_PAUSED):PLAYER_STATE_STOPPED):Player_getState();}
static inline int NxtPlayback_position(void){return NxtStartup_preview()?NxtStartup_position():NxtSoundCloud_hasMetadata()?NxtSoundCloud_position():NxtSpotify_hasMetadata()?NxtSpotify_position():Player_getPosition();}
static inline int NxtPlayback_duration(void){return NxtStartup_preview()?NxtStartup_info()->duration_ms:NxtSoundCloud_hasMetadata()?NxtSoundCloud_info()->duration_ms:NxtSpotify_hasMetadata()?NxtSpotify_info()->duration_ms:Player_getDuration();}
static inline const char *NxtPlayback_path(void){return NxtStartup_preview()?NxtStartup_path():NxtSoundCloud_hasMetadata()?NxtSoundCloud_uri():NxtSpotify_hasMetadata()?NxtSpotify_uri():Player_getCurrentFile();}
static inline SDL_Surface *NxtPlayback_art(void){return NxtStartup_preview()?NxtStartup_art():NxtSoundCloud_hasMetadata()?NxtSoundCloud_art():NxtSpotify_hasMetadata()?NxtSpotify_art():Player_getAlbumArt();}
static inline unsigned NxtPlayback_generation(void){return NxtStartup_preview()?0x70000000u:NxtSoundCloud_hasMetadata()?(0xc0000000u|NxtSoundCloud_generation()):NxtSpotify_hasMetadata()?(0x80000000u|NxtSpotify_generation()):Player_getTrackGeneration();}
static inline void NxtPlayback_toggle(void){if(NxtSoundCloud_hasMetadata())NxtSoundCloud_togglePause();else if(NxtSpotify_hasMetadata())NxtSpotify_togglePause();else Player_togglePause();}
static inline void NxtPlayback_seek(int ms){if(NxtSoundCloud_hasMetadata())NxtSoundCloud_seek(ms);else if(NxtSpotify_hasMetadata())NxtSpotify_seek(ms);else Player_seek(ms);}
static inline bool NxtPlayback_favorite(void){return NxtStartup_preview()?false:NxtSoundCloud_hasMetadata()?NxtSoundCloud_isFavorite():NxtSpotify_hasMetadata()?NxtSpotify_isFavorite():Nxt_isFavorite(Player_getCurrentFile());}
static inline void NxtPlayback_toggleFavorite(void){if(NxtSoundCloud_hasMetadata())NxtSoundCloud_favorite();else if(NxtSpotify_hasMetadata())NxtSpotify_favorite();else Nxt_favorite(Player_getCurrentFile());}
#endif
