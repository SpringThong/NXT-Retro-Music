#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "defines.h"
#include "api.h"
#include "config.h"
#include "help_screen.h"
#include "module_common.h"
#include "module_player.h"
#include "player.h"
#include "podcast.h"
#include "radio.h"
#include "spectrum.h"
#include "browser.h"
#include "playlist.h"
#include "ui_music.h"
#include "ui_music_playing.h"
#include "ui_layers.h"
#include "ui_album_art.h"
#include "ui_main.h"
#include "lyrics.h"
#include "settings.h"
#include "add_to_playlist.h"
#include "display_helper.h"
#include "ui_utils.h"
#include "resume.h"
#include "playlist_m3u.h"
#include "background.h"
#include "album_art.h"
#include "nxt.h"
#include "nxt_controls.h"
#include "nxt_drive.h"
#include "file_utils.h"

// Internal states
typedef enum {
    PLAYER_INTERNAL_BROWSER,
    PLAYER_INTERNAL_PLAYING
} PlayerInternalState;

// Module state
static BrowserContext browser = {0};
static bool shuffle_enabled = false;
static int repeat_enabled = 0;
static bool manually_stopped = false;
static unsigned queue_generation = 0;
static int next_candidate = -1, last_mode = -1;
static PlaylistContext playlist = {0};
static bool playlist_active = false;
static int restore_position=-1;
static bool restore_pending=false,restore_play=false;
static bool initialized = false;

// Delete confirmation state
static bool show_delete_confirm = false;
static char delete_target_path[512] = "";
static char delete_target_name[256] = "";

// Screen off state (module-local)
static bool screen_off = false;

// Navigation stack — push on A-into-folder, pop on B-back
typedef struct {
    char path[512];
    int  selected;
} NavEntry;
#define NAV_STACK_DEPTH 32

// The seek of a short press of LEFT or RIGHT
#define MUSIC_SEEK_STEP_MS 5000
static NavEntry nav_stack[NAV_STACK_DEPTH];
static int      nav_stack_top = 0;

// Resume: M3U playlist path (set by PlaylistModule before runWithPlaylist)
static char resume_playlist_path[512] = "";

// Resume: last save timestamp for periodic updates
static uint32_t last_resume_save = 0;

// A display recreate drops every GPU layer. The next frame paints each one
// again, whatever the caches of the screen say.
static void display_recreated(void) {
    MusicPlaying_invalidate();
}

// Helper to load directory
static void load_directory(const char* path) {
    Browser_loadDirectory(&browser, path, MUSIC_PATH);
}

// Initialize player module
static void init_player(void) {
    if (initialized) return;
    repeat_enabled=nxt.repeat_mode;shuffle_enabled=nxt.shuffle;
    mkdir(MUSIC_PATH, 0755);
    load_directory(MUSIC_PATH);
    nav_stack_top = 0;
    DisplayHelper_addRecreatedCallback(display_recreated);
    initialized = true;
}

void PlayerModule_quit(void) {
    if (initialized) DisplayHelper_removeRecreatedCallback(display_recreated);
    Browser_freeEntries(&browser);
    memset(&browser, 0, sizeof(browser));
    Playlist_free(&playlist);
    playlist_active = false;
    queue_generation = 0;
    next_candidate = last_mode = restore_position = -1;
    manually_stopped = false;
    show_delete_confirm = screen_off = false;
    delete_target_path[0] = delete_target_name[0] = '\0';
    nav_stack_top = 0;
    memset(nav_stack, 0, sizeof(nav_stack));
    resume_playlist_path[0] = '\0';
    last_resume_save = 0;
    initialized = false;
}

// Try to load and play a track, returns true on success
static bool try_load_and_play(const char *path) {
    Player_setPlaybackSpeed(1);
    manually_stopped = false;
    if(NxtDrive_isStream(path)&&!NxtDrive_preparePlayback(path))return false;
    if (Player_load(path) == 0) {
        int saved_position=restore_pending?restore_position:0;
        if(restore_pending){if(saved_position>0)Player_seek(saved_position);if(restore_play)Player_play();restore_pending=false;restore_position=-1;}else Player_play();
        const TrackInfo* info = Player_getTrackInfo();

        // Fetch album art (async) and lyrics after playback starts
        if (info && !Player_getAlbumArt()) {
            const char* artist = info->artist[0] ? info->artist : "";
            const char* title = info->title[0] ? info->title : "";
            if (artist[0] || title[0]) {
                album_art_fetch(artist, title);
            }
        }
        // The lyrics belong to the track, thus a new track takes the old ones
        // away, and fetches its own where the lyrics are on
        Lyrics_clear();
        if (Settings_getBool(&SETTING_LYRICS_ENABLED) && info) {
            Lyrics_fetch(info->artist, info->title, info->duration_ms / 1000,
                         Player_getCurrentFile(), Player_getEmbeddedLyrics());
        }

        // Save resume state on every track change
        const char* name = (info && info->title[0]) ? info->title : NULL;
        if (!name) {
            const char* slash = strrchr(path, '/');
            name = slash ? slash + 1 : path;
        }
        if (resume_playlist_path[0] && playlist_active) {
            Resume_savePlaylist(resume_playlist_path, path, name,
                                Playlist_getCurrentIndex(&playlist), saved_position);
        } else {
            int idx = playlist_active ? Playlist_getCurrentIndex(&playlist) : browser.selected;
            Resume_saveFiles(browser.current_path, path, name, idx, saved_position);
        }
        last_resume_save = SDL_GetTicks();

        return true;
    }
    return false;
}

// Try to play a playlist track by index (-1 means current). Returns true on success.
static bool playlist_try_play(int idx) {
    const PlaylistTrack* track = (idx < 0)
        ? Playlist_getCurrentTrack(&playlist)
        : Playlist_getTrack(&playlist, idx);
    return track && try_load_and_play(track->path);
}

// Pick a random audio file from the browser (excluding current). Returns true on success.
static bool browser_pick_random(void) {
    int audio_count = Browser_countAudioFiles(&browser);
    if (audio_count <= 1) return false;

    int random_idx = rand() % (audio_count - 1);
    int count = 0;
    for (int i = 0; i < browser.entry_count; i++) {
        if (!browser.entries[i].is_dir && i != browser.selected) {
            if (count == random_idx) {
                browser.selected = i;
                return try_load_and_play(browser.entries[i].path);
            }
            count++;
        }
    }
    return false;
}

// Pick the next audio file in the browser after current. Returns true on success.
static bool browser_pick_next(void) {
    for (int i = browser.selected + 1; i < browser.entry_count; i++) {
        if (!browser.entries[i].is_dir) {
            browser.selected = i;
            return try_load_and_play(browser.entries[i].path);
        }
    }
    return false;
}

