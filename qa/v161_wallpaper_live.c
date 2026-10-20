/* Real HTTPS production-worker smoke test; no hardware/audio assertions. */
#include "nxt_wallpapers.c"
#include <assert.h>
void NxtLog_step(const char*a,const char*b,bool ok,const char*c,...){}
static void finish(Gallery *g){for(int i=0;i<2000;i++){if(__atomic_load_n(&g->job.done,__ATOMIC_ACQUIRE)){pthread_join(g->job.thread,NULL);g->job.joined=0;if(!g->job.ok)fprintf(stderr,"Live worker failed: type=%d; file=%s\n",g->job.type,g->job.target);assert(g->job.ok);return;}usleep(50000);}job_stop(g);assert(!"live request timed out");}
int main(int argc,char **argv){
 assert(argc==2);Gallery g={0};g.genre=-1;g.page=1;g.items=calloc(NXT_WALLPAPER_CAP,sizeof(*g.items));assert(g.items);snprintf(g.cache,sizeof(g.cache),"%s",argv[1]);char path[512],url[1200];
 catalog_path(&g,path);catalog_url(&g,url);assert(job_start(&g,JOB_CATALOG,NULL,url,path));finish(&g);assert(read_catalog(&g)>0&&g.total>1000&&g.last_page>1);int first=g.count,total=g.total;
 g.page=2;g.catalog_append=true;catalog_path(&g,path);catalog_url(&g,url);assert(job_start(&g,JOB_CATALOG,NULL,url,path));finish(&g);assert(read_catalog(&g)>0&&g.count>first);NxtWallpaperEntry selected=g.items[0];
 snprintf(path,sizeof(path),"%s/live-thumb.png",argv[1]);assert(job_start(&g,JOB_THUMB,&selected,selected.thumb,path));finish(&g);uint32_t x,y;assert(NxtWallpaper_imageSize(path,&x,&y));
 image_path(&g,&selected,path);assert(job_start(&g,JOB_IMAGE,&selected,selected.url,path));finish(&g);assert(NxtWallpaper_imageSize(path,&x,&y)&&x==1024&&y==768);char original[512];original_path(&g,&selected,original);assert(usable_original(original));
 selected.render=NXT_WP_PIXEL;image_path(&g,&selected,path);assert(job_start(&g,JOB_IMAGE,&selected,"https://fixture.invalid/no-network",path));finish(&g);assert(NxtWallpaper_imageSize(path,&x,&y)&&x==1024&&y==768);
 printf("PASS actual HTTPS worker / bundled CA: source total %d, %d pages; %d compatible SFW images across two pages; full original retained, screen-sized PNG %ux%u, offline pixel reprocessing.\n",total,g.last_page,g.count,x,y);free(g.items);return 0;
}
