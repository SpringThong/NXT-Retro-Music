/* SPDX-License-Identifier: GPL-3.0-only
 * Private bounded PCM receiver. The audio callback only pops memory; socket
 * polling, validation and producer backpressure belong to this worker. */
#ifndef NXT_ONLINE_PCM_QUEUE_H
#define NXT_ONLINE_PCM_QUEUE_H
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#define NXT_PCM_QUEUE_SLOTS 64
#define NXT_PCM_QUEUE_PRIME 16
#define NXT_PCM_PACKET_MAX 8208
typedef struct { unsigned char bytes[NXT_PCM_PACKET_MAX]; size_t length; } NxtPCMPacket;
typedef struct {
    pthread_mutex_t lock; pthread_cond_t wake; pthread_t thread;
    int fd, header, read, write, count;
    bool running, buffering, finished, idle_after_finish;
    uint32_t epoch;
    NxtPCMPacket packets[NXT_PCM_QUEUE_SLOTS];
} NxtPCMQueue;
static uint32_t NxtPCMQueue_u32(const unsigned char *p) {
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}
static void *NxtPCMQueue_receive(void *opaque) {
    NxtPCMQueue *q=opaque; unsigned char packet[NXT_PCM_PACKET_MAX];
    for (;;) {
        pthread_mutex_lock(&q->lock);
        while(q->running&&q->count==NXT_PCM_QUEUE_SLOTS) pthread_cond_wait(&q->wake,&q->lock);
        bool running=q->running; pthread_mutex_unlock(&q->lock);
        if(!running) break;
        struct pollfd p={.fd=q->fd,.events=POLLIN}; int ready=poll(&p,1,100);
        if(ready<0&&errno==EINTR) continue;
        if(ready==0) {
            pthread_mutex_lock(&q->lock);
            if(q->finished) q->idle_after_finish=true;
            pthread_mutex_unlock(&q->lock); continue;
        }
        if(ready<0) break;
        ssize_t bytes=recv(q->fd,packet,sizeof(packet),MSG_DONTWAIT|MSG_TRUNC);
        if(bytes<0&&(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK)) continue;
        if(bytes<=0) break;
        if(bytes<q->header||bytes>NXT_PCM_PACKET_MAX) continue;
        uint32_t epoch=NxtPCMQueue_u32(packet),samples=NxtPCMQueue_u32(packet+4);
        bool eof=q->header==16&&bytes==16&&samples==0&&NxtPCMQueue_u32(packet+12)==1;
        if(!eof&&(samples<2||samples>2048||(samples&1)||bytes!=q->header+(ssize_t)samples*4)) continue;
        pthread_mutex_lock(&q->lock);
        if(q->running&&epoch==q->epoch) {
            NxtPCMPacket *out=&q->packets[q->write];
            memcpy(out->bytes,packet,(size_t)bytes); out->length=(size_t)bytes;
            q->write=(q->write+1)%NXT_PCM_QUEUE_SLOTS; q->count++;
            q->idle_after_finish=false; if(eof) q->finished=true;
        }
        pthread_mutex_unlock(&q->lock);
    }
    pthread_mutex_lock(&q->lock); q->finished=true; q->idle_after_finish=true;
    pthread_mutex_unlock(&q->lock); return NULL;
}
static NxtPCMQueue *NxtPCMQueue_open(int fd,int header) {
    if(header!=8&&header!=16) return NULL;
    NxtPCMQueue *q=calloc(1,sizeof(*q)); if(!q) return NULL;
    q->fd=fd;q->header=header;q->running=q->buffering=true;
    pthread_mutex_init(&q->lock,NULL);pthread_cond_init(&q->wake,NULL);
    if(pthread_create(&q->thread,NULL,NxtPCMQueue_receive,q)) {
        pthread_cond_destroy(&q->wake);pthread_mutex_destroy(&q->lock);free(q);return NULL;
    }
    return q;
}
static void NxtPCMQueue_reset(NxtPCMQueue *q,uint32_t epoch) {
    if(!q) return; pthread_mutex_lock(&q->lock);
    q->epoch=epoch;q->read=q->write=q->count=0;q->buffering=true;
    q->finished=q->idle_after_finish=false;
    pthread_cond_broadcast(&q->wake);pthread_mutex_unlock(&q->lock);
}
static void NxtPCMQueue_finish(NxtPCMQueue *q) {
    if(!q) return;pthread_mutex_lock(&q->lock);
    q->finished=true;q->idle_after_finish=false;pthread_mutex_unlock(&q->lock);
}
static bool NxtPCMQueue_drained(NxtPCMQueue *q) {
    if(!q) return true;pthread_mutex_lock(&q->lock);
    bool empty=q->finished&&q->idle_after_finish&&!q->count;
    pthread_mutex_unlock(&q->lock);return empty;
}
static ssize_t NxtPCMQueue_pop(NxtPCMQueue *q,void *out,size_t capacity) {
    if(!q) return 0;pthread_mutex_lock(&q->lock);
    if(q->buffering&&q->count<NXT_PCM_QUEUE_PRIME&&!q->finished) {
        pthread_mutex_unlock(&q->lock);return 0;
    }
    if(!q->count) {
        q->buffering=true;pthread_mutex_unlock(&q->lock);return 0;
    }
    q->buffering=false;NxtPCMPacket *p=&q->packets[q->read];
    size_t n=p->length;if(n>capacity){pthread_mutex_unlock(&q->lock);return -1;}
    memcpy(out,p->bytes,n);q->read=(q->read+1)%NXT_PCM_QUEUE_SLOTS;q->count--;
    pthread_cond_signal(&q->wake);pthread_mutex_unlock(&q->lock);return (ssize_t)n;
}
static void NxtPCMQueue_close(NxtPCMQueue *q) {
    if(!q) return;pthread_mutex_lock(&q->lock);q->running=false;
    pthread_cond_broadcast(&q->wake);pthread_mutex_unlock(&q->lock);
    shutdown(q->fd,SHUT_RD);pthread_join(q->thread,NULL);
    pthread_cond_destroy(&q->wake);pthread_mutex_destroy(&q->lock);free(q);
}
#endif