// Handle next track logic
static bool handle_track_ended(void) {
    if (manually_stopped) return false;
    if (repeat_enabled == 1) {
        if (playlist_active) return playlist_try_play(-1);
        return try_load_and_play(browser.entries[browser.selected].path);
    }

    if (shuffle_enabled) {
        if (playlist_active) return playlist_try_play(Playlist_shuffle(&playlist));
        return browser_pick_random();
    }

    if (playlist_active) {
        int next_idx = Playlist_next(&playlist);
        if (next_idx < 0 && repeat_enabled == 2) { next_idx=0; Playlist_setCurrentIndex(&playlist,0); }
        if (next_idx < 0) return false;
        return playlist_try_play(next_idx);
    }
    return browser_pick_next();
}

// The layers of the playing screen, once for each frame of the loop. A frame
// that redraws the screen paints them after that redraw, thus this one leaves
// them to the render block.
static void refresh_gpu_layers(int* dirty) {
    if (ModuleCommon_isScreenOffHintActive()) return;
    if (MusicPlaying_frame(*dirty != 0)) *dirty = 1;
}

// Start playback of a track (load + play + init spectrum)
static bool start_playback(const char* path) {
    // Stop any other background player before starting music playback
    if (Background_getActive() != BG_MUSIC) {
        Background_stopAll();
    }
    if (try_load_and_play(path)) {
        Spectrum_init();
        ModuleCommon_recordInputTime();
        ModuleCommon_setAutosleepDisabled(true);
        return true;
    }
    return false;
}

// Clean up playback state. Pass true in `quit_spectrum` where the playing screen
// goes as well, thus the spectrum releases its FFT and its layer.
static void cleanup_playback(bool quit_spectrum) {
    MusicPlaying_leave();
    Lyrics_clear();
    if (quit_spectrum) {
        Spectrum_quit();
    }
    Playlist_free(&playlist);
    playlist_active = false;
    ModuleCommon_setAutosleepDisabled(false);
}

// Clean up playback UI only (audio keeps playing in background). The lyrics
// stay with the track, thus the screen shows them again on the way back.
static void cleanup_playback_ui(void) {
    MusicPlaying_leave();
    Spectrum_quit();
}

// Build a playlist from a directory and start playing the first track
static bool build_and_start_playlist(const char* dir_path, const char* start_file) {
    Playlist_free(&playlist);
    int track_count = Playlist_buildFromDirectory(&playlist, dir_path, start_file);
    if (track_count > 0) {
        playlist_active = true;
        const PlaylistTrack* track = Playlist_getCurrentTrack(&playlist);
        if (track && start_playback(track->path)) {
            return true;
        }
    }
    return false;
}

// Render delete confirmation dialog
static void render_delete_dialog(SDL_Surface* screen) {
    render_confirmation_dialog(screen, delete_target_name, NULL);
    ModuleCommon_markSurfaceDrawn();
}

// Handle USB/Bluetooth media button events
static void handle_hid_events(void) {
    USBHIDEvent hid_event;
    while ((hid_event = Player_pollUSBHID()) != USB_HID_EVENT_NONE) {
        if (hid_event == USB_HID_EVENT_PLAY_PAUSE) {
            Player_togglePause();
        } else if (hid_event == USB_HID_EVENT_NEXT_TRACK) {
            PlayerModule_nextTrack();
        } else if (hid_event == USB_HID_EVENT_PREV_TRACK) {
            PlayerModule_prevTrack();
        } else {
            ModuleCommon_handleHIDVolume(hid_event);
        }
    }
}

// Try to start playback from a browser entry (play-all or single file). Returns true on success.
static bool browser_play_entry(FileEntry *entry) {
    if (entry->is_play_all)
        return build_and_start_playlist(entry->path, "");
    if (build_and_start_playlist(browser.current_path, entry->path))
        return true;
    playlist_active = false;
    return start_playback(entry->path);
}

// Goes to the parent of the current directory, which the ".." row of the list
// names. The cursor goes to the row that the user was on when they entered this
// directory, where the history knows it, and to the row of the directory that
// they left otherwise. A stale history, such as after a resume, is dropped.
static void browser_go_up(void) {
    char child[512];
    snprintf(child, sizeof(child), "%s", browser.current_path);
    char parent[512];
    if (Browser_hasParent(&browser)) {
        snprintf(parent, sizeof(parent), "%s", browser.entries[0].path);
    } else {
        // The directory did not load, thus it has no ".." row: the path names
        // the parent, and a path outside the library goes to its root
        snprintf(parent, sizeof(parent), "%s", child);
        char* last_slash = strrchr(parent, '/');
        if (last_slash) *last_slash = '\0';
        if (strncmp(parent, MUSIC_PATH, strlen(MUSIC_PATH)) != 0) {
            snprintf(parent, sizeof(parent), "%s", MUSIC_PATH);
        }
    }

    int sel = -1;
    if (nav_stack_top > 0 && strcmp(nav_stack[nav_stack_top - 1].path, parent) == 0) {
        nav_stack_top--;
        sel = nav_stack[nav_stack_top].selected;
    } else {
        nav_stack_top = 0;
    }

    load_directory(parent);

    if (sel < 0) {
        sel = 0;
        for (int i = 0; i < browser.entry_count; i++) {
            if (browser.entries[i].is_dir && strcmp(browser.entries[i].path, child) == 0) {
                sel = i;
                break;
            }
        }
    }
    if (sel >= browser.entry_count) sel = browser.entry_count > 0 ? browser.entry_count - 1 : 0;
    browser.selected = sel;
    int half = browser.items_per_page / 2;
    browser.scroll_offset = sel - half;
    if (browser.scroll_offset < 0) browser.scroll_offset = 0;
    int max_scroll = browser.entry_count - browser.items_per_page;
    if (max_scroll < 0) max_scroll = 0;
    if (browser.scroll_offset > max_scroll) browser.scroll_offset = max_scroll;
}

