/* SPDX-License-Identifier: GPL-3.0-only
 * A bounded, path-coalescing disk worker. JSON fsync and PNG compression never
 * run in a normal UI tick. Jobs own paths/data/pixels, not profile globals. */
#include "nxt_write.h"
#include <SDL2/SDL_image.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define SLOTS 32
typedef struct {char path[1500];char *text;SDL_Surface *image;bool durable,remove;} Job;
static Job queue[SLOTS];static int count;static bool started,stopping,active,failed;
static pthread_t worker;static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wake=PTHREAD_COND_INITIALIZER,drained=PTHREAD_COND_INITIALIZER;
static void dispose(Job *j){free(j->text);if(j->image)SDL_FreeSurface(j->image);memset(j,0,sizeof(*j));}
static bool write_job(Job *j){if(j->remove)return !unlink(j->path)||errno==ENOENT;char temp[1520];snprintf(temp,sizeof(temp),"%s.nxt-writing",j->path);bool ok;
 if(j->image)ok=IMG_SavePNG(j->image,temp)==0;
 else {FILE *f=fopen(temp,"wb");ok=f&&fputs(j->text,f)>=0;if(f){if(fflush(f))ok=false;if(j->durable&&fsync(fileno(f)))ok=false;if(fclose(f))ok=false;}}
 if(ok)ok=!rename(temp,j->path);if(!ok)unlink(temp);return ok;
}
static void *run(void *unused){(void)unused;nice(5);pthread_mutex_lock(&lock);
 for(;;){while(!count&&!stopping)pthread_cond_wait(&wake,&lock);if(!count&&stopping)break;
  Job j=queue[0];memmove(queue,queue+1,(--count)*sizeof(Job));memset(&queue[count],0,sizeof(Job));active=true;pthread_mutex_unlock(&lock);
  bool ok=write_job(&j);if(!ok){usleep(20000);ok=write_job(&j);}dispose(&j);
  pthread_mutex_lock(&lock);if(!ok)failed=true;active=false;if(!count)pthread_cond_broadcast(&drained);
 }pthread_cond_broadcast(&drained);pthread_mutex_unlock(&lock);return NULL;
}
static bool submit(Job *j){pthread_mutex_lock(&lock);if(stopping){pthread_mutex_unlock(&lock);dispose(j);return false;}
 if(!started){if(pthread_create(&worker,NULL,run,NULL)){pthread_mutex_unlock(&lock);dispose(j);return false;}started=true;}
 for(int i=0;i<count;i++)if(!strcmp(queue[i].path,j->path)){dispose(&queue[i]);queue[i]=*j;pthread_cond_signal(&wake);pthread_mutex_unlock(&lock);return true;}
 if(count==SLOTS){pthread_mutex_unlock(&lock);dispose(j);return false;}queue[count++]=*j;pthread_cond_signal(&wake);pthread_mutex_unlock(&lock);return true;
}
bool NxtWrite_text(const char *p,const char *text,bool durable){if(!p||!*p||strlen(p)>=sizeof(queue[0].path)||!text)return false;Job j={.durable=durable};snprintf(j.path,sizeof(j.path),"%s",p);j.text=strdup(text);return j.text&&submit(&j);}
bool NxtWrite_image(const char *p,SDL_Surface *s){if(!p||!*p||strlen(p)>=sizeof(queue[0].path)||!s)return false;Job j={0};snprintf(j.path,sizeof(j.path),"%s",p);j.image=SDL_ConvertSurface(s,s->format,0);return j.image&&submit(&j);}
bool NxtWrite_remove(const char *p){if(!p||!*p||strlen(p)>=sizeof(queue[0].path))return false;Job j={.remove=true};snprintf(j.path,sizeof(j.path),"%s",p);return submit(&j);}
void NxtWrite_flush(void){pthread_mutex_lock(&lock);while(started&&(count||active))pthread_cond_wait(&drained,&lock);pthread_mutex_unlock(&lock);}
void NxtWrite_quit(void){pthread_mutex_lock(&lock);bool join=started;stopping=true;pthread_cond_signal(&wake);pthread_mutex_unlock(&lock);if(join)pthread_join(worker,NULL);pthread_mutex_lock(&lock);started=stopping=false;pthread_mutex_unlock(&lock);}
bool NxtWrite_failed(void){pthread_mutex_lock(&lock);bool v=failed;failed=false;pthread_mutex_unlock(&lock);return v;}
