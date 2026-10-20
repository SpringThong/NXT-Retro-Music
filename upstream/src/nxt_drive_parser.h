/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_DRIVE_PARSER_H
#define NXT_DRIVE_PARSER_H
#include <stdbool.h>
#include <stddef.h>
typedef struct {
    char id[128],resource_key[128],name[256];
    bool folder;
} NxtDriveEntry;
bool NxtDrive_parseURL(const char *,NxtDriveEntry *);
bool NxtDrive_url(const NxtDriveEntry *,bool download,char *out,size_t capacity);
/* Public embedded folder HTML. Returns -1 for inaccessible/unrecognized pages. */
int NxtDrive_parseFolder(const char *,NxtDriveEntry *,int,char *title,size_t);
bool NxtDrive_fileName(const char *,char *,size_t);
/* A confirmation form is accepted only for Google's download endpoint and ID. */
bool NxtDrive_confirmation(const char *,const NxtDriveEntry *,char *,size_t);
#endif
