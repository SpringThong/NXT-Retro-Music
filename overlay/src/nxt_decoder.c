/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#include "nxt_decoder.h"
#include "nxt_dsd.h"
#include "nxt_text.h"
#include "nxt_drive.h"
#include "nxt_stream.h"
#include <libavformat/avio.h>
#include <libavutil/mem.h>
#include <errno.h>
#include <limits.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
struct NxtDecoder { AVFormatContext *f; AVCodecContext *c; AVPacket *packet; AVFrame *frame; SwrContext *swr; int stream,rate,drain,eof,used,count; int64_t samples,seek_target; float *pcm; int capacity; bool native; TrackInfo info; };
static const char *tag(AVFormatContext *f,int s,const char *key) { AVDictionaryEntry *e=av_dict_get(f->streams[s]->metadata,key,NULL,0); if(!e) e=av_dict_get(f->metadata,key,NULL,0); return e?e->value:""; }
static void metadata(AVFormatContext *f,int s,const char *path,TrackInfo *i) {
    AVStream *st=f->streams[s]; AVCodecParameters *p=st->codecpar; memset(i,0,sizeof(*i));
    const char *base=strrchr(path,'/'); base=base?base+1:path;
    NxtText_copy(i->title,*tag(f,s,"title")?tag(f,s,"title"):base,sizeof(i->title));
    NxtText_copy(i->artist,tag(f,s,"artist"),sizeof(i->artist)); NxtText_copy(i->album,tag(f,s,"album"),sizeof(i->album));
    snprintf(i->codec,sizeof(i->codec),"%s",avcodec_get_name(p->codec_id));
    i->sample_rate=p->sample_rate; i->channels=p->ch_layout.nb_channels;
    i->bit_depth=p->bits_per_raw_sample?p->bits_per_raw_sample:av_get_bits_per_sample(p->codec_id);
    i->bitrate=p->bit_rate?p->bit_rate:f->bit_rate; i->track_number=atoi(tag(f,s,"track"));
    if(st->duration!=AV_NOPTS_VALUE) i->duration_ms=av_rescale_q(st->duration,st->time_base,(AVRational){1,1000});
    else if(f->duration!=AV_NOPTS_VALUE) i->duration_ms=f->duration/1000;
    const char *g=tag(f,s,"replaygain_track_gain"); i->replay_track=*g?strtof(g,NULL):0;
    g=tag(f,s,"replaygain_album_gain");bool has_album=*g!=0; i->replay_album=*g?strtof(g,NULL):i->replay_track;
    if(!isfinite(i->replay_track)) i->replay_track=0; if(!isfinite(i->replay_album)) i->replay_album=0;
    const char *r128=tag(f,s,"R128_TRACK_GAIN"); if(*r128&&!i->replay_track) i->replay_track=strtol(r128,NULL,10)/256.0f+5;
    if(!has_album)i->replay_album=i->replay_track;
    r128=tag(f,s,"R128_ALBUM_GAIN"); if(*r128&&!i->replay_album) i->replay_album=strtol(r128,NULL,10)/256.0f+5;
    NxtDrive_metadata(path,i);
}
static int stream_read(void *opaque,uint8_t *out,int size){
    int n=NxtStream_read(opaque,out,size);return n==0?AVERROR_EOF:n;
}
static int64_t stream_seek(void *opaque,int64_t offset,int whence){
    NxtStream *stream=opaque;
    if(whence&AVSEEK_SIZE)return NxtStream_size(stream);
    whence&=~AVSEEK_FORCE;int64_t base=whence==SEEK_SET?0:whence==SEEK_CUR?NxtStream_position(stream):whence==SEEK_END?NxtStream_size(stream):-1;
    if(base<0||(offset>0&&base>INT64_MAX-offset)||(offset<0&&base<INT64_MIN-offset))return AVERROR(EINVAL);
    return NxtStream_seek(stream,base+offset);
}
static void close_io(AVIOContext *io){
    if(!io)return;NxtStream *stream=io->opaque;av_freep(&io->buffer);avio_context_free(&io);NxtStream_close(stream);
}
static void close_format(AVFormatContext **f){
    AVIOContext *io=*f&&((*f)->flags&AVFMT_FLAG_CUSTOM_IO)?(*f)->pb:NULL;
    avformat_close_input(f);close_io(io);
}
static AVFormatContext *open_format_epoch(const char *path,int *s,unsigned epoch){
    AVFormatContext *f=NULL;AVIOContext *io=NULL;AVDictionary *options=NULL;
    av_dict_set(&options,"protocol_whitelist","file",0);
    if(NxtDrive_isStream(path)){
        NxtDriveEntry entry;if(!NxtDrive_streamEntry(path,&entry))goto fail;
        NxtStream *stream=NxtStream_open(&entry,epoch);if(!stream)goto fail;
        uint8_t *buffer=av_malloc(32768);if(!buffer){NxtStream_close(stream);goto fail;}
        io=avio_alloc_context(buffer,32768,0,stream,stream_read,NULL,stream_seek);
        if(!io){av_free(buffer);NxtStream_close(stream);goto fail;}
        io->seekable=NxtStream_seekable(stream)?AVIO_SEEKABLE_NORMAL:0;
        f=avformat_alloc_context();if(!f)goto fail;
        f->pb=io;f->flags|=AVFMT_FLAG_CUSTOM_IO;
        av_dict_set(&options,"probesize","262144",0);av_dict_set(&options,"analyzeduration","1000000",0);
    }
    if(avformat_open_input(&f,io?NULL:path,NULL,&options)<0)goto fail;
    av_dict_free(&options);
    if(avformat_find_stream_info(f,NULL)<0||(*s=av_find_best_stream(f,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0))<0){close_format(&f);return NULL;}
    return f;
fail:
    av_dict_free(&options);if(f)avformat_close_input(&f);close_io(io);return NULL;
}
static AVFormatContext *open_format(const char *path,int *s){return open_format_epoch(path,s,NxtStream_epoch());}
void NxtDecoder_cancelStreams(void){NxtStream_cancelAll();}

