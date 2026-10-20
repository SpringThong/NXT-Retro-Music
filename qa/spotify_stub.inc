/* Only for isolated SoundCloud/native renderer tests: Spotify is inactive. */
#include "nxt_spotify.h"
bool NxtSpotify_active(void){return false;}
bool NxtSpotify_hasMetadata(void){return false;}
bool NxtSpotify_isPlaying(void){return false;}
int NxtSpotify_position(void){return 0;}
const TrackInfo *NxtSpotify_info(void){static TrackInfo i;return &i;}
const char *NxtSpotify_uri(void){return "";}
SDL_Surface *NxtSpotify_art(void){return NULL;}
unsigned NxtSpotify_generation(void){return 0;}
const char *NxtSpotify_lyricsStatus(void){return "";}
void NxtSpotify_togglePause(void){}
void NxtSpotify_seek(int ms){}
bool NxtSpotify_isFavorite(void){return false;}
void NxtSpotify_favorite(void){}
void NxtSpotify_quit(void){}