// Handle input in browser state. Returns true if module should exit to menu.
static bool handle_browser_input(PlayerInternalState *state, int *dirty) {
    if (NxtPad_justPressed(BTN_B)) {
        if (strcmp(browser.current_path, MUSIC_PATH) != 0) {
            browser_go_up();
            *dirty = 1;
        } else {
            UiLayer_clear(UI_LAYER_ANIMATION);
            if (!Background_isPlaying()) {
                Spectrum_quit();
                Browser_freeEntries(&browser);
            }
            return true;
        }
    }
    else if (browser.entry_count > 0) {
        if (NxtPad_justRepeated(BTN_UP)) {
            browser.selected = (browser.selected > 0) ? browser.selected - 1 : browser.entry_count - 1;
            *dirty = 1;
        }
        else if (NxtPad_justRepeated(BTN_DOWN)) {
            browser.selected = (browser.selected < browser.entry_count - 1) ? browser.selected + 1 : 0;
            *dirty = 1;
        }
        else if (NxtPad_justPressed(BTN_A)) {
            FileEntry* entry = &browser.entries[browser.selected];
            if (entry->is_dir && strcmp(entry->name, "..") == 0) {
                // The parent row is the way up, the same as B
                browser_go_up();
                *dirty = 1;
            } else if (entry->is_dir) {
                if (nav_stack_top < NAV_STACK_DEPTH) {
                    snprintf(nav_stack[nav_stack_top].path, sizeof(nav_stack[nav_stack_top].path),
                             "%s", browser.current_path);
                    nav_stack[nav_stack_top].selected = browser.selected;
                    nav_stack_top++;
                }
                char path_copy[512];
                snprintf(path_copy, sizeof(path_copy), "%s", entry->path);
                load_directory(path_copy);
                *dirty = 1;
            } else if (browser_play_entry(entry)) {
                *state = PLAYER_INTERNAL_PLAYING;
                *dirty = 1;
            }
        }
        else if (NxtPad_justPressed(BTN_X)) {
            FileEntry* entry = &browser.entries[browser.selected];
            if (!entry->is_dir && !entry->is_play_all) {
                snprintf(delete_target_path, sizeof(delete_target_path), "%s", entry->path);
                snprintf(delete_target_name, sizeof(delete_target_name), "%s", entry->name);
                show_delete_confirm = true;
                UiLayer_clear(UI_LAYER_ANIMATION);
                *dirty = 1;
            }
        }
        else if (NxtPad_justPressed(BTN_Y)) {
            FileEntry* entry = &browser.entries[browser.selected];
            // Inactive on parent-dir (".."): user must navigate up to add its contents.
            // Inactive on "Play All": it's a playback shortcut, not a target.
            if (entry->is_play_all || strcmp(entry->name, "..") == 0) {
                // no-op
            } else if (entry->is_dir) {
                AddToPlaylist_openDir(entry->path);
                *dirty = 1;
            } else {
                AddToPlaylist_open(entry->path, entry->name);
                *dirty = 1;
            }
        }
        else if (NxtPad_justPressed(BTN_LEFT)) {
            list_page_up(&browser.selected, &browser.scroll_offset, browser.entry_count, browser.items_per_page);
            *dirty = 1;
        }
        else if (NxtPad_justPressed(BTN_RIGHT)) {
            list_page_down(&browser.selected, &browser.scroll_offset, browser.entry_count, browser.items_per_page);
            *dirty = 1;
        }
    }

    // Animate browser scroll
    if (browser_needs_scroll_refresh()) {
        browser_animate_scroll();
    }
    if (browser_scroll_needs_render()) *dirty = 1;

    return false;
}

// Handle input in playing state. Returns true when main loop should continue (skip render).
// Both the folder and playlist loops share these v1.0 shoulder actions.
static bool handle_nxt_playing_buttons(void) {
    if(NxtControl_pressed(NC_SPECTRUM)) Spectrum_cycleNext();
    else if(NxtControl_pressed(NC_FAVORITE)) {
        const char *path=Player_getCurrentFile();
        if(path&&*path&&(Player_getState()==PLAYER_STATE_PLAYING||Player_getState()==PLAYER_STATE_PAUSED))
            Nxt_favorite(path);
    } else return false;
    MusicPlaying_invalidate();
    return true;
}

static bool handle_playing_input(SDL_Surface *screen, PlayerInternalState *state, int *dirty) {
    // Handle screen off hint
    if (ModuleCommon_isScreenOffHintActive()) {
        handle_hid_events();
        Player_update();

        // SELECT+A during hint -> full wake
        if (NxtPad_isPressed(BTN_SELECT) && NxtPad_isPressed(BTN_A)) {
            ModuleCommon_resetScreenOffHint();
            ModuleCommon_recordInputTime();
            *dirty = 1;
            return true;  // skip this frame's input so A doesn't toggle pause
        } else {
            // Any other button resets the hint timer
            if (PAD_anyPressed()) {
                ModuleCommon_startScreenOffHint();
            }
            if (ModuleCommon_processScreenOffHintTimeout()) {
                screen_off = true;
                GFX_clear(screen);
                ModuleCommon_markSurfaceDrawn();
            }
            ModuleCommon_frameEnd(screen);
            return true;
        }
    }

    // Handle screen off
    if (screen_off) {
        handle_hid_events();
        Player_update();

        // Any button -> show hint
        if (PAD_anyPressed()) {
            screen_off = false;
            PLAT_enableBacklight(1);
            ModuleCommon_startScreenOffHint();
            GFX_clear(screen);
            render_screen_off_hint(screen);
            ModuleCommon_markSurfaceDrawn();
        }

        if (Player_getState() == PLAYER_STATE_STOPPED) {
            if (!handle_track_ended() && Player_getState() == PLAYER_STATE_STOPPED) {
                if (!manually_stopped) Resume_clear();  // All tracks finished naturally
                screen_off = false;
                PLAT_enableBacklight(1);
                cleanup_playback(false);
                load_directory(MUSIC_PATH);
                nav_stack_top = 0;
                *state = PLAYER_INTERNAL_BROWSER;
                *dirty = 1;
            }
        }
        ModuleCommon_frameEnd(screen);
        return true;
    }

    // Normal input handling
    if (PAD_anyPressed()) {
        ModuleCommon_recordInputTime();
    }

    if (NxtControl_seek(MUSIC_SEEK_STEP_MS, MUSIC_SEEK_STEP_MS)) {
        *dirty = 1;
    }

    if (NxtControl_aux() || handle_nxt_playing_buttons()) {
        *dirty = 1;
    } else if (NxtControl_pressed(NC_PLAY)) {
        Player_togglePause();
        *dirty = 1;
    }
    else if (NxtControl_pressed(NC_MENU)) {
        NxtSession_playerBack();cleanup_album_art_background();cleanup_playback_ui();Background_setActive(BG_MUSIC);*state=PLAYER_INTERNAL_BROWSER;
        *dirty=1;return true;
    }
    else if (NxtControl_pressed(NC_PREV)) {
        PlayerModule_prevTrack();
        *dirty = 1;
    }
    else if (NxtControl_pressed(NC_NEXT)) {
        PlayerModule_nextTrack();
        *dirty = 1;
    }
    else if (NxtControl_pressed(NC_QUEUE)) {
        if(Nxt_queue(DisplayHelper_current())==MODULE_EXIT_QUIT)ModuleCommon_requestQuit();
        MusicPlaying_invalidate();*dirty=1;
    }else if (NxtControl_pressed(NC_LYRICS)) {
        Settings_setBool(&SETTING_LYRICS_ENABLED,true);
        const TrackInfo *info=Player_getTrackInfo();
        Lyrics_fetch(info->artist,info->title,info->duration_ms/1000,Player_getCurrentFile(),Player_getEmbeddedLyrics());
        if(Nxt_lyricsScreen(DisplayHelper_current())==MODULE_EXIT_QUIT)ModuleCommon_requestQuit();
        MusicPlaying_invalidate();*dirty=1;
    }else if (NxtControl_pressed(NC_EQ)) {
        if(Nxt_dspScreen(DisplayHelper_current())==MODULE_EXIT_QUIT)ModuleCommon_requestQuit();
        MusicPlaying_invalidate();*dirty=1;
    }else if (NxtControl_pressed(NC_QUICK)) {
        if(Nxt_quickOptions(DisplayHelper_current())==MODULE_EXIT_QUIT)ModuleCommon_requestQuit();
        MusicPlaying_invalidate();*dirty=1;
    }

    // Check if track ended
    Player_update();
    if (Player_getState() == PLAYER_STATE_STOPPED) {
        if (!handle_track_ended() && Player_getState() == PLAYER_STATE_STOPPED) {
            if (!manually_stopped) Resume_clear();  // All tracks finished naturally
            cleanup_playback(false);
            load_directory(MUSIC_PATH);
            nav_stack_top = 0;
            *state = PLAYER_INTERNAL_BROWSER;
        }
        *dirty = 1;
    }

    // Save resume position periodically
    if (Player_getState() == PLAYER_STATE_PLAYING) {
        uint32_t now = SDL_GetTicks();
        if (now - last_resume_save > 5000) {
            Resume_updatePosition(Player_getPosition());
            last_resume_save = now;
        }
    }

    // Auto screen-off after inactivity
    if (Player_getState() == PLAYER_STATE_PLAYING && ModuleCommon_checkAutoScreenOffTimeout()) {
        MusicPlaying_leave();
        *dirty = 1;
    }

    // Re-render when async album art fetch completes
    if (album_art_is_fetching()) *dirty = 1;

    refresh_gpu_layers(dirty);

    return false;
}

