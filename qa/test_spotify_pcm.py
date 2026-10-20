#!/usr/bin/env python3
"""Run production PCM receiver/resampler with real seqpacket sockets."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[1];q=r/'.cache/spotify-qa';q.mkdir(parents=True,exist_ok=True)
sdk=r/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include';h=q/'include';h.mkdir(exist_ok=True)
if (h/'samplerate.h').is_symlink():(h/'samplerate.h').unlink()
(h/'samplerate.h').symlink_to(r/'.nxt-sdk/tg5040/local/include/samplerate.h')
c=q/'pcm.c';c.write_text(r'''
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <assert.h>
#include <time.h>
#include "nxt_spotify_pcm.inc"
static void packet(int fd,unsigned epoch,float a,float b){
 unsigned char data[8+882*8];uint32_t count=1764;memcpy(data,&epoch,4);memcpy(data+4,&count,4);
 for(int i=0;i<882;i++){memcpy(data+8+i*8,&a,4);memcpy(data+12+i*8,&b,4);}assert(send(fd,data,sizeof(data),MSG_NOSIGNAL)==sizeof(data));
}
int main(void){
 int sv[2];assert(!socketpair(AF_UNIX,SOCK_SEQPACKET,0,sv));assert(pcm_open(sv[0]));pcm_begin();float out[2048];int total=0;
 for(int k=0;k<10;k++){packet(sv[1],pcm_epoch,.25f,-.5f);int got=NxtSpotify_audioSamples(out,1024,48000);assert(got>950&&got<970);total+=got;for(int j=4;j<got;j++){assert(fabsf(out[j*2]-.25f)<.0001);assert(fabsf(out[j*2+1]+.5f)<.0001);}}
 fprintf(stderr,"Resampled frames=%d\n",total);assert(total>=9598&&total<=9602);assert(NxtSpotify_audioSamples(out,1024,48000)==0);
 pcm_enable(false);packet(sv[1],pcm_epoch,.25f,-.5f);assert(NxtSpotify_audioSamples(out,1024,48000)==0);
 unsigned old=pcm_epoch;pcm_begin();packet(sv[1],old,.9f,.9f);packet(sv[1],pcm_epoch,.125f,-.125f);int got=NxtSpotify_audioSamples(out,1024,48000);assert(got>950);assert(fabsf(out[8]-.125f)<.0001);
 pcm_begin();packet(sv[1],pcm_epoch,NAN,INFINITY);got=NxtSpotify_audioSamples(out,1024,44100);assert(got>870);for(int i=0;i<got*2;i++)assert(out[i]==0);
 struct timespec a,b;clock_gettime(CLOCK_MONOTONIC,&a);for(int i=0;i<10000;i++)assert(NxtSpotify_audioSamples(out,1024,48000)==0);clock_gettime(CLOCK_MONOTONIC,&b);assert(b.tv_sec-a.tv_sec<2);
 pcm_close();close(sv[1]);assert(NxtSpotify_audioSamples(out,1024,48000)==0);return 0;
}
''')
subprocess.run(['gcc','-O2','-I'+str(h),'-I'+str(r/'upstream/src'),str(c),'-l:libsamplerate.so.0','-lm','-lpthread','-o',str(q/'pcm')],check=True)
subprocess.run([str(q/'pcm')],check=True,timeout=5)
print('PASS production PCM: stereo levels, 44.1→48 kHz duration/pitch, bounded nonblocking reads, pause drain, stale epoch rejection, invalid samples, clean shutdown.')
