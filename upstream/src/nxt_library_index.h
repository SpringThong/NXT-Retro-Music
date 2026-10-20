/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_LIBRARY_INDEX_H
#define NXT_LIBRARY_INDEX_H
#include <stdbool.h>
#include <sqlite3.h>
typedef struct {char title[256],artist[256],album[256];int track;} NxtIndexTrack;
typedef struct {int checked,decoded,added,updated,removed,writes,commits,deferred;bool complete,cancelled,unavailable;} NxtIndexResult;
typedef struct {
 bool (*audio)(const char*,void*);
 bool (*probe)(const char*,NxtIndexTrack*,void*);
 bool (*checkpoint)(void*);
 void (*changed)(void*);
 void (*progress)(const NxtIndexResult*,void*);
 void *context;
} NxtIndexHooks;
bool NxtIndex_schema(sqlite3*);
bool NxtIndex_inScope(const char*,const char*,bool);
/* Incremental walks read stat only for unchanged media; main DB writes occur
 * only for changed signatures/metadata or confirmed missing files, in batches.
 * force=true re-probes all media without discarding the saved index first. */
bool NxtIndex_run(const char *dbpath,const char *root,bool recursive,bool force,const NxtIndexHooks*,NxtIndexResult*);
#endif
