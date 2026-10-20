#!/usr/bin/env python3
from pathlib import Path
import subprocess,json,os,tempfile,time
R=Path(__file__).resolve().parents[1];S=R/'upstream/src'
with tempfile.TemporaryDirectory() as td:
 p=Path(td);c=p/'test.c';c.write_text(r'''#include "nxt_ime.h"
#include "keyboard_map.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static bool supported(void*a,const char*b){return true;}
static void check(int l,const char*k,const char*w){char b[4096];assert(NxtIME_compose(l,k,b,sizeof(b)));if(strcmp(b,w)){fprintf(stderr,"%s: %s != %s\n",k,b,w);assert(0);}}
int main(void){
check(4,"gksrmf","한글");check(4,"gkrry","학교");check(4,"rhoscksgdk","괜찮아");check(4,"rkTek","갔다");check(4,"rkqtdl","값이");check(4,"dksl","아니");check(5,"nihon","にほん");check(5,"konnichiha","こんにちは");check(5,"gakkou","がっこう");check(5,"shinbun","しんぶん");check(5,"nn","ん");
for(int l=0;l<14;l++){char b[2]={12,34};assert(!NxtIME_compose(l,"abcdef",b,sizeof(b)));}
char c[32][128];int n=NxtIME_candidates(7,"nihao",c);assert(n>0);int found=0;for(int i=0;i<n;i++)if(!strcmp(c[i],"你好"))found=1;assert(found);n=NxtIME_candidates(8,"diannao",c);assert(n>0);found=0;for(int i=0;i<n;i++)if(!strcmp(c[i],"電腦"))found=1;assert(found);n=NxtIME_candidates(5,"にほん",c);assert(n>0);found=0;for(int i=0;i<n;i++)if(!strcmp(c[i],"日本"))found=1;assert(found);
assert(KeyboardMap_prepare(supported,NULL));const Keyboard*k=KeyboardMap_get();assert(k->layout_count==15);
for(int l=0;l<15;l++)for(int r=0;r<k->geometry->rows;r++)for(int col=0;k->geometry->keys[r][col];col++){const Key*key=k->geometry->keys[r][col];int n=KeyboardMap_charCount(k->layouts[l],key);for(int i=0;i<n;i++){assert(KeyboardMap_char(k->layouts[l],key,false,i));assert(KeyboardMap_char(k->layouts[l],key,true,i));}}
KeyboardMap_quit();puts("PASS Hangul compounds/finals, Japanese kana and kanji, Simplified/Traditional Pinyin, bounded output and 15 keyboard maps.");return 0;}
''')
 includes=['-I'+str(S),'-I'+str(R/'.host-tools/usr/include')]
 exe=p/'test';subprocess.run(['gcc','-no-pie','-O1','-g','-fsanitize=address,undefined',*includes,str(c),str(S/'nxt_ime.c'),str(S/'keyboard_map.c'),str(S/'utf8.c'),'-l:libsqlite3.so.0','-o',str(exe)],check=True)
 start=time.monotonic();r=subprocess.run([str(exe)],cwd=R/'upstream',capture_output=True,text=True,timeout=30,env={**os.environ,"ASAN_OPTIONS":"detect_leaks=0:halt_on_error=1"});print(r.stdout+r.stderr);r.check_returncode()
 (R/'logs/v181-ime-test.json').write_text(json.dumps({'exit':0,'ASan_UBSan':True,'Hangul':True,'Romaji_kana_kanji':True,'Pinyin_simplified_traditional':True,'keyboards':15,'elapsed_host_seconds':time.monotonic()-start},indent=2))
