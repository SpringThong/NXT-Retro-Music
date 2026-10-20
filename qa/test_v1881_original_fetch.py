from pathlib import Path
import subprocess,tempfile,json,hashlib,os
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform']]
with tempfile.TemporaryDirectory(prefix='nxt-original-fetch-') as td:
 w=Path(td);code=r'''
#include "nxt.h"
#include "api.h"
#undef SDCARD_PATH
#define SDCARD_PATH "__ROOT__"
#include "__SOURCE__"
#include <assert.h>
#include <unistd.h>
NxtConfig nxt;static int mode,calls;
const char *Nxt_text(const char*a,const char*b){return a;}
void PLAT_pinToCores(int n){}
int radio_net_fetch(const char *url,uint8_t *out,int maximum,char *type,int count){
 calls++;const char *value=NULL;
 if(mode==0)value="{\"plainLyrics\":\"A gentle first sentence.\\nA quiet second sentence.\"}";
 else if(mode==1)value="{\"syncedLyrics\":\"[00:01.500]Exact first.\\n[00:03.000]Exact second.\"}";
 else if(mode==2){if(strstr(url,"/get?"))return -1;if(strstr(url,"q=Cover"))value="[]";else value="[{\"trackName\":\"Wrong title\",\"artistName\":\"Original\",\"duration\":137,\"plainLyrics\":\"WRONG\"},{\"trackName\":\"Song\",\"artistName\":\"Original\",\"duration\":140,\"syncedLyrics\":\"[00:01]WRONG TIMING\",\"plainLyrics\":\"Matching plain first.\\nMatching plain second.\"}]";}
 else if(mode==3)value="[]";else if(mode==4)return -1;
 else{Lyrics_clear();value="{\"syncedLyrics\":\"[00:01]STALE\"}";}
 int n=strlen(value);assert(n<maximum);memcpy(out,value,n);return n;
}
static void run(const char*title,const char*artist){
 Lyrics_clear();int gen=++fetch_generation;original_state=LS_FETCHING;FetchArgs *a=calloc(1,sizeof(*a));assert(a);snprintf(a->title,sizeof(a->title),"%s",title);snprintf(a->artist,sizeof(a->artist),"%s",artist);a->duration_sec=137;a->generation=gen;fetch_thread_func(a);
}
int main(void){Lyrics_init();
 mode=0;run("Plain","Artist");assert(Lyrics_lineCount()==2&&!Lyrics_isSynced()&&!strcmp(Lyrics_lineText(0),"A gentle first sentence."));
 mode=1;run("Exact","Artist");assert(Lyrics_isSynced()&&Lyrics_currentIndex(1499)==-1&&Lyrics_currentIndex(1500)==0&&Lyrics_currentIndex(3000)==1);
 mode=2;calls=0;run("Song(R&B cover)","Cover");assert(calls==3&&Lyrics_lineCount()==2&&!Lyrics_isSynced()&&!strcmp(Lyrics_lineText(0),"Matching plain first."));
 mode=3;run("Missing","Artist");assert(!Lyrics_lineCount()&&original_state==LS_NOT_FOUND&&strstr(Lyrics_status(),"No original lyrics"));
 mode=4;run("Network","Artist");assert(!Lyrics_lineCount()&&original_state==LS_NETWORK);
 mode=5;run("Cancelled","Artist");assert(!Lyrics_lineCount()&&original_state==LS_NONE);
 mode=3;Lyrics_fetch("Retry Artist","Retry Song",137,"",NULL);for(int i=0;i<500&&original_state==LS_FETCHING;i++)usleep(1000);assert(original_state==LS_NOT_FOUND);
 mode=0;Lyrics_retry();for(int i=0;i<500&&original_state==LS_FETCHING;i++)usleep(1000);assert(Lyrics_lineCount()==2&&original_state==LS_READY);
 Lyrics_cleanup();puts("PASS production original-lyrics fetch: exact plain/synced, conservative cover fallback, no borrowed cover timings, unrelated result rejected, missing vs network status, stale response discarded and retry.");}
'''.replace('__ROOT__',str(w)).replace('__SOURCE__',str(S/'lyrics.c'))
 c=w/'test.c';c.write_text(code);b=w/'test';cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-Werror=implicit-function-declaration','-Werror=int-conversion','-fsanitize=address,undefined',*incs,str(c),str(S/'lyric_window.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v1881-original-fetch-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 result=subprocess.run([str(b)],capture_output=True,text=True,timeout=20,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'});(R/'logs/v1881-original-fetch-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr);result.check_returncode()
 (R/'logs/v1881-original-fetch-test.json').write_text(json.dumps({'exit':0,'actual_production_lyrics_module':True,'provider_replies_explicit_fixture':True,'plain_fallback':True,'cover_text_without_wrong_timing':True,'no_original_vs_network_status':True,'retry':True,'stale_no_publish':True,'ASan_UBSan':True,'live_provider_tested':False,'source_sha256':hashlib.sha256((S/'lyrics.c').read_bytes()).hexdigest()},indent=2)+'\n')
