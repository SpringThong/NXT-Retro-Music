/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_telex.h"
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/* Each row: unaccented, acute, grave, hook, tilde, dot below. */
static const uint32_t vowels[12][6] = {
    {'a',0xe1,0xe0,0x1ea3,0xe3,0x1ea1},
    {0x103,0x1eaf,0x1eb1,0x1eb3,0x1eb5,0x1eb7},
    {0xe2,0x1ea5,0x1ea7,0x1ea9,0x1eab,0x1ead},
    {'e',0xe9,0xe8,0x1ebb,0x1ebd,0x1eb9},
    {0xea,0x1ebf,0x1ec1,0x1ec3,0x1ec5,0x1ec7},
    {'i',0xed,0xec,0x1ec9,0x129,0x1ecb},
    {'o',0xf3,0xf2,0x1ecf,0xf5,0x1ecd},
    {0xf4,0x1ed1,0x1ed3,0x1ed5,0x1ed7,0x1ed9},
    {0x1a1,0x1edb,0x1edd,0x1edf,0x1ee1,0x1ee3},
    {'u',0xfa,0xf9,0x1ee7,0x169,0x1ee5},
    {0x1b0,0x1ee9,0x1eeb,0x1eed,0x1eef,0x1ef1},
    {'y',0xfd,0x1ef3,0x1ef7,0x1ef9,0x1ef5},
};
static const uint32_t capitals[12][6] = {
    {'A',0xc1,0xc0,0x1ea2,0xc3,0x1ea0},
    {0x102,0x1eae,0x1eb0,0x1eb2,0x1eb4,0x1eb6},
    {0xc2,0x1ea4,0x1ea6,0x1ea8,0x1eaa,0x1eac},
    {'E',0xc9,0xc8,0x1eba,0x1ebc,0x1eb8},
    {0xca,0x1ebe,0x1ec0,0x1ec2,0x1ec4,0x1ec6},
    {'I',0xcd,0xcc,0x1ec8,0x128,0x1eca},
    {'O',0xd3,0xd2,0x1ece,0xd5,0x1ecc},
    {0xd4,0x1ed0,0x1ed2,0x1ed4,0x1ed6,0x1ed8},
    {0x1a0,0x1eda,0x1edc,0x1ede,0x1ee0,0x1ee2},
    {'U',0xda,0xd9,0x1ee6,0x168,0x1ee4},
    {0x1af,0x1ee8,0x1eea,0x1eec,0x1eee,0x1ef0},
    {'Y',0xdd,0x1ef2,0x1ef6,0x1ef8,0x1ef4},
};
static uint32_t read_cp(const char **p) {
    const unsigned char *s=(const unsigned char *)*p;
    uint32_t cp=*s++; int n=0;
    if(cp>=0xf0){cp&=7;n=3;}else if(cp>=0xe0){cp&=15;n=2;}else if(cp>=0xc0){cp&=31;n=1;}
    while(n-- && *s)cp=(cp<<6)|(*s++&63);
    *p=(const char *)s; return cp;
}
static int vowel(uint32_t cp,bool *upper,int *tone) {
    for(int r=0;r<12;r++)for(int t=0;t<6;t++) {
        if(cp==vowels[r][t]){*upper=false;*tone=t;return r;}
        if(cp==capitals[r][t]){*upper=true;*tone=t;return r;}
    }
    return -1;
}
static uint32_t shaped(uint32_t cp,int row) {
    bool upper=false;int t=0;vowel(cp,&upper,&t);
    return upper?capitals[row][0]:vowels[row][0];
}
static int basic(int row) {
    static const int base[]={0,0,0,3,3,5,6,6,6,9,9,11};
    return base[row];
}
static bool put(char *out,size_t capacity,size_t *used,uint32_t cp) {
    unsigned char bytes[4];int n;
    if(cp<0x80){n=1;bytes[0]=cp;}
    else if(cp<0x800){n=2;bytes[0]=0xc0|(cp>>6);bytes[1]=0x80|(cp&63);}
    else if(cp<0x10000){n=3;bytes[0]=0xe0|(cp>>12);bytes[1]=0x80|((cp>>6)&63);bytes[2]=0x80|(cp&63);}
    else{n=4;bytes[0]=0xf0|(cp>>18);bytes[1]=0x80|((cp>>12)&63);bytes[2]=0x80|((cp>>6)&63);bytes[3]=0x80|(cp&63);}
    if(*used+n>=capacity)return false;
    memcpy(out+*used,bytes,n);*used+=n;out[*used]=0;return true;
}
static bool word(const char *start,const char *end,char *out,size_t capacity,size_t *used) {
    uint32_t letters[NXT_TELEX_CAPACITY];int count=0,tone=0;
    const char *p=start;
    while(p<end) {
        uint32_t cp=read_cp(&p);int key=cp<128?tolower((unsigned char)cp):0;
        const char *tones="sfrxj";const char *t=key?strchr(tones,key):NULL;
        int last_v=-1;
        for(int i=count-1;i>=0;i--){bool u;int x;if(vowel(letters[i],&u,&x)>=0){last_v=i;break;}}
        if(t&&last_v>=0) {
            int next=(int)(t-tones)+1;
            if(tone!=next){tone=next;continue;}
            tone=0; /* ss/ff/rr/xx/jj escape a literal final key. */
        } else if(key=='z'&&count) {
            bool changed=tone!=0;tone=0;
            for(int i=0;i<count;i++) {
                bool upper;int x,r=vowel(letters[i],&upper,&x);
                if(r>=0&&(r!=basic(r)||x)){letters[i]=shaped(letters[i],basic(r));changed=true;}
                if(letters[i]==0x111||letters[i]==0x110){letters[i]=letters[i]==0x110?'D':'d';changed=true;}
            }
            if(changed)continue;
        } else if(count && key=='d' && (letters[count-1]=='d'||letters[count-1]=='D')) {
            letters[count-1]=letters[count-1]=='D'?0x110:0x111;continue;
        } else if(count && key=='d' && (letters[count-1]==0x111||letters[count-1]==0x110)) {
            letters[count-1]=letters[count-1]==0x110?'D':'d';
        } else if(count && (key=='a'||key=='e'||key=='o')) {
            bool upper;int x,r=vowel(letters[count-1],&upper,&x);
            int base=key=='a'?0:key=='e'?3:6, target=key=='a'?2:key=='e'?4:7;
            if(r==base){letters[count-1]=shaped(letters[count-1],target);continue;}
            if(r==target)letters[count-1]=shaped(letters[count-1],base);
        } else if(key=='w'&&last_v>=0) {
            bool upper;int x,r=vowel(letters[last_v],&upper,&x);
            if((r==6||r==8)&&last_v>0) {
                bool u;int v,prev=vowel(letters[last_v-1],&u,&v);
                if(prev==9||prev==10) {
                    bool undo=r==8&&prev==10;
                    letters[last_v-1]=shaped(letters[last_v-1],undo?9:10);
                    letters[last_v]=shaped(letters[last_v],undo?6:8);
                    if(!undo)continue;
                } else if(r==6){letters[last_v]=shaped(letters[last_v],8);continue;}
                else letters[last_v]=shaped(letters[last_v],6);
            } else if(r==0||r==6||r==9) {
                letters[last_v]=shaped(letters[last_v],r==0?1:r==6?8:10);continue;
            } else if(r==1||r==8||r==10)letters[last_v]=shaped(letters[last_v],basic(r));
        }
        letters[count++]=cp;
    }
    int indices[NXT_TELEX_CAPACITY],n=0,target=-1;
    for(int i=0;i<count;i++) {
        bool u;int t,r=vowel(letters[i],&u,&t);
        if(r<0)continue;
        /* qu/gi are consonant onsets when followed by another vowel. */
        bool onset=(i==1&&(letters[0]=='q'||letters[0]=='Q')&&basic(r)==9)||
                   (i==1&&(letters[0]=='g'||letters[0]=='G')&&r==5);
        if(onset&&i+1<count){bool u2;int t2;if(vowel(letters[i+1],&u2,&t2)>=0)continue;}
        indices[n++]=i;
        if(r==4||r==8)target=i; /* ê/ơ take the mark in iê/uô/ươ families. */
    }
    if(target<0&&n) {
        if(n>=3)target=indices[n-2];
        else if(n==2)target=indices[(indices[1]<count-1)?1:0];
        else target=indices[0];
        /* A shaped final vowel is the nucleus (tiê, tuô, quyê). */
        bool u;int t,r=vowel(letters[indices[n-1]],&u,&t);
        if(r>=0&&r!=basic(r))target=indices[n-1];
    }
    for(int i=0;i<count;i++) {
        if(i==target&&tone){bool u=false;int t=0,r=vowel(letters[i],&u,&t);if(r>=0)letters[i]=u?capitals[r][tone]:vowels[r][tone];}
        if(!put(out,capacity,used,letters[i]))return false;
    }
    return true;
}
static bool render(const NxtTelex *state,char *out,size_t capacity) {
    if(!capacity)return false;
    size_t used=state->literal_bytes;
    if(used>=capacity)return false;
    memcpy(out,state->raw,used);out[used]=0;
    const char *p=state->raw+used;
    while(*p) {
        const char *start=p;
        while(*p&&((unsigned char)*p>=128||isalpha((unsigned char)*p)))p++;
        if(p>start&&!word(start,p,out,capacity,&used))return false;
        if(*p){uint32_t cp=read_cp(&p);if(!put(out,capacity,&used,cp))return false;}
    }
    return true;
}
void NxtTelex_reset(NxtTelex *state,const char *text) {
    size_t n=strlen(text?text:"");if(n>=sizeof(state->raw))n=sizeof(state->raw)-1;
    memcpy(state->raw,text?text:"",n);state->raw[n]=0;state->literal_bytes=n;
}
bool NxtTelex_append(NxtTelex *state,char *out,size_t capacity,const char *input) {
    size_t n=strlen(state->raw),m=strlen(input);
    if(n+m>=sizeof(state->raw)||capacity>NXT_TELEX_CAPACITY)return false;
    char candidate[NXT_TELEX_CAPACITY];memcpy(state->raw+n,input,m+1);
    if(!render(state,candidate,capacity)){state->raw[n]=0;return false;}
    memcpy(out,candidate,strlen(candidate)+1);return m!=0;
}
bool NxtTelex_backspace(NxtTelex *state,char *out,size_t capacity) {
    size_t n=strlen(state->raw);if(!n)return false;
    do{n--;}while(n&&((unsigned char)state->raw[n]&0xc0)==0x80);
    state->raw[n]=0;if(state->literal_bytes>n)state->literal_bytes=n;
    return render(state,out,capacity);
}
