/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_DRIVE_H
#define NXT_DRIVE_H
#include "nxt.h"
#include "player.h"
#include "nxt_drive_parser.h"
ModuleExitReason NxtDrive_run(DisplayContext *);
bool NxtDrive_isPath(const char *);
bool NxtDrive_isStream(const char *);
bool NxtDrive_streamEntry(const char *,NxtDriveEntry *);
bool NxtDrive_preparePlayback(const char *);
void NxtDrive_metadata(const char *,TrackInfo *);
#endif
