/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_wallpaper_store.h"
#include "nxt_sort.h"
#include "include/parson/parson.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

static bool ident(const char *s){
    if(!s||!*s||strlen(s)>=96)return false;
    for(;*s;s++)if(!isalnum((unsigned char)*s)&&*s!='-'&&*s!='_')return false;
    return true;
}
bool NxtWallpaper_https(const char *s){
    if(!s||strncmp(s,"https://",8)||strlen(s)>=1024)return false;
    const char *host=s+8,*end=host+strcspn(host,"/?#");
    if(end==host)return false;
    for(const char *p=host;p<end;p++)if(!isalnum((unsigned char)*p)&&*p!='.'&&*p!='-')return false;
    for(const char *p=end;*p;p++)if((unsigned char)*p<=32||*p=='\\')return false;
    return true;
}
static void strfield(JSON_Object *o,const char *key,char *out,size_t size){
    const char *s=json_object_get_string(o,key);snprintf(out,size,"%s",s?s:"");
}
int NxtWallpaper_parseCatalog(const char *text,NxtWallpaperEntry *out,int cap,int *last){
    if(last)*last=1;if(!text||!out||cap<1)return -1;
    JSON_Value *v=json_parse_string(text);JSON_Object *o=json_value_get_object(v);
    JSON_Array *data=json_object_get_array(o,"data");
    if(!data){json_value_free(v);return -1;}
    int n=0;double pages=json_object_dotget_number(o,"meta.last_page");
    if(last&&isfinite(pages)&&pages>=1&&pages<=100000)*last=(int)pages;
    for(size_t i=0;i<json_array_get_count(data)&&n<cap;i++){
        JSON_Object *row=json_array_get_object(data,i);const char *id=json_object_get_string(row,"id"),*purity=json_object_get_string(row,"purity"),*type=json_object_get_string(row,"file_type"),*url=json_object_get_string(row,"path"),*thumb=json_object_dotget_string(row,"thumbs.large");
        double x=json_object_get_number(row,"dimension_x"),y=json_object_get_number(row,"dimension_y"),bytes=json_object_get_number(row,"file_size");
        if(!ident(id)||strlen(id)>80||!purity||strcmp(purity,"sfw")||!type||(strcmp(type,"image/jpeg")&&strcmp(type,"image/png"))||!NxtWallpaper_https(url)||!NxtWallpaper_https(thumb))continue;
        if(!isfinite(x)||!isfinite(y)||!isfinite(bytes)||x<1||y<1||x>8192||y>8192||x*y>16000000||bytes<1||bytes>NXT_WALLPAPER_BYTES)continue;
        bool duplicate=false;for(int j=0;j<n;j++)if(!strcmp(out[j].id+3,id)){duplicate=true;break;}if(duplicate)continue;
        NxtWallpaperEntry *w=&out[n++];memset(w,0,sizeof(*w));w->kind=NXT_WP_ONLINE;w->width=x;w->height=y;w->bytes=bytes;w->genre=-1;
        snprintf(w->id,sizeof(w->id),"wh-%s",id);snprintf(w->name,sizeof(w->name),"Wallhaven · %s",id);snprintf(w->url,sizeof(w->url),"%s",url);snprintf(w->thumb,sizeof(w->thumb),"%s",thumb);strfield(row,"url",w->web,sizeof(w->web));
    }
    json_value_free(v);return n;
}
bool NxtWallpaper_imageSize(const char *path,uint32_t *width,uint32_t *height){
    FILE *f=fopen(path,"rb");if(!f)return false;unsigned char b[32];uint32_t w=0,h=0;
    if(fread(b,1,24,f)==24&&!memcmp(b,"\x89PNG\r\n\x1a\n",8)){
        w=((uint32_t)b[16]<<24)|((uint32_t)b[17]<<16)|((uint32_t)b[18]<<8)|b[19];h=((uint32_t)b[20]<<24)|((uint32_t)b[21]<<16)|((uint32_t)b[22]<<8)|b[23];
    }else{rewind(f);if(fgetc(f)==255&&fgetc(f)==216)for(int i=0;i<2000&&!w;i++){
        int x;do{x=fgetc(f);}while(x!=255&&x!=EOF);if(x==EOF)break;do{x=fgetc(f);}while(x==255);if(x==EOF||x==217||x==218)break;if(x==1||(x>=208&&x<=215))continue;
        int a=fgetc(f),c=fgetc(f);if(a<0||c<0)break;int length=(a<<8)|c;if(length<2)break;
        if((x>=192&&x<=195)||(x>=197&&x<=199)||(x>=201&&x<=203)||(x>=205&&x<=207)){if(fread(b,1,5,f)==5){h=((uint32_t)b[1]<<8)|b[2];w=((uint32_t)b[3]<<8)|b[4];}break;}
        if(fseek(f,length-2,SEEK_CUR))break;
    }}fclose(f);if(width)*width=w;if(height)*height=h;
    return w>0&&h>0&&w<=8192&&h<=8192&&(uint64_t)w*h<=16000000;
}
/* One pass from the source crop to its final size. Area integration removes
 * aliasing during reduction; bilinear interpolation handles enlargement.
 * Alpha is accumulated premultiplied so transparent edges do not gain halos.
 * The worker owns the source; no filter work is repeated in the frame loop. */
