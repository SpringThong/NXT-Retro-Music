#include "nxt.h"
#include "nxt_text.h"
#include "playlist_m3u.h"
#include "player.h"
#include <utf8proc.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

NxtConfig nxt;
static char root[PATH_MAX];
void NxtDSP_configure(const NxtConfig *config,int rate) { assert(config && rate==48000); }
void Player_refreshOptions(void) {}
int userdata_snpath(const char *relative,char *out,size_t size) { return snprintf(out,size,"%s/%s",root,relative); }
AudioFormat Player_detectFormat(const char *file) { const char *ext=strrchr(file,'.'); return ext && !strcmp(ext,".wav") ? AUDIO_FORMAT_WAV : AUDIO_FORMAT_UNKNOWN; }
static void utf8_valid(const char *s) { while(*s) { utf8proc_int32_t c; int n=utf8proc_iterate((const unsigned char*)s,-1,&c); assert(n>0); s+=n; } }
static void fixture(const char *path,const char *text) { FILE *f=fopen(path,"wb"); assert(f); assert(fputs(text,f)>=0); assert(!fclose(f)); }
int main(void) {
    char temporary[]="/tmp/nxt-persistence-XXXXXX"; assert(mkdtemp(temporary)); snprintf(root,sizeof(root),"%s",temporary);
    memset(&nxt,0,sizeof(nxt)); nxt.eq_bands=31; nxt.preamp=-4.5f; nxt.bass=2.5f; nxt.treble=-1; nxt.width=1; nxt.limiter=1;
    for(int i=0;i<31;i++)nxt.eq[i]=(i%5)-2;
    for(int i=0;i<6;i++){nxt.peq[i].hz=100*(i+1);nxt.peq[i].q=1;nxt.peq[i].gain=i-3;nxt.peq[i].enabled=i%2;}
    char saved[PATH_MAX]; snprintf(saved,sizeof(saved),"%s/test.nxt-eq",root); assert(Nxt_presetWrite(saved));
    nxt.preamp=0; memset(nxt.eq,0,sizeof(nxt.eq)); assert(Nxt_presetRead(saved)); assert(nxt.preamp==-4.5f && nxt.eq[30]==-2 && nxt.peq[5].gain==2);
    Nxt_profileSave(0); nxt.preamp=-9; Nxt_profileSave(2); Nxt_profileLoad(0); assert(nxt.preamp==-4.5f); Nxt_profileLoad(2); assert(nxt.preamp==-9);
    NxtConfig before=nxt; fixture(saved,"NXT-EQ-1\n31 nan 0 0 0 1 0 0 1\n"); assert(!Nxt_presetRead(saved)); assert(!memcmp(&before,&nxt,sizeof(nxt)));
    fixture(saved,"NXT-EQ-1\n31 0 0 0 0 1 0 0 1\n"); assert(!Nxt_presetRead(saved)); assert(!memcmp(&before,&nxt,sizeof(nxt)));
    fixture(saved,"NOT-A-PRESET\n"); assert(!Nxt_presetRead(saved));
    char a[PATH_MAX],b[PATH_MAX],list[PATH_MAX]; snprintf(a,sizeof(a),"%s/เพลงไทย.wav",root); snprintf(b,sizeof(b),"%s/밤.wav",root); snprintf(list,sizeof(list),"%s/import.m3u8",root); fixture(a,"");fixture(b,"");
    FILE *f=fopen(list,"wb");assert(f);fprintf(f,"\xef\xbb\xbf#EXTM3U\r\n#EXTINF:-1,");for(int i=0;i<100;i++)fputs("한국어",f);fprintf(f,"\r\n./เพลงไทย.wav\r\nmissing.wav\n#EXTINF:-1,日本語 · 漢字\n%s\n",b);assert(!fclose(f));
    PlaylistTrack tracks[3];int count=-1;assert(!M3U_loadTracks(list,tracks,3,&count) && count==2);assert(!strcmp(tracks[0].path,a) && !strcmp(tracks[1].path,b)); assert(strlen(tracks[0].name)<=255); utf8_valid(tracks[0].name); assert(!strcmp(tracks[1].name,"日本語 · 漢字"));
    assert(!M3U_loadTracks(list,tracks,1,&count) && count==1);assert(M3U_loadTracks(list,tracks,-1,&count)==-1);
    puts("PASS: ARM64 preset roundtrip; independent speaker/DAC profiles; malformed presets leave settings unchanged; UTF-8 BOM/relative M3U8 paths; long multilingual names remain valid.");
    return 0;
}
