/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#ifndef NXT_DECODER_H
#define NXT_DECODER_H
#include "player.h"
typedef struct NxtDecoder NxtDecoder;
NxtDecoder *NxtDecoder_open(const char *,int rate);
NxtDecoder *NxtDecoder_openEpoch(const char *,int rate,unsigned epoch);
/* rate=0 selects native integer PCM; unsupported codecs/layouts fail. */
int NxtDecoder_readNative(NxtDecoder *,int32_t *interleaved,int frames);
void NxtDecoder_cancelStreams(void);
void NxtDecoder_close(NxtDecoder *);
int NxtDecoder_read(NxtDecoder *,float *stereo,int frames);
bool NxtDecoder_canSeek(const NxtDecoder *);
bool NxtDecoder_seek(NxtDecoder *,int ms);
bool NxtDecoder_seekEpoch(NxtDecoder *,int ms,unsigned epoch);
const TrackInfo *NxtDecoder_info(NxtDecoder *);
int NxtDecoder_position(NxtDecoder *);
bool NxtDecoder_probe(const char *,TrackInfo *);
SDL_Surface *NxtDecoder_art(const char *);
char *NxtDecoder_lyrics(const char *);
#endif
