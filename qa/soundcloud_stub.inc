/* Only for isolated Spotify/native renderer tests: SoundCloud is inactive. */
#include "nxt_soundcloud.h"
bool NxtSoundCloud_active(void){return false;}
bool NxtSoundCloud_hasMetadata(void){return false;}
bool NxtSoundCloud_isPlaying(void){return false;}
int NxtSoundCloud_position(void){return 0;}
const TrackInfo *NxtSoundCloud_info(void){static TrackInfo i;return &i;}
const char *NxtSoundCloud_uri(void){return "";}
SDL_Surface *NxtSoundCloud_art(void){return NULL;}
unsigned NxtSoundCloud_generation(void){return 0;}
const char *NxtSoundCloud_lyricsStatus(void){return "";}
void NxtSoundCloud_togglePause(void){}
void NxtSoundCloud_seek(int ms){}
bool NxtSoundCloud_isFavorite(void){return false;}
void NxtSoundCloud_favorite(void){}
void NxtSoundCloud_quit(void){}