NxtDecoder *NxtDecoder_openEpoch(const char *path,int rate,unsigned epoch) {
    NxtDecoder *d=calloc(1,sizeof(*d)); if(!d) return NULL; d->rate=rate; d->native=rate==0;d->seek_target=-1;
    d->f=open_format_epoch(path,&d->stream,epoch); if(!d->f) goto fail;
    const AVCodec *codec=avcodec_find_decoder(d->f->streams[d->stream]->codecpar->codec_id); if(!codec) goto fail;
    d->c=avcodec_alloc_context3(codec); if(!d->c||avcodec_parameters_to_context(d->c,d->f->streams[d->stream]->codecpar)<0||avcodec_open2(d->c,codec,NULL)<0) goto fail;
    metadata(d->f,d->stream,path,&d->info);
    if(d->native){
        enum AVCodecID id=d->c->codec_id;
        bool lossless=id==AV_CODEC_ID_FLAC||id==AV_CODEC_ID_ALAC||id==AV_CODEC_ID_APE||id==AV_CODEC_ID_WAVPACK||id==AV_CODEC_ID_TTA||id==AV_CODEC_ID_TAK||id==AV_CODEC_ID_PCM_S8||id==AV_CODEC_ID_PCM_U8||id==AV_CODEC_ID_PCM_S16LE||id==AV_CODEC_ID_PCM_S16BE||id==AV_CODEC_ID_PCM_S24LE||id==AV_CODEC_ID_PCM_S24BE||id==AV_CODEC_ID_PCM_S32LE||id==AV_CODEC_ID_PCM_S32BE;
        if(!lossless||d->info.channels<1||d->info.channels>2||d->info.bit_depth<1||d->info.bit_depth>32||d->c->sample_rate<=0)goto fail;
        d->rate=d->c->sample_rate;
    }else{
        AVChannelLayout stereo=AV_CHANNEL_LAYOUT_STEREO;
        if(swr_alloc_set_opts2(&d->swr,&stereo,AV_SAMPLE_FMT_FLT,rate,&d->c->ch_layout,d->c->sample_fmt,d->c->sample_rate,0,NULL)<0||swr_init(d->swr)<0) goto fail;
    }
    d->packet=av_packet_alloc(); d->frame=av_frame_alloc(); d->capacity=131072; d->pcm=malloc(d->capacity*2*sizeof(float));
    if(!d->packet||!d->frame||!d->pcm) goto fail;
    return d;
fail: NxtDecoder_close(d); return NULL;
}
NxtDecoder *NxtDecoder_open(const char *path,int rate){return NxtDecoder_openEpoch(path,rate,NxtStream_epoch());}
void NxtDecoder_close(NxtDecoder *d) { if(!d)return; free(d->pcm); swr_free(&d->swr); av_packet_free(&d->packet); av_frame_free(&d->frame); avcodec_free_context(&d->c); close_format(&d->f); free(d); }
static int fill(NxtDecoder *d) {
    for(;;) {
        int r=avcodec_receive_frame(d->c,d->frame);
        if(r==0) {
            uint8_t *out=(uint8_t*)d->pcm;
            AVStream *st=d->f->streams[d->stream];
            int64_t pts=d->frame->best_effort_timestamp,delay=d->swr?swr_get_delay(d->swr,d->c->sample_rate):0;
            int64_t start=pts==AV_NOPTS_VALUE?d->seek_target:av_rescale_q(pts-(st->start_time==AV_NOPTS_VALUE?0:st->start_time),st->time_base,(AVRational){1,d->rate})-av_rescale(delay,d->rate,d->c->sample_rate);
            int n;
            if(d->native){
                AVFrame *f=d->frame;int channels=d->info.channels;enum AVSampleFormat fmt=f->format,packed=av_get_packed_sample_fmt(fmt);bool planar=av_sample_fmt_is_planar(fmt);
                if(f->nb_samples>d->capacity||f->sample_rate!=d->rate||f->ch_layout.nb_channels!=channels||(packed!=AV_SAMPLE_FMT_U8&&packed!=AV_SAMPLE_FMT_S16&&packed!=AV_SAMPLE_FMT_S32)){av_frame_unref(f);d->eof=1;return -1;}
                n=f->nb_samples;int32_t *native=(int32_t*)d->pcm;
                for(int i=0;i<n;i++)for(int c=0;c<channels;c++){
                    const uint8_t *p=f->extended_data[planar?c:0];int k=planar?i:i*channels+c;int32_t value;
                    if(packed==AV_SAMPLE_FMT_U8)value=((int)p[k]-128)*16777216;
                    else if(packed==AV_SAMPLE_FMT_S16){int16_t v;memcpy(&v,p+k*2,2);value=(int32_t)v*65536;}
                    else memcpy(&value,p+k*4,4);
                    native[i*channels+c]=value;
                }
            }else n=swr_convert(d->swr,&out,d->capacity,(const uint8_t**)d->frame->extended_data,d->frame->nb_samples);
            av_frame_unref(d->frame);
            if(n<=0)continue;
            if(d->seek_target>=0) {
                int64_t skip=d->seek_target-start;if(skip<0)skip=0;
                if(skip>=n)continue;
                if(skip){int ch=d->native?d->info.channels:2;memmove(d->pcm,d->pcm+skip*ch,(n-skip)*ch*sizeof(float));n-=skip;}
                d->seek_target=-1;
            }
            return n;
        }
        if(r==AVERROR_EOF) { uint8_t *out=(uint8_t*)d->pcm; int n=d->swr?swr_convert(d->swr,&out,d->capacity,NULL,0):0; d->eof=n<=0; return n>0?n:0; }
        if(r!=AVERROR(EAGAIN)) { d->eof=1; return 0; }
        if(d->drain) { d->eof=1; return 0; }
        for(;;) {
            r=av_read_frame(d->f,d->packet); if(r<0) { avcodec_send_packet(d->c,NULL); d->drain=1; break; }
            if(d->packet->stream_index==d->stream) { r=avcodec_send_packet(d->c,d->packet); av_packet_unref(d->packet); if(r<0) { d->eof=1; return 0; } break; } av_packet_unref(d->packet);
        }
    }
}
int NxtDecoder_read(NxtDecoder *d,float *pcm,int frames) {
    if(!d||d->native)return -1;
    int n=0; while(n<frames&&!d->eof) { if(d->used>=d->count) { d->count=fill(d); d->used=0; if(!d->count) break; } int k=d->count-d->used; if(k>frames-n) k=frames-n; memcpy(pcm+n*2,d->pcm+d->used*2,k*2*sizeof(float)); d->used+=k; n+=k; }
    d->samples+=n; return n;
}
int NxtDecoder_readNative(NxtDecoder *d,int32_t *pcm,int frames){
    if(!d||!d->native||!pcm||frames<0)return -1;
    int n=0,ch=d->info.channels;
    while(n<frames&&!d->eof){
        if(d->used>=d->count){d->count=fill(d);d->used=0;if(d->count<0)return -1;if(!d->count)break;}
        int k=d->count-d->used;if(k>frames-n)k=frames-n;
        memcpy(pcm+n*ch,(int32_t*)d->pcm+d->used*ch,k*ch*sizeof(int32_t));d->used+=k;n+=k;
    }
    d->samples+=n;return n;
}
bool NxtDecoder_canSeek(const NxtDecoder *d){return d&&(!(d->f->flags&AVFMT_FLAG_CUSTOM_IO)||(d->f->pb->seekable&AVIO_SEEKABLE_NORMAL));}
bool NxtDecoder_seekEpoch(NxtDecoder *d,int ms,unsigned epoch) {
    if(!NxtDecoder_canSeek(d))return false;
    AVStream *st=d->f->streams[d->stream];
    if(d->f->flags&AVFMT_FLAG_CUSTOM_IO){NxtStream_rearm(d->f->pb->opaque,epoch);d->f->pb->error=0;d->f->pb->eof_reached=0;}
    int64_t t=av_rescale_q(ms,(AVRational){1,1000},st->time_base)+(st->start_time==AV_NOPTS_VALUE?0:st->start_time);
    if(avformat_seek_file(d->f,d->stream,INT64_MIN,t,t,0)<0) return false;
    avcodec_flush_buffers(d->c);if(d->swr){swr_close(d->swr);swr_init(d->swr);}d->drain=d->eof=d->used=d->count=0;
    d->samples=av_rescale(ms,d->rate,1000); d->seek_target=d->samples; return true;
}
bool NxtDecoder_seek(NxtDecoder *d,int ms){return NxtDecoder_seekEpoch(d,ms,NxtStream_epoch());}
const TrackInfo *NxtDecoder_info(NxtDecoder *d) { return &d->info; }
int NxtDecoder_position(NxtDecoder *d) { return av_rescale(d->samples,1000,d->rate); }
bool NxtDecoder_probe(const char *path,TrackInfo *info) {
 if(NxtDSD_isPath(path)){NxtDSDInfo d;if(!NxtDSD_probe(path,&d))return false;memset(info,0,sizeof(*info));NxtText_copy(info->title,d.title,sizeof(info->title));NxtText_copy(info->artist,d.artist,sizeof(info->artist));NxtText_copy(info->album,d.album,sizeof(info->album));snprintf(info->codec,sizeof(info->codec),"%s DSD%u",d.container,d.rate/(d.rate%44100?48000:44100));info->sample_rate=d.rate;info->bit_depth=1;info->channels=d.channels;info->bitrate=d.rate*d.channels;info->duration_ms=d.duration_ms;return true;}
 int s; AVFormatContext *f=open_format(path,&s); if(!f)return false; metadata(f,s,path,info); close_format(&f); return true; }
