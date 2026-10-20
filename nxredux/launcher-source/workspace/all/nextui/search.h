#ifndef SEARCH_H
#define SEARCH_H

#include "sdl.h"
#include "ui_listview.h"
#include <stdbool.h>

typedef struct {
	bool dirty;
	bool startgame;
	bool folderbgchanged;
	int screen;
} SearchResult;

void Search_init(void);
void Search_quit(void);

// Opens keyboard, performs search, returns true if user entered a query
bool Search_open(void);

SearchResult Search_handleInput(unsigned long now);
void Search_render(SDL_Surface* screen, int lastScreen);

// The results-list widget — nextui.c drives its busy/marquee/tick hooks
// (UI_listViewBusy / UI_listViewMarqueeBusy / UI_listViewTickIdle) directly.
ListView* Search_view(void);

#endif // SEARCH_H
