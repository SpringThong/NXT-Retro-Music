#include "nxt_translate.h"
#include "nxt_karaoke.h"
#include "nxt_nxredux.h"
#include "nxt_personal.h"
#include "nxt_startup.h"
#include "nxt_cpu.h"
#include "nxt_write.h"
#include "nxt_volume.h"
#include "nxt_nextui.h"
#include "nxt_transfer.h"
#include "nxt_ota.h"
#include "nxt_soundcloud.h"
#include "nxt_spotify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <msettings.h>

#include "psa/crypto.h"
#include "defines.h"
#include "api.h"
#include "debug.h"
#include "utils.h"
#include "config.h"
#include "player.h"
#include "selfupdate.h"

// UI modules
#include "ui_fonts.h"
#include "keyboard_map.h"
#include "ui_keyboard.h"
#include "ui_icons.h"
#include "ui_theme.h"
#include "toast.h"

// Module architecture
#include "module_common.h"
#include "module_menu.h"
#include "module_library.h"
#include "module_player.h"
#include "module_radio.h"
#include "module_podcast.h"
#include "downloader.h"
#include "module_settings.h"
#include "settings.h"
#include "resume.h"
#include "background.h"
#include "display_helper.h"
#include "spectrum.h"
#include "db.h"
#include "test_control.h"
#include "nxt.h"
#include "nxt_skin.h"
#include "nxt_stock.h"

// Global quit flag
static bool quit = false;
static DisplayContext* display;

// A handler must call nothing that is not async-signal-safe, thus it only
// records the request. ModuleCommon_handleGlobalInput() acts on it, thus the
// module that is in operation returns and the app runs its cleanup.
static void sigHandler(int sig) {
    switch (sig) {
    case SIGINT:
    case SIGTERM:
        ModuleCommon_signalStop();
        break;
    default:
        break;
    }
}

static void print_usage(const char* program) {
    printf(
        "NXT Retro Music v" NXT_VERSION " for " NXT_FIRMWARE_NAME "\n"
        "\n"
        "Usage: %s [options]\n"
        "\n"
        "Options:\n",
        program);
#ifdef TEST_CONTROL
    printf(
        "  --test-control <in>[,<out>]  Read button commands from <in> and write the\n"
        "                               replies to <out>. Each side is 'std' for the\n"
        "                               standard streams, 'fd:<n>' for a descriptor\n"
        "                               that the caller opened, or the path of a file\n"
        "                               or a FIFO. Without <out> the replies go to\n"
        "                               standard output, with '@' before each one.\n"
        "                               One bidirectional descriptor is given twice,\n"
        "                               as in fd:3,fd:3. The form\n"
        "                               --test-control=<in> is also accepted.\n");
#endif
    printf("  --build-info                 Show frontend revision and stop.\n"
           "  -h, --help                   Give this text and stop.\n");
#ifdef TEST_CONTROL
    printf(
        "\n"
        "Commands of the control channel, one step for each line:\n"
        "  press(BTN)  press(BTN, n)  hold(BTN, ms)  hold(BTN, keep)  release(BTN)\n"
        "  wait(ms)  screenshot(path)  sql(sql)  keep()  quit()\n"
        "\n"
        "BTN is UP, DOWN, LEFT, RIGHT, A, B, X, Y, START, SELECT, L1, R1, L2, R2,\n"
        "MENU, PLUS, MINUS or POWER.\n"
        "\n"
        "The app replies '@ok <line>' after each step, '@err <line> <message>' for a\n"
        "command that it cannot execute, and '@bye' before it stops. The end of a file\n"
        "or of a pipe stops the app, if the script did not give keep(). See README.md.\n");
#endif
}

int main(int argc, char* argv[]) {
    Debug_init();
    int session_result=NxtSession_start(argc,argv);
    if(session_result!=0)return session_result>0?EXIT_SUCCESS:session_result==-2?2:EXIT_FAILURE;
    signal(SIGHUP,SIG_IGN);

    // Read the options before anything else, so --help gives its text without a
    // display, and an option that is not correct stops the app at once.
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];
        if (!strcmp(arg,"--session") || !strcmp(arg,"--attach") || !strcmp(arg,"--attach-current")) continue;
