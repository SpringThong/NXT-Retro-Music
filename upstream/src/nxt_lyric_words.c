/* SPDX-License-Identifier: GPL-3.0-only
 * Bounded UTF-8 tokenisation shared by offline estimates and ASR alignment.
 * Estimates are explicitly labelled and never promoted to measured timing. */
#include "nxt_lyric_words.h"
#include <utf8proc.h>
#include <stdlib.h>
#include <string.h>
static bool separate(int c){return (c>=0x2e80&&c<=0x9fff)||(c>=0xac00&&c<=0xd7af)||(c>=0xe00&&c<=0xe7f)||(c>=0xf900&&c<=0xfaff);}
static void key(const char *s,int len,char *out){
 utf8proc_uint8_t *mapped=NULL;int n=utf8proc_map((const unsigned char*)s,len,&mapped,UTF8PROC_CASEFOLD|UTF8PROC_DECOMPOSE|UTF8PROC_STRIPMARK|UTF8PROC_STABLE);int at=0;
 if(n>0)for(int i=0;i<n;){utf8proc_int32_t cp;int k=utf8proc_iterate(mapped+i,n-i,&cp);if(k<1)break;utf8proc_category_t c=utf8proc_category(cp);if(cp==0x111)cp='d';if((c>=UTF8PROC_CATEGORY_LU&&c<=UTF8PROC_CATEGORY_LO)||(c>=UTF8PROC_CATEGORY_ND&&c<=UTF8PROC_CATEGORY_NO)){unsigned char b[4];int m=utf8proc_encode_char(cp,b);if(at+m>=128)break;memcpy(out+at,b,m);at+=m;}i+=k;}
 out[at]=0;free(mapped);
}
int NxtWords_tokens(const char *text,NxtWordToken *tokens,int cap){
 if(!text||!tokens||cap<1)return 0;int n=0,at=0,start=-1,weight=0,prev=0;utf8proc_int32_t state=0;
 while(1){utf8proc_int32_t cp=0;int bytes=text[at]?utf8proc_iterate((const unsigned char*)text+at,-1,&cp):0;if(bytes<0)return 0;
  utf8proc_category_t cat=bytes?utf8proc_category(cp):UTF8PROC_CATEGORY_ZS;
  bool letter=bytes&&((cat>=UTF8PROC_CATEGORY_LU&&cat<=UTF8PROC_CATEGORY_ME)||(cat>=UTF8PROC_CATEGORY_ND&&cat<=UTF8PROC_CATEGORY_NO));
  bool split=start>=0&&letter&&(separate(cp)||separate(prev))&&utf8proc_grapheme_break_stateful(prev,cp,&state);
  if(start>=0&&(!letter||split)){
   if(n>=cap)return n;NxtWordToken *t=&tokens[n++];memset(t,0,sizeof(*t));t->offset=start;t->length=at-start;t->weight=weight;key(text+start,t->length,t->key);start=-1;weight=0;
  }
  if(!bytes)break;
  if(letter){if(start<0){if(n==cap-1){NxtWordToken*t=&tokens[n++];memset(t,0,sizeof(*t));t->offset=at;t->length=strlen(text+at);t->weight=t->length;key(text+at,t->length,t->key);return n;}start=at;}if(cat<UTF8PROC_CATEGORY_MN||cat>UTF8PROC_CATEGORY_ME)weight++;}
  prev=cp;at+=bytes;
 }
 return n;
}
int NxtWords_estimate(LyricLine *line,int end){
 if(!line||line->word_count)return line?line->word_count:0;NxtWordToken t[LYRICS_MAX_WORDS];int n=NxtWords_tokens(line->text,t,LYRICS_MAX_WORDS);if(!n)return 0;
 int start=line->time_ms;if(end<=start)return 0;if(end-start>15000)end=start+15000;if(end-start<n)return 0;int total=0;for(int i=0;i<n;i++)total+=t[i].weight>0?t[i].weight:1;int used=0;
 for(int i=0;i<n;i++){int a=start+(long long)(end-start-n)*used/total+i;used+=t[i].weight>0?t[i].weight:1;int b=start+(long long)(end-start-n)*used/total+i+1;line->words[i]=(LyricWord){a,b,t[i].offset,t[i].length};}
 line->word_count=n;line->estimated=1;return n;
}
