/* SPDX-License-Identifier: GPL-3.0-only
 * Unified wallpaper gallery. Network/large image work never runs in the UI loop. */
#include "nxt_wallpapers.h"
#include "nxt_wallpaper_store.h"
#include "include/parson/parson.h"
#include "nxt_appearance.h"
#include "nxt_skin.h"
#include "nxt_personal.h"
#include "nxt_log.h"
#include "file_utils.h"
#include "background.h"
#include "module_player.h"
#include "toast.h"
#include "api.h"
#include <SDL2/SDL_image.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <math.h>

enum {WP_EXPLORE,WP_GENRES,WP_FAVORITES,WP_DOWNLOADED,WP_CARD};
enum {JOB_CATALOG,JOB_THUMB,JOB_IMAGE,JOB_LOCAL_THUMB};
typedef struct {
    pthread_t thread;int joined,done,cancel,type,ok;
    NxtWallpaperEntry entry;
    char url[1200],target[512],temp[540],cert[1024],original[512];
} WallpaperJob;
typedef struct {
    int tab,source,genre,page,last_page,selected,count,view,pending;
    NxtWallpaperEntry *items,*favorites,action;
    int nfav,thumb_first,total,first_page,previous_page;bool limited,dirty,preview,catalog_append;
    SDL_Surface *thumbs[6],*genre_thumbs[8],*detail;bool thumb_failed[6];
    char detail_key[620];
    char root[512],folder[512],cache[512],favfile[512],status[256];
    char old_path[512];int old_mode,old_panel,old_theme,old_dim;
    WallpaperJob job;
} Gallery;
static const char *genre_en[]={"Retro & Pixel","Anime & Illustration","Game","Nature","City & Neon","Space","Music","Minimal & Abstract","Landscapes","Ocean & Beaches","Mountains","Forests","Flowers","Animals","Cats","Dogs","Cars","Motorcycles","Architecture","Fantasy","Sci-fi","Abstract Art","Dark & OLED","Seasons"};
static const char *genre_vi[]={"Retro & Pixel","Anime & Minh họa","Game","Thiên nhiên","Thành phố & Neon","Vũ trụ","Âm nhạc","Tối giản & Trừu tượng","Phong cảnh","Biển & Bãi biển","Núi non","Rừng xanh","Hoa & Cây","Động vật","Mèo","Chó","Ô tô","Mô tô","Kiến trúc","Fantasy","Khoa học viễn tưởng","Nghệ thuật trừu tượng","Tối & OLED","Bốn mùa"};
static const char *queries[]={"pixel+art","anime+scenery","video+games","nature","city+neon","space","music","minimalism","landscape","ocean","mountains","forest","flowers","animals","cats","dogs","cars","motorcycle","architecture","fantasy+art","science+fiction","abstract","dark","seasons"};
static const char *genre_icons[]={"gamepad-2","flower-2","gamepad-2","trees","building-2","orbit","disc-3","shapes","trees","shapes","trees","trees","flower-2","heart","heart","heart","shapes","shapes","building-2","orbit","orbit","shapes","moon","sun-moon"};
static int u(SDL_Surface *s,int x){return x*s->w/1024;}
static bool exists(const char *path){struct stat st;return !stat(path,&st)&&S_ISREG(st.st_mode)&&st.st_size>0;}
static bool busy(Gallery *g){return g->job.joined&&!__atomic_load_n(&g->job.done,__ATOMIC_ACQUIRE);}
static const char *source_name(Gallery *g){return g->source==2?Nxt_text("Built-in backgrounds","Nền có sẵn"):g->source==1?"Wallhaven":Nxt_text("NXT picks · Wallhaven","NXT chọn lọc · Wallhaven");}
static const char *origin(const NxtWallpaperEntry *w){return w->kind==NXT_WP_LOCAL?Nxt_text("SD card · offline","Thẻ nhớ · offline"):w->kind==NXT_WP_PRESET?Nxt_text("NXT original · offline","NXT có sẵn · offline"):"Wallhaven";}
static void image_path(Gallery *g,const NxtWallpaperEntry *w,char out[512]){snprintf(out,512,"%s/files/%s-hq2-%c.png",g->cache,w->id,w->render==NXT_WP_PIXEL?'p':'s');}
static void original_path(Gallery *g,const NxtWallpaperEntry *w,char out[512]){snprintf(out,512,"%s/files/%s.original",g->cache,w->id);}
static void thumb_path(Gallery *g,const NxtWallpaperEntry *w,char out[512]){snprintf(out,512,"%s/thumbs/%s-hq2.img",g->cache,w->id);}
static bool usable_original(const char *path){struct stat st;uint32_t w,h;return !lstat(path,&st)&&S_ISREG(st.st_mode)&&st.st_size>0&&st.st_size<=NXT_WALLPAPER_BYTES&&NxtWallpaper_imageSize(path,&w,&h);}
static bool cached_image(Gallery *g,const NxtWallpaperEntry *w,char out[512]){
    image_path(g,w,out);struct stat st,original;uint32_t width,height;if(lstat(out,&st)||!S_ISREG(st.st_mode)||!NxtWallpaper_imageSize(out,&width,&height)||width!=1024||height!=768)return false;
    if(w->kind==NXT_WP_LOCAL&&(!stat(w->path,&original)&&original.st_mtime>st.st_mtime))return false;
    return true;
}
static void job_stop(Gallery *g){
    if(!g->job.joined)return;bool discarded_page=g->job.type==JOB_CATALOG&&g->catalog_append&&g->page!=g->previous_page;
    __atomic_store_n(&g->job.cancel,1,__ATOMIC_RELAXED);pthread_join(g->job.thread,NULL);g->job.joined=0;unlink(g->job.temp);if(discarded_page)g->page=g->previous_page;
}
static bool job_cancelled(WallpaperJob *j){return __atomic_load_n(&j->cancel,__ATOMIC_RELAXED)!=0;}
static bool fetch_image(WallpaperJob *j){
    if(!NxtWallpaper_https(j->url)||job_cancelled(j))return false;
    char limit[32];snprintf(limit,sizeof(limit),"%u",j->type==JOB_IMAGE?NXT_WALLPAPER_BYTES:j->type==JOB_CATALOG?2u*1024u*1024u:1024u*1024u);
    const char *seconds=j->type==JOB_IMAGE?"90":j->type==JOB_CATALOG?"45":"30";
    const char *args[]={"curl","-sSL","--fail","--proto","=https","--proto-redir","=https","--cacert",j->cert,"--connect-timeout","15","--max-time",seconds,"--max-filesize",limit,"--speed-time","20","--speed-limit","128","-A","NXT-Retro-Music-Wallpapers/1.6.1","-o",j->temp,j->url,NULL};
    pid_t child=fork();if(child<0)return false;
    if(!child){int fd=open("/dev/null",O_WRONLY);if(fd>=0){dup2(fd,STDOUT_FILENO);dup2(fd,STDERR_FILENO);close(fd);}execvp(args[0],(char *const *)args);_exit(127);}
    int result=0;pid_t ended;
    while((ended=waitpid(child,&result,WNOHANG))==0){
        if(job_cancelled(j)){kill(child,SIGTERM);for(int i=0;i<10;i++){usleep(50000);ended=waitpid(child,&result,WNOHANG);if(ended)break;}if(!ended){kill(child,SIGKILL);while(waitpid(child,&result,0)<0&&errno==EINTR){}}return false;}usleep(100000);
    }
    struct stat st;return ended==child&&WIFEXITED(result)&&WEXITSTATUS(result)==0&&!stat(j->temp,&st)&&st.st_size>0&&st.st_size<=strtol(limit,NULL,10);
}
static void *job_worker(void *argument){
    WallpaperJob *j=argument;bool ok=false;
    if(j->type==JOB_LOCAL_THUMB)ok=NxtWallpaper_thumbnail(j->entry.path,j->target);
    else if(j->type==JOB_IMAGE&&j->entry.kind==NXT_WP_LOCAL)ok=NxtWallpaper_prepare(j->entry.path,j->target,j->entry.render);
    else if(j->type==JOB_IMAGE&&usable_original(j->original))ok=NxtWallpaper_prepare(j->original,j->target,j->entry.render);
    else if(fetch_image(j)&&!job_cancelled(j)){
        if(j->type==JOB_IMAGE){ok=NxtWallpaper_prepare(j->temp,j->target,j->entry.render);if(ok&&!job_cancelled(j))ok=!rename(j->temp,j->original);}
        else if(j->type==JOB_CATALOG){
            FILE *f=fopen(j->temp,"rb");char *text=NULL;if(f){fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);if(n>0&&n<=2*1024*1024){text=calloc(n+1,1);if(text&&fread(text,1,n,f)!=(size_t)n){free(text);text=NULL;}}fclose(f);}
            NxtWallpaperEntry *items=malloc(24*sizeof(*items));int pages;
            ok=text&&items&&NxtWallpaper_parseCatalog(text,items,24,&pages)>=0&&!rename(j->temp,j->target);free(items);free(text);
        }else{uint32_t w,h;ok=NxtWallpaper_imageSize(j->temp,&w,&h)&&(uint64_t)w*h<=2000000&&NxtWallpaper_thumbnail(j->temp,j->target);}
    }
    if(ok&&j->type==JOB_IMAGE&&j->entry.kind==NXT_WP_ONLINE){
        snprintf(j->entry.path,sizeof(j->entry.path),"%s",j->target);char meta[540];snprintf(meta,sizeof(meta),"%s.json",j->target);ok=NxtWallpaper_writeEntries(meta,&j->entry,1);
    }
    unlink(j->temp);j->ok=ok&&!job_cancelled(j);__atomic_store_n(&j->done,1,__ATOMIC_RELEASE);return NULL;
}
static bool job_start(Gallery *g,int type,const NxtWallpaperEntry *entry,const char *url,const char *target){
    job_stop(g);WallpaperJob *j=&g->job;memset(j,0,sizeof(*j));j->type=type;if(entry)j->entry=*entry;
    snprintf(j->url,sizeof(j->url),"%s",url?url:"");snprintf(j->target,sizeof(j->target),"%s",target);snprintf(j->temp,sizeof(j->temp),"%s.part",target);
    if(entry&&entry->kind==NXT_WP_ONLINE&&type==JOB_IMAGE)original_path(g,entry,j->original);
    char cwd[800];if(!getcwd(cwd,sizeof(cwd)))return false;snprintf(j->cert,sizeof(j->cert),"%s/res/cacert.pem",cwd);
    if(pthread_create(&j->thread,NULL,job_worker,j))return false;j->joined=1;g->dirty=true;return true;
}
static void thumbs_clear(Gallery *g){for(int i=0;i<6;i++){if(g->thumbs[i])SDL_FreeSurface(g->thumbs[i]);g->thumbs[i]=NULL;g->thumb_failed[i]=false;}g->thumb_first=-1;}
static void detail_clear(Gallery *g){if(g->detail)SDL_FreeSurface(g->detail);g->detail=NULL;g->detail_key[0]=0;}
static void prune_thumbnails(Gallery *g){
    char dir[540];snprintf(dir,sizeof(dir),"%s/thumbs",g->cache);DIR *d=opendir(dir);if(!d)return;uint64_t size=0;int count=0;struct dirent *ent;while((ent=readdir(d))){if(ent->d_name[0]=='.')continue;char path[800];snprintf(path,sizeof(path),"%s/%s",dir,ent->d_name);struct stat st;if(!lstat(path,&st)&&S_ISREG(st.st_mode)){size+=st.st_size;count++;}}closedir(d);
    /* Only disposable thumbnails are pruned. Applied/downloaded files are retained. */
    while(size>24u*1024u*1024u||count>96){
        char oldest[800]="";time_t stamp=0;off_t bytes=0;d=opendir(dir);if(!d)break;
        while((ent=readdir(d))){if(ent->d_name[0]=='.')continue;char path[800];snprintf(path,sizeof(path),"%s/%s",dir,ent->d_name);struct stat st;if(!lstat(path,&st)&&S_ISREG(st.st_mode)&&(!oldest[0]||st.st_mtime<stamp)){snprintf(oldest,sizeof(oldest),"%s",path);stamp=st.st_mtime;bytes=st.st_size;}}
        closedir(d);if(!oldest[0]||unlink(oldest))break;size-=bytes;count--;
    }
}
static int favorite_index(Gallery *g,const NxtWallpaperEntry *w){for(int i=0;i<g->nfav;i++)if(!strcmp(g->favorites[i].id,w->id))return i;return -1;}
static void favorite(Gallery *g,NxtWallpaperEntry w){
    if(w.kind==NXT_WP_FOLDER)return;int at=favorite_index(g,&w);NxtWallpaperEntry *backup=malloc(g->nfav*sizeof(*backup));int old=g->nfav;if(old&&!backup)return;if(old)memcpy(backup,g->favorites,old*sizeof(*backup));
    if(at>=0){memmove(&g->favorites[at],&g->favorites[at+1],(g->nfav-at-1)*sizeof(w));g->nfav--;}
    else if(g->nfav<NXT_WALLPAPER_FAVORITES)g->favorites[g->nfav++]=w;else{free(backup);Toast_show(Nxt_text("Favorite list is full","Danh sách yêu thích đã đầy"),2000);return;}
    bool ok=NxtWallpaper_writeEntries(g->favfile,g->favorites,g->nfav);if(!ok){g->nfav=old;if(old)memcpy(g->favorites,backup,old*sizeof(*backup));}free(backup);
    Toast_show(ok?(at>=0?Nxt_text("Removed from favorites","Đã bỏ yêu thích"):Nxt_text("Added to favorites","Đã thêm yêu thích")):Nxt_text("Cannot save favorites; export log","Không lưu được yêu thích; hãy xuất log"),1800);NxtLog_step("wallpaper","favorites",ok,"profile=%d; count=%d",NxtPersonal_id(),g->nfav);g->dirty=true;
    if(g->tab==WP_FAVORITES&&g->view==0){memcpy(g->items,g->favorites,g->nfav*sizeof(w));g->count=g->nfav;if(g->selected>=g->count)g->selected=g->count?g->count-1:0;thumbs_clear(g);}
}
static void catalog_path(Gallery *g,char out[512]){snprintf(out,512,"%s/catalog-hq2-%d-%d-%d.json",g->cache,g->source,g->genre+1,g->page);}
static int read_catalog(Gallery *g){
    char path[512];catalog_path(g,path);FILE *f=fopen(path,"rb");if(!f)return -1;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);char *text=n>0&&n<=2*1024*1024?calloc(n+1,1):NULL;if(text&&fread(text,1,n,f)!=(size_t)n){free(text);text=NULL;}fclose(f);
    NxtWallpaperEntry items[24];int count=text?NxtWallpaper_parseCatalog(text,items,24,&g->last_page):-1;
    if(count>=0){
        JSON_Value *v=json_parse_string(text);double total=json_object_dotget_number(json_value_get_object(v),"meta.total");if(total>=0&&total<=1000000000)g->total=total;json_value_free(v);
        /* At most 512 small metadata records and six visible thumbnails.
         * Crossing that window continues at the next server page. */
        if(!g->catalog_append||g->count+count>NXT_WALLPAPER_CAP){g->count=0;g->first_page=g->page;}
        int first=g->count;
        for(int i=0;i<count;i++){bool duplicate=false;for(int k=0;k<g->count;k++)if(!strcmp(g->items[k].id,items[i].id)){duplicate=true;break;}if(duplicate)continue;items[i].genre=g->genre;g->items[g->count++]=items[i];}
        if(g->catalog_append&&g->count>first)g->selected=first;else if(g->selected>=g->count)g->selected=g->count?g->count-1:0;
        g->previous_page=g->page;
    }free(text);return count;
}
static const char *category_mask(Gallery *g){return g->genre==1?"010":"110";}
static void catalog_url(Gallery *g,char out[1200]){snprintf(out,1200,"https://wallhaven.cc/api/v1/search?purity=100&categories=%s&sorting=%s&topRange=1y&atleast=1024x768&q=%s&page=%d",category_mask(g),g->source==0?"toplist":"date_added",g->genre>=0?queries[g->genre]:"",g->page);}
static void load_more(Gallery *g){
    if(g->page>=g->last_page||(busy(g)&&g->job.type==JOB_CATALOG))return;
    job_stop(g);g->previous_page=g->page;g->page++;g->catalog_append=true;
    char path[512];catalog_path(g,path);struct stat st;
    if(!stat(path,&st)&&time(NULL)-st.st_mtime<=6*3600&&read_catalog(g)>=0){thumbs_clear(g);g->status[0]=0;g->dirty=true;return;}
    char url[1200];catalog_url(g,url);
    if(job_start(g,JOB_CATALOG,NULL,url,path))snprintf(g->status,sizeof(g->status),"%s",Nxt_text("Loading more wallpapers…","Đang tải thêm hình nền…"));
    else{g->page=g->previous_page;snprintf(g->status,sizeof(g->status),"%s",Nxt_text("R2: retry loading more","R2: thử tải thêm"));}g->dirty=true;
}
static void refresh(Gallery *g,bool force){
    g->status[0]=0;g->limited=false;g->catalog_append=false;g->total=0;thumbs_clear(g);detail_clear(g);g->count=0;g->first_page=g->previous_page=g->page;
    if(g->tab==WP_CARD){g->count=NxtWallpaper_scan(g->root,g->folder,g->items,NXT_WALLPAPER_CAP,&g->limited);if(g->count<0){g->count=0;snprintf(g->status,sizeof(g->status),"%s",Nxt_text("Cannot open this folder","Không mở được thư mục này"));}}
    else if(g->tab==WP_FAVORITES){g->count=g->nfav;memcpy(g->items,g->favorites,g->nfav*sizeof(*g->items));}
    else if(g->tab==WP_DOWNLOADED){char dir[540];snprintf(dir,sizeof(dir),"%s/files",g->cache);DIR *d=opendir(dir);struct dirent *ent;if(d){while((ent=readdir(d))){if(!strstr(ent->d_name,".png.json"))continue;char path[900];snprintf(path,sizeof(path),"%s/%s",dir,ent->d_name);NxtWallpaperEntry w;if(NxtWallpaper_readEntries(path,&w,1)!=1||!NxtWallpaper_inside(dir,w.path)||!exists(w.path))continue;int at=-1;for(int i=0;i<g->count;i++)if(!strcmp(g->items[i].id,w.id)){at=i;break;}if(at<0){if(g->count<NXT_WALLPAPER_CAP)g->items[g->count++]=w;else g->limited=true;}else{struct stat a,b;if(!stat(w.path,&a)&&!stat(g->items[at].path,&b)&&(a.st_mtime>b.st_mtime||(a.st_mtime==b.st_mtime&&strstr(w.path,"-hq2-"))))g->items[at]=w;}}closedir(d);}}
    else if(g->tab==WP_GENRES)g->count=NXT_WALLPAPER_GENRES;
    else if(g->source==2){
        for(int i=0;i<22;i++){if(i==4)continue;NxtWallpaperEntry *w=&g->items[g->count++];memset(w,0,sizeof(*w));w->kind=NXT_WP_PRESET;w->preset=i;w->genre=-1;snprintf(w->id,sizeof(w->id),"nxt-%02d",i);snprintf(w->name,sizeof(w->name),"%s",NxtAppearance_wallpaperName(i));}
    }else{
        read_catalog(g);char path[512];catalog_path(g,path);struct stat st;
        if(force||stat(path,&st)||time(NULL)-st.st_mtime>6*3600){
            char url[1200];catalog_url(g,url);
            if(job_start(g,JOB_CATALOG,NULL,url,path))snprintf(g->status,sizeof(g->status),"%s",Nxt_text("Loading online gallery…","Đang tải kho ảnh online…"));else snprintf(g->status,sizeof(g->status),"%s",Nxt_text("Cannot start download","Không bắt đầu tải được"));
        }
    }
    if(g->selected>=g->count)g->selected=g->count?g->count-1:0;g->dirty=true;
}
static void restore_preview(Gallery *g){
    if(!g->preview)return;nxt.wallpaper_mode=g->old_mode;nxt.panel_opacity=g->old_panel;nxt.wallpaper_dim=g->old_dim;nxt.theme=g->old_theme;snprintf(nxt.wallpaper_path,sizeof(nxt.wallpaper_path),"%s",g->old_path);NxtAppearance_release();g->preview=false;
}
static bool set_wallpaper(Gallery *g,const NxtWallpaperEntry *w,bool save){
    if(!g->preview){g->old_mode=nxt.wallpaper_mode;g->old_panel=nxt.panel_opacity;g->old_dim=nxt.wallpaper_dim;g->old_theme=nxt.theme;snprintf(g->old_path,sizeof(g->old_path),"%s",nxt.wallpaper_path);g->preview=true;}
    if(w->kind==NXT_WP_PRESET)nxt.wallpaper_mode=w->preset;
    else{char path[512];if(!cached_image(g,w,path)){restore_preview(g);return false;}snprintf(nxt.wallpaper_path,sizeof(nxt.wallpaper_path),"%s",path);nxt.wallpaper_mode=4;}
    NxtAppearance_release();bool ok=!save||Nxt_save();if(save){if(ok)g->preview=false;else restore_preview(g);NxtLog_step("wallpaper","apply",ok,"profile=%d; kind=%d",NxtPersonal_id(),w->kind);}return ok;
}
static void ready_image(Gallery *g,NxtWallpaperEntry w,int pending){
    if(pending==3){detail_clear(g);g->dirty=true;}
    else if(pending==1){g->action=w;if(set_wallpaper(g,&w,false)){g->view=2;g->dirty=true;}}
    else if(pending==2){Toast_show(set_wallpaper(g,&w,true)?Nxt_text("Wallpaper applied","Đã áp dụng hình nền"):Nxt_text("Cannot save wallpaper","Không lưu được hình nền"),2200);g->dirty=true;}
    else Toast_show(Nxt_text("Downloaded; available offline","Đã tải; dùng được offline"),2000);
}
static void request_image(Gallery *g,NxtWallpaperEntry w,int pending){
    /* Detail selection must stay stable if the user opens an image while a
     * following catalog page is still arriving. The next page can be retried. */
    if(g->job.joined&&g->job.type==JOB_CATALOG){job_stop(g);g->status[0]=0;}
    if(w.kind==NXT_WP_FOLDER)return;char path[512];if(w.kind==NXT_WP_PRESET||cached_image(g,&w,path)){ready_image(g,w,pending);return;}
    if(busy(g)&&g->job.type==JOB_IMAGE&&!strcmp(g->job.entry.id,w.id)&&g->job.entry.render==w.render){g->pending=pending;return;}
    struct statvfs space;if(statvfs(g->cache,&space)||(uint64_t)space.f_bavail*space.f_frsize<32u*1024u*1024u){Toast_show(Nxt_text("Need at least 32 MiB free on SD","Cần trống ít nhất 32 MiB trên thẻ"),2500);return;}
    if(w.kind==NXT_WP_LOCAL&&!NxtWallpaper_inside(g->root,w.path)){Toast_show(Nxt_text("Image is not available on this SD card","Ảnh không còn trên thẻ này"),2500);return;}
    image_path(g,&w,path);g->action=w;g->pending=pending;
    if(job_start(g,JOB_IMAGE,&w,w.kind==NXT_WP_ONLINE?w.url:NULL,path))snprintf(g->status,sizeof(g->status),"%s",w.kind==NXT_WP_ONLINE&&!usable_original(g->job.original)?Nxt_text("Downloading full image…","Đang tải ảnh đầy đủ…"):Nxt_text("Preparing full image…","Đang chuẩn bị ảnh đầy đủ…"));else{g->pending=0;Toast_show(Nxt_text("Cannot start image job","Không chuẩn bị ảnh được"),2500);}
}
static void toggle_render(Gallery *g){
    if(!g->count)return;NxtWallpaperEntry *w=&g->items[g->selected];if(w->kind==NXT_WP_PRESET)return;
    job_stop(g);g->pending=0;w->render=w->render==NXT_WP_PIXEL?NXT_WP_SMOOTH:NXT_WP_PIXEL;detail_clear(g);
    int at=favorite_index(g,w);if(at>=0){g->favorites[at].render=w->render;bool ok=NxtWallpaper_writeEntries(g->favfile,g->favorites,g->nfav);NxtLog_step("wallpaper","render-choice",ok,"render=%d",w->render);}
    request_image(g,*w,3);g->dirty=true;
}
static void fit(SDL_Surface *s,SDL_Surface *image,SDL_Rect r){
    if(!image||r.w<1||r.h<1)return;SDL_Rect crop={0,0,image->w,image->h};
    if((int64_t)image->w*r.h>(int64_t)image->h*r.w){crop.w=image->h*r.w/r.h;crop.x=(image->w-crop.w)/2;}
    else{crop.h=image->w*r.h/r.w;crop.y=(image->h-crop.h)/2;}
    SDL_BlitScaled(image,&crop,s,&r);
}
static void tick(Gallery *g){
    WallpaperJob *j=&g->job;
    if(j->joined&&__atomic_load_n(&j->done,__ATOMIC_ACQUIRE)){
        pthread_join(j->thread,NULL);j->joined=0;g->dirty=true;
        if(j->type==JOB_CATALOG){int n=j->ok?read_catalog(g):-1;if(n<0){if(g->catalog_append)g->page=g->previous_page;snprintf(g->status,sizeof(g->status),"%s",Nxt_text("Source unavailable. R2: more / START: sources.","Nguồn chưa tải được. R2 tải tiếp / START chọn nguồn."));}else{snprintf(g->status,sizeof(g->status),"%s",n?"":Nxt_text("No compatible images on this page; R2 for more.","Trang này chưa có ảnh phù hợp; R2 tải tiếp."));if(g->selected>=g->count)g->selected=0;thumbs_clear(g);}NxtLog_step("wallpaper","catalog",n>=0,"source=%d; genre=%d; page=%d; entries=%d",g->source,g->genre,g->page,g->count);}
        else if(j->type==JOB_IMAGE){int pending=g->pending;g->pending=0;g->status[0]=0;if(j->ok){ready_image(g,g->action,pending);if(g->tab==WP_DOWNLOADED&&g->view==0)refresh(g,false);}else{snprintf(g->status,sizeof(g->status),"%s",Nxt_text("Image failed. Check Wi-Fi / JPG-PNG size; START retries.","Ảnh chưa tải được. Kiểm tra Wi-Fi / cỡ JPG-PNG; START thử lại."));Toast_show(g->status,3000);}NxtLog_step("wallpaper","image",j->ok,"kind=%d; cover=1024x768; render=%d",j->entry.kind,j->entry.render);}
        else if(!j->ok){for(int i=0;i<6&&g->thumb_first+i<g->count;i++)if(g->thumb_first>=0&&!strcmp(g->items[g->thumb_first+i].id,j->entry.id))g->thumb_failed[i]=true;}
        else prune_thumbnails(g);
    }
    if(g->view==2||g->tab==WP_GENRES)return;
    int first=g->selected/6*6;if(first!=g->thumb_first){thumbs_clear(g);g->thumb_first=first;}
    for(int i=0;i<6&&first+i<g->count;i++){
        NxtWallpaperEntry *w=&g->items[first+i];if(g->thumbs[i]||g->thumb_failed[i]||w->kind==NXT_WP_FOLDER)continue;
        if(w->kind==NXT_WP_PRESET){g->thumbs[i]=NxtAppearance_thumbnail(w->preset);g->dirty=true;continue;}
        char path[512];thumb_path(g,w,path);
        struct stat cached,original;if(w->kind==NXT_WP_LOCAL&&!stat(path,&cached)&&!stat(w->path,&original)&&original.st_mtime>cached.st_mtime)unlink(path);
        if(exists(path)){
            uint32_t x,y;if(NxtWallpaper_imageSize(path,&x,&y)&&(uint64_t)x*y<=2000000)g->thumbs[i]=IMG_Load(path);
            if(g->thumbs[i])g->dirty=true;else{unlink(path);g->thumb_failed[i]=true;}
        }else if(!busy(g)){
            if(w->kind==NXT_WP_LOCAL&&!NxtWallpaper_inside(g->root,w->path)){g->thumb_failed[i]=true;continue;}
            NxtWallpaperEntry source=*w;char full[512];bool local=w->kind==NXT_WP_LOCAL;
            if(w->kind==NXT_WP_ONLINE&&cached_image(g,w,full)){local=true;snprintf(source.path,sizeof(source.path),"%s",full);}
            if(!job_start(g,local?JOB_LOCAL_THUMB:JOB_THUMB,&source,local?NULL:w->thumb,path))g->thumb_failed[i]=true;break;
        }
    }
}
static void draw_tabs(SDL_Surface *s,Gallery *g){
    const char *tabs[]={Nxt_text("Discover","Khám phá"),Nxt_text("Categories","Thể loại"),Nxt_text("Favorites","Yêu thích"),Nxt_text("Downloaded","Đã tải"),Nxt_text("SD images","Ảnh trên thẻ")};int p=u(s,32),gap=u(s,6),w=(s->w-2*p-gap*4)/5,y=NxtSkin_top(s);
    for(int i=0;i<5;i++){int x=p+i*(w+gap);SDL_Color ink=NxtSkin_color(i==g->tab?NXT_ACCENT_INK:NXT_MUTED);if(i==g->tab)NxtSkin_rect(s,(SDL_Rect){x,y,w,u(s,40)},NxtSkin_color(NXT_ACCENT),u(s,8));NxtSkin_text(s,20,tabs[i],x+u(s,10),y+u(s,4),w-u(s,16),ink);}
}
static void draw_gallery(SDL_Surface *s,Gallery *g){
    const char *sub=g->tab==WP_CARD?Nxt_text("SD card","Thẻ nhớ"):g->tab==WP_FAVORITES?NxtPersonal_name():g->tab==WP_DOWNLOADED?Nxt_text("Offline","Offline"):source_name(g);
    NxtSkin_begin(s,Nxt_text("Wallpaper gallery","Kho hình nền"),sub,false);draw_tabs(s,g);
    int top=NxtSkin_top(s)+u(s,54),bottom=NxtSkin_bottom(s,false)-u(s,34),p=u(s,32),gap=u(s,12);
    if(g->tab==WP_CARD){const char *path=g->folder+strlen(g->root);char label[620];snprintf(label,sizeof(label),"%s%s  ·  JPG / PNG",Nxt_text("SD card","Thẻ nhớ"),*path?path:"/");NxtSkin_text(s,17,label,p,top,s->w-2*p,NxtSkin_color(NXT_MUTED));top+=u(s,31);}
    if(g->tab==WP_GENRES){
        int w=(s->w-2*p-gap*3)/4,h=(bottom-top-gap)/2;
        const int presets[]={11,7,9,3,16,20,2,21};int first=g->selected/8*8;
        for(int i=0;i<8&&first+i<NXT_WALLPAPER_GENRES;i++){int at=first+i;SDL_Rect r={p+(i%4)*(w+gap),top+(i/4)*(h+gap),w,h};if(!g->genre_thumbs[i])g->genre_thumbs[i]=NxtAppearance_thumbnail(presets[i]);if(g->genre_thumbs[i])fit(s,g->genre_thumbs[i],r);SDL_Color overlay=NxtSkin_color(NXT_BG);overlay.a=190;NxtSkin_rect(s,r,overlay,u(s,8));if(at==g->selected)NxtSkin_rect(s,(SDL_Rect){r.x,r.y,u(s,4),r.h},NxtSkin_color(NXT_ACCENT),0);NxtSkin_icon(s,genre_icons[at],r.x+u(s,14),r.y+u(s,20),u(s,29),NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,19,nxt.language==1?genre_vi[at]:genre_en[at],r.x+u(s,14),r.y+r.h-u(s,48),r.w-u(s,24),NxtSkin_color(NXT_INK));}
    }else if(!g->count){
        const char *empty=g->tab==WP_FAVORITES?Nxt_text("No favorites yet","Chưa có ảnh yêu thích"):g->tab==WP_DOWNLOADED?Nxt_text("No downloaded images","Chưa có ảnh đã tải"):g->tab==WP_CARD?Nxt_text("No JPG/PNG images in this folder","Thư mục này chưa có ảnh JPG/PNG"):busy(g)?Nxt_text("Loading online gallery…","Đang tải kho ảnh online…"):Nxt_text("No images; START for sources or retry","Chưa có ảnh; START chọn nguồn hoặc thử lại");
        NxtSkin_icon(s,g->tab==WP_CARD?"folder-open":"images",s->w/2-u(s,24),top+u(s,55),u(s,48),NxtSkin_color(NXT_ACCENT));NxtSkin_text(s,23,empty,p,top+u(s,123),s->w-2*p,NxtSkin_color(NXT_INK));
    }else{
        int w=(s->w-2*p-gap*2)/3,h=(bottom-top-gap)/2,first=g->selected/6*6;
        for(int i=0;i<6&&first+i<g->count;i++){
            NxtWallpaperEntry *item=&g->items[first+i];bool chosen=first+i==g->selected;SDL_Rect r={p+(i%3)*(w+gap),top+(i/3)*(h+gap),w,h};NxtSkin_rect(s,r,NxtSkin_color(chosen?NXT_ACCENT:NXT_PANEL),u(s,9));
            SDL_Rect photo={r.x+u(s,4),r.y+u(s,4),r.w-u(s,8),r.h-u(s,60)};
            if(item->kind==NXT_WP_FOLDER){NxtSkin_rect(s,photo,NxtSkin_color(NXT_PANEL),u(s,5));NxtSkin_icon(s,"folder-open",photo.x+photo.w/2-u(s,24),photo.y+photo.h/2-u(s,24),u(s,48),NxtSkin_color(NXT_ACCENT));}
            else if(g->thumbs[i])fit(s,g->thumbs[i],photo);else NxtSkin_icon(s,"image",photo.x+photo.w/2-u(s,17),photo.y+photo.h/2-u(s,17),u(s,34),NxtSkin_color(chosen?NXT_ACCENT_INK:NXT_MUTED));
            SDL_Color ink=NxtSkin_color(chosen?NXT_ACCENT_INK:NXT_INK),muted=chosen?ink:NxtSkin_color(NXT_MUTED);
            NxtSkin_text(s,20,item->name,r.x+u(s,11),r.y+r.h-u(s,55),r.w-u(s,22),ink);
            char path[512];const char *label=item->kind==NXT_WP_FOLDER?Nxt_text("Folder · A Open","Thư mục · A Mở"):item->kind==NXT_WP_LOCAL?Nxt_text("SD card","Trên thẻ"):item->kind==NXT_WP_PRESET?Nxt_text("Built-in","Có sẵn"):cached_image(g,item,path)?Nxt_text("Downloaded","Đã tải"):"Online";
            NxtSkin_text(s,16,label,r.x+u(s,11),r.y+r.h-u(s,28),r.w-u(s,50),muted);if(favorite_index(g,item)>=0)NxtSkin_icon(s,"heart",r.x+r.w-u(s,28),r.y+r.h-u(s,28),u(s,17),ink);
        }
    }
    char caption[300];if(g->status[0])snprintf(caption,sizeof(caption),"%s",g->status);
    else if(g->tab==WP_GENRES)snprintf(caption,sizeof(caption),Nxt_text("24 categories · Page %d/3 · L2/R2 pages","24 thể loại · Trang %d/3 · L2/R2 đổi trang"),g->selected/8+1);
    else if(g->tab==WP_EXPLORE&&g->source!=2)snprintf(caption,sizeof(caption),Nxt_text("Source: %d images · Page %d/%d · Scroll / R2 for more","Nguồn: %d ảnh · Trang %d/%d · Cuộn / R2 tải tiếp"),g->total,g->page,g->last_page);
    else snprintf(caption,sizeof(caption),"%s%s",g->limited?Nxt_text("First 512 entries; use subfolders · ","512 mục đầu; chọn thư mục con · "):"",Nxt_text("L2/R2: pages","L2/R2: đổi trang"));
    NxtSkin_text(s,17,caption,p,bottom+u(s,6),s->w-2*p,NxtSkin_color(NXT_MUTED));
    NxtSkin_footer(s,g->tab==WP_GENRES?Nxt_text("A Open category · L1/R1 Tabs · START Source","A Mở thể loại · L1/R1 Tab · START Nguồn"):g->tab==WP_CARD?Nxt_text("A Open · X Use · Y Like · L1/R1 Tab · L2/R2 Page","A Mở · X Dùng nền · Y Thích · L1/R1 Tab · L2/R2 Trang"):Nxt_text("A View · X Download · Y Like · L1/R1 Tab · START Source","A Xem · X Tải · Y Thích · L1/R1 Tab · START Nguồn"),false);
}
static void detail_image(Gallery *g,const NxtWallpaperEntry *w,SDL_Rect photo){
    char path[512];if(w->kind==NXT_WP_PRESET)return;if(!cached_image(g,w,path))return;
    char key[620];snprintf(key,sizeof(key),"%s/%dx%d/%d",path,photo.w,photo.h,w->render);if(!strcmp(key,g->detail_key))return;
    detail_clear(g);SDL_Surface *full=IMG_Load(path);if(!full)return;
    double scale=fmin((double)photo.w/full->w,(double)photo.h/full->h);int width=fmax(1,floor(full->w*scale)),height=fmax(1,floor(full->h*scale));
    g->detail=NxtWallpaper_resample(full,width,height,false,w->render);SDL_FreeSurface(full);if(g->detail)snprintf(g->detail_key,sizeof(g->detail_key),"%s",key);
}
static void draw_detail(SDL_Surface *s,Gallery *g){
    NxtWallpaperEntry *w=&g->items[g->selected];NxtSkin_begin(s,Nxt_text("Wallpaper preview","Xem hình nền"),NxtPersonal_name(),false);int p=u(s,32),top=NxtSkin_top(s),bottom=NxtSkin_bottom(s,false),pw=u(s,540),x=p+pw+u(s,25),width=s->w-p-x;
    SDL_Rect photo={p,top,pw,bottom-top-u(s,35)};NxtSkin_rect(s,photo,NxtSkin_color(NXT_PANEL),u(s,10));detail_image(g,w,photo);int slot=g->selected%6;
    if(g->detail){SDL_Rect at={photo.x+(photo.w-g->detail->w)/2,photo.y+(photo.h-g->detail->h)/2,0,0};SDL_BlitSurface(g->detail,NULL,s,&at);}
    else if(g->thumbs[slot])fit(s,g->thumbs[slot],photo);else NxtSkin_icon(s,"image",photo.x+photo.w/2-u(s,28),photo.y+photo.h/2-u(s,28),u(s,56),NxtSkin_color(NXT_ACCENT));
    NxtSkin_text(s,25,w->name,x,top,width,NxtSkin_color(NXT_INK));NxtSkin_text(s,18,origin(w),x,top+u(s,48),width,NxtSkin_color(NXT_MUTED));
    Nxt_drawWrapped(s,NxtSkin_font(s,18),w->kind==NXT_WP_LOCAL?w->name:w->kind==NXT_WP_PRESET?Nxt_text("Original NXT background. No download needed.","Nền có sẵn của NXT. Không cần tải."):Nxt_text("Online image. Preview your menu before applying.","Ảnh online. Xem thử trên menu trước khi áp dụng."),x,top+u(s,83),width,u(s,82),NxtSkin_color(NXT_MUTED));
    const char *buttons[]={Nxt_text("A · View on menu","A · Xem trên menu"),w->kind==NXT_WP_ONLINE?Nxt_text("X · Save offline","X · Lưu offline"):Nxt_text("X · Use wallpaper","X · Dùng làm nền"),favorite_index(g,w)>=0?Nxt_text("Y · Liked","Y · Đã yêu thích"):Nxt_text("Y · Favorite","Y · Yêu thích"),w->render==NXT_WP_PIXEL?Nxt_text("L2 · Preserve pixels","L2 · Giữ nét pixel"):Nxt_text("L2 · Smooth image","L2 · Mượt")};
    for(int i=0;i<4;i++){int y=top+u(s,170+i*55);NxtSkin_rect(s,(SDL_Rect){x,y,width,u(s,46)},NxtSkin_color(i==0?NXT_ACCENT:NXT_PANEL),u(s,9));NxtSkin_text(s,20,buttons[i],x+u(s,14),y+u(s,7),width-u(s,24),NxtSkin_color(i==0?NXT_ACCENT_INK:NXT_INK));}
    NxtSkin_text(s,17,g->status,x,top+u(s,400),width,NxtSkin_color(NXT_MUTED));if(busy(g)&&g->job.type==JOB_IMAGE){struct stat st;double progress=!stat(g->job.temp,&st)&&w->bytes?(double)st.st_size/w->bytes:0;if(progress>1)progress=1;SDL_Rect bar={x,top+u(s,430),width,u(s,4)};NxtSkin_rect(s,bar,NxtSkin_color(NXT_PANEL),0);bar.w=width*progress;NxtSkin_rect(s,bar,NxtSkin_color(NXT_ACCENT),0);}
    NxtSkin_footer(s,Nxt_text("A Preview · X Save/use · Y Like · L2 Smooth/pixel","A Xem menu · X Lưu/dùng · Y Thích · L2 Mượt/pixel"),false);
}
static void draw_preview(SDL_Surface *s,Gallery *g){
    NxtSkin_homeHint(s,4,Nxt_text("A Apply · ←/→ Cover · SELECT Dark/light","A Áp dụng · ←/→ Độ phủ · SELECT Sáng/tối"));char label[160];snprintf(label,sizeof(label),"%s: %d%%",Nxt_text("Menu cover","Độ phủ ô menu"),nxt.panel_opacity);NxtSkin_text(s,17,label,u(s,38),u(s,551),u(s,330),NxtSkin_color(NXT_INK));
    SDL_Rect track={u(s,380),u(s,566),u(s,580),u(s,5)};NxtSkin_rect(s,track,NxtSkin_color(NXT_LINE),0);track.w=track.w*nxt.panel_opacity/100;NxtSkin_rect(s,track,NxtSkin_color(NXT_ACCENT),0);
}
static ModuleExitReason source_menu(DisplayContext *d,Gallery *g){
    int sel=g->source;const char *rows[]={Nxt_text("NXT picks · online Wallhaven","NXT chọn lọc · online Wallhaven"),Nxt_text("Wallhaven · latest images","Wallhaven · ảnh mới"),Nxt_text("NXT built-in backgrounds · offline","Nền NXT có sẵn · offline"),Nxt_text("Refresh current online collection","Làm mới bộ ảnh online đang xem")};
    NxtPick p=Nxt_pick(d,Nxt_text("Wallpaper source","Nguồn hình nền"),rows,4,&sel,Nxt_text("A Select · B Back","A Chọn · B Về"));if(p.action==9)return MODULE_EXIT_QUIT;
    if(p.action==1){job_stop(g);g->pending=0;if(sel<3){g->source=sel;g->genre=-1;}g->tab=WP_EXPLORE;g->page=1;g->selected=0;refresh(g,sel==3);}return MODULE_EXIT_TO_MENU;
}
static void turn_page(Gallery *g,int delta){
    if(g->tab==WP_GENRES){g->selected=(g->selected/8*8+delta*8+NXT_WALLPAPER_GENRES)%NXT_WALLPAPER_GENRES;g->dirty=true;return;}int target=g->selected/6*6+delta*6;
    if(target>=0&&target<g->count){g->selected=target;g->dirty=true;return;}
    if(g->tab==WP_EXPLORE&&g->source!=2&&delta>0&&g->page<g->last_page)load_more(g);
    else if(g->tab==WP_EXPLORE&&g->source!=2&&delta<0&&g->first_page>1){job_stop(g);g->pending=0;g->page=g->first_page-1;g->selected=0;refresh(g,false);}
    else if(g->count){g->selected=delta>0?0:(g->count-1)/6*6;g->dirty=true;}
}
ModuleExitReason NxtWallpapers_run(DisplayContext *d){
    Gallery *g=calloc(1,sizeof(*g));if(!g)return MODULE_EXIT_TO_MENU;g->items=calloc(NXT_WALLPAPER_CAP,sizeof(*g->items));g->favorites=calloc(NXT_WALLPAPER_FAVORITES,sizeof(*g->favorites));if(!g->items||!g->favorites){free(g->items);free(g->favorites);free(g);return MODULE_EXIT_TO_MENU;}
    g->genre=-1;g->page=g->last_page=1;g->thumb_first=-1;g->dirty=true;snprintf(g->root,sizeof(g->root),"%s",SDCARD_PATH);snprintf(g->folder,sizeof(g->folder),"%s",g->root);
    const char *first[]={"Pictures/Wallpapers","Pictures","Wallpapers","DCIM"};for(int i=0;i<4;i++){char path[512];snprintf(path,sizeof(path),"%s/%s",g->root,first[i]);struct stat st;if(!stat(path,&st)&&S_ISDIR(st.st_mode)){snprintf(g->folder,sizeof(g->folder),"%s",path);break;}}
    userdata_mkdir("wallpapers/files");userdata_mkdir("wallpapers/thumbs");userdata_snpath("wallpapers",g->cache,sizeof(g->cache));char rel[160];snprintf(rel,sizeof(rel),"listeners/%d",NxtPersonal_id());userdata_mkdir(rel);snprintf(rel,sizeof(rel),"listeners/%d/wallpaper-favorites.json",NxtPersonal_id());userdata_snpath(rel,g->favfile,sizeof(g->favfile));g->nfav=NxtWallpaper_readEntries(g->favfile,g->favorites,NXT_WALLPAPER_FAVORITES);
    prune_thumbnails(g);refresh(g,false);int show=0;uint32_t drawn=0;ModuleExitReason result=MODULE_EXIT_TO_MENU;BackgroundPlayerType previous=Background_getActive();if(previous==BG_NONE&&PlayerModule_isActive())Background_setActive(BG_MUSIC);
    for(;;){
        ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);GlobalInputResult global=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);if(global.should_quit){result=MODULE_EXIT_QUIT;break;}tick(g);
        if(!global.input_consumed&&!Nxt_lockInput()){
            if(NxtPad_justPressed(BTN_B)){
                g->pending=0;
                if(g->view==2){restore_preview(g);g->view=1;g->dirty=true;}
                else if(g->view==1){g->view=0;detail_clear(g);if(g->tab==WP_FAVORITES)refresh(g,false);g->dirty=true;}
                else if(g->tab==WP_CARD&&strcmp(g->folder,g->root)){char *slash=strrchr(g->folder,'/');if(slash&&slash>=g->folder+strlen(g->root))*slash=0;g->selected=0;refresh(g,false);}
                else if(g->tab!=WP_EXPLORE){job_stop(g);g->tab=WP_EXPLORE;g->selected=0;refresh(g,false);}
                else break;
            }else if(g->view==2){
                if(NxtPad_justRepeated(BTN_LEFT)||NxtPad_justRepeated(BTN_RIGHT)){nxt.panel_opacity+=NxtPad_justRepeated(BTN_LEFT)?-5:5;if(nxt.panel_opacity<15)nxt.panel_opacity=15;if(nxt.panel_opacity>85)nxt.panel_opacity=85;g->dirty=true;}
                if(NxtPad_justPressed(BTN_SELECT)){nxt.theme=nxt.theme==1?2:1;NxtAppearance_release();g->dirty=true;}
                if(NxtPad_justPressed(BTN_A)){bool ok=Nxt_save();if(ok){g->preview=false;g->view=0;Toast_show(Nxt_text("Wallpaper applied to this profile","Đã áp dụng nền cho hồ sơ này"),2500);}else Toast_show(Nxt_text("Cannot save; export app log","Không lưu được; hãy xuất log app"),2500);NxtLog_step("wallpaper","preview-apply",ok,"profile=%d",NxtPersonal_id());g->dirty=true;}
            }else{
                if(g->view==0){
                    if(NxtPad_justPressed(BTN_L1)||NxtPad_justPressed(BTN_R1)){job_stop(g);g->pending=0;g->tab=(g->tab+5+(NxtPad_justPressed(BTN_L1)?-1:1))%5;g->selected=0;g->page=1;refresh(g,false);}
                    else if(NxtPad_justPressed(BTN_L2)||NxtPad_justPressed(BTN_R2))turn_page(g,NxtPad_justPressed(BTN_L2)?-1:1);
                    else{int delta=0,cols=g->tab==WP_GENRES?4:3;if(NxtPad_justRepeated(BTN_LEFT))delta=-1;else if(NxtPad_justRepeated(BTN_RIGHT))delta=1;else if(NxtPad_justRepeated(BTN_UP))delta=-cols;else if(NxtPad_justRepeated(BTN_DOWN))delta=cols;if(delta&&g->count){int target=g->selected+delta;if(target>=g->count&&delta>0&&g->tab==WP_EXPLORE&&g->source!=2&&g->page<g->last_page)load_more(g);else{if(target<0)target=0;if(target>=g->count)target=g->count-1;g->selected=target;g->dirty=true;}}}
                }
                else if(g->view==1&&NxtPad_justPressed(BTN_L2))toggle_render(g);
                if(NxtPad_justPressed(BTN_A)&&g->count){
                    if(g->tab==WP_GENRES){g->genre=g->selected;g->source=g->source==2?0:g->source;g->tab=WP_EXPLORE;g->selected=0;g->page=1;job_stop(g);refresh(g,true);}
                    else if(g->view==0){NxtWallpaperEntry w=g->items[g->selected];if(w.kind==NXT_WP_FOLDER){snprintf(g->folder,sizeof(g->folder),"%s",w.path);g->selected=0;refresh(g,false);}else{g->view=1;detail_clear(g);request_image(g,w,3);g->dirty=true;}}
                    else request_image(g,g->items[g->selected],1);
                }
                if(g->tab!=WP_GENRES&&g->count){
                    if(NxtPad_justPressed(BTN_X)){NxtWallpaperEntry w=g->items[g->selected];request_image(g,w,w.kind==NXT_WP_ONLINE?0:2);}
                    if(NxtPad_justPressed(BTN_Y))favorite(g,g->items[g->selected]);
                }
                if(NxtPad_justPressed(BTN_START)){
                    if(g->view==1&&busy(g)){job_stop(g);g->pending=0;g->status[0]=0;Toast_show(Nxt_text("Image job cancelled","Đã hủy tải ảnh"),1800);g->dirty=true;}
                    else if(g->view==0){if(source_menu(d,g)==MODULE_EXIT_QUIT){result=MODULE_EXIT_QUIT;break;}g->dirty=true;}
                    else if(g->count)request_image(g,g->items[g->selected],1);
                }
            }
        }
        if(NxtSkin_shouldDraw(drawn,g->dirty||global.dirty||(busy(g)&&SDL_GetTicks()-drawn>600))){if(g->view==2)draw_preview(s,g);else if(g->view==1&&g->count)draw_detail(s,g);else draw_gallery(s,g);ModuleCommon_markSurfaceDrawn();drawn=SDL_GetTicks();g->dirty=false;}
        int dirty=0;ModuleCommon_PWR_update(&dirty,&show);if(dirty)g->dirty=true;ModuleCommon_frameEnd(s);
    }
    restore_preview(g);job_stop(g);thumbs_clear(g);detail_clear(g);for(int i=0;i<8;i++)if(g->genre_thumbs[i])SDL_FreeSurface(g->genre_thumbs[i]);if(previous==BG_NONE&&Background_getActive()==BG_MUSIC)Background_setActive(BG_NONE);free(g->items);free(g->favorites);free(g);return result;
}