if (!strcmp(arg,"--build-info")) {
            printf("%s v%s\n%s\n",NXT_APP_NAME,NXT_VERSION,NXT_BUILD_REVISION);
            return EXIT_SUCCESS;
        }
#ifdef NXT_ROM_STOCK
        if (!strcmp(arg, "--stock-info")) return NxtStock_info();
#endif

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
#ifdef TEST_CONTROL
        if (strncmp(arg, "--test-control=", 15) == 0) {
            if (!TestControl_init(arg + 15)) return EXIT_FAILURE;
            continue;
        }
        if (strcmp(arg, "--test-control") == 0) {
            if (i + 1 >= argc) {
                LOG_error("--test-control needs a value\n");
                return EXIT_FAILURE;
            }
            if (!TestControl_init(argv[++i])) return EXIT_FAILURE;
            continue;
        }
#endif

        LOG_error("unknown option '%s', use --help for the options\n", arg);
        return EXIT_FAILURE;
    }

    NxtStock_init();NxtNextUI_init();
    // Initialize the database before any application modules.
    if (!Db_init()) {
        LOG_error("Database is unavailable, continuing without it\n");
    }

    Settings_init();

    Nxt_init();
    SDL_Surface* const screen = GFX_init(MODE_MAIN);
    if (!screen) {
        LOG_error("Display initialization failed: %s\n", SDL_GetError());
        NxtSession_quit();
        Settings_quit();
        Db_quit();
        return EXIT_FAILURE;
    }
    Theme_init(screen);
    display = DisplayHelper_init(screen);
    Toast_init(display);
    PWR_pinToCores(CPU_CORE_PERFORMANCE);
    // Load the font that the user selected in the settings of the system
    Fonts_load();

    // The keyboard's characters are filtered by what the font can draw, so the
    // map is built once the fonts are up rather than on the first keyboard
    KeyboardMap_prepare(Fonts_hasGlyph, Fonts_getMedium());

    // Show splash screen immediately while heavy subsystems initialize
    {
        GFX_clear(screen);
        SDL_Surface* title = TTF_RenderUTF8_Blended(
            Fonts_getTitle(), NXT_APP_NAME, Theme_getColor(THEME_ROLE_PRIMARY, false));
        if (title) {
            SDL_BlitSurface(title, NULL, screen, &(SDL_Rect){
                (screen->w - title->w) / 2,
                screen->h / 2 - title->h
            });
            SDL_FreeSurface(title);
        }
        SDL_Surface* loading = TTF_RenderUTF8_Blended(
            Fonts_getSmall(), "Loading...", Theme_getColor(THEME_ROLE_SECONDARY, false));
        if (loading) {
            SDL_BlitSurface(loading, NULL, screen, &(SDL_Rect){
                (screen->w - loading->w) / 2,
                screen->h / 2 + SCALE1(4)
            });
            SDL_FreeSurface(loading);
        }
        // The init that follows blocks, thus the splash presents its frame here
        ModuleCommon_markSurfaceDrawn();
        ModuleCommon_frameEnd(screen);
    }

    InitSettings();
    PAD_init();
    PWR_init();
    WIFI_init();
    psa_crypto_init();
    Icons_init();
    NxtHardware_tick();

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // Seed random number generator for shuffle
    srand((unsigned int)time(NULL));

	// Mute hardware before opening audio device to prevent amplifier pop on TG5050
	if(GetAudioSink()==AUDIO_SINK_DEFAULT)SetRawVolume(0);

    // Initialize player core
    if (Player_init() != 0) {
        LOG_error("Failed to initialize audio player\n");
        goto cleanup;
    }

    // Restore the volume on the mixer of the output, after the audio device is open and stable
    Player_syncOutputVolume();

    // Initialize self-update module
    SelfUpdate_init(".");

    // Initialize common module (global input handling)
    ModuleCommon_init();

    // Initialize resume state
    Resume_init();

    // Initialize YouTube downloader (loads queue, auto-resumes pending downloads)
    Downloader_init();

    // Foreground cold launch chooses a listener once; no mood questionnaire.
    if(NxtPersonal_login(display)==MODULE_EXIT_QUIT)goto cleanup;
    NxtLibrary_start();
    NxtCPU_apply(nxt.performance_mode);
    bool cold_start=true;

    // Reattach directly to the live player without reloading its track.
    bool recovered_session=false;
    // Main application loop
    while (!ModuleCommon_stopSignalled()) {
        if (quit) {
            if (!NxtSession_background(display)) break;
            quit=false;recovered_session=true;
        }
        Toast_screenChanged();
        MenuSelection selection = cold_start||(recovered_session&&Background_isPlaying())?MENU_NOW_PLAYING:MenuModule_run(display);
        cold_start=false;
        recovered_session=false;

        if (selection == MENU_QUIT) {
            quit = true;
            continue;
        }

        // Run the selected module
        Toast_screenChanged();
        ModuleExitReason reason = MODULE_EXIT_TO_MENU;

        switch (selection) {
            case MENU_NOW_PLAYING:
                if(NxtSoundCloud_active()){reason=NxtSoundCloud_nowPlaying(display);break;}
                if(NxtSpotify_active()){reason=NxtSpotify_nowPlaying(display);break;}
                switch (Background_getActive()) {
                    case BG_SOUNDCLOUD:
                        reason = NxtSoundCloud_nowPlaying(display);
                        break;
                    case BG_SPOTIFY:
                        reason = NxtSpotify_nowPlaying(display);
                        break;
                    case BG_MUSIC:
                        reason = PlayerModule_run(display, true);  // Now Playing entry
                        break;
                    case BG_RADIO:
                        reason = RadioModule_run(display);
                        break;
                    case BG_PODCAST:
                        reason = PodcastModule_run(display);
                        break;
                    default:
                        reason=NxtStartup_run(display);
                        break;
                }
                break;

            case MENU_RESUME: {
                const ResumeState* rs = Resume_getState();
                if (rs) {
                    reason = PlayerModule_runResume(display, rs);
                }
                break;
            }
            case MENU_LIBRARY:
                reason = Nxt_library(display);
                break;
            case MENU_RADIO:
                reason = RadioModule_run(display);
                break;
            case MENU_PODCAST:
                reason = PodcastModule_run(display);
                break;
            case MENU_SETTINGS:
                reason = Nxt_settings(display);
                break;

            case MENU_NONE:
            case MENU_QUIT:
                break;
        }

        if (reason == MODULE_EXIT_QUIT) {
            quit = true;
        }
    }

cleanup:
    NxtNX_quit();NxtKaraoke_quit();NxtTranslate_quit();
    NxtStartup_save();NxtVolume_flush();NxtWrite_quit();NxtStartup_quit();NxtCPU_release();NxtVolume_release();
    NxtTransfer_stop();NxtNextUI_quit();NxtPersonal_flush();NxtOTA_quit();NxtPersonal_quit();
    NxtSpotify_quit();
    NxtSoundCloud_quit();
    Background_stopAll();
    Nxt_libraryQuit();
    NxtHardware_release();
    NxtSession_quit();
    Downloader_cleanup();
    Settings_quit();
    ModuleCommon_quit();
    SelfUpdate_cleanup();
    PlayerModule_quit();
    Player_quit();
    Toast_quit();
    Icons_quit();
    UIKeyboard_quit();
    KeyboardMap_quit();
    NxtSkin_release();
    Fonts_unload();

    Db_quit();
    QuitSettings();
    TestControl_quit();
    PWR_quit();
    PAD_quit();
    if (!NxtSession_isHidden()) GFX_quit();
    NxtStock_quit();

    return EXIT_SUCCESS;
}
