/* Compare production ARM64 integer decoding with original PCM fixtures. */
#include "nxt_decoder.h"
#include "nxt_native_pcm.h"
#include "nxt_drive.h"
#include "nxt_stream.h"
#include "nxt_text.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
bool NxtDrive_isStream(const char *p){return false;}
void NxtDrive_metadata(const char *p,TrackInfo *i){}
bool NxtDrive_streamEntry(const char *p,NxtDriveEntry *e){return false;}
unsigned NxtStream_epoch(void){return 1;}
NxtStream *NxtStream_open(const NxtDriveEntry *e,unsigned g){return NULL;}
int NxtStream_read(NxtStream *s,void *d,size_t n){return -1;}
int64_t NxtStream_size(const NxtStream *s){return 0;}
int64_t NxtStream_position(const NxtStream *s){return 0;}
int64_t NxtStream_seek(NxtStream *s,int64_t n){return -1;}
bool NxtStream_seekable(const NxtStream *s){return false;}
void NxtStream_close(NxtStream *s){}
void NxtStream_rearm(NxtStream *s,unsigned g){}
void NxtStream_cancelAll(void){}
void NxtText_copy(char *out,const char *in,size_t n){snprintf(out,n,"%s",in);}