ModuleExitReason PlayerModule_run(DisplayContext* display, bool now_playing_entry) {
    init_player();
    load_directory(browser.current_path[0] ? browser.current_path : MUSIC_PATH);

    PlayerInternalState state = PLAYER_INTERNAL_BROWSER;
    int dirty = 1;
    int show_setting = 0;
    // A path that changes while the title moves scrolls in, thus the marquee
    // does not jump on each directory.
    ScreenTitle_start(true);

    screen_off = false;
    ModuleCommon_resetScreenOffHint();
    ModuleCommon_recordInputTime();

    // Reclaim background music — re-enter playing state (only when entered via Now Playing)
    bool entered_via_now_playing = false;
    if (now_playing_entry && Background_getActive() == BG_MUSIC && PlayerModule_isActive()) {
        Background_setActive(BG_NONE);
        Spectrum_init();
        ModuleCommon_setAutosleepDisabled(true);
        state = PLAYER_INTERNAL_PLAYING;
        entered_via_now_playing = true;
    }

    while (1) {
        ModuleCommon_frameBegin();
        SDL_Surface* const screen = DisplayHelper_getSurface(display);

        // Handle add-to-playlist dialog overlay
        if (AddToPlaylist_isActive()) {
            if (AddToPlaylist_handleInput()) {
                // Dialog closed — skip rest of input to avoid double-handling buttons
                dirty = 1;
                continue;
            }
            // Still active, render dialog (covers entire screen)
            AddToPlaylist_render(screen);
            ModuleCommon_markSurfaceDrawn();
            ModuleCommon_frameEnd(screen);
            continue;
        }

        // Handle delete confirmation dialog (module-specific)
        // A pending stop skips the dialog, thus the global input below reports the
        // quit and the module leaves through the path that it already has.
        if (show_delete_confirm && !ModuleCommon_stopSignalled()) {
            if (NxtPad_justPressed(BTN_A)) {
                if (unlink(delete_target_path) == 0) {
                    load_directory(browser.current_path);
                    if (browser.selected >= browser.entry_count) {
                        browser.selected = browser.entry_count > 0 ? browser.entry_count - 1 : 0;
                    }
                }
            }
            if (NxtPad_justPressed(BTN_A) || NxtPad_justPressed(BTN_B)) {
                delete_target_path[0] = '\0';
                delete_target_name[0] = '\0';
                show_delete_confirm = false;
                dirty = 1;
                continue;
            }
            // Render delete dialog
            render_delete_dialog(screen);
            ModuleCommon_frameEnd(screen);
            continue;
        }

        // Handle global input (skip if screen off or hint active)
        // A signal must reach the global input, thus a pending stop passes this
        // guard. ModuleCommon_handleGlobalInput() reports the quit and returns
        // before it reads any input, thus the screen-off states keep their
        // behavior.
        if (ModuleCommon_stopSignalled()
            || (!screen_off && !ModuleCommon_isScreenOffHintActive())) {
            if (Nxt_shortcuts(DisplayHelper_current())) { dirty=1; ModuleCommon_PWR_update(&dirty,&show_setting); ModuleCommon_frameEnd(screen); continue; }
            HelpId help_id = (state == PLAYER_INTERNAL_BROWSER) ? HELP_BROWSER : HELP_PLAYER;
            GlobalInputResult global = ModuleCommon_handleGlobalInput(screen, &show_setting, help_id);
            if (global.should_quit) {
                if (NxtSession_wantsBackground() && !ModuleCommon_stopSignalled() && PlayerModule_isActive()) {
                    Background_setActive(BG_MUSIC); cleanup_playback_ui();
                } else { cleanup_playback(true); Browser_freeEntries(&browser); }
                return MODULE_EXIT_QUIT;
            }
            if (global.input_consumed) {
                if (global.dirty) dirty = 1;
                ModuleCommon_frameEnd(screen);
                continue;
            }
        }

        if (state == PLAYER_INTERNAL_BROWSER) {
            if (handle_browser_input(&state, &dirty)) {
                return MODULE_EXIT_TO_MENU;
            }
        }
        else if (state == PLAYER_INTERNAL_PLAYING) {
            bool skip_render = handle_playing_input(screen, &state, &dirty);
            if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}
            if (entered_via_now_playing && state != PLAYER_INTERNAL_PLAYING) {
                return MODULE_EXIT_TO_MENU;
            }
            if (skip_render) continue;
        }

        // Handle power management
        if (!screen_off && !ModuleCommon_isScreenOffHintActive()) {
            ModuleCommon_PWR_update(&dirty, &show_setting);
        }

        // Render
        if (dirty && !screen_off && !PWR_getDisplayOff()) {
            if (ModuleCommon_isScreenOffHintActive()) {
                GFX_clear(screen);
                render_screen_off_hint(screen);
            } else if (state == PLAYER_INTERNAL_BROWSER) {
                render_browser(screen, show_setting, &browser);
            } else {
                int pl_track = playlist_active ? Playlist_getCurrentIndex(&playlist) + 1 : 0;
                int pl_total = playlist_active ? Playlist_getCount(&playlist) : 0;
                render_playing(screen, show_setting, &browser, shuffle_enabled, repeat_enabled, pl_track, pl_total);
                MusicPlaying_paintLayers();
            }

            ModuleCommon_markSurfaceDrawn();
            dirty = 0;
        }
        ScreenTitle_frameEnd(&dirty, state == PLAYER_INTERNAL_BROWSER && !screen_off);
        ModuleCommon_frameEnd(screen);
    }
}

