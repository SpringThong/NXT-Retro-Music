/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_ime.h"
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>
#include <sqlite3.h>
static bool put(char *out,size_t cap,const char *s){size_t n=strlen(out),k=strlen(s);if(n+k>=cap)return false;memcpy(out+n,s,k+1);return true;}
static bool cp(char *out,size_t cap,unsigned c){char b[5];int n;if(c<128){b[0]=c;n=1;}else if(c<2048){b[0]=0xc0|(c>>6);b[1]=0x80|(c&63);n=2;}else{b[0]=0xe0|(c>>12);b[1]=0x80|((c>>6)&63);b[2]=0x80|(c&63);n=3;}b[n]=0;return put(out,cap,b);}
typedef struct {const char *key,*kana;} Roman;
static const Roman roman[]={
 {"kya","きゃ"},{"kyu","きゅ"},{"kyo","きょ"},{"sha","しゃ"},{"shu","しゅ"},{"sho","しょ"},{"cha","ちゃ"},{"chu","ちゅ"},{"cho","ちょ"},{"nya","にゃ"},{"nyu","にゅ"},{"nyo","にょ"},{"hya","ひゃ"},{"hyu","ひゅ"},{"hyo","ひょ"},{"mya","みゃ"},{"myu","みゅ"},{"myo","みょ"},{"rya","りゃ"},{"ryu","りゅ"},{"ryo","りょ"},{"gya","ぎゃ"},{"gyu","ぎゅ"},{"gyo","ぎょ"},{"bya","びゃ"},{"byu","びゅ"},{"byo","びょ"},{"pya","ぴゃ"},{"pyu","ぴゅ"},{"pyo","ぴょ"},{"shi","し"},{"chi","ち"},{"tsu","つ"},
 {"ka","か"},{"ki","き"},{"ku","く"},{"ke","け"},{"ko","こ"},{"sa","さ"},{"si","し"},{"su","す"},{"se","せ"},{"so","そ"},{"ta","た"},{"ti","ち"},{"tu","つ"},{"te","て"},{"to","と"},{"na","な"},{"ni","に"},{"nu","ぬ"},{"ne","ね"},{"no","の"},{"ha","は"},{"hi","ひ"},{"fu","ふ"},{"hu","ふ"},{"he","へ"},{"ho","ほ"},{"ma","ま"},{"mi","み"},{"mu","む"},{"me","め"},{"mo","も"},{"ya","や"},{"yu","ゆ"},{"yo","よ"},{"ra","ら"},{"ri","り"},{"ru","る"},{"re","れ"},{"ro","ろ"},{"wa","わ"},{"wo","を"},{"ga","が"},{"gi","ぎ"},{"gu","ぐ"},{"ge","げ"},{"go","ご"},{"za","ざ"},{"zi","じ"},{"ji","じ"},{"zu","ず"},{"ze","ぜ"},{"zo","ぞ"},{"da","だ"},{"di","ぢ"},{"du","づ"},{"de","で"},{"do","ど"},{"ba","ば"},{"bi","び"},{"bu","ぶ"},{"be","べ"},{"bo","ぼ"},{"pa","ぱ"},{"pi","ぴ"},{"pu","ぷ"},{"pe","ぺ"},{"po","ぽ"},{"ja","じゃ"},{"ju","じゅ"},{"jo","じょ"},{"xa","ぁ"},{"xi","ぃ"},{"xu","ぅ"},{"xe","ぇ"},{"xo","ぉ"},{"a","あ"},{"i","い"},{"u","う"},{"e","え"},{"o","お"}};
