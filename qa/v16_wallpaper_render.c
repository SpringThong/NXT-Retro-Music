/* Production gallery/store regression, with explicit hardware fixture boundaries. */
#define IMG_Load Fixture_IMG_Load
#define IMG_SavePNG Fixture_IMG_SavePNG
#define NxtSort_compare Fixture_NxtSort_compare
#define ModuleCommon_frameBegin Fixture_frameBegin
#define ModuleCommon_frameEnd Fixture_frameEnd
#define NXT_UI_FIXTURE_MAIN unused_v15_ui_main
#include "v15_ui_render.c"
#undef IMG_Load
#undef IMG_SavePNG
#undef NxtSort_compare
#undef ModuleCommon_frameBegin
#undef ModuleCommon_frameEnd
extern SDL_Surface *IMG_Load(const char *);
extern int IMG_SavePNG(SDL_Surface *,const char *);
#define busy gallery_busy
#define favorite gallery_favorite
#include "nxt_wallpapers.c"
#undef busy
#undef favorite
static void screenshot(const char *name){char path[1400];snprintf(path,sizeof(path),"%s/%s-%d.bmp",out_dir,name,page->w);assert(!SDL_SaveBMP(page,path));}
void ModuleCommon_frameBegin(void){
 assert(++frame_no<110);pressed_keys=0;released_keys=0;ticks+=100;
 switch(frame_no){case 5:case 10:case 15:case 20:pressed_keys=BTN_R1;break;case 26:case 32:pressed_keys=BTN_A;break;case 62:pressed_keys=BTN_LEFT;break;case 64:case 70:case 74:case 76:case 78:case 80:pressed_keys=BTN_B;break;case 68:pressed_keys=BTN_Y;break;}
 usleep(20000);
}
void ModuleCommon_frameEnd(SDL_Surface *s){if(frame_no==61)screenshot("flow-preview");}
static Gallery *gallery(const char *dir){
 Gallery *g=calloc(1,sizeof(*g));assert(g);g->items=calloc(NXT_WALLPAPER_CAP,sizeof(*g->items));g->favorites=calloc(NXT_WALLPAPER_FAVORITES,sizeof(*g->favorites));assert(g->items&&g->favorites);g->genre=-1;g->page=g->last_page=1;g->thumb_first=-1;
 snprintf(g->root,sizeof(g->root),"%s",SDCARD_PATH);snprintf(g->folder,sizeof(g->folder),"%s",dir);userdata_snpath("wallpapers",g->cache,sizeof(g->cache));userdata_snpath("listeners/0/wallpaper-favorites.json",g->favfile,sizeof(g->favfile));return g;
}
static void cleanup(Gallery *g){restore_preview(g);job_stop(g);thumbs_clear(g);for(int i=0;i<8;i++)if(g->genre_thumbs[i])SDL_FreeSurface(g->genre_thumbs[i]);free(g->items);free(g->favorites);free(g);}
static void wait_image(Gallery *g){for(int i=0;i<100;i++){if(__atomic_load_n(&g->job.done,__ATOMIC_ACQUIRE)){pthread_join(g->job.thread,NULL);g->job.joined=0;assert(g->job.ok);return;}usleep(20000);}assert(!"image job timed out");}
static void catalog_checks(void){
 NxtWallpaperEntry w[24];int pages;const char *good="{\"meta\":{\"last_page\":9},\"data\":[{\"id\":\"a12345\",\"purity\":\"sfw\",\"file_type\":\"image/jpeg\",\"path\":\"https://w.wallhaven.cc/full/a1/a12345.jpg\",\"thumbs\":{\"large\":\"https://th.wallhaven.cc/lg/a1/a12345.jpg\"},\"dimension_x\":1920,\"dimension_y\":1080,\"file_size\":100000}]}";
 assert(NxtWallpaper_parseCatalog(good,w,24,&pages)==1&&pages==9&&!strcmp(w[0].id,"wh-a12345"));
 assert(NxtWallpaper_https(w[0].url)&&!NxtWallpaper_https("http://x/image.png")&&!NxtWallpaper_https("https://user@x/image.png")&&!NxtWallpaper_https("https://x/a b")&&!NxtWallpaper_https("https://x\\bad/a")&&!NxtWallpaper_https("file:///tmp/a.png"));
 JSON_Value *v=json_parse_string(good);JSON_Array *a=json_object_get_array(json_value_get_object(v),"data");JSON_Object *r=json_array_get_object(a,0);
 const char *bad[]={"nsfw","sketchy",NULL};for(int i=0;i<3;i++){if(bad[i])json_object_set_string(r,"purity",bad[i]);else json_object_remove(r,"purity");char *t=json_serialize_to_string(v);assert(!NxtWallpaper_parseCatalog(t,w,24,NULL));json_free_serialized_string(t);}json_object_set_string(r,"purity","sfw");
 json_object_set_number(r,"dimension_x",8193);char *t=json_serialize_to_string(v);assert(!NxtWallpaper_parseCatalog(t,w,24,NULL));json_free_serialized_string(t);json_object_set_number(r,"dimension_x",5000);json_object_set_number(r,"dimension_y",5000);t=json_serialize_to_string(v);assert(!NxtWallpaper_parseCatalog(t,w,24,NULL));json_free_serialized_string(t);json_object_set_number(r,"dimension_x",1920);json_object_set_number(r,"dimension_y",1080);json_object_set_number(r,"file_size",NXT_WALLPAPER_BYTES+1);t=json_serialize_to_string(v);assert(!NxtWallpaper_parseCatalog(t,w,24,NULL));json_free_serialized_string(t);json_value_free(v);
 assert(NxtWallpaper_parseCatalog("<html>sign in</html>",w,24,NULL)==-1&&NxtWallpaper_parseCatalog("{\"other\":[]}",w,24,NULL)==-1);
 puts("PASS real catalog parser: pagination, SFW only, HTTPS/no credentials, malformed HTML, image bounds and byte cap.");
}
static void storage_checks(const char *dir){
 NxtWallpaperEntry w[32];bool limited;int n=NxtWallpaper_scan(SDCARD_PATH,dir,w,32,&limited);assert(n==5&&!limited);const char *names[]={"1.png","2.jpg","9.png","10.png","Alpha.png"};for(int i=0;i<5;i++)assert(!strcmp(w[i].name,names[i]));assert(NxtWallpaper_scan(SDCARD_PATH,dir,w,2,&limited)==2&&limited);assert(NxtWallpaper_scan(SDCARD_PATH,"/tmp",w,32,&limited)==-1);assert(NxtWallpaper_inside(SDCARD_PATH,dir)&&!NxtWallpaper_inside(SDCARD_PATH,"/tmp"));
 char normalized[512],thumb[512];userdata_snpath("normalized.png",normalized,sizeof(normalized));userdata_snpath("small.png",thumb,sizeof(thumb));char input[512];snprintf(input,sizeof(input),"%s/2.jpg",dir);assert(NxtWallpaper_normalize(input,normalized)&&NxtWallpaper_thumbnail(input,thumb));uint32_t x,y;assert(NxtWallpaper_imageSize(normalized,&x,&y)&&x<=1024&&y<=768);assert(NxtWallpaper_imageSize(thumb,&x,&y)&&x<=320&&y<=240);
 userdata_mkdir("listeners/0");userdata_mkdir("listeners/2");char favorites[512],other[512];userdata_snpath("listeners/0/wallpaper-favorites.json",favorites,sizeof(favorites));userdata_snpath("listeners/2/wallpaper-favorites.json",other,sizeof(other));NxtWallpaper_scan(SDCARD_PATH,dir,w,32,&limited);assert(NxtWallpaper_writeEntries(favorites,w,5));NxtWallpaperEntry a[32];assert(NxtWallpaper_readEntries(favorites,a,32)==5&&!strcmp(a[1].path,w[1].path));assert(!NxtWallpaper_readEntries(other,a,32));assert(NxtWallpaper_writeEntries(favorites,w,0)&&!NxtWallpaper_readEntries(favorites,a,32));
 puts("PASS real JPG/PNG resize/thumbnail, natural SD sort, ignored symlinks/nonimages, SD boundary, truncated directory notice, atomic favorites and separate profile files.");
}
int main(int argc,char **argv){
 assert(argc==3&&!SDL_Init(0)&&!TTF_Init());out_dir=argv[1];const char *dir=argv[2];nxt.language=1;nxt.theme=1;nxt.font_size=100;nxt.lyric_size=28;nxt.wallpaper_dim=15;nxt.panel_opacity=25;nxt.wallpaper_mode=11;nxt.accent_color=2;NxtPersonal_init();typed_name="spring";assert(create_listener());typed_name=NULL;userdata_mkdir("wallpapers/files");userdata_mkdir("wallpapers/thumbs");catalog_checks();storage_checks(dir);controls();
 for(int size=0;size<2;size++){
  page=SDL_CreateRGBSurfaceWithFormat(0,size?640:1024,size?480:768,32,SDL_PIXELFORMAT_ARGB8888);assert(page);render_efficiency_checks();Gallery *g=gallery(dir);g->tab=WP_CARD;refresh(g,false);assert(g->count==5);
  for(int i=0;i<5;i++){char path[512];thumb_path(g,&g->items[i],path);assert(NxtWallpaper_thumbnail(g->items[i].path,path));g->thumbs[i]=IMG_Load(path);assert(g->thumbs[i]);}g->thumb_first=0;draw_gallery(page,g);screenshot("sd-gallery");g->view=1;draw_detail(page,g);screenshot("sd-detail");
  int oldmode=nxt.wallpaper_mode,oldpanel=nxt.panel_opacity,oldtheme=nxt.theme;char target[512];image_path(g,&g->items[0],target);assert(NxtWallpaper_normalize(g->items[0].path,target));assert(set_wallpaper(g,&g->items[0],false));nxt.panel_opacity=15;draw_preview(page,g);screenshot("menu-preview");restore_preview(g);assert(nxt.wallpaper_mode==oldmode&&nxt.panel_opacity==oldpanel&&nxt.theme==oldtheme);g->view=0;
  gallery_favorite(g,g->items[0]);assert(g->nfav==1);g->tab=WP_FAVORITES;refresh(g,false);draw_gallery(page,g);screenshot("favorites");g->tab=WP_GENRES;refresh(g,false);draw_gallery(page,g);screenshot("categories");SDL_Surface *keep=g->genre_thumbs[0];draw_gallery(page,g);assert(keep==g->genre_thumbs[0]);
  g->tab=WP_EXPLORE;g->source=2;refresh(g,false);for(int i=0;i<6;i++)g->thumbs[i]=NxtAppearance_thumbnail(g->items[i].preset);g->thumb_first=0;draw_gallery(page,g);screenshot("built-in");
  NxtWallpaperEntry online={.kind=NXT_WP_ONLINE,.genre=3};strcpy(online.id,"wh-live");strcpy(online.name,"Wallhaven · pomle9");strcpy(online.url,"https://fixture.example/full.jpg");strcpy(online.thumb,"https://fixture.example/thumb.jpg");char file[512];image_path(g,&online,file);assert(job_start(g,JOB_IMAGE,&online,online.url,file));wait_image(g);assert(exists(file));char meta[540];snprintf(meta,sizeof(meta),"%s.json",file);NxtWallpaperEntry saved;assert(NxtWallpaper_readEntries(meta,&saved,1)==1);g->tab=WP_DOWNLOADED;refresh(g,false);assert(g->count==1);char missing_thumb[512];thumb_path(g,&g->items[0],missing_thumb);unlink(missing_thumb);tick(g);assert(g->job.joined&&g->job.type==JOB_LOCAL_THUMB);wait_image(g);char small[512];thumb_path(g,&g->items[0],small);assert(NxtWallpaper_thumbnail(file,small));g->thumbs[0]=IMG_Load(small);g->thumb_first=0;draw_gallery(page,g);screenshot("downloaded");
  g->tab=WP_EXPLORE;g->source=0;g->count=6;for(int i=0;i<6;i++){g->items[i]=online;snprintf(g->items[i].id,sizeof(g->items[i].id),"wh-demo%d",i);snprintf(g->items[i].name,sizeof(g->items[i].name),"Wallhaven · SFW %d",i+1);if(g->thumbs[i])SDL_FreeSurface(g->thumbs[i]);g->thumbs[i]=IMG_Load(small);}draw_gallery(page,g);screenshot("online-gallery");
  assert(job_start(g,JOB_CATALOG,NULL,"https://fixture.example/slow",file));usleep(50000);job_stop(g);int child;assert(waitpid(-1,&child,WNOHANG)==-1&&errno==ECHILD);cleanup(g);SDL_FreeSurface(page);
 }
 page=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);frame_no=0;pressed_keys=released_keys=0;int oldmode=nxt.wallpaper_mode,oldpanel=nxt.panel_opacity;assert(NxtWallpapers_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU);assert(nxt.wallpaper_mode==oldmode&&nxt.panel_opacity==oldpanel);SDL_FreeSurface(page);NxtPersonal_quit();NxtSkin_release();TTF_Quit();SDL_Quit();puts("PASS actual gallery render (5 tabs, 8 categories, preview/details) at 640/1024; online image worker, offline metadata, cache reuse, cancel/reap; real UI input loop and preview rollback. Hardware functions are fixtures; no device test claimed.");return 0;
}
