/* Production enhanced-LRC parser and API checks; no service or audio fixture. */
#include "../upstream/src/lyrics.c"
#include <assert.h>
NxtConfig nxt;
static void check(const char *input,const char *expected,int words){
 LyricLine *lines=calloc(LYRICS_MAX_LINES,sizeof(*lines));bool synced=false;int n=parse_lyrics_text(input,180000,lines,LYRICS_MAX_LINES,&synced);assert(n>0);assert(!strcmp(lines[0].text,expected));assert(lines[0].word_count==words);free(lines);
}
int main(int argc,char **argv){assert(argc==2);
 check("[00:10.000]<00:10.000>Đường <00:10.650>về <00:11.100>nhà<00:12.000>","Đường về nhà",3);
 check("[00:10.01]Untimed words stay together","Untimed words stay together",0);
 check("Ordinary lyrics without timings","Ordinary lyrics without timings",0);
 check("[00:00.000]<00:00.500>한국어 <00:01.000>日本語 <00:02.000>中文<00:03.000>","한국어 日本語 中文",3);
 check("[00:00.000]<00:00.500>🇻🇳 <00:01.000>é<00:02.000>","🇻🇳 é",2);
 check("[00:10.000]<00:09.000>Invalid word start","Invalid word start",0);
 check("[00:10.000]<00:10.000>Not <00:09.500>monotonic","Not monotonic",0);
 LyricLine *lines=calloc(LYRICS_MAX_LINES,sizeof(*lines));bool sync;int n=parse_lyrics_text("[ar:Artist]\n[00:20.00]Second\n[00:10.5][00:30.500]<00:10.500>First <00:11.000>line<00:12.000>\n",100000,lines,512,&sync);assert(sync&&n==3);assert(lines[0].time_ms==10500&&lines[1].time_ms==20000&&lines[2].time_ms==30500);assert(lines[2].word_count==2&&lines[2].words[0].time_ms==30500&&lines[2].words[1].end_ms==32000);
 char path[2048];snprintf(path,sizeof(path),"%s/word.lrc",argv[1]);FILE*f=fopen(path,"w");assert(f);fputs("[00:10.000]<00:10.000>Đường <00:10.650>về <00:11.100>nhà<00:12.000>\n",f);fclose(f);assert(Lyrics_loadTextFile(path,18000));assert(Lyrics_currentIndex(9999)==-1&&Lyrics_currentIndex(10000)==0);assert(Lyrics_wordCount(0)==3&&Lyrics_wordCount(-1)==0&&Lyrics_wordAt(0,3)==NULL);assert(Lyrics_wordMotion(0,11000)&&!Lyrics_wordMotion(0,12000));nxt.lyric_offset=500;assert(!Lyrics_wordMotion(0,10200)&&Lyrics_wordMotion(0,10500));assert(Lyrics_wordAt(0,2)->offset==strlen("Đường về "));unsigned rev=Lyrics_revision();LyricLine patch=lyrics_lines[0];assert(Lyrics_lineTime(0)==10000);assert(Lyrics_applyWordTimings(rev,&patch,1,true));assert(!Lyrics_applyWordTimings(rev,&patch,1,true));rev=Lyrics_revision();patch.words[0].length=2048;assert(!Lyrics_applyWordTimings(rev,&patch,1,true));assert(Lyrics_revision()==rev);Lyrics_clear();assert(Lyrics_wordCount(0)==0);
 nxt.lyric_offset=0;nxt.karaoke_estimate=1;f=fopen(path,"w");assert(f);fputs("[00:01.000]Đường về nhà\n[00:05.000]你好世界\n",f);fclose(f);assert(Lyrics_loadTextFile(path,10000)&&Lyrics_isSynced()&&Lyrics_canFollow());assert(Lyrics_wordEstimated(0)&&Lyrics_wordCount(0)==3);assert(Lyrics_wordAt(0,2)->end_ms==5000);LyricLine actual;assert(Lyrics_copyLine(0,&actual)&&actual.word_count==0);assert(Lyrics_wordCount(1)==4&&Lyrics_wordAt(1,3)->end_ms==10000);nxt.karaoke_estimate=0;assert(!Lyrics_wordCount(0));nxt.karaoke_estimate=1;f=fopen(path,"w");fputs("Hello world\nAnother line",f);fclose(f);assert(Lyrics_loadTextFile(path,10000)&&!Lyrics_isSynced()&&Lyrics_canFollow()&&Lyrics_wordEstimated(0));nxt.karaoke_estimate=0;assert(!Lyrics_canFollow());Lyrics_clear();
 char many[8192]="[00:00.000]";for(int i=0;i<100;i++){char part[80];snprintf(part,sizeof(part),"<00:%02d.%03d>字 ",i/2,(i%2)*500);strcat(many,part);}n=parse_lyrics_text(many,100000,lines,512,&sync);assert(n==1&&lines[0].word_count==0&&strlen(lines[0].text)<sizeof(lines[0].text));
 for(unsigned seed=1;seed<=800;seed++){char input[4096];unsigned x=seed;for(int j=0;j<4095;j++){x=x*1664525u+1013904223u;input[j]="0123456789[]<>:.abc \n"[(x>>16)%21];}input[4095]=0;n=parse_lyrics_text(input,10000,lines,512,&sync);assert(n>=0&&n<=512);for(int j=0;j<n;j++){assert(strlen(lines[j].text)<2048&&lines[j].word_count<=64);}}
 free(lines);puts("PASS production Enhanced LRC real word timestamps, UTF-8, repeated tags, sorting, offset, interval bounds and plain-line fallback.");
}