// Check if music player module is active
bool PlayerModule_isActive(void) {
    if(Podcast_isActive()||Radio_isActive())return false;
    PlayerState state = Player_getState();
    return (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_PAUSED);
}

// Play next track (for USB HID button support)
void PlayerModule_nextTrack(void) {
    /* Manual previous/next follow the same shuffle switch as auto advance. */
    if (shuffle_enabled) {
        if (playlist_active) {
            int idx=Playlist_shuffle(&playlist);
            if(idx>=0){Player_stop();playlist_try_play(idx);}
        } else if(initialized) browser_pick_random();
        return;
    }

    if (playlist_active) {
        int new_idx = Playlist_next(&playlist);
        if (new_idx < 0) {
            new_idx = 0;
            Playlist_setCurrentIndex(&playlist, new_idx);
        }
        Player_stop();
        playlist_try_play(new_idx);
    } else if (initialized) {
        for (int i = browser.selected + 1; i < browser.entry_count; i++) {
            if (!browser.entries[i].is_dir) {
                Player_stop();
                browser.selected = i;
                try_load_and_play(browser.entries[i].path);
                break;
            }
        }
    }
}

// Play previous track (for USB HID button support)
void PlayerModule_prevTrack(void) {
    /* Manual previous/next follow the same shuffle switch as auto advance. */
    if (shuffle_enabled) {
        if (playlist_active) {
            int idx=Playlist_shuffle(&playlist);
            if(idx>=0){Player_stop();playlist_try_play(idx);}
        } else if(initialized) browser_pick_random();
        return;
    }

    if (playlist_active) {
        int new_idx = Playlist_prev(&playlist);
        if (new_idx < 0) {
            new_idx = playlist.track_count - 1;
            Playlist_setCurrentIndex(&playlist, new_idx);
        }
        Player_stop();
        playlist_try_play(new_idx);
    } else if (initialized) {
        for (int i = browser.selected - 1; i >= 0; i--) {
            if (!browser.entries[i].is_dir) {
                Player_stop();
                browser.selected = i;
                try_load_and_play(browser.entries[i].path);
                break;
            }
        }
    }
}

