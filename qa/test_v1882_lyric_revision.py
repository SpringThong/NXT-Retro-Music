from pathlib import Path
import hashlib,json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
code=r'''
#include "nxt.h"
#include "lyrics.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
NxtConfig nxt;
static void put(const char*p,const char*s){FILE*f=fopen(p,"w");assert(f);fputs(s,f);assert(!fclose(f));}
int main(int argc,char**argv){
 const char*p=argv[1];Lyrics_init();unsigned r=Lyrics_revision();assert(!Lyrics_lineCount());
 put(p,"[00:00.000]The morning light is warm.\n[00:01.500]Today is a peaceful day.\n");
 assert(Lyrics_loadTextFile(p,10000)&&Lyrics_isSynced()&&Lyrics_lineCount()==2);assert(Lyrics_revision()!=r);r=Lyrics_revision();
 assert(Lyrics_currentIndex(1499)==0&&Lyrics_currentIndex(1500)==1);nxt.lyric_offset=100;assert(Lyrics_currentIndex(1599)==0&&Lyrics_currentIndex(1600)==1);
 for(int i=0;i<1000;i++){assert(Lyrics_revision()==r);assert(Lyrics_lineText(0)&&Lyrics_isSynced());}nxt.lyric_offset=0;
 put(p,"[00:00.000]明天会更好。\n[00:01.500]今日は良い日です。\n");assert(Lyrics_loadTextFile(p,10000));assert(Lyrics_lineCount()==2&&Lyrics_revision()!=r);r=Lyrics_revision();
 assert(!strcmp(Lyrics_lineText(0),"明天会更好。")&&Lyrics_currentIndex(1500)==1);
 put(p,"A plain first sentence.\nA plain second sentence.\n");assert(Lyrics_loadTextFile(p,10000)&&!Lyrics_isSynced());assert(Lyrics_revision()!=r);r=Lyrics_revision();
 Lyrics_clear();assert(!Lyrics_lineCount()&&!Lyrics_lineText(0)&&Lyrics_revision()!=r);
 Lyrics_cleanup();puts("PASS actual original lyrics: stable revisions, same-count replacement invalidates, UTF-8 text kept, original timestamps/offsets and plain unsynced semantics preserved.");
}
'''
with tempfile.TemporaryDirectory(prefix='nxt-v1882-lyrics-') as td:
 w=Path(td);c=w/'test.c';c.write_text(code);b=w/'test'
 cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(c),str(S/'lyrics.c'),str(S/'lyric_window.c'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
 with (R/'logs/v1882-lyric-revision-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(b),str(w/'sample.lrc')],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True)
 (R/'logs/v1882-lyric-revision-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
 (R/'logs/v1882-lyric-revision-test.json').write_text(json.dumps({'exit':0,'actual_original_lyrics_parser_revision':True,'same_count_replacement':True,'timestamps_and_offset_preserved':True,'UTF8_unchanged':True,'plain_unsynced_preserved':True,'no_revision_churn_when_unchanged':True,'ASan_UBSan':True,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['lyrics.c','lyrics.h','lyric_window.c']}},indent=2)+'\n')

