#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "nxt_online_pcm_queue.h"
#include "nxt_online_ramp.h"
static void packet(int fd,uint32_t epoch,float sample){
    unsigned char p[16]={0};uint32_t ns=2;
    memcpy(p,&epoch,4);memcpy(p+4,&ns,4);memcpy(p+8,&sample,4);memcpy(p+12,&sample,4);
    assert(send(fd,p,sizeof(p),0)==sizeof(p));
}
static void wait_count(NxtPCMQueue *q,int count){
    for(int i=0;i<500;i++){pthread_mutex_lock(&q->lock);bool ready=q->count==count;pthread_mutex_unlock(&q->lock);if(ready)return;usleep(1000);}assert(!"receiver did not reach expected bounded count");
}
int main(void){
    int fd[2];assert(!socketpair(AF_UNIX,SOCK_SEQPACKET,0,fd));
    NxtPCMQueue *q=NxtPCMQueue_open(fd[0],8);assert(q);NxtPCMQueue_reset(q,7);
    packet(fd[1],6,.9f);packet(fd[1],7,.25f);wait_count(q,1);
    unsigned char out[8208];assert(NxtPCMQueue_pop(q,out,sizeof(out))==0);
    for(int i=1;i<NXT_PCM_QUEUE_PRIME;i++)packet(fd[1],7,.25f);
    wait_count(q,NXT_PCM_QUEUE_PRIME);
    for(int i=0;i<NXT_PCM_QUEUE_PRIME;i++){assert(NxtPCMQueue_pop(q,out,sizeof(out))==16);float value;memcpy(&value,out+8,4);assert(value==.25f);}
    assert(NxtPCMQueue_pop(q,out,sizeof(out))==0);
    packet(fd[1],7,.5f);wait_count(q,1);assert(NxtPCMQueue_pop(q,out,sizeof(out))==0);
    NxtPCMQueue_finish(q);assert(NxtPCMQueue_pop(q,out,sizeof(out))==16);
    for(int i=0;i<500&&!NxtPCMQueue_drained(q);i++)usleep(1000);
    assert(NxtPCMQueue_drained(q));
    NxtPCMQueue_reset(q,8);for(int i=0;i<NXT_PCM_QUEUE_SLOTS;i++)packet(fd[1],8,.75f);
    wait_count(q,NXT_PCM_QUEUE_SLOTS);NxtPCMQueue_reset(q,9);
    packet(fd[1],8,.9f);packet(fd[1],9,.1f);wait_count(q,1);NxtPCMQueue_finish(q);
    assert(NxtPCMQueue_pop(q,out,sizeof(out))==16);float value;memcpy(&value,out+8,4);assert(value==.1f);
    NxtPCMQueue_close(q);close(fd[0]);close(fd[1]);
    assert(!socketpair(AF_UNIX,SOCK_SEQPACKET,0,fd));q=NxtPCMQueue_open(fd[0],16);assert(q);NxtPCMQueue_reset(q,3);
    unsigned char sc[24]={0};uint32_t epoch=3,ns=2,sr=48000,flag=0;float sample=.25f;
    memcpy(sc,&epoch,4);memcpy(sc+4,&ns,4);memcpy(sc+8,&sr,4);memcpy(sc+12,&flag,4);memcpy(sc+16,&sample,4);memcpy(sc+20,&sample,4);
    assert(send(fd[1],sc,sizeof(sc),0)==sizeof(sc));wait_count(q,1);assert(!NxtPCMQueue_pop(q,out,sizeof(out)));
    ns=0;flag=1;memcpy(sc+4,&ns,4);memcpy(sc+12,&flag,4);assert(send(fd[1],sc,16,0)==16);wait_count(q,2);
    assert(NxtPCMQueue_pop(q,out,sizeof(out))==24);assert(NxtPCMQueue_pop(q,out,sizeof(out))==16);
    NxtPCMQueue_close(q);close(fd[0]);close(fd[1]);
    NxtOnlineRamp ramp={0};float pcm[1024];for(int i=0;i<1024;i++)pcm[i]=1;
    NxtOnlineRamp_process(&ramp,pcm,512,256,48000);
    assert(pcm[0]>0&&pcm[0]<.01f);assert(pcm[510]==1);assert(pcm[512]<1&&pcm[512]>.99f);assert(pcm[1022]==0);
    for(int i=0;i<1024;i++)pcm[i]=1;
    NxtOnlineRamp_process(&ramp,pcm,512,512,48000);assert(pcm[0]>0&&pcm[0]<.01f);assert(pcm[1022]==1);
    puts("PASS bounded online PCM queue: prime, jitter refill, stale epoch, tail drain, full-queue reset, close, anti-click ramps");
}