// Run the player directly with a pre-built playlist (from PlaylistModule)
ModuleExitReason PlayerModule_runWithPlaylist(DisplayContext* display,
                                              PlaylistTrack* tracks,
                                              int track_count,
                                              int start_index) {
    if (!tracks || track_count <= 0) return MODULE_EXIT_TO_MENU;

    init_player();
    ScreenTitle_start(true);

    // Set up the playlist context
    Playlist_free(&playlist);
    Playlist_init(&playlist);
    if (!playlist.tracks) return MODULE_EXIT_TO_MENU;
    for (int i = 0; i < track_count && i < PLAYLIST_MAX_TRACKS; i++) {
        playlist.tracks[i] = tracks[i];
    }
    playlist.track_count = track_count>PLAYLIST_MAX_TRACKS?PLAYLIST_MAX_TRACKS:track_count;
    playlist.current_index = start_index;
    playlist_active = true;

    // Start playback
    const PlaylistTrack* track = Playlist_getCurrentTrack(&playlist);
    if (!track || !start_playback(track->path)) {
        Playlist_free(&playlist);
        playlist_active = false;
        return MODULE_EXIT_TO_MENU;
    }

    
    int dirty = 1;
    int show_setting = 0;
    screen_off = false;
    ModuleCommon_resetScreenOffHint();
    ModuleCommon_recordInputTime();

    while (1) {
        ModuleCommon_frameBegin();
        SDL_Surface* const screen = DisplayHelper_getSurface(display);

        // Handle add-to-playlist dialog overlay
        if (AddToPlaylist_isActive()) {
            if (AddToPlaylist_handleInput()) {
                // Dialog closed — skip rest of input to avoid double-handling buttons
                dirty = 1;
                continue;
            }
            // Dialog covers entire screen, no need to render underlying content
            AddToPlaylist_render(screen);
            ModuleCommon_markSurfaceDrawn();
            ModuleCommon_frameEnd(screen);
            continue;
        }

        // Handle global input (skip if screen off or hint active)
        // A signal must reach the global input, thus a pending stop passes this
        // guard. ModuleCommon_handleGlobalInput() reports the quit and returns
        // before it reads any input, thus the screen-off states keep their
        // behavior.
        if (ModuleCommon_stopSignalled()
            || (!screen_off && !ModuleCommon_isScreenOffHintActive())) {
            if (Nxt_shortcuts(DisplayHelper_current())) { dirty=1; ModuleCommon_PWR_update(&dirty,&show_setting); ModuleCommon_frameEnd(screen); continue; }
            GlobalInputResult global = ModuleCommon_handleGlobalInput(screen, &show_setting, HELP_PLAYER);
            if (global.should_quit) {
                if (NxtSession_wantsBackground() && !ModuleCommon_stopSignalled()) {
                    Background_setActive(BG_MUSIC);
                    cleanup_playback_ui();
                } else {
                    Player_stop();
                    cleanup_album_art_background();
                    cleanup_playback(true);
                }
                return MODULE_EXIT_QUIT;
            }
            if (global.input_consumed) {
                if (global.dirty) dirty = 1;
                ModuleCommon_frameEnd(screen);
                continue;
            }
        }

        // Handle screen off hint
        if (ModuleCommon_isScreenOffHintActive()) {
            handle_hid_events();
            if (NxtPad_isPressed(BTN_SELECT) && NxtPad_isPressed(BTN_A)) {
                ModuleCommon_resetScreenOffHint();
                ModuleCommon_recordInputTime();
                dirty = 1;
                continue;  // skip this frame's input so A doesn't toggle pause
            } else {
                if (PAD_anyPressed()) {
                    ModuleCommon_startScreenOffHint();
                }
                if (ModuleCommon_processScreenOffHintTimeout()) {
                    screen_off = true;
                    GFX_clear(screen);
                    ModuleCommon_markSurfaceDrawn();
                }
                Player_update();
                ModuleCommon_frameEnd(screen);
                continue;
            }
        }

        // Handle screen off mode
        if (screen_off) {
            if (PAD_anyPressed()) {
                screen_off = false;
                PLAT_enableBacklight(1);
                ModuleCommon_startScreenOffHint();
                GFX_clear(screen);
                render_screen_off_hint(screen);
                ModuleCommon_markSurfaceDrawn();
            }
            handle_hid_events();
            Player_update();

            if (Player_getState() == PLAYER_STATE_STOPPED) {
                if (!handle_track_ended() && Player_getState() == PLAYER_STATE_STOPPED) {
                    if (!manually_stopped) Resume_clear();  // All tracks finished naturally
                    screen_off = false;
                    PLAT_enableBacklight(1);
                    Player_stop();
                    cleanup_album_art_background();
                    cleanup_playback(true);
                    return MODULE_EXIT_TO_MENU;
                }
            }
            ModuleCommon_frameEnd(screen);
            continue;
        }

        // Normal input handling
        if (PAD_anyPressed()) {
            ModuleCommon_recordInputTime();
        }

        if (NxtControl_seek(MUSIC_SEEK_STEP_MS, MUSIC_SEEK_STEP_MS)) {
            dirty = 1;
        }

        if (NxtControl_aux() || handle_nxt_playing_buttons()) {
            dirty = 1;
        } else if (NxtControl_pressed(NC_PLAY)) {
            Player_togglePause();
            dirty = 1;
        }
        else if (NxtControl_pressed(NC_MENU)) {
            cleanup_playback_ui();Background_setActive(BG_MUSIC);return NxtSession_playerBack();
        }
        else if (NxtControl_pressed(NC_PREV)) {
            PlayerModule_prevTrack();
            dirty = 1;
        }
        else if (NxtControl_pressed(NC_NEXT)) {
            PlayerModule_nextTrack();
            dirty = 1;
        }
        else if (NxtControl_pressed(NC_QUEUE)) {
            if(Nxt_queue(display)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
            MusicPlaying_invalidate();dirty=1;
        }else if(NxtControl_pressed(NC_LYRICS)) {
            Settings_setBool(&SETTING_LYRICS_ENABLED,true);
            const TrackInfo *info=Player_getTrackInfo();
            Lyrics_fetch(info->artist,info->title,info->duration_ms/1000,Player_getCurrentFile(),Player_getEmbeddedLyrics());
            if(Nxt_lyricsScreen(display)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
            MusicPlaying_invalidate();dirty=1;
        }else if(NxtControl_pressed(NC_EQ)) {
            if(Nxt_dspScreen(display)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
            MusicPlaying_invalidate();dirty=1;
        }else if(NxtControl_pressed(NC_QUICK)) {
            if(Nxt_quickOptions(display)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
            MusicPlaying_invalidate();dirty=1;
        }

        // Check if track ended
        Player_update();
        if (Player_getState() == PLAYER_STATE_STOPPED) {
            if (!handle_track_ended() && Player_getState() == PLAYER_STATE_STOPPED) {
                if (!manually_stopped) Resume_clear();  // All tracks finished naturally
                cleanup_album_art_background();
                cleanup_playback(true);
                return MODULE_EXIT_TO_MENU;
            }
            dirty = 1;
        }

        // Save resume position periodically
        if (Player_getState() == PLAYER_STATE_PLAYING) {
            uint32_t now = SDL_GetTicks();
            if (now - last_resume_save > 5000) {
                Resume_updatePosition(Player_getPosition());
                last_resume_save = now;
            }
        }

        // Auto screen-off after inactivity
        if (Player_getState() == PLAYER_STATE_PLAYING && ModuleCommon_checkAutoScreenOffTimeout()) {
            MusicPlaying_leave();
            dirty = 1;
        }

        refresh_gpu_layers(&dirty);

        // Handle power management
        if (!screen_off && !ModuleCommon_isScreenOffHintActive()) {
            ModuleCommon_PWR_update(&dirty, &show_setting);
        }

        // Render
        if (dirty && !screen_off && !PWR_getDisplayOff()) {
            if (ModuleCommon_isScreenOffHintActive()) {
                GFX_clear(screen);
                render_screen_off_hint(screen);
            } else {
                int pl_track = Playlist_getCurrentIndex(&playlist) + 1;
                int pl_total = Playlist_getCount(&playlist);
                render_playing(screen, show_setting, &browser, shuffle_enabled, repeat_enabled, pl_track, pl_total);
                MusicPlaying_paintLayers();
            }

            ModuleCommon_markSurfaceDrawn();
            dirty = 0;
        }
        ModuleCommon_frameEnd(screen);
    }
}

// Set the M3U playlist path for resume tracking (call before runWithPlaylist)
void PlayerModule_setResumePlaylistPath(const char* m3u_path) {
    snprintf(resume_playlist_path, sizeof(resume_playlist_path), "%s", m3u_path ? m3u_path : "");
}

static void queue_saved(void) {
    char path[1024],temp[1030];
    userdata_snpath("queue.m3u",path,sizeof(path)); snprintf(temp,sizeof(temp),"%s.tmp",path);
    FILE *f=fopen(temp,"w"); if(!f)return; fputs("#EXTM3U\n",f);
    for(int i=0;i<playlist.track_count;i++) if(!strchr(playlist.tracks[i].path,'\n')&&!strchr(playlist.tracks[i].path,'\r')) fprintf(f,"%s\n",playlist.tracks[i].path);
    if(fclose(f)==0&&rename(temp,path)==0) snprintf(resume_playlist_path,sizeof(resume_playlist_path),"%s",path);
}
PlaylistContext *PlayerModule_queue(void) {
    if(!playlist_active) {
        Playlist_free(&playlist); Playlist_init(&playlist);
        if(!playlist.tracks)return &playlist;
        for(int i=0;i<browser.entry_count&&playlist.track_count<PLAYLIST_MAX_TRACKS;i++) if(!browser.entries[i].is_dir&&!browser.entries[i].is_play_all) {
            PlaylistTrack *t=&playlist.tracks[playlist.track_count]; snprintf(t->path,sizeof(t->path),"%s",browser.entries[i].path); NxtText_copy(t->name,browser.entries[i].name,sizeof(t->name)); t->format=browser.entries[i].format;
            if(!strcmp(t->path,Player_getCurrentFile()))playlist.current_index=playlist.track_count;
            playlist.track_count++;
        }
        playlist_active=playlist.track_count>0;
        queue_saved();
    }
    return &playlist;
}
bool PlayerModule_selectQueue(int index) {
    PlayerModule_queue(); if(index<0||index>=playlist.track_count)return false;
    playlist.current_index=index; last_mode=-1; queue_saved(); return playlist_try_play(index);
}
bool PlayerModule_enqueue(const char *path,const char *name,bool next) {
    PlayerModule_queue(); if(!playlist.tracks||playlist.track_count>=PLAYLIST_MAX_TRACKS)return false;
    int i=next?playlist.current_index+1:playlist.track_count; if(i<0)i=0;if(i>playlist.track_count)i=playlist.track_count;
    memmove(&playlist.tracks[i+1],&playlist.tracks[i],(playlist.track_count-i)*sizeof(PlaylistTrack));
    PlaylistTrack *t=&playlist.tracks[i]; memset(t,0,sizeof(*t)); snprintf(t->path,sizeof(t->path),"%s",path); NxtText_copy(t->name,name,sizeof(t->name)); t->format=Player_detectFormat(path);
    playlist.track_count++;playlist_active=true;last_mode=-1;queue_saved();return true;
}
bool PlayerModule_moveQueue(int from,int to) {
    PlayerModule_queue(); if(from<0||to<0||from>=playlist.track_count||to>=playlist.track_count)return false;
    PlaylistTrack t=playlist.tracks[from];
    if(from<to)memmove(&playlist.tracks[from],&playlist.tracks[from+1],(to-from)*sizeof(t));else memmove(&playlist.tracks[to+1],&playlist.tracks[to],(from-to)*sizeof(t));
    playlist.tracks[to]=t;
    if(playlist.current_index==from)playlist.current_index=to;else if(from<playlist.current_index&&playlist.current_index<=to)playlist.current_index--;else if(to<=playlist.current_index&&playlist.current_index<from)playlist.current_index++;
    last_mode=-1;queue_saved();return true;
}
bool PlayerModule_removeQueue(int index) {
    PlayerModule_queue(); if(index<0||index>=playlist.track_count)return false;
    if(index==playlist.current_index)return false; /* Current audio keeps its identity. */
    memmove(&playlist.tracks[index],&playlist.tracks[index+1],(playlist.track_count-index-1)*sizeof(PlaylistTrack));playlist.track_count--;
    if(index<playlist.current_index)playlist.current_index--;
    last_mode=-1;queue_saved();return true;
}
void PlayerModule_markStopped(void) { if(playlist_active){queue_saved();Resume_savePlaylist(resume_playlist_path,Player_getCurrentFile(),Player_getTrackInfo()->title,playlist.current_index,Player_getPosition());}manually_stopped=true;Player_prepareNext(NULL); }
int PlayerModule_repeatMode(void) { return repeat_enabled; }
void PlayerModule_invalidateBrowser(void) { browser.current_path[0]=0;last_mode=-1; }
void PlayerModule_syncQueue(void) {
    if(!initialized||Podcast_isActive()||Radio_isActive()||Player_getState()==PLAYER_STATE_STOPPED)return;
    unsigned generation=Player_getTrackGeneration();
    int mode=repeat_enabled*10+(shuffle_enabled?1:0)+nxt.transition*100;
    if(generation!=queue_generation) {
        queue_generation=generation;last_mode=-1;
        const char *path=Player_getCurrentFile();
        if(playlist_active) { for(int i=0;i<playlist.track_count;i++)if(!strcmp(playlist.tracks[i].path,path)){playlist.current_index=i;break;} }
        else { for(int i=0;i<browser.entry_count;i++)if(!strcmp(browser.entries[i].path,path)){browser.selected=i;break;} }
        const TrackInfo *info=Player_getTrackInfo(); Lyrics_clear();
        if(Settings_getBool(&SETTING_LYRICS_ENABLED))Lyrics_fetch(info->artist,info->title,info->duration_ms/1000,path,Player_getEmbeddedLyrics());
        if(playlist_active) { queue_saved();Resume_savePlaylist(resume_playlist_path,path,info->title,playlist.current_index,Player_getPosition()); }
        else Resume_saveFiles(browser.current_path,path,info->title,browser.selected,Player_getPosition());
    }
    if(mode==last_mode)return;last_mode=mode;
    if(playlist_active&&playlist.track_count) {
        int cur=playlist.current_index;
        next_candidate=repeat_enabled==1?cur:shuffle_enabled?(rand()%playlist.track_count):cur+1;
        if(next_candidate>=playlist.track_count)next_candidate=repeat_enabled==2?0:-1;
        Player_prepareNext(next_candidate>=0?playlist.tracks[next_candidate].path:NULL);
    } else {
        next_candidate=-1;
        if(repeat_enabled==1)next_candidate=browser.selected;
        else if(shuffle_enabled) { int tries=browser.entry_count;while(tries-->0&&browser.entry_count){int i=rand()%browser.entry_count;if(!browser.entries[i].is_dir){next_candidate=i;break;}} }
        else { for(int i=browser.selected+1;i<browser.entry_count;i++)if(!browser.entries[i].is_dir&&!browser.entries[i].is_play_all){next_candidate=i;break;}if(next_candidate<0&&repeat_enabled==2)for(int i=0;i<browser.entry_count;i++)if(!browser.entries[i].is_dir&&!browser.entries[i].is_play_all){next_candidate=i;break;} }
        Player_prepareNext(next_candidate>=0?browser.entries[next_candidate].path:NULL);
    }
}

// Run player restoring a saved resume state
static ModuleExitReason run_resume(DisplayContext* display, const ResumeState* resume) {
    if (!resume) return MODULE_EXIT_TO_MENU;
    ResumeState snapshot=*resume;resume=&snapshot;

    if (resume->type == RESUME_TYPE_FILES) {
        // Initialize browser with saved folder
        init_player();
        load_directory(resume->folder_path);
        nav_stack_top = 0;

        // Build playlist from directory starting at the saved track
        Playlist_free(&playlist);
        int count = Playlist_buildFromDirectory(&playlist, resume->folder_path, resume->track_path);
        if (count <= 0) return MODULE_EXIT_TO_MENU;
        const PlaylistTrack *saved_track=Playlist_getCurrentTrack(&playlist);
        if(!saved_track||strcmp(saved_track->path,resume->track_path)){Playlist_free(&playlist);return MODULE_EXIT_TO_MENU;}
        playlist_active = true;

        // Start playback
        const PlaylistTrack* track = Playlist_getCurrentTrack(&playlist);
        if (!track || !start_playback(track->path)) {
            cleanup_playback(false);
            return MODULE_EXIT_TO_MENU;
        }

        // Saved position and paused/play state were applied before the first sample.
        // Set browser.selected to match the current track for display
        for (int i = 0; i < browser.entry_count; i++) {
            if (strcmp(browser.entries[i].path, track->path) == 0) {
                browser.selected = i;
                break;
            }
        }

        // Use the shared playing loop via handle_playing_input
        int dirty = 1;
        int show_setting = 0;
        screen_off = false;
        ScreenTitle_start(true);
        ModuleCommon_resetScreenOffHint();
        ModuleCommon_recordInputTime();
        PlayerInternalState state = PLAYER_INTERNAL_PLAYING;

        while (1) {
            ModuleCommon_frameBegin();
            SDL_Surface* const screen = DisplayHelper_getSurface(display);

            // Handle add-to-playlist dialog overlay
            if (AddToPlaylist_isActive()) {
                if (AddToPlaylist_handleInput()) {
                    dirty = 1;
                    continue;
                }
                AddToPlaylist_render(screen);
                ModuleCommon_markSurfaceDrawn();
                ModuleCommon_frameEnd(screen);
                continue;
            }

            // Handle global input
            // A signal must reach the global input, thus a pending stop passes this
            // guard. ModuleCommon_handleGlobalInput() reports the quit and returns
            // before it reads any input, thus the screen-off states keep their
            // behavior.
            if (ModuleCommon_stopSignalled()
                || (!screen_off && !ModuleCommon_isScreenOffHintActive())) {
                if (Nxt_shortcuts(DisplayHelper_current())) { dirty=1; ModuleCommon_PWR_update(&dirty,&show_setting); ModuleCommon_frameEnd(screen); continue; }
            GlobalInputResult global = ModuleCommon_handleGlobalInput(screen, &show_setting, HELP_PLAYER);
                if (global.should_quit) {
                    if (NxtSession_wantsBackground() && !ModuleCommon_stopSignalled()) { Background_setActive(BG_MUSIC); cleanup_playback_ui(); }
                    else { Player_stop(); cleanup_album_art_background(); cleanup_playback(true); }
                    return MODULE_EXIT_QUIT;
                }
                if (global.input_consumed) {
                    if (global.dirty) dirty = 1;
                    ModuleCommon_frameEnd(screen);
                    continue;
                }
            }

            // Delegate to shared playing input handler
            if (handle_playing_input(screen, &state, &dirty)) {
                if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}
                // If state left playing (BTN_B or all tracks ended), return to menu immediately
                // Don't continue the loop or handle_playing_input will re-trigger track-ended logic
                if (state != PLAYER_INTERNAL_PLAYING) {
                    return MODULE_EXIT_TO_MENU;
                }
                continue;
            }

            // If state left playing, return to menu
            if (state != PLAYER_INTERNAL_PLAYING) {
                return MODULE_EXIT_TO_MENU;
            }

            // Handle power management
            if (!screen_off && !ModuleCommon_isScreenOffHintActive()) {
                ModuleCommon_PWR_update(&dirty, &show_setting);
            }

            // Render
            if (dirty && !screen_off && !PWR_getDisplayOff()) {
                if (ModuleCommon_isScreenOffHintActive()) {
                    GFX_clear(screen);
                    render_screen_off_hint(screen);
                } else {
                    int pl_track = Playlist_getCurrentIndex(&playlist) + 1;
                    int pl_total = Playlist_getCount(&playlist);
                    render_playing(screen, show_setting, &browser, shuffle_enabled, repeat_enabled, pl_track, pl_total);
                    MusicPlaying_paintLayers();
                }

                ModuleCommon_markSurfaceDrawn();
                dirty = 0;
            }
            ModuleCommon_frameEnd(screen);
        }

    } else if (resume->type == RESUME_TYPE_PLAYLIST) {
        // Load the M3U playlist tracks
        PlaylistTrack *m3u_tracks=calloc(PLAYLIST_MAX_TRACKS,sizeof(PlaylistTrack));if(!m3u_tracks)return MODULE_EXIT_TO_MENU;
        int m3u_count = 0;
        if (M3U_loadTracks(resume->playlist_path, m3u_tracks, PLAYLIST_MAX_TRACKS, &m3u_count) != 0 || m3u_count <= 0) {
            free(m3u_tracks);return MODULE_EXIT_TO_MENU;
        }

        // Find the track index in the loaded playlist
        int start_idx = -1;
        for (int i = 0; i < m3u_count; i++) {
            if (strcmp(m3u_tracks[i].path, resume->track_path) == 0) {
                start_idx = i;
                break;
            }
        }

        if(start_idx<0){free(m3u_tracks);return MODULE_EXIT_TO_MENU;}
        // Set resume playlist path so the playing loop saves correctly
        PlayerModule_setResumePlaylistPath(resume->playlist_path);

        // Run with the playlist
        restore_position=resume->position_ms;
        ModuleExitReason reason = PlayerModule_runWithPlaylist(display, m3u_tracks, m3u_count, start_idx);free(m3u_tracks);

        resume_playlist_path[0] = '\0';
        return reason;
    }

    return MODULE_EXIT_TO_MENU;
}

// Background tick: handle track advancement and resume saving while in menu
void PlayerModule_backgroundTick(void) {
    Player_update();

    // Handle track ended (auto-advance)
    if (Player_getState() == PLAYER_STATE_STOPPED) {
        if (!handle_track_ended() && Player_getState() == PLAYER_STATE_STOPPED) {
            // All tracks finished
            if (!manually_stopped) Resume_clear();
            cleanup_playback(false);
            Background_setActive(BG_NONE);
            ModuleCommon_setAutosleepDisabled(false);
        }
        return;
    }

    // Save resume position periodically
    if (Player_getState() == PLAYER_STATE_PLAYING) {
        uint32_t now = SDL_GetTicks();
        if (now - last_resume_save > 5000) {
            Resume_updatePosition(Player_getPosition());
            last_resume_save = now;
        }
    }
}

void Nxt_setShuffle(bool value){shuffle_enabled=value;nxt.shuffle=value;Nxt_save();}
void Nxt_setRepeat(int value){repeat_enabled=value%3;nxt.repeat_mode=repeat_enabled;Nxt_save();}
void Nxt_requestScreenOff(void){extern void Nxt_screenOff(void);Nxt_screenOff();MusicPlaying_leave();}

static bool return_home;
void Nxt_returnHome(void){return_home=true;}
bool Nxt_takeReturnHome(void){bool r=return_home;return_home=false;return r;}

ModuleExitReason PlayerModule_runResumeMode(DisplayContext *d,const ResumeState *r,bool play){restore_pending=true;restore_play=play;restore_position=r?r->position_ms:0;ModuleExitReason result=run_resume(d,r);restore_pending=false;restore_position=-1;return result;}
ModuleExitReason PlayerModule_runResume(DisplayContext *d,const ResumeState *r){return PlayerModule_runResumeMode(d,r,false);}
