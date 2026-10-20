#ifndef __UI_SYSTEM_H__
#define __UI_SYSTEM_H__

#include <SDL2/SDL.h>

// Render the app update screen
void render_app_updating(SDL_Surface* screen, int show_setting);

// Render the about screen
void render_about(SDL_Surface* screen, int show_setting);

#endif