typedef struct {int first,last;double start,end,frac;bool area;} SampleAxis;
static SampleAxis sample_axis(int p,double start,double step,int limit){
    SampleAxis a={0};a.area=step>1.;
    if(a.area){a.start=start+p*step;a.end=a.start+step;a.first=(int)floor(a.start);a.last=(int)ceil(a.end)-1;}
    else{double c=start+(p+.5)*step-.5;a.first=(int)floor(c);a.last=a.first+1;a.frac=c-a.first;}
    if(a.first<0)a.first=0;if(a.last>=limit)a.last=limit-1;
    return a;
}
static double sample_weight(SampleAxis a,int p){
    if(a.area){double lo=fmax(a.start,p),hi=fmin(a.end,p+1.);return fmax(0.,hi-lo);}
    /* Clamping at either boundary merges the two bilinear weights. */
    if(a.first==a.last)return 1.;return p==a.first?1.-a.frac:a.frac;
}
SDL_Surface *NxtWallpaper_resample(SDL_Surface *image,int width,int height,bool cover,int render){
    if(!image||image->w<1||image->h<1||width<1||height<1||width>8192||height>8192||(uint64_t)width*height>16000000)return NULL;
    SDL_Surface *source=image->format->format==SDL_PIXELFORMAT_RGBA32?image:SDL_ConvertSurfaceFormat(image,SDL_PIXELFORMAT_RGBA32,0);
    SDL_Surface *out=SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_RGBA32);
    if(!source||!out){if(source&&source!=image)SDL_FreeSurface(source);if(out)SDL_FreeSurface(out);return NULL;}
    double step_x=(double)source->w/width,step_y=(double)source->h/height,start_x=0,start_y=0;
    if(cover){double step=fmin(step_x,step_y);step_x=step_y=step;start_x=(source->w-width*step)/2.;start_y=(source->h-height*step)/2.;
        if(render==NXT_WP_PIXEL&&step<1.){
            /* Integer enlargement gives equally sized pixel blocks. Crop the
             * virtual enlarged canvas rather than stretching its edge cells. */
            int scale=(int)ceil(1./step);step_x=step_y=1./scale;
            start_x=(double)((source->w*scale-width)/2)/scale;start_y=(double)((source->h*scale-height)/2)/scale;
        }
    }
    SampleAxis *xs=malloc(width*sizeof(*xs));
    if(!xs){if(source!=image)SDL_FreeSurface(source);SDL_FreeSurface(out);return NULL;}
    for(int x=0;x<width;x++)xs[x]=sample_axis(x,start_x,step_x,source->w);
    for(int y=0;y<height;y++){
        unsigned char *dst=(unsigned char *)out->pixels+y*out->pitch;SampleAxis ay=sample_axis(y,start_y,step_y,source->h);
        for(int x=0;x<width;x++,dst+=4){
            if(render==NXT_WP_PIXEL){int ix=(int)floor(start_x+(x+.5)*step_x),iy=(int)floor(start_y+(y+.5)*step_y);if(ix>=source->w)ix=source->w-1;if(iy>=source->h)iy=source->h-1;if(ix<0)ix=0;if(iy<0)iy=0;memcpy(dst,(unsigned char *)source->pixels+iy*source->pitch+ix*4,4);continue;}
            SampleAxis ax=xs[x];double red=0,green=0,blue=0,alpha=0,total=0;
            for(int sy=ay.first;sy<=ay.last;sy++){double wy=sample_weight(ay,sy);const unsigned char *row=(unsigned char *)source->pixels+sy*source->pitch;
                for(int sx=ax.first;sx<=ax.last;sx++){double weight=wy*sample_weight(ax,sx);const unsigned char *p=row+sx*4;double a=p[3]*weight;red+=p[0]*a;green+=p[1]*a;blue+=p[2]*a;alpha+=a;total+=weight;}
            }
            dst[0]=alpha>0?(unsigned char)lround(red/alpha):0;dst[1]=alpha>0?(unsigned char)lround(green/alpha):0;dst[2]=alpha>0?(unsigned char)lround(blue/alpha):0;dst[3]=total>0?(unsigned char)lround(alpha/total):0;
        }
    }
    free(xs);if(source!=image)SDL_FreeSurface(source);return out;
}
static bool normalize_size(const char *path,const char *target,int max_width,int max_height,bool cover,int render){
    struct stat st;uint32_t w,h;
    if(lstat(path,&st)||!S_ISREG(st.st_mode)||st.st_size<1||st.st_size>NXT_WALLPAPER_BYTES||!NxtWallpaper_imageSize(path,&w,&h))return false;
    SDL_Surface *image=IMG_Load(path);if(!image)return false;
    double scale=1.;if(image->w>max_width)scale=(double)max_width/image->w;if(image->h*scale>max_height)scale=(double)max_height/image->h;
    int nx=cover?max_width:(int)(image->w*scale),ny=cover?max_height:(int)(image->h*scale);if(nx<1)nx=1;if(ny<1)ny=1;
    SDL_Surface *copy=NxtWallpaper_resample(image,nx,ny,cover,render);
    char temp[1024]="";int len=snprintf(temp,sizeof(temp),"%s.tmp",target);bool ok=copy&&len>0&&len<(int)sizeof(temp);
    if(ok)ok=IMG_SavePNG(copy,temp)==0&&!rename(temp,target);
    if(copy)SDL_FreeSurface(copy);SDL_FreeSurface(image);if(!ok&&len>0&&len<(int)sizeof(temp))unlink(temp);return ok;
}
bool NxtWallpaper_prepare(const char *path,const char *target,int render){return normalize_size(path,target,1024,768,true,render==NXT_WP_PIXEL?NXT_WP_PIXEL:NXT_WP_SMOOTH);}
bool NxtWallpaper_normalize(const char *path,const char *target){return NxtWallpaper_prepare(path,target,NXT_WP_SMOOTH);}
bool NxtWallpaper_thumbnail(const char *path,const char *target){return normalize_size(path,target,320,240,false,NXT_WP_SMOOTH);}
bool NxtWallpaper_inside(const char *root,const char *path){
    char a[PATH_MAX],b[PATH_MAX];if(!root||!path||!realpath(root,a)||!realpath(path,b))return false;
    size_t n=strlen(a);return !strncmp(a,b,n)&&(a[n-1]=='/'||b[n]=='/'||b[n]==0);
}
static int compare_entry(const void *a,const void *b){
    const NxtWallpaperEntry *x=a,*y=b;if(x->kind==NXT_WP_FOLDER&&y->kind!=NXT_WP_FOLDER)return -1;if(y->kind==NXT_WP_FOLDER&&x->kind!=NXT_WP_FOLDER)return 1;return NxtSort_compare(x->name,y->name);
}
int NxtWallpaper_scan(const char *root,const char *dir,NxtWallpaperEntry *out,int cap,bool *truncated){
    if(truncated)*truncated=false;if(!NxtWallpaper_inside(root,dir)||!out||cap<1)return -1;DIR *d=opendir(dir);if(!d)return -1;
    struct dirent *ent;int n=0;
    while((ent=readdir(d))){
        if(ent->d_name[0]=='.')continue;char path[512];if(snprintf(path,sizeof(path),"%s/%s",dir,ent->d_name)>=(int)sizeof(path))continue;
        struct stat st;if(lstat(path,&st)||S_ISLNK(st.st_mode))continue;bool folder=S_ISDIR(st.st_mode);const char *ext=strrchr(ent->d_name,'.');
        if(!folder&&(!S_ISREG(st.st_mode)||!ext||(strcasecmp(ext,".jpg")&&strcasecmp(ext,".jpeg")&&strcasecmp(ext,".png"))))continue;
        if(n>=cap){if(truncated)*truncated=true;continue;}
        NxtWallpaperEntry *w=&out[n++];memset(w,0,sizeof(*w));w->kind=folder?NXT_WP_FOLDER:NXT_WP_LOCAL;w->genre=-1;w->bytes=st.st_size;snprintf(w->path,sizeof(w->path),"%s",path);snprintf(w->name,sizeof(w->name),"%s",ent->d_name);
        uint64_t hash=14695981039346656037ull;for(const unsigned char *p=(unsigned char *)path;*p;p++){hash^=*p;hash*=1099511628211ull;}
        snprintf(w->id,sizeof(w->id),"%s-%016llx",folder?"dir":"sd",(unsigned long long)hash);
    }
    closedir(d);qsort(out,n,sizeof(*out),compare_entry);return n;
}
static bool decode_entry(JSON_Object *o,NxtWallpaperEntry *w){
    const char *id=json_object_get_string(o,"id");if(!ident(id))return false;memset(w,0,sizeof(*w));
    strfield(o,"id",w->id,sizeof(w->id));strfield(o,"name",w->name,sizeof(w->name));strfield(o,"path",w->path,sizeof(w->path));strfield(o,"url",w->url,sizeof(w->url));strfield(o,"thumb",w->thumb,sizeof(w->thumb));strfield(o,"web",w->web,sizeof(w->web));
    double kind=json_object_get_number(o,"kind"),preset=json_object_get_number(o,"preset"),genre=json_object_get_number(o,"genre"),render=json_object_get_number(o,"render");
    if(!isfinite(kind)||kind<0||kind>NXT_WP_PRESET||!isfinite(preset)||preset<0||preset>21||!isfinite(genre)||genre< -1||genre>=NXT_WALLPAPER_GENRES)return false;
    if(floor(kind)!=kind||floor(preset)!=preset||floor(genre)!=genre||!isfinite(render)||render<0||render>1||floor(render)!=render)return false;
    w->kind=kind;w->preset=preset;w->genre=genre;w->render=render;
    double width=json_object_get_number(o,"width"),height=json_object_get_number(o,"height"),bytes=json_object_get_number(o,"bytes");
    if(isfinite(width)&&width>=0&&width<=8192)w->width=width;if(isfinite(height)&&height>=0&&height<=8192)w->height=height;if(isfinite(bytes)&&bytes>=0&&bytes<=NXT_WALLPAPER_BYTES)w->bytes=bytes;
    if(w->kind==NXT_WP_ONLINE&&(!NxtWallpaper_https(w->url)||!NxtWallpaper_https(w->thumb)))return false;
    if(w->kind==NXT_WP_LOCAL&&w->path[0]!='/')return false;
    return true;
}
int NxtWallpaper_readEntries(const char *path,NxtWallpaperEntry *out,int cap){
    struct stat st;if(stat(path,&st)||st.st_size>1024*1024)return 0;
    JSON_Value *v=json_parse_file(path);JSON_Array *a=json_value_get_array(v);int n=0;
    for(size_t i=0;i<json_array_get_count(a)&&n<cap;i++)if(decode_entry(json_array_get_object(a,i),&out[n]))n++;
    json_value_free(v);return n;
}
bool NxtWallpaper_writeEntries(const char *path,const NxtWallpaperEntry *data,int n){
    if(n<0||n>NXT_WALLPAPER_CAP)return false;JSON_Value *v=json_value_init_array();JSON_Array *a=json_value_get_array(v);
    for(int i=0;i<n;i++){const NxtWallpaperEntry *w=&data[i];JSON_Value *item=json_value_init_object();JSON_Object *o=json_value_get_object(item);
        json_object_set_number(o,"kind",w->kind);json_object_set_number(o,"genre",w->genre);json_object_set_number(o,"preset",w->preset);json_object_set_number(o,"render",w->render);json_object_set_number(o,"width",w->width);json_object_set_number(o,"height",w->height);json_object_set_number(o,"bytes",(double)w->bytes);
        json_object_set_string(o,"id",w->id);json_object_set_string(o,"name",w->name);json_object_set_string(o,"path",w->path);json_object_set_string(o,"url",w->url);json_object_set_string(o,"thumb",w->thumb);json_object_set_string(o,"web",w->web);json_array_append_value(a,item);
    }
    char temp[1024]="";int len=snprintf(temp,sizeof(temp),"%s.tmp",path);bool ok=len>0&&len<(int)sizeof(temp)&&json_serialize_to_file_pretty(v,temp)==JSONSuccess&&!rename(temp,path);json_value_free(v);if(!ok&&len>0&&len<(int)sizeof(temp))unlink(temp);return ok;
}
