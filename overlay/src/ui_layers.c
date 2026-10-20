#include "defines.h"
#include "api.h"
#include "ui_layers.h"
#include "module_common.h"
#include "display_helper.h"

void UiLayer_clear(UiLayer layer) {
    if (!DisplayHelper_getSurface(DisplayHelper_current())) return;
    PLAT_clearLayers(layer);
    ModuleCommon_markLayerDrawn();
}

void UiLayer_blit(SDL_Surface* surface, int x, int y, UiLayer layer) {
    if (!DisplayHelper_getSurface(DisplayHelper_current())) return;
    if (!surface) return;
    PLAT_drawOnLayer(surface, x, y, surface->w, surface->h, 1.0f, false, layer);
    ModuleCommon_markLayerDrawn();
}
