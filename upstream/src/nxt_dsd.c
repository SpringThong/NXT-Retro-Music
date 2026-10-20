/* SPDX-License-Identifier: GPL-3.0-only
 * Direct raw transport, Sony DSF 1.01 / Philips DSDIFF 1.5 / DoP 1.1.
 * Canonical byte chronology: oldest DSD bit in the MSB. No PCM decoding.
 * Audio memory is bounded to two channel blocks; metadata <= 4 MiB. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "nxt_dsd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
struct NxtDSD {FILE*f;NxtDSDInfo info;uint64_t size,start,data_size,bytes,position,meta,cache_base;unsigned block,cached;bool planar,lsb,valid;uint8_t marker,*cache;};
static uint32_t le32(const uint8_t*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint32_t be32(const uint8_t*p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static uint64_t le64(const uint8_t*p){return le32(p)|(uint64_t)le32(p+4)<<32;}
static uint64_t be64(const uint8_t*p){return (uint64_t)be32(p)<<32|be32(p+4);}
static uint32_t syncsafe(const uint8_t*p){return ((p[0]|p[1]|p[2]|p[3])&128)?UINT32_MAX:(uint32_t)p[0]<<21|(uint32_t)p[1]<<14|(uint32_t)p[2]<<7|p[3];}
static bool at(NxtDSD*d,uint64_t off,void*p,size_t n){return off<=d->size&&n<=d->size-off&&off<=INT64_MAX&&!fseeko(d->f,off,SEEK_SET)&&fread(p,1,n,d->f)==n;}
static bool rateok(uint32_t r){return r==2822400||r==5644800||r==11289600||r==22579200||r==3072000||r==6144000||r==12288000||r==24576000;}
static uint8_t reverse(uint8_t x){x=x>>4|x<<4;x=(x&0xcc)>>2|(x&0x33)<<2;return (x&0xaa)>>1|(x&0x55)<<1;}
bool NxtDSD_isPath(const char*p){const char*e=p?strrchr(p,'.'):NULL;return e&&(!strcasecmp(e,".dsf")||!strcasecmp(e,".dff"));}
static bool dsf(NxtDSD*d){
 uint8_t h[52];if(!at(d,0,h,28)||le64(h+4)!=28||le64(h+12)!=d->size)return false;d->meta=le64(h+20);
 if(d->meta&&(d->meta<28||d->meta>=d->size))return false;uint64_t pos=28;bool fmt=false,data=false;
 for(int k=0;k<1024&&pos+12<=d->size;k++){
  if(!at(d,pos,h,12))return false;uint64_t len=le64(h+4);if(len<12||len>d->size-pos)return false;
  if(!memcmp(h,"fmt ",4)){if(fmt||len<52||!at(d,pos,h,52)||le32(h+12)!=1||le32(h+16))return false;d->info.channels=le32(h+24);d->info.rate=le32(h+28);unsigned bits=le32(h+32),layout=le32(h+20);d->info.samples=le64(h+36);d->block=le32(h+44);if((bits!=1&&bits!=8)||!d->block||d->block>65536||d->block%4||layout!=(unsigned)d->info.channels)return false;d->lsb=bits==1;fmt=true;}
  else if(!memcmp(h,"data",4)){if(!fmt)return false;d->start=pos+12;d->data_size=len-12;data=true;break;}pos+=len;
 }
 if(!fmt||!data||d->info.channels<1||d->info.channels>2||!rateok(d->info.rate)||!d->info.samples||d->info.samples%8)return false;
 d->bytes=d->info.samples/8;uint64_t blocks=(d->bytes+d->block-1)/d->block;if(blocks>UINT64_MAX/d->block/d->info.channels||blocks*d->block*d->info.channels>d->data_size)return false;
 if(d->meta&&d->meta<d->start+d->data_size)return false;d->planar=true;strcpy(d->info.container,"DSF");return true;
}
static bool prop(NxtDSD*d,uint64_t pos,uint64_t end,bool*raw){
 uint8_t h[12];if(end-pos<4||!at(d,pos,h,4)||memcmp(h,"SND ",4))return false;pos+=4;bool fs=false,ch=false,cm=false;
 for(int k=0;k<1024&&pos+12<=end;k++){if(!at(d,pos,h,12))return false;uint64_t len=be64(h+4),start=pos+12;if(len>end-start)return false;
  if(!memcmp(h,"FS  ",4)){if(fs||len!=4||!at(d,start,h,4))return false;d->info.rate=be32(h);fs=true;}
  else if(!memcmp(h,"CHNL",4)){if(ch||len<2||!at(d,start,h,2))return false;d->info.channels=(unsigned)h[0]<<8|h[1];if(d->info.channels<1||d->info.channels>2||len<2+4u*d->info.channels)return false;ch=true;}
  else if(!memcmp(h,"CMPR",4)){if(cm||len<5||!at(d,start,h,4))return false;*raw=!memcmp(h,"DSD ",4);cm=true;}
  if((len&1)&&start+len==end)return false;pos=start+len+(len&1);
 }return pos==end&&fs&&ch&&cm;
}
static bool dff(NxtDSD*d,char error[256]){
 uint8_t h[16];if(!at(d,0,h,16)||be64(h+4)<4||be64(h+4)>d->size-12||memcmp(h+12,"DSD ",4))return false;
 uint64_t end=be64(h+4)+12,pos=16;bool version=false,property=false,raw=false,data=false;
 for(int k=0;k<4096&&pos+12<=end;k++){if(!at(d,pos,h,12))return false;uint64_t len=be64(h+4),start=pos+12;if(len>end-start)return false;
  if(!memcmp(h,"FVER",4)){if(version||len!=4||!at(d,start,h,4)||h[0]!=1)return false;version=true;}
  else if(!memcmp(h,"PROP",4)){if(property||!prop(d,start,start+len,&raw))return false;property=true;}
  else if(!memcmp(h,"DST ",4)){snprintf(error,256,"DST-compressed DFF unsupported; use raw DSF/DFF");return false;}
  else if(!memcmp(h,"DSD ",4)){if(!property||!raw||data)return false;d->start=start;d->data_size=len;data=true;}
  else if(!memcmp(h,"ID3 ",4))d->meta=start;
  if((len&1)&&start+len==end)return false;pos=start+len+(len&1);
 }
 if(pos!=end||!version||!property||!raw||!data||!rateok(d->info.rate)||!d->data_size||d->data_size%d->info.channels)return false;
 d->bytes=d->data_size/d->info.channels;if(d->bytes>UINT64_MAX/8)return false;d->info.samples=d->bytes*8;d->block=4096;strcpy(d->info.container,"DFF");return true;
}
static void text(const uint8_t*p,size_t n,char out[256]){
 if(!n)return;unsigned enc=*p++;n--;size_t used=0;
 if(enc==0||enc==3){while(n--&&*p&&used<255){unsigned c=*p++;if(enc==0&&c>=128){if(used>252)break;out[used++]=0xc0|(c>>6);out[used++]=0x80|(c&63);}else if(c>=32)out[used++]=c;}}
 else if(enc==1||enc==2){bool le=false;if(enc==1){if(n<2)return;le=p[0]==255&&p[1]==254;if(!le&&!(p[0]==254&&p[1]==255))return;p+=2;n-=2;}while(n>=2&&used<253){unsigned c=le?p[0]|p[1]<<8:p[0]<<8|p[1];p+=2;n-=2;if(!c)break;if(c<32||(c>=0xd800&&c<=0xdfff))continue;if(c<128)out[used++]=c;else if(c<2048){out[used++]=0xc0|(c>>6);out[used++]=0x80|(c&63);}else{out[used++]=0xe0|(c>>12);out[used++]=0x80|((c>>6)&63);out[used++]=0x80|(c&63);}}}out[used]=0;
}
static void tags(NxtDSD*d,uint8_t**cover,size_t*size){
 uint8_t h[10];if(!d->meta||!at(d,d->meta,h,10)||memcmp(h,"ID3",3)||(h[3]!=3&&h[3]!=4)||(h[5]&0xc0))return;unsigned ver=h[3];uint32_t n=syncsafe(h+6);if(n==UINT32_MAX||n>4*1024*1024||n>d->size-d->meta-10)return;uint8_t*tag=malloc(n?n:1);if(!tag)return;if(!at(d,d->meta+10,tag,n)){free(tag);return;}
 for(size_t i=0;i+10<=n;){uint8_t*f=tag+i;if(!*f)break;uint32_t len=ver==4?syncsafe(f+4):be32(f+4);if(len==UINT32_MAX||len>n-i-10)break;uint8_t*p=f+10;
  if(!f[8]&&!f[9]){if(!memcmp(f,"TIT2",4))text(p,len,d->info.title);else if(!memcmp(f,"TPE1",4))text(p,len,d->info.artist);else if(!memcmp(f,"TALB",4))text(p,len,d->info.album);
   else if(cover&&!memcmp(f,"APIC",4)&&len>4){unsigned enc=*p;size_t at_=1;while(at_<len&&p[at_])at_++;if(at_+2<len){at_+=2;if(enc==0||enc==3){while(at_<len&&p[at_])at_++;at_++;}else if(enc==1||enc==2){while(at_+1<len&&(p[at_]||p[at_+1]))at_+=2;at_+=2;}else at_=len;
    if(at_<len){size_t bytes=len-at_;if(bytes>3&&((p[at_]==255&&p[at_+1]==216)||(bytes>=8&&!memcmp(p+at_,"\x89PNG\r\n\x1a\n",8)))){*cover=malloc(bytes);if(*cover){memcpy(*cover,p+at_,bytes);*size=bytes;break;}}}}}}
  i+=10+len;
 }free(tag);
}
NxtDSD*NxtDSD_open(const char*path,char error[256]){
 char fallback[256]="";if(!error)error=fallback;error[0]=0;NxtDSD*d=calloc(1,sizeof(*d));if(!d)return NULL;
 int fd=path?open(path,O_RDONLY|O_NONBLOCK|O_CLOEXEC):-1;if(fd<0)goto fail;d->f=fdopen(fd,"rb");if(!d->f){close(fd);goto fail;}struct stat st;if(fstat(fd,&st)||!S_ISREG(st.st_mode)||st.st_size<28)goto fail;d->size=st.st_size;
 uint8_t id[4];if(!at(d,0,id,4))goto fail;if(!memcmp(id,"DSD ",4)){if(!dsf(d))goto fail;}else if(!memcmp(id,"FRM8",4)){if(!dff(d,error))goto fail;}else goto fail;
 uint64_t ms=d->info.samples/d->info.rate*1000+(d->info.samples%d->info.rate)*1000/d->info.rate;if(ms>INT_MAX)goto fail;d->info.duration_ms=ms;d->cache=malloc(d->block*d->info.channels);if(!d->cache)goto fail;d->marker=5;
 const char*base=strrchr(path,'/');snprintf(d->info.title,256,"%s",base?base+1:path);tags(d,NULL,NULL);if(!*d->info.title)snprintf(d->info.title,256,"%s",base?base+1:path);return d;
fail:if(!*error)snprintf(error,256,"Invalid/truncated raw DSF/DFF; supported: mono/stereo DSD64-512");NxtDSD_close(d);return NULL;
}
void NxtDSD_close(NxtDSD*d){if(d){if(d->f)fclose(d->f);free(d->cache);free(d);}}
const NxtDSDInfo*NxtDSD_info(const NxtDSD*d){return d?&d->info:NULL;}
bool NxtDSD_probe(const char*p,NxtDSDInfo*i){NxtDSD*d=NxtDSD_open(p,NULL);if(!d)return false;if(i)*i=d->info;NxtDSD_close(d);return true;}
uint8_t*NxtDSD_cover(const char*p,size_t*n){*n=0;NxtDSD*d=NxtDSD_open(p,NULL);if(!d)return NULL;uint8_t*out=NULL;tags(d,&out,n);NxtDSD_close(d);return out;}
int NxtDSD_width(NxtDSDFormat f){static const int n[]={1,2,4,2,4,3,4,4};return f>=0&&f<8?n[f]:0;}
int NxtDSD_stride(NxtDSDFormat f){return f>=5&&f<8?2:NxtDSD_width(f);}
int NxtDSD_frameRate(const NxtDSDInfo*i,NxtDSDFormat f){int n=NxtDSD_stride(f);return i&&n?i->rate/(8*n):0;}
static bool cached(NxtDSD*d,uint64_t pos){uint64_t base=pos/d->block*d->block;if(d->valid&&base==d->cache_base)return true;size_t n=d->planar?d->block:d->bytes-base<d->block?d->bytes-base:d->block;if(!at(d,d->start+base*d->info.channels,d->cache,n*d->info.channels))return false;d->cache_base=base;d->cached=n;d->valid=true;return true;}
static bool byte_at(NxtDSD*d,uint64_t pos,int ch,uint8_t*out){if(!cached(d,pos))return false;size_t at_=pos-d->cache_base;if(at_>=d->cached)return false;uint8_t b=d->cache[d->planar?ch*d->block+at_:at_*d->info.channels+ch];*out=d->lsb?reverse(b):b;return true;}
int NxtDSD_read(NxtDSD*d,uint8_t*out,int frames,NxtDSDFormat f){
 int width=NxtDSD_width(f),stride=NxtDSD_stride(f);if(!d||!out||frames<0||frames>65536||!width)return -1;if(!frames||d->position>=d->bytes)return 0;uint64_t remain=d->bytes-d->position;int count=remain/stride<(unsigned)frames?remain/stride:frames;if(!count)return -1;
 for(int k=0;k<count;k++){for(int ch=0;ch<d->info.channels;ch++){uint8_t b[4];for(int j=0;j<stride;j++)if(!byte_at(d,d->position+j,ch,b+j))return -1;uint8_t*p=out+((size_t)k*d->info.channels+ch)*width;
  if(f<5){for(int j=0;j<width;j++)p[j]=b[(f==NXT_DSD_U16_LE||f==NXT_DSD_U32_LE)?width-1-j:j];}
  else if(f==NXT_DSD_DOP32_LE){p[0]=0;p[1]=b[1];p[2]=b[0];p[3]=d->marker;}
  else{p[0]=b[1];p[1]=b[0];p[2]=d->marker;if(width==4)p[3]=0;}
 }d->position+=stride;d->marker=d->marker==5?250:5;}return count;
}
int NxtDSD_seek(NxtDSD*d,int ms,NxtDSDFormat f){int stride=NxtDSD_stride(f);if(!d||ms<0||!stride)return -1;uint64_t p=(uint64_t)ms*d->info.rate/8000;if(p>d->bytes)p=d->bytes;p-=p%stride;d->position=p;d->valid=false;d->marker=5;clearerr(d->f);return p*8000/d->info.rate;}
