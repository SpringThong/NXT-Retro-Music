/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_WALLPAPER_STORE_H
#define NXT_WALLPAPER_STORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#define NXT_WALLPAPER_CAP 512
#define NXT_WALLPAPER_FAVORITES 128
#define NXT_WALLPAPER_BYTES (10u*1024u*1024u)
#define NXT_WALLPAPER_GENRES 24
enum {NXT_WP_ONLINE,NXT_WP_LOCAL,NXT_WP_PRESET,NXT_WP_FOLDER};
enum {NXT_WP_SMOOTH,NXT_WP_PIXEL};
typedef struct {
    int kind,genre,preset,width,height,render;
    uint64_t bytes;
    char id[96],name[160],path[512],url[1024],thumb[1024],web[256];
} NxtWallpaperEntry;
bool NxtWallpaper_https(const char *);
int NxtWallpaper_parseCatalog(const char *,NxtWallpaperEntry *,int,int *);
bool NxtWallpaper_imageSize(const char *,uint32_t *,uint32_t *);
bool NxtWallpaper_normalize(const char *,const char *);
bool NxtWallpaper_prepare(const char *,const char *,int);
SDL_Surface *NxtWallpaper_resample(SDL_Surface *,int,int,bool,int);
bool NxtWallpaper_thumbnail(const char *,const char *);
bool NxtWallpaper_inside(const char *,const char *);
int NxtWallpaper_scan(const char *,const char *,NxtWallpaperEntry *,int,bool *);
int NxtWallpaper_readEntries(const char *,NxtWallpaperEntry *,int);
bool NxtWallpaper_writeEntries(const char *,const NxtWallpaperEntry *,int);
#endif
