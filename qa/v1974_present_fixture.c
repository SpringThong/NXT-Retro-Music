#include <SDL2/SDL.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
static struct {SDL_Renderer*renderer;SDL_Surface*screen;SDL_Texture *target_layer1,*target_layer2,*stream_layer1,*target_layer3,*target_layer4,*target_layer5;}vid;
static int presents;static Uint32 pixels[32*24];
static void capture(SDL_Renderer*r){assert(SDL_GetRenderTarget(r)==NULL);assert(!SDL_RenderReadPixels(r,NULL,SDL_PIXELFORMAT_ARGB8888,pixels,32*4));presents++;SDL_RenderPresent(r);}
#define SDL_RenderPresent capture