static bool kana(const char *raw,char *out,size_t cap){
 for(size_t i=0;raw[i];){unsigned char c=tolower((unsigned char)raw[i]),next=tolower((unsigned char)raw[i+1]);bool found=false;
  if(c==next&&isalpha(c)&&!strchr("aeioun",c)){if(!put(out,cap,"っ"))return false;i++;continue;}
  if(c=='n'&&next=='n'&&!raw[i+2]){if(!put(out,cap,"ん"))return false;i+=2;continue;}
  for(size_t k=0;k<sizeof(roman)/sizeof(*roman);k++){size_t n=strlen(roman[k].key);if(!strncasecmp(raw+i,roman[k].key,n)){if(!put(out,cap,roman[k].kana))return false;i+=n;found=true;break;}}
  if(found)continue;if(c=='n'&&(!next||!strchr("aeiouy",next))){if(!put(out,cap,"ん"))return false;i++;continue;}
  char b[2]={raw[i++],0};if(!put(out,cap,b))return false;
 }return true;
}
static const unsigned initials[]={0x3131,0x3132,0x3134,0x3137,0x3138,0x3139,0x3141,0x3142,0x3143,0x3145,0x3146,0x3147,0x3148,0x3149,0x314a,0x314b,0x314c,0x314d,0x314e};
static const int tail_l[]={-1,0,1,-1,2,-1,-1,3,5,-1,-1,-1,-1,-1,-1,-1,6,7,-1,9,10,11,12,14,15,16,17,18};
static int vowel_join(int a,int b){if(a==8){if(b==0)return 9;if(b==1)return 10;if(b==20)return 11;}if(a==13){if(b==4)return 14;if(b==5)return 15;if(b==20)return 16;}if(a==18&&b==20)return 19;if(a==9&&b==20)return 10;if(a==14&&b==20)return 15;return -1;}
static int tail_join(int a,int b){if(a==1&&b==19)return 3;if(a==4&&b==22)return 5;if(a==4&&b==27)return 6;if(a==8){int vals[]={1,16,17,19,25,26,27};for(int i=0;i<7;i++)if(b==vals[i])return 9+i;}if(a==17&&b==19)return 18;return 0;}
static bool ko_flush(char *out,size_t cap,int l,int v,int t){if(l<0)return true;return cp(out,cap,v<0?initials[l]:0xac00+(l*21+v)*28+t);}
static bool hangul(const char *raw,char *out,size_t cap){int l=-1,v=-1,t=0;for(size_t i=0;raw[i];i++){
 int nl=-1,nv=-1,nt=0;char c=raw[i];const char *cs="rRseEfaqQtTdwWczxvg",*vs="koiOjpuPhynbml";const int li[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18},vi[]={0,1,2,3,4,5,6,7,8,12,13,17,18,20};const char *p=strchr(cs,c);if(p)nl=li[p-cs];p=strchr(vs,c);if(p)nv=vi[p-vs];for(int k=1;k<28;k++)if(tail_l[k]==nl&&nl>=0){nt=k;break;}
 if(nv>=0){if(l<0){if(!cp(out,cap,0x314f+nv))return false;continue;}if(v<0){v=nv;continue;}if(t){int carry=tail_l[t],remain=0;if(carry<0){static const int first[]={0,0,0,1,0,4,4,0,0,8,8,8,8,8,8,8,0,0,17},last[]={0,0,0,9,0,12,18,0,0,0,6,7,9,16,17,18,0,0,9};remain=first[t];carry=last[t];}if(!ko_flush(out,cap,l,v,remain))return false;l=carry;v=nv;t=0;continue;}int joined=vowel_join(v,nv);if(joined>=0){v=joined;continue;}if(!ko_flush(out,cap,l,v,0)||!cp(out,cap,0x314f+nv))return false;l=v=-1;t=0;continue;}
 if(nl>=0){if(l<0){l=nl;continue;}if(v<0){if(!ko_flush(out,cap,l,v,0))return false;l=nl;continue;}if(!t&&nt){t=nt;continue;}int joined=tail_join(t,nt);if(joined){t=joined;continue;}if(!ko_flush(out,cap,l,v,t))return false;l=nl;v=-1;t=0;continue;}
 if(!ko_flush(out,cap,l,v,t))return false;l=v=-1;t=0;char b[2]={c,0};if(!put(out,cap,b))return false;
 }return ko_flush(out,cap,l,v,t);}
bool NxtIME_compose(int lang,const char *raw,char *out,size_t cap){if(!raw||!out||!cap)return false;out[0]=0;if(lang==4)return hangul(raw,out,cap);if(lang==5)return kana(raw,out,cap);return put(out,cap,raw);}
int NxtIME_candidates(int lang,const char *key,char values[32][128]){
 const char *code=lang==5?"ja":lang==7?"zh-Hans":lang==8?"zh-Hant":NULL;if(!code||!key||!*key)return 0;static sqlite3 *db;
 if(!db&&sqlite3_open_v2("./res/ime/candidates.sqlite3",&db,SQLITE_OPEN_READONLY,NULL)!=SQLITE_OK){if(db)sqlite3_close(db);db=NULL;return 0;}
 sqlite3_stmt *q=NULL;if(sqlite3_prepare_v2(db,"SELECT value FROM candidates WHERE lang=? AND key=? ORDER BY score DESC LIMIT 32",-1,&q,NULL)!=SQLITE_OK)return 0;sqlite3_bind_text(q,1,code,-1,SQLITE_STATIC);sqlite3_bind_text(q,2,key,-1,SQLITE_TRANSIENT);int n=0;while(n<32&&sqlite3_step(q)==SQLITE_ROW){const char *v=(const char*)sqlite3_column_text(q,0);if(v&&strlen(v)<128)snprintf(values[n++],128,"%s",v);}sqlite3_finalize(q);return n;
}
