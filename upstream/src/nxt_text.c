/* SPDX-License-Identifier: GPL-3.0-only
 * Offline font fallback with NFC, grapheme boundaries, HarfBuzz and FriBidi. */
#include "nxt_text.h"
#undef TTF_RenderUTF8_Blended
#undef TTF_SizeUTF8
#undef TTF_OpenFont
#undef TTF_CloseFont
#include <utf8proc.h>
#include <fribidi.h>
#include <hb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static const char *names[]={"Noto Sans","Inter","Roboto","Source Sans 3","Nunito Sans","Manrope","Rubik","Ubuntu","Lato","Montserrat","Noto Serif","Source Serif 4","Lora","Merriweather","JetBrains Mono","Fira Code","MiSans · Xiaomi / NX Redux","HarmonyOS Sans · Huawei","HarmonyOS Sans Condensed · Huawei","OPPO Sans 4.0 · OPPO","Roboto Condensed · Google","Roboto Slab · Google","Be Vietnam Pro","Plus Jakarta Sans","Work Sans","Mulish","Public Sans","IBM Plex Sans","Lexend","Onest","Geist","Wix Madefor Text","Readex Pro","Barlow","Asap","Archivo","Exo 2","Chakra Petch","Prompt","Kanit","Sarabun","Quicksand","Comfortaa","Space Grotesk","Cabin","Open Sans","Arimo","Josefin Sans","Bitter","Playfair Display","Literata","Vollkorn","Cormorant Garamond","Alegreya","Crimson Pro","Fraunces","Spectral","Gelasio","Epilogue","Newsreader"};
static const char *paths[]={"./res/fonts/notosans.ttf","./res/fonts/inter.ttf","./res/fonts/roboto.ttf","./res/fonts/sourcesans3.ttf","./res/fonts/nunitosans.ttf","./res/fonts/manrope.ttf","./res/fonts/rubik.ttf","./res/fonts/ubuntu.ttf","./res/fonts/lato.ttf","./res/fonts/montserrat.ttf","./res/fonts/notoserif.ttf","./res/fonts/sourceserif4.ttf","./res/fonts/lora.ttf","./res/fonts/merriweather.ttf","./res/fonts/jetbrainsmono.ttf","./res/fonts/firacode.ttf","./res/fonts/misans-semibold.ttf","./res/fonts/harmonyos-semibold.ttf","./res/fonts/harmonyos-condensed-semibold.ttf","./res/fonts/opposans4.ttf","./res/fonts/robotocondensed.ttf","./res/fonts/robotoslab.ttf","./res/fonts/catalog-bevietnampro.ttf","./res/fonts/catalog-plusjakartasans.ttf","./res/fonts/catalog-worksans.ttf","./res/fonts/catalog-mulish.ttf","./res/fonts/catalog-publicsans.ttf","./res/fonts/catalog-ibmplexsans.ttf","./res/fonts/catalog-lexend.ttf","./res/fonts/catalog-onest.ttf","./res/fonts/catalog-geist.ttf","./res/fonts/catalog-wixmadefortext.ttf","./res/fonts/catalog-readexpro.ttf","./res/fonts/catalog-barlow.ttf","./res/fonts/catalog-asap.ttf","./res/fonts/catalog-archivo.ttf","./res/fonts/catalog-exo2.ttf","./res/fonts/catalog-chakrapetch.ttf","./res/fonts/catalog-prompt.ttf","./res/fonts/catalog-kanit.ttf","./res/fonts/catalog-sarabun.ttf","./res/fonts/catalog-quicksand.ttf","./res/fonts/catalog-comfortaa.ttf","./res/fonts/catalog-spacegrotesk.ttf","./res/fonts/catalog-cabin.ttf","./res/fonts/catalog-opensans.ttf","./res/fonts/catalog-arimo.ttf","./res/fonts/catalog-josefinsans.ttf","./res/fonts/catalog-bitter.ttf","./res/fonts/catalog-playfairdisplay.ttf","./res/fonts/catalog-literata.ttf","./res/fonts/catalog-vollkorn.ttf","./res/fonts/catalog-cormorantgaramond.ttf","./res/fonts/catalog-alegreya.ttf","./res/fonts/catalog-crimsonpro.ttf","./res/fonts/catalog-fraunces.ttf","./res/fonts/catalog-spectral.ttf","./res/fonts/catalog-gelasio.ttf","./res/fonts/catalog-epilogue.ttf","./res/fonts/catalog-newsreader.ttf"};
const char *NxtText_fontName(int n){return names[n>=0&&n<NXT_FONT_COUNT?n:0];}
const char *NxtText_fontPath(int n){return paths[n>=0&&n<NXT_FONT_COUNT?n:0];}
const char *NxtText_italicPath(int n){static const char *italic[NXT_FONT_COUNT]={[17]="./res/fonts/harmonyos-semibold-italic.ttf",[18]="./res/fonts/harmonyos-condensed-semibold-italic.ttf",[20]="./res/fonts/robotocondensed-italic.ttf",[22]="./res/fonts/catalog-bevietnampro-italic.ttf",[23]="./res/fonts/catalog-plusjakartasans-italic.ttf",[24]="./res/fonts/catalog-worksans-italic.ttf",[25]="./res/fonts/catalog-mulish-italic.ttf",[26]="./res/fonts/catalog-publicsans-italic.ttf",[27]="./res/fonts/catalog-ibmplexsans-italic.ttf",[30]="./res/fonts/catalog-geist-italic.ttf",[33]="./res/fonts/catalog-barlow-italic.ttf",[34]="./res/fonts/catalog-asap-italic.ttf",[35]="./res/fonts/catalog-archivo-italic.ttf",[36]="./res/fonts/catalog-exo2-italic.ttf",[37]="./res/fonts/catalog-chakrapetch-italic.ttf",[38]="./res/fonts/catalog-prompt-italic.ttf",[39]="./res/fonts/catalog-kanit-italic.ttf",[40]="./res/fonts/catalog-sarabun-italic.ttf",[44]="./res/fonts/catalog-cabin-italic.ttf",[45]="./res/fonts/catalog-opensans-italic.ttf",[46]="./res/fonts/catalog-arimo-italic.ttf",[47]="./res/fonts/catalog-josefinsans-italic.ttf",[48]="./res/fonts/catalog-bitter-italic.ttf",[49]="./res/fonts/catalog-playfairdisplay-italic.ttf",[50]="./res/fonts/catalog-literata-italic.ttf",[51]="./res/fonts/catalog-vollkorn-italic.ttf",[52]="./res/fonts/catalog-cormorantgaramond-italic.ttf",[53]="./res/fonts/catalog-alegreya-italic.ttf",[54]="./res/fonts/catalog-crimsonpro-italic.ttf",[55]="./res/fonts/catalog-fraunces-italic.ttf",[56]="./res/fonts/catalog-spectral-italic.ttf",[57]="./res/fonts/catalog-gelasio-italic.ttf",[58]="./res/fonts/catalog-epilogue-italic.ttf",[59]="./res/fonts/catalog-newsreader-italic.ttf"};return n>=0&&n<NXT_FONT_COUNT?italic[n]:NULL;}
_Static_assert(sizeof(names)/sizeof(names[0])==NXT_FONT_COUNT,"font names/count mismatch");
_Static_assert(sizeof(paths)/sizeof(paths[0])==NXT_FONT_COUNT,"font paths/count mismatch");
typedef struct {TTF_Font *primary,*fallback[13];int pixels;} Face;
static Face faces[96];
static Face *record(TTF_Font *font){for(int i=0;i<96;i++)if(faces[i].primary==font)return &faces[i];return NULL;}
TTF_Font *NxtText_open(const char *path,int pixels){
    TTF_Font *font=TTF_OpenFont(path,pixels);
    if(!font)font=TTF_OpenFont(paths[0],pixels);
    if(!font)return NULL;
    for(int i=0;i<96;i++)if(!faces[i].primary){faces[i]=(Face){.primary=font,.pixels=pixels};return font;}
    TTF_CloseFont(font);SDL_SetError("Too many open font sizes");return NULL;
}
void NxtText_close(TTF_Font *font){if(!font)return;Face *face=record(font);if(face){for(int i=0;i<13;i++)if(face->fallback[i])TTF_CloseFont(face->fallback[i]);memset(face,0,sizeof(*face));}TTF_CloseFont(font);}
static TTF_Font *fallback(Face *face,int n){
    if(!face||n<0||n>=13)return NULL;
    if(!face->fallback[n]){
        static const char *files[]={"noto-cjk.ttc","noto-cjk.ttc","noto-cjk.ttc","noto-cjk.ttc","notosansthai.ttf","notosanslao.ttf","notosanskhmer.ttf","notosansdevanagari.ttf","notonaskharabic.ttf","notosanssymbols2.ttf","notosans.ttf","unifont.otf","unifont-upper.otf"};
        char path[256];snprintf(path,sizeof(path),"./res/fonts/%s",files[n]);
        face->fallback[n]=TTF_OpenFontIndex(path,face->pixels,n<4?n:0);
        if(face->fallback[n])TTF_SetFontStyle(face->fallback[n],TTF_GetFontStyle(face->primary));
    }
    return face->fallback[n];
}
static int preferred(hb_script_t script,int cjk){
    if(script==HB_SCRIPT_HAN)return cjk;
    if(script==HB_SCRIPT_HIRAGANA||script==HB_SCRIPT_KATAKANA)return 0;
    if(script==HB_SCRIPT_HANGUL)return 1;
    if(script==HB_SCRIPT_THAI)return 4;
    if(script==HB_SCRIPT_LAO)return 5;
    if(script==HB_SCRIPT_KHMER)return 6;
    if(script==HB_SCRIPT_DEVANAGARI)return 7;
    if(script==HB_SCRIPT_ARABIC)return 8;
    return 10;
}
static bool covered(TTF_Font *font,const FriBidiChar *cp,int begin,int end){
    if(!font)return false;
    for(int i=begin;i<end;i++){
        int cat=utf8proc_category(cp[i]);
        if(cp[i]==0x200c||cp[i]==0x200d||cp[i]==0xFE0F||cp[i]==0xFE0E||cat==UTF8PROC_CATEGORY_CF||cp[i]<32)continue;
        if(!TTF_GlyphIsProvided32(font,cp[i]))return false;
    }
    return true;
}
static TTF_Font *select_face(Face *face,TTF_Font *primary,const FriBidiChar *cp,int begin,int end,hb_script_t script,int cjk){
    if(covered(primary,cp,begin,end))return primary;
    int best=preferred(script,cjk);TTF_Font *font=fallback(face,best);
    if(covered(font,cp,begin,end))return font;
    static const int rest[]={10,9,2,0,1,3,4,5,6,7,8,11,12};
    for(unsigned i=0;i<sizeof(rest)/sizeof(*rest);i++){font=fallback(face,rest[i]);if(covered(font,cp,begin,end))return font;}
    return primary;
}
bool NxtText_hasGlyph(TTF_Font *font,Uint32 cp){Face *face=record(font);FriBidiChar c=cp;hb_script_t script=hb_unicode_script(hb_unicode_funcs_get_default(),cp);return covered(select_face(face,font,&c,0,1,script,2),&c,0,1);}
const char *NxtText_nextCluster(const char *text){
    if(!text||!*text)return text;
    utf8proc_int32_t previous,cp,state=0;int n=utf8proc_iterate((const utf8proc_uint8_t*)text,-1,&previous);if(n<1)return text+1;
    const char *p=text+n;
    while(*p){n=utf8proc_iterate((const utf8proc_uint8_t*)p,-1,&cp);if(n<1||utf8proc_grapheme_break_stateful(previous,cp,&state))break;previous=cp;p+=n;}
    return p;
}
void NxtText_copy(char *out,const char *input,size_t size){if(!size)return;const char *p=input?input:"";size_t used=0;while(*p){utf8proc_int32_t cp;int n=utf8proc_iterate((const utf8proc_uint8_t*)p,-1,&cp);if(n<1){p++;continue;}if(used+(size_t)n>=size)break;memcpy(out+used,p,n);used+=n;p+=n;}out[used]=0;}
char *NxtText_searchKey(const char *text){
    utf8proc_uint8_t *key=NULL;
    if(utf8proc_map((const utf8proc_uint8_t*)(text?text:""),0,&key,UTF8PROC_NULLTERM|UTF8PROC_STABLE|UTF8PROC_CASEFOLD|UTF8PROC_DECOMPOSE|UTF8PROC_STRIPMARK)<0)return strdup(text?text:"");
    /* Vietnamese đ has no canonical d decomposition. */
    char *p=(char*)key,*out=(char*)key;while(*p){if((unsigned char)p[0]==0xc4&&(unsigned char)p[1]==0x91){*out++='d';p+=2;}else *out++=*p++;}*out=0;return (char*)key;
}
typedef struct {TTF_Font *font;hb_script_t script;int start,end,visual,level,width,height;} Run;
static int visual_order(const void *a,const void *b){return ((const Run*)a)->visual-((const Run*)b)->visual;}
static SDL_Surface *layout(TTF_Font *font,const char *input,SDL_Color color,int *outw,int *outh,bool paint){
    if(outw)*outw=0;if(outh)*outh=0;if(!font)return NULL;
    utf8proc_uint8_t *normalized=NULL;
    if(utf8proc_map((const utf8proc_uint8_t*)(input?input:""),0,&normalized,UTF8PROC_NULLTERM|UTF8PROC_STABLE|UTF8PROC_COMPOSE)<0)return NULL;
    char *text=(char*)normalized;size_t bytes=strlen(text);if(bytes>65536){free(text);return NULL;}
    int cap=(int)bytes+1;
    FriBidiChar *cp=calloc(cap,sizeof(*cp));FriBidiCharType *types=calloc(cap,sizeof(*types));FriBidiStrIndex *visual=calloc(cap,sizeof(*visual));FriBidiLevel *levels=calloc(cap,sizeof(*levels));int *offset=calloc(cap+1,sizeof(*offset));Run *runs=calloc(cap,sizeof(*runs));
    if(!cp||!types||!visual||!levels||!offset||!runs)goto fail;
    int count=0,cjk=2;const char *p=text;
    while(*p){utf8proc_int32_t code;int n=utf8proc_iterate((const utf8proc_uint8_t*)p,-1,&code);if(n<1)break;offset[count]=(int)(p-text);cp[count++]=code;p+=n;}
    offset[count]=(int)(p-text);
    for(int i=0;i<count;i++){if((cp[i]>=0x3040&&cp[i]<=0x30ff)){cjk=0;break;}if(cp[i]>=0xac00&&cp[i]<=0xd7af){cjk=1;break;}}
    if(cjk==2){ /* Character hints choose TC only when the text distinguishes it. */
        const char *trad[]={"裡","後","這","與","國","華","聲","風","臺","樂","們","書","愛","體","萬"};
        for(unsigned i=0;i<sizeof(trad)/sizeof(*trad);i++)if(strstr(text,trad[i])){cjk=3;break;}
    }
    FriBidiParType base=FRIBIDI_PAR_ON;
    fribidi_get_bidi_types(cp,count,types);
    if(count&&!fribidi_log2vis(cp,count,&base,NULL,visual,NULL,levels)){for(int i=0;i<count;i++){visual[i]=i;levels[i]=0;}}
    Face *face=record(font);hb_script_t previous=HB_SCRIPT_LATIN;int nruns=0,at=0;
    while(at<count){
        const char *endptr=NxtText_nextCluster(text+offset[at]);int end=at+1;while(end<count&&offset[end]<(int)(endptr-text))end++;
        hb_script_t script=HB_SCRIPT_COMMON;for(int i=at;i<end;i++){hb_script_t found=hb_unicode_script(hb_unicode_funcs_get_default(),cp[i]);if(found!=HB_SCRIPT_COMMON&&found!=HB_SCRIPT_INHERITED){script=found;break;}}
        if(script==HB_SCRIPT_COMMON||script==HB_SCRIPT_INHERITED)script=previous;else previous=script;
        TTF_Font *selected=select_face(face,font,cp,at,end,script,cjk);
        if(nruns&&runs[nruns-1].font==selected&&runs[nruns-1].script==script&&runs[nruns-1].level==levels[at])runs[nruns-1].end=end;
        else runs[nruns++]=(Run){.font=selected,.script=script,.start=at,.end=end,.level=levels[at]};
        at=end;
    }
    int ascent=TTF_FontAscent(font),descent=-TTF_FontDescent(font),width=0;
    for(int i=0;i<nruns;i++){
        Run *run=&runs[i];run->visual=INT_MAX;for(int j=run->start;j<run->end;j++)if(visual[j]<run->visual)run->visual=visual[j];
        char tag[5]={0};hb_tag_to_string(hb_script_to_iso15924_tag(run->script),tag);
        TTF_SetFontDirection(run->font,(run->level&1)?TTF_DIRECTION_RTL:TTF_DIRECTION_LTR);TTF_SetFontScriptName(run->font,tag);
        char saved=text[offset[run->end]];text[offset[run->end]]=0;
        TTF_SizeUTF8(run->font,text+offset[run->start],&run->width,&run->height);text[offset[run->end]]=saved;
        width+=run->width;if(TTF_FontAscent(run->font)>ascent)ascent=TTF_FontAscent(run->font);if(-TTF_FontDescent(run->font)>descent)descent=-TTF_FontDescent(run->font);
    }
    int height=ascent+descent;if(height<TTF_FontHeight(font))height=TTF_FontHeight(font);
    if(outw)*outw=width;if(outh)*outh=height;
    SDL_Surface *surface=NULL;
    if(paint){
        surface=SDL_CreateRGBSurfaceWithFormat(0,width>0?width:1,height>0?height:1,32,SDL_PIXELFORMAT_ARGB8888);
        if(surface){SDL_FillRect(surface,NULL,0);qsort(runs,nruns,sizeof(*runs),visual_order);int x=0;
            for(int i=0;i<nruns;i++){Run *run=&runs[i];char saved=text[offset[run->end]];text[offset[run->end]]=0;SDL_Surface *part=TTF_RenderUTF8_Blended(run->font,text+offset[run->start],color);text[offset[run->end]]=saved;
                if(part){SDL_SetSurfaceBlendMode(part,SDL_BLENDMODE_NONE);SDL_BlitSurface(part,NULL,surface,&(SDL_Rect){x,ascent-TTF_FontAscent(run->font),0,0});SDL_FreeSurface(part);}x+=run->width;
            }
        }
    }
    free(cp);free(types);free(visual);free(levels);free(offset);free(runs);free(text);return surface;
fail:
    free(cp);free(types);free(visual);free(levels);free(offset);free(runs);free(text);return NULL;
}
int NxtText_size(TTF_Font *font,const char *text,int *w,int *h){layout(font,text,(SDL_Color){0,0,0,255},w,h,false);return font?0:-1;}
SDL_Surface *NxtText_render(TTF_Font *font,const char *text,SDL_Color color){return layout(font,text,color,NULL,NULL,true);}
