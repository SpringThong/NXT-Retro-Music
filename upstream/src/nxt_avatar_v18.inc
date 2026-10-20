typedef struct {char path[512];SDL_Surface *image,*scaled[2];int edge[2];unsigned used;} AvatarImage;
static AvatarImage avatar_images[12];static unsigned avatar_clock;
static SDL_Surface *fit_photo(SDL_Surface *raw){
 if(!raw||raw->w<=0||raw->h<=0)return NULL;SDL_Surface *out=SDL_CreateRGBSurfaceWithFormat(0,160,160,32,SDL_PIXELFORMAT_ARGB8888);if(!out)return NULL;
 SDL_FillRect(out,NULL,0);int w=160,h=160;if(raw->w>raw->h)h=160LL*raw->h/raw->w;else w=160LL*raw->w/raw->h;if(!NxtSkin_smoothScale(raw,NULL,out,&(SDL_Rect){(160-w)/2,(160-h)/2,w,h}))SDL_BlitScaled(raw,NULL,out,&(SDL_Rect){(160-w)/2,(160-h)/2,w,h});SDL_SetSurfaceBlendMode(out,SDL_BLENDMODE_BLEND);return out;
}
static SDL_Surface *avatar_image(const char *path){
 AvatarImage *p=NULL,*old=&avatar_images[0];for(int i=0;i<12;i++){AvatarImage *q=&avatar_images[i];if(q->image&&!strcmp(q->path,path)){q->used=++avatar_clock;return q->image;}if(!q->image)p=q;if(q->used<old->used)old=q;}
 if(!p)p=old;if(p->image)SDL_FreeSurface(p->image);for(int k=0;k<2;k++)SDL_FreeSurface(p->scaled[k]);memset(p,0,sizeof(*p));uint32_t w,h;
 if(!NxtWallpaper_imageSize(path,&w,&h)||w>8192||h>8192||(uint64_t)w*h>16000000)return NULL;SDL_Surface *raw=IMG_Load(path);if(raw){p->image=fit_photo(raw);SDL_FreeSurface(raw);}snprintf(p->path,sizeof(p->path),"%s",path);p->used=++avatar_clock;return p->image;
}
static void avatar_blit(SDL_Surface *s,SDL_Surface *raw,int x,int y,int size){
 for(int i=0;i<12;i++){AvatarImage *a=&avatar_images[i];if(a->image!=raw)continue;
  int slot=a->edge[0]==size?0:a->edge[1]==size?1:!a->scaled[0]?0:1;
  if(a->edge[slot]!=size){SDL_FreeSurface(a->scaled[slot]);a->scaled[slot]=SDL_CreateRGBSurfaceWithFormat(0,size,size,32,SDL_PIXELFORMAT_ARGB8888);a->edge[slot]=size;
   if(a->scaled[slot]&&!NxtSkin_smoothScale(raw,NULL,a->scaled[slot],NULL)){SDL_FreeSurface(a->scaled[slot]);a->scaled[slot]=NULL;}if(a->scaled[slot])SDL_SetSurfaceBlendMode(a->scaled[slot],SDL_BLENDMODE_BLEND);}
  if(a->scaled[slot])SDL_BlitSurface(a->scaled[slot],NULL,s,&(SDL_Rect){x,y,0,0});return;
 }
}
void NxtCustom_drawAvatar(SDL_Surface *s,int mode,int icon,int em,const char *letters,const char *path,int x,int y,int size,SDL_Color c){
 if(mode==1){char file[96];if(em<0||em>=180)em=0;snprintf(file,sizeof(file),"./res/emoji3d/%03d.png",em);SDL_Surface *p=avatar_image(file);if(p)avatar_blit(s,p,x,y,size);else glyph(s,"?",x,y,size,c);}
 else if(mode==2)glyph(s,letters&&*letters?letters:"NXT",x,y,size,c);
 else if(mode==3&&path&&*path){SDL_Surface *p=avatar_image(path);if(p)avatar_blit(s,p,x,y,size);else glyph(s,"?",x,y,size,c);}
 else NxtSkin_icon(s,NxtCustom_iconName(icon),x,y,size,c);
}
bool NxtCustom_draw(SDL_Surface *s,const char *name,int x,int y,int size,SDL_Color c){
 if(!strncmp(name,"emoji-",6)){char *end;long i=strtol(name+6,&end,10);if(!*end&&i>=0&&i<180)NxtCustom_drawAvatar(s,1,0,i,NULL,NULL,x,y,size,c);return true;}
 if(strcmp(name,"personal-avatar"))return false;NxtCustom_drawAvatar(s,nxt.avatar_mode,nxt.avatar_icon,nxt.avatar_emoji,nxt.avatar_text,nxt.avatar_path,x,y,size,c);return true;
}
void NxtCustom_release(void){for(int i=0;i<12;i++){if(avatar_images[i].image)SDL_FreeSurface(avatar_images[i].image);for(int k=0;k<2;k++)SDL_FreeSurface(avatar_images[i].scaled[k]);memset(&avatar_images[i],0,sizeof(avatar_images[i]));}avatar_clock=0;}
