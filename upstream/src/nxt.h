/* SPDX-License-Identifier: GPL-3.0-only
 * NXT Retro Music 0.3.0, 2026. Concept: Nguyễn Xuân Thông. */
#ifndef NXT_H
#define NXT_H
#include "defines.h"
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "display_helper.h"
#include "module_common.h"
#define NXT_VERSION "1.10.2"
#define NXT_APP_NAME "NXT Retro Music"
#ifdef NXT_ROM_NXREDUX
#define NXT_BUILD_REVISION "NX Redux · v1.10.2 · QR-NativeCatalog"
#define NXT_SESSION_BUILD_KEY "1.10.2-NXRedux-QR-NativeCatalog"
#elif defined(NXT_ROM_STOCK)
#define NXT_BUILD_REVISION "Stock · v1.10.2 · QR-NativeCatalog"
#define NXT_SESSION_BUILD_KEY "1.10.2-Stock-QR-NativeCatalog"
#else
#define NXT_BUILD_REVISION "NextUI · v1.10.2 · QR-NativeCatalog"
#define NXT_SESSION_BUILD_KEY "1.10.2-NextUI-QR-NativeCatalog"
#endif

#define NXT_EQ_BANDS 31
#define NXT_PEQ_BANDS 6
typedef struct { int action, index; } NxtPick;
NxtPick Nxt_pick(DisplayContext *,const char *,const char **,int,int *,const char *);
NxtPick Nxt_pickLive(DisplayContext *,const char *,const char **,int,int *,const char *);
NxtPick Nxt_pickTabs(DisplayContext *,const char *,const char **,int,int *,const char *,const char **,int,int);
bool Nxt_chooseFolder(DisplayContext *);
void Nxt_favorite(const char *);
bool Nxt_isFavorite(const char *);
void Nxt_libraryQuit(void);
void NxtLibrary_start(void);
void NxtLibrary_tick(void);
void NxtLibrary_request(bool full);
void NxtLibrary_notify(void);
unsigned NxtLibrary_revision(void);
void Nxt_renderPlaying(SDL_Surface *,int,int,int,bool,int);
typedef struct { float hz, gain, q; int enabled; } NxtFilter;
typedef struct {
    int language, theme, font_size, font_index, lyric_size, lyric_offset, lyric_fullscreen;
    int dsp, eq_bands, limiter, replaygain, transition, crossfade_ms, repeat_mode, shuffle;
    int led, led_strength, unplug_pause, output, background, shortcut[5];
    int recursive, auto_dac, led_color;
    int accent_color,accent_rgb,wallpaper_mode,wallpaper_dim,panel_opacity,auto_bt;
    char wallpaper_path[512];
    int volume[3];
    int volume_percent[3]; /* Exact per-output level; old 0..20 values retained for ROM compatibility. */
    int keys[16], playing_keys[16];
    int led_rgb,led_mode;
    int spotify_auth_source; /* 0 auto, 1 explicit PC import, 2 approved QR */
    int interface_style,cover_mode,animations,eq_preset;
    int lyric_effect,auralis_reference_font,karaoke_auto,karaoke_estimate;
    int lyric_rgb,lyric_unsung_rgb,lyric_translation_rgb,lyric_bold,lyric_outline,lyric_outline_rgb,lyric_shadow;
    int song_title_rgb,song_artist_rgb,song_title_size;
    int auralis_now_brightness,auralis_full_brightness; /* Cover veil only; per listener. */
    int translation_mode,translation_source; /* 0 off, 1 translated, 2 bilingual; source 0 auto, 1..14 explicit */
    int screen_off_controls; /* Device-wide; power wakes, other keys keep backlight off. */
    int performance_mode; /* Device-wide; personal profiles do not override it. */
    int bit_perfect,auto_play,audio_mode; /* bit_perfect is effective; audio_mode is the saved 0/1/2 policy. */
    int dsd_output; /* 0 Native; 1 confirmed DoP; 2 explicit Native -> DoP */
    int spectrum_palette,spectrum_mode,spectrum_rgb,spectrum_rgb2;
    int led_effect,led_rgb2;
    int avatar_icon,avatar_mode,avatar_emoji,avatar_pixel,birth_day,birth_month,genre,mood,country;
    int menu_icons[7]; /* 0 = original, 1..64 icons, 65..128 emoji */
    char avatar_text[64],avatar_path[512];
    float preamp, bass, treble, balance, width, compressor, crossfeed;
    float eq[NXT_EQ_BANDS];
    NxtFilter peq[NXT_PEQ_BANDS];
    char music_root[512], font_path[512], momo[64];
} NxtConfig;
extern NxtConfig nxt;
/* Wake/Power gestures stay consumed through release, even with dark controls enabled. */
#define Nxt_displayInputBlocked() (PWR_displayInputConsumed() || (PWR_getDisplayOff() && !nxt.screen_off_controls))
void Nxt_init(void);
bool Nxt_save(void);
void Nxt_loadPreferences(const char *);
const char *Nxt_musicRoot(void);
void Nxt_tick(void);
void Nxt_stop(void);
void Nxt_sleepTimer(int minutes);
int Nxt_timerRemaining(void);
const char *Nxt_tr(const char *english);
const char *Nxt_text(const char *english, const char *vietnamese);
SDL_Surface *Nxt_render(TTF_Font *font, const char *text, SDL_Color color);
void Nxt_drawText(SDL_Surface *, TTF_Font *, const char *, int, int, int, SDL_Color);
int Nxt_drawWrapped(SDL_Surface *, TTF_Font *, const char *, int, int, int, int, SDL_Color);
ModuleExitReason Nxt_library(DisplayContext *);
ModuleExitReason Nxt_settings(DisplayContext *);
ModuleExitReason Nxt_queue(DisplayContext *);
ModuleExitReason Nxt_dspScreen(DisplayContext *);
ModuleExitReason Nxt_lyricsScreen(DisplayContext *);
ModuleExitReason Nxt_help(DisplayContext *);
ModuleExitReason Nxt_changes(DisplayContext *);
ModuleExitReason Nxt_document(DisplayContext *,const char *,const char *const *,int);
ModuleExitReason Nxt_about(DisplayContext *);
bool Nxt_shortcuts(DisplayContext *);
bool Nxt_isLocked(void);
bool Nxt_lockInput(void);
bool Nxt_selectReleased(void);
void NxtDSP_configure(const NxtConfig *, int rate);
void NxtDSP_process(float *stereo, int frames, float replay_db);
float NxtDSP_response(float hz);
float NxtDSP_peak(void);
bool NxtDSP_clipped(void);
void NxtHardware_tick(void);
bool NxtHardware_controlsLEDs(void);
void NxtHardware_release(void);
bool NxtHardware_writeDiagnostic(void);
const char *NxtHardware_output(void);
const char *NxtHardware_details(void);
const char *NxtHardware_score(void);
const char *NxtHardware_pcm(void);
bool NxtHardware_select(int output);
bool NxtHardware_setDACName(const char *name);
const char *NxtHardware_dacName(void);
bool NxtHardware_hasJack(void);
bool NxtHardware_hasDAC(void);
int NxtHardware_dacCard(void);
const char *NxtHardware_bottomStatus(void);
int NxtHardware_bottomUSB(bool host);
int NxtSession_start(int argc, char **argv);
bool NxtSession_background(DisplayContext *);
ModuleExitReason NxtSession_playerBack(void);
ModuleExitReason NxtSession_exitPrompt(DisplayContext *);
bool NxtSession_wantsBackground(void);
void NxtSession_status(bool visible);
void NxtSession_quit(void);
void NxtSession_tick(void);
bool NxtSession_isHidden(void);
extern const int Nxt_keyIds[16];
extern const char *Nxt_keyNames[16];
int NxtPad_physical(int);
bool Nxt_selectLongUsed(void);
void Nxt_selectConsume(void);
int NxtPad_directionMask(int);
void Nxt_setShuffle(bool);
void Nxt_setRepeat(int);
ModuleExitReason Nxt_quickOptions(DisplayContext *);
ModuleExitReason Nxt_playlists(DisplayContext *);
ModuleExitReason Nxt_favorites(DisplayContext *);
ModuleExitReason Nxt_online(DisplayContext *);
void Nxt_requestScreenOff(void);
void Nxt_EQBands(int);
void Nxt_returnHome(void);
bool Nxt_takeReturnHome(void);
bool Nxt_chooseFile(DisplayContext *,const char *,const char *,char out[512]);
bool Nxt_presetWrite(const char *);
bool Nxt_presetRead(const char *);
void Nxt_profileSave(int);
void Nxt_profileLoad(int);
ModuleExitReason Nxt_presets(DisplayContext *);
int NxtPad_isPressed(int);
int NxtPad_justPressed(int);
int NxtPad_justReleased(int);
int NxtPad_justRepeated(int);
#endif
