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
int main(int argc,char **argv){
 assert(argc==8);int rate=atoi(argv[4]),bits=atoi(argv[5]),channels=atoi(argv[6]);
 NxtDecoder *d=NxtDecoder_open(argv[1],0);if(!strcmp(argv[7],"reject")){assert(!d);puts("PASS incompatible/lossy source rejected");return 0;}
 assert(d);const TrackInfo *info=NxtDecoder_info(d);assert(info->sample_rate==rate&&info->bit_depth==bits&&info->channels==channels);
 FILE *f=fopen(argv[2],"rb");assert(f);fseek(f,0,SEEK_END);size_t size=ftell(f);rewind(f);uint8_t *reference=malloc(size),*decoded=malloc(size+16384);assert(reference&&decoded);assert(fread(reference,1,size,f)==size);fclose(f);
 NxtNativeFormat format=bits==16?NXT_PCM_S16:bits==24?NXT_PCM_S24_3:NXT_PCM_S32;size_t width=NxtNative_width(format),done=0;int32_t pcm[2048];int n;
 while((n=NxtDecoder_readNative(d,pcm,1024))>0){size_t bytes=n*channels*width;assert(done+bytes<=size);NxtNative_pack(pcm,n*channels,format,decoded+done);done+=bytes;}
 assert(n==0&&done==size);if(memcmp(reference,decoded,size)){for(size_t i=0;i<size;i++)if(reference[i]!=decoded[i]){fprintf(stderr,"PCM mismatch at byte %zu: %02x != %02x\n",i,reference[i],decoded[i]);break;}abort();}
 int ms=333;assert(NxtDecoder_seek(d,ms));n=NxtDecoder_readNative(d,pcm,1024);assert(n>0);NxtNative_pack(pcm,n*channels,format,decoded);
 /* Integer native seek trims to the exact requested source sample. */
 size_t frame=(size_t)((int64_t)ms*rate+500)/1000;assert(frame*channels*width+(size_t)n*channels*width<=size);assert(!memcmp(decoded,reference+frame*channels*width,n*channels*width));
 NxtDecoder_close(d);free(reference);free(decoded);printf("PASS bit-exact decode + seek: %s; %d Hz / %d bit / %d ch; %zu bytes\n",argv[1],rate,bits,channels,size);return 0;
}
