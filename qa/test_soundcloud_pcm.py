#!/usr/bin/env python3
"""Production C PCM receiver: exact source rate, pause, seek epoch and EOF drain."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[1];q=r/'.cache/soundcloud-qa';q.mkdir(parents=True,exist_ok=True)
h=q/'include';h.mkdir(exist_ok=True)
if not (h/'samplerate.h').exists():(h/'samplerate.h').symlink_to(r/'.nxt-sdk/tg5040/local/include/samplerate.h')
c=q/'pcm.c';c.write_text(r'''
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <assert.h>
#include <time.h>
#include "nxt_soundcloud_pcm.inc"
static void packet(int fd,unsigned epoch,unsigned rate,float a,float b){
 unsigned char data[16+882*8];uint32_t count=1764,zero=0;memcpy(data,&epoch,4);memcpy(data+4,&count,4);memcpy(data+8,&rate,4);memcpy(data+12,&zero,4);
 for(int i=0;i<882;i++){memcpy(data+16+i*8,&a,4);memcpy(data+20+i*8,&b,4);}assert(send(fd,data,sizeof(data),MSG_NOSIGNAL)==sizeof(data));
}
int main(void){
 int sv[2];assert(!socketpair(AF_UNIX,SOCK_SEQPACKET,0,sv));assert(pcm_open(sv[0]));pcm_begin();float out[2048];int total=0;
 for(int i=0;i<10;i++){packet(sv[1],pcm_epoch,44100,.25,-.5);int got=NxtSoundCloud_audioSamples(out,1024,48000);assert(got>950&&got<970);total+=got;for(int k=4;k<got;k++){assert(fabsf(out[k*2]-.25)<.00001);assert(fabsf(out[k*2+1]+.5)<.00001);}}
 assert(total>=9598&&total<=9602);assert(pcm_source_rate==44100);assert(fabs(pcm_position-200)<.05);
 packet(sv[1],pcm_epoch,44100,.125,-.125);double pos=pcm_position;pcm_enable(false);assert(NxtSoundCloud_audioSamples(out,1024,48000)==0);assert(pcm_position==pos);pcm_enable(true);assert(NxtSoundCloud_audioSamples(out,1024,48000)>950);assert(fabsf(out[20]-.125)<.0001);
 unsigned old=pcm_epoch;pcm_begin();packet(sv[1],old,44100,.9,.9);packet(sv[1],pcm_epoch,48000,.2,-.2);int got=NxtSoundCloud_audioSamples(out,1024,44100);assert(got>790&&got<820);assert(pcm_source_rate==48000);assert(fabsf(out[20]-.2)<.0001);assert(pcm_position<20);
 pcm_begin();packet(sv[1],pcm_epoch,44100,NAN,INFINITY);got=NxtSoundCloud_audioSamples(out,1024,48000);assert(got>950);for(int i=0;i<got*2;i++)assert(out[i]==0);
 pcm_begin();packet(sv[1],pcm_epoch,44100,.1,-.1);uint32_t eof[4]={pcm_epoch,0,0,1};assert(send(sv[1],eof,sizeof(eof),0)==16);assert(!pcm_eof);got=NxtSoundCloud_audioSamples(out,200,48000);assert(got>190&&!pcm_eof);assert(NxtSoundCloud_audioSamples(out,1024,48000)>700);assert(pcm_eof);
 pcm_begin();unsigned char bad[8220]={0};uint32_t ns=2048,sr=44100;memcpy(bad,&pcm_epoch,4);memcpy(bad+4,&ns,4);memcpy(bad+8,&sr,4);assert(send(sv[1],bad,sizeof(bad),0)==sizeof(bad));assert(NxtSoundCloud_audioSamples(out,1024,48000)==0);
 struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);for(int i=0;i<10000;i++)assert(NxtSoundCloud_audioSamples(out,1024,48000)==0);clock_gettime(CLOCK_MONOTONIC,&b);assert(b.tv_sec-a.tv_sec<2);
 pcm_close();close(sv[1]);assert(NxtSoundCloud_audioSamples(out,1024,48000)==0);return 0;
}
''')
subprocess.run(['gcc','-O2','-I'+str(h),'-I'+str(r/'upstream/src'),str(c),'-l:libsamplerate.so.0','-lm','-lpthread','-o',str(q/'pcm')],check=True)
subprocess.run([str(q/'pcm')],check=True,timeout=5)
print('PASS SoundCloud native PCM: 44.1/48 kHz resampling and pitch, playback clock from consumed audio, pause preserves samples, stale seek epochs, NaN clamp, bounded nonblocking callbacks, EOF after drain, oversize packets, teardown.')
