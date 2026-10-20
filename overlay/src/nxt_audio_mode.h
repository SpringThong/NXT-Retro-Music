/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_AUDIO_MODE_H
#define NXT_AUDIO_MODE_H
#include <stdbool.h>
enum {NXT_AUDIO_NORMAL=0,NXT_AUDIO_BIT=1,NXT_AUDIO_AUTO=2};
void NxtAudio_setMode(int);
const char *NxtAudio_modeName(int);
void NxtAudio_prepare(const char *);
void NxtAudio_tick(void);
void NxtAudio_streaming(void);
void NxtAudio_routeChanged(void);
void NxtAudio_volumeFallback(void);
#endif