SDL_Surface *NxtDecoder_art(const char *path) {
 if(NxtDSD_isPath(path)){size_t n=0;uint8_t*b=NxtDSD_cover(path,&n);SDL_Surface*a=b?IMG_Load_RW(SDL_RWFromConstMem(b,(int)n),1):NULL;free(b);return a;}
 if(NxtDrive_isStream(path))return NULL;int s; AVFormatContext *f=open_format(path,&s); if(!f)return NULL; SDL_Surface *art=NULL; for(unsigned k=0;k<f->nb_streams;k++) if(f->streams[k]->disposition&AV_DISPOSITION_ATTACHED_PIC) { AVPacket *p=&f->streams[k]->attached_pic; if(p->size>0&&p->size<=8*1024*1024) art=IMG_Load_RW(SDL_RWFromConstMem(p->data,p->size),1); if(art) break; } close_format(&f); return art; }
char *NxtDecoder_lyrics(const char *path) {
 if(NxtDSD_isPath(path))return NULL; /* sidecar LRC still uses existing lyric pipeline */
 if(NxtDrive_isStream(path))return NULL;int s; AVFormatContext *f=open_format(path,&s); if(!f)return NULL; const char *t=tag(f,s,"lyrics"); if(!*t)t=tag(f,s,"unsyncedlyrics"); char *result=*t?strdup(t):NULL; close_format(&f); return result; }
