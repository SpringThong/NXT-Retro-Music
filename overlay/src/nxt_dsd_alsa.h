/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_DSD_ALSA_H
#define NXT_DSD_ALSA_H
#include <alsa/asoundlib.h>
#include "nxt_dsd.h"
int NxtDSD_openALSA(snd_pcm_t**,int,const NxtDSDInfo*,int,NxtDSDFormat*,char[256]);
#endif
