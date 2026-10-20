/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_STREAM_H
#define NXT_STREAM_H
#include "nxt_drive_parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef struct NxtStream NxtStream;
/* HTTPS with private, bounded temporary disk buffering. */
NxtStream *NxtStream_open(const NxtDriveEntry *entry,unsigned epoch);
int NxtStream_read(NxtStream *,void *,size_t);
int64_t NxtStream_seek(NxtStream *,int64_t);
int64_t NxtStream_size(const NxtStream *);
int64_t NxtStream_position(const NxtStream *);
bool NxtStream_seekable(const NxtStream *);
unsigned NxtStream_epoch(void);
void NxtStream_rearm(NxtStream *,unsigned epoch);
void NxtStream_close(NxtStream *);
/* Interrupt blocked reads/open/seek when changing/stopping the player. */
void NxtStream_cancelAll(void);
void NxtStream_prefetch(const NxtDriveEntry *);
bool NxtStream_prefetched(const NxtDriveEntry *);
void NxtStream_cleanup(void);
#endif
