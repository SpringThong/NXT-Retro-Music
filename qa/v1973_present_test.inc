#undef SDL_RenderPresent
static void expect(int x,int y,int r,int g,int b){SDL_PixelFormat*f=SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);Uint8 rr,gg,bb;SDL_GetRGB(pixels[y*32+x],f,&rr,&gg,&bb);SDL_FreeFormat(f);assert(abs(rr-r)<=1&&abs(gg-g)<=1&&abs(bb-b)<=1);}
static void blank(SDL_Texture*t){assert(!SDL_SetRenderTarget(vid.renderer,t));SDL_RenderSetClipRect(vid.renderer,NULL);SDL_SetRenderDrawColor(vid.renderer,0,0,0,0);SDL_RenderClear(vid.renderer);}
int main(void){assert(!SDL_Init(0));SDL_Surface*canvas=SDL_CreateRGBSurfaceWithFormat(0,32,24,32,SDL_PIXELFORMAT_ARGB8888);vid.renderer=SDL_CreateSoftwareRenderer(canvas);assert(vid.renderer);vid.screen=SDL_CreateRGBSurfaceWithFormat(0,32,24,32,SDL_PIXELFORMAT_ARGB8888);assert(vid.screen);
 SDL_Texture**layers[]={&vid.target_layer1,&vid.target_layer2,&vid.target_layer3,&vid.target_layer4,&vid.target_layer5};
 for(int i=0;i<5;i++){*layers[i]=SDL_CreateTexture(vid.renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_TARGET,32,24);assert(*layers[i]);assert(!SDL_SetTextureBlendMode(*layers[i],SDL_BLENDMODE_BLEND));blank(*layers[i]);}
 vid.stream_layer1=SDL_CreateTexture(vid.renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,32,24);assert(vid.stream_layer1);SDL_SetTextureBlendMode(vid.stream_layer1,SDL_BLENDMODE_BLEND);
 SDL_FillRect(vid.screen,NULL,0xff0000ff);presentNativeFrame(1);expect(0,0,0,0,255);expect(31,23,0,0,255);assert(presents==1);
 /* New page has transparent areas. Poisoned old window contents, target,
  * viewport and clip must not bleed into the next composition. */
 SDL_FillRect(vid.screen,NULL,0);SDL_FillRect(vid.screen,&(SDL_Rect){5,6,10,6},0xffff0000);
 SDL_SetRenderTarget(vid.renderer,vid.target_layer5);SDL_RenderSetViewport(vid.renderer,&(SDL_Rect){2,3,10,12});SDL_RenderSetClipRect(vid.renderer,&(SDL_Rect){1,1,3,2});presentNativeFrame(1);
 for(int y=0;y<24;y++)for(int x=0;x<32;x++)expect(x,y,x>=5&&x<15&&y>=6&&y<12?255:0,0,0);assert(presents==2);
 /* A layer-only update shares the same composition/order, without reupload. */
 SDL_SetRenderTarget(vid.renderer,vid.target_layer3);SDL_SetRenderDrawColor(vid.renderer,0,255,0,255);SDL_RenderFillRect(vid.renderer,&(SDL_Rect){1,1,2,2});PLAT_GPU_Flip();expect(1,1,0,255,0);expect(5,6,255,0,0);expect(0,0,0,0,0);assert(presents==3);
 blank(vid.target_layer3);PLAT_GPU_Flip();expect(1,1,0,0,0);expect(5,6,255,0,0);assert(presents==4);
 for(int i=0;i<5;i++)SDL_DestroyTexture(*layers[i]);SDL_DestroyTexture(vid.stream_layer1);SDL_FreeSurface(vid.screen);SDL_DestroyRenderer(vid.renderer);SDL_FreeSurface(canvas);SDL_Quit();puts("PASS production native compositor: clean window, reset target/clip/viewport, no old-page pixels, same layer order, layer-only retains latest base, one present per composition.");
}
