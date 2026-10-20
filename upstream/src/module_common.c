#include <signal.h>
#include <string.h>
#include <time.h>
#include <msettings.h>
#include "defines.h"
#include "api.h"
#include "module_common.h"
#include "nxt_volume.h"
#include "nxt_motion.h"
#include "nxt.h"
#include "nxt_spotify.h"
#include "nxt_soundcloud.h"
#include "module_player.h"
#include "settings.h"
#include "ui_main.h"
#include "ui_layers.h"
#include "frame_state.h"
#include "ui_utils.h"
#include "player.h"
#include "radio.h"
#include "spectrum.h"
#include "background.h"
#include "toast.h"
#include "test_control.h"
#include "seek_scan.h"
#include "resume.h"
#include "nxt_stock.h"

static bool autosleep_disabled = false;
static FrameState frame = {0};
static uint32_t last_input_time = 0;

// Screen off hint state
static bool screen_off_hint_active = false;
static uint32_t screen_off_hint_start = 0;
static time_t screen_off_hint_start_wallclock = 0;

// Dialog states
static bool show_quit_confirm = false;

// Set once the user confirms the quit dialog.
// Sticky, any subsequent call to ModuleCommon_handleGlobalInput() will return should_quit = true
static bool quit_requested = false;

// Set by a signal handler, thus sig_atomic_t and not bool. ModuleCommon_init()
// does not clear it: the handlers are in place before the app initializes the
// modules, and a signal in that interval must not be lost.
static volatile sig_atomic_t stop_signalled = 0;
static bool show_controls_help = false;

// START button long press detection
static uint32_t start_press_time = 0;
static bool start_was_pressed = false;
#define START_LONG_PRESS_MS 500

// Overlay state tracking - force hide after button release
static bool overlay_buttons_were_active = false;
static uint32_t overlay_release_time = 0;
#define OVERLAY_VISIBLE_AFTER_RELEASE_MS 800  // How long overlay stays visible after release
#define OVERLAY_FORCE_HIDE_DURATION_MS 500    // How long to keep forcing hide

void ModuleCommon_init(void) {
    PWR_setDisplayOnlySleep(1);
    PWR_setDisplayOff(0);
    autosleep_disabled = false;
    last_input_time = SDL_GetTicks();
    screen_off_hint_active = false;
    show_quit_confirm = false;
    quit_requested = false;
    show_controls_help = false;
    start_was_pressed = false;
    overlay_buttons_were_active = false;
    overlay_release_time = 0;
}

// The status pill shows Bluetooth, and no event of the platform redraws it when a
// device connects or goes. Two things do here: a change of the audio sink (at once),
// and a change of the Bluetooth state that the network poll of the platform caches
// (each 5 s). The poll also sees a link that outlives its audio stream, as a headset
// that is switched off does until its supervision timeout.
static bool audio_sink_status_changed(void) {
    static unsigned seen_generation = 0;
    static int seen_bluetooth = -1;
    bool changed = false;

    unsigned generation = Player_getAudioSinkGeneration();
    if (generation != seen_generation) {
        seen_generation = generation;
        changed = true;
    }
    int bluetooth = PLAT_btIsConnected() ? 1 : 0;
    if (bluetooth != seen_bluetooth) {
        if (seen_bluetooth != -1) changed = true;
        seen_bluetooth = bluetooth;
    }
    static int seen_jack = -1, seen_dac = -1;
    int jack=NxtHardware_hasJack(),dac=NxtHardware_hasDAC();
    if(jack!=seen_jack||dac!=seen_dac){changed=true;seen_jack=jack;seen_dac=dac;}
    return changed;
}

void Nxt_screenOff(void){PWR_setDisplayOff(1);}
static bool nxt_adjust_levels(int *show_setting) {
    if(Nxt_isLocked())return false;
    bool modifier=PAD_isPressed(BTN_SELECT);
    int up=NxtPad_directionMask(BTN_UP),down=NxtPad_directionMask(BTN_DOWN);
    int left=NxtPad_directionMask(BTN_LEFT),right=NxtPad_directionMask(BTN_RIGHT);
    bool combo=modifier&&PAD_isPressed(up|down|left|right);
    if(combo)Nxt_selectConsume();
    int volume=0,brightness=0;
    if(modifier){
        volume=!!PAD_justRepeated(up)-!!PAD_justRepeated(down);
        brightness=!!PAD_justRepeated(right)-!!PAD_justRepeated(left);
    }else volume=!!PAD_justRepeated(BTN_PLUS)-!!PAD_justRepeated(BTN_MINUS);
    
    if(volume)NxtVolume_setPercent(NxtVolume_getPercent()+volume);
    if(brightness){int value=GetBrightness()+brightness;if(value<0)value=0;if(value>10)value=10;SetBrightness(value);}
    if(volume||brightness){
        char message[96];snprintf(message,sizeof(message),"%s: %d%s",brightness?Nxt_text("Brightness","Độ sáng"):Nxt_text("Volume","Âm lượng"),brightness?GetBrightness()*10:NxtVolume_getPercent(),brightness?"%":"");
        Toast_show(message,900);if(show_setting)*show_setting=brightness?1:2;
        ModuleCommon_recordInputTime();
    }
    return combo||volume||brightness||(modifier&&Nxt_selectLongUsed());
}
GlobalInputResult ModuleCommon_handleGlobalInput(SDL_Surface* screen, int* show_setting, HelpId help_id) {
    GlobalInputResult result = {false, false, false};

    if (stop_signalled) {
        quit_requested = true;
    }

    // Keep reporting a confirmed quit until somebody acts on it
    if (quit_requested) {
        result.input_consumed = true;
        result.should_quit = true;
        return result;
    }
#ifdef NXT_ROM_STOCK
    static bool panel_was_active;
    if(NxtStock_panelActive()){
        panel_was_active=true;result.input_consumed=true;result.dirty=true;return result;
    }
    if(panel_was_active){panel_was_active=false;PAD_reset();result.input_consumed=true;result.dirty=true;return result;}
#endif

    bool display_input = Nxt_displayInputBlocked();
    // SELECT + directions must consume the short SELECT release before lock timing.
    if(!display_input&&!Nxt_isLocked()&&PAD_isPressed(BTN_SELECT)&&
       PAD_isPressed(NxtPad_directionMask(BTN_UP)|NxtPad_directionMask(BTN_DOWN)|
                     NxtPad_directionMask(BTN_LEFT)|NxtPad_directionMask(BTN_RIGHT)))Nxt_selectConsume();
    // Poll USB HID events (earphone buttons)
    if(Nxt_lockInput()){PWR_update(NULL,show_setting,NULL,NULL);result.input_consumed=true;result.dirty=true;return result;}
    USBHIDEvent hid_event;
    while ((hid_event = Player_pollUSBHID()) != USB_HID_EVENT_NONE) {
        if (ModuleCommon_handleHIDVolume(hid_event)) {
            result.dirty = true;
            result.input_consumed = true;
        }
        else if (hid_event == USB_HID_EVENT_PLAY_PAUSE) {
            // Check what's currently playing and handle accordingly
            // Check radio first (takes priority when streaming)
            RadioState radio_state = Radio_getState();
            PlayerState player_state = Player_getState();

            if (NxtSoundCloud_active()) {
                NxtSoundCloud_togglePause();
            } else if (NxtSpotify_active()) {
                NxtSpotify_togglePause();
                result.dirty = true;
                result.input_consumed = true;
            }
            else if (radio_state == RADIO_STATE_PLAYING || radio_state == RADIO_STATE_BUFFERING) {
                // Radio is streaming - stop it
                Radio_stop();
                result.dirty = true;
                result.input_consumed = true;
            }
            else if (player_state == PLAYER_STATE_PLAYING || player_state == PLAYER_STATE_PAUSED) {
                // Music player is active - toggle pause
                Player_togglePause();
                result.dirty = true;
                result.input_consumed = true;
            }
            else {
                // Nothing playing - check if we can resume radio
                const char* last_url = Radio_getCurrentUrl();
                if (last_url && last_url[0] != '\0') {
                    // Resume last radio station
                    Radio_play(last_url);
                    result.dirty = true;
                    result.input_consumed = true;
                }
            }
        }
        else if (hid_event == USB_HID_EVENT_NEXT_TRACK || hid_event == USB_HID_EVENT_PREV_TRACK) {
            // Next/previous track - check what's currently active
            RadioState radio_state = Radio_getState();

            if (radio_state == RADIO_STATE_PLAYING || radio_state == RADIO_STATE_BUFFERING || radio_state == RADIO_STATE_CONNECTING) {
                // Radio is active - switch stations
                RadioStation* stations;
                int station_count = Radio_getStations(&stations);
                if (station_count > 1) {
                    // Find current station index
                    int current_idx = Radio_findCurrentStationIndex();
                    if (current_idx < 0) continue;
                    // Calculate new index
                    int new_idx;
                    if (hid_event == USB_HID_EVENT_NEXT_TRACK) {
                        new_idx = (current_idx + 1) % station_count;
                    } else {
                        new_idx = (current_idx - 1 + station_count) % station_count;
                    }
                    // Switch to new station
                    Radio_stop();
                    Radio_play(stations[new_idx].url);
                    result.dirty = true;
                    result.input_consumed = true;
                }
            }
            else if (PlayerModule_isActive()) {
                // Music player is active - next/previous song
                if (hid_event == USB_HID_EVENT_NEXT_TRACK) {
                    PlayerModule_nextTrack();
                } else {
                    PlayerModule_prevTrack();
                }
                result.dirty = true;
                result.input_consumed = true;
            }
        }
    }

    if (display_input) {
        result.dirty = true;
        result.input_consumed = true;
        return result;
    }
    if(nxt_adjust_levels(show_setting)){result.input_consumed=true;result.dirty=true;return result;}

    // Handle quit confirmation dialog
    if (show_quit_confirm) {
        if (NxtPad_justPressed(BTN_A)) {
            show_quit_confirm = false;
            quit_requested = true;
            result.input_consumed = true;
            result.should_quit = true;
            return result;
        }
        else if (NxtPad_justPressed(BTN_B) || NxtPad_justPressed(BTN_START)) {
            show_quit_confirm = false;
            result.input_consumed = true;
            result.dirty = true;
            return result;
        }
        // Dialog is shown, consume input and render (covers entire screen)
        render_confirmation_dialog(screen, NULL, "Quit Music Player?");
        ModuleCommon_markSurfaceDrawn();
        result.input_consumed = true;
        return result;
    }

    // Handle controls help dialog - press any button to close
    if (show_controls_help) {
        if (NxtPad_justPressed(BTN_A) || NxtPad_justPressed(BTN_B) || NxtPad_justPressed(BTN_X) ||
            NxtPad_justPressed(BTN_Y) || NxtPad_justPressed(BTN_START) || NxtPad_justPressed(BTN_SELECT) ||
            NxtPad_justPressed(BTN_UP) || NxtPad_justPressed(BTN_DOWN) ||
            NxtPad_justPressed(BTN_LEFT) || NxtPad_justPressed(BTN_RIGHT) ||
            NxtPad_justPressed(BTN_L1) || NxtPad_justPressed(BTN_R1) || NxtPad_justPressed(BTN_MENU)) {
            show_controls_help = false;
            result.input_consumed = true;
            result.dirty = true;
            return result;
        }
        // Dialog is shown, consume input and render (covers entire screen)
        render_controls_help(screen, help_id);
        ModuleCommon_markSurfaceDrawn();
        result.input_consumed = true;
        return result;
    }

    // Handle power management
    {
        int dirty_before = result.dirty ? 1 : 0;
        int dirty_tmp = dirty_before;
        PWR_update(&dirty_tmp, show_setting, NULL, NULL);

        if (dirty_tmp && !dirty_before) {
            result.dirty = true;
        }
    }

    return result;
}

void ModuleCommon_setAutosleepDisabled(bool disabled) {
    if (disabled && !autosleep_disabled) {
        PWR_disableAutosleep();
        autosleep_disabled = true;
    } else if (!disabled && autosleep_disabled) {
        // Don't re-enable autosleep if background audio is still playing
        if (!Background_isPlaying()) {
            PWR_enableAutosleep();
            autosleep_disabled = false;
        }
    }
}

bool ModuleCommon_isScreenOffHintActive(void) {
    return screen_off_hint_active;
}

void ModuleCommon_startScreenOffHint(void) {
    // The music app uses one backlight state for Power, quick options and idle.
    screen_off_hint_active = false;
    Nxt_screenOff();
    ModuleCommon_recordInputTime();
}

void ModuleCommon_resetScreenOffHint(void) {
    screen_off_hint_active = false;
}

bool ModuleCommon_updateSeekScan(int back_step_ms, int fwd_step_ms) {
    SeekScan* scan = SeekScan_shared();

    // A track that stopped has nothing to seek
    if (Player_getState() == PLAYER_STATE_STOPPED) {
        bool was_scanning = SeekScan_isScanning(scan);
        SeekScan_cancel(scan);
        return was_scanning;
    }

    bool was_scanning = SeekScan_isScanning(scan);
    int seek_to_ms;
    bool seek = SeekScan_update(scan, SDL_GetTicks(), NxtPad_isPressed(BTN_LEFT),
                                NxtPad_isPressed(BTN_RIGHT), Player_getPosition(),
                                Player_getDuration(), back_step_ms, fwd_step_ms, &seek_to_ms);
    if (seek) {
        Player_seek(seek_to_ms);
        ModuleCommon_recordInputTime();
    }
    return seek || was_scanning || SeekScan_isScanning(scan);
}

void ModuleCommon_recordInputTime(void) {
    last_input_time = SDL_GetTicks();
}

bool ModuleCommon_checkAutoScreenOffTimeout(void) {
    if (screen_off_hint_active || PWR_getDisplayOff()) return false;
    uint32_t screen_timeout_ms = Settings_getInt(&SETTING_SCREEN_OFF_TIMEOUT) * 1000;
    if (screen_timeout_ms > 0 && SDL_GetTicks() - last_input_time >= screen_timeout_ms) {
        ModuleCommon_startScreenOffHint();
        return true;
    }
    return false;
}

bool ModuleCommon_processScreenOffHintTimeout(void) {
    if (!screen_off_hint_active) return false;
    uint32_t now = SDL_GetTicks();
    time_t now_wallclock = time(NULL);
    bool timeout_sdl = (now - screen_off_hint_start >= SCREEN_OFF_HINT_DURATION_MS);
    bool timeout_wallclock = (now_wallclock - screen_off_hint_start_wallclock >= (SCREEN_OFF_HINT_DURATION_MS / 1000));
    if (timeout_sdl || timeout_wallclock) {
        screen_off_hint_active = false;
        PLAT_enableBacklight(0);
        return true;
    }
    return false;
}

void ModuleCommon_quit(void) {
    PWR_setDisplayOnlySleep(0);
    // Ensure autosleep is re-enabled
    if (autosleep_disabled) {
        PWR_enableAutosleep();
        autosleep_disabled = false;
    }

    UiLayer_clear(UI_LAYER_STATUS);
    UiLayer_clear(UI_LAYER_ANIMATION);
}

void ModuleCommon_PWR_update(int* dirty, int* show_setting) {
    // Track overlay-triggering buttons for auto-hide (check BEFORE PWR_update)
    // MENU = brightness, SELECT = color temp, PLUS/MINUS = volume
    bool overlay_buttons_active = NxtPad_isPressed(BTN_PLUS) || NxtPad_isPressed(BTN_MINUS)
                               || NxtPad_isPressed(BTN_MENU) || NxtPad_isPressed(BTN_SELECT);

    if (overlay_buttons_were_active && !overlay_buttons_active) {
        // Buttons just released - start timer
        overlay_release_time = SDL_GetTicks();
    }
    else if (overlay_buttons_active) {
        // A press ends the release timer, thus a hold that follows a release
        // keeps its pill. The platform shows the pill on each auto-repeat.
        overlay_release_time = 0;
    }

    // Call platform PWR_update
    PWR_update(dirty, show_setting, NULL, NULL);
    if (audio_sink_status_changed()) *dirty = 1;

    // After visible period, force hide overlay
    if (overlay_release_time > 0) {
        uint32_t elapsed = SDL_GetTicks() - overlay_release_time;
        if (elapsed >= OVERLAY_VISIBLE_AFTER_RELEASE_MS) {
            // Visible period passed, now force hide
            *show_setting = 0;
            *dirty = 1;
            // Stop forcing after the duration
            if (elapsed >= OVERLAY_VISIBLE_AFTER_RELEASE_MS + OVERLAY_FORCE_HIDE_DURATION_MS) {
                overlay_release_time = 0;
            }
        }
    }

    overlay_buttons_were_active = overlay_buttons_active;
}

void ModuleCommon_requestQuit(void) {
    quit_requested = true;
}

void ModuleCommon_signalStop(void) {
    stop_signalled = 1;
}

bool ModuleCommon_stopSignalled(void) {
    return stop_signalled != 0;
}

static void nxt_before_poweroff(void) {
    if(Player_getState()==PLAYER_STATE_PLAYING||Player_getState()==PLAYER_STATE_PAUSED)
        Resume_updatePosition(Player_getPosition());
    Nxt_save();
}

void ModuleCommon_frameBegin(void) {
    Nxt_tick();
    GFX_startFrame();
    ScreenTitle_frameBegin();
    PAD_poll();
    TestControl_tick();
    if(!PAD_isPressed(BTN_SELECT)&&(PAD_anyJustPressed()||NxtPad_justRepeated(BTN_UP)||NxtPad_justRepeated(BTN_DOWN))&&!PAD_isPressed(BTN_PLUS|BTN_MINUS))NxtMotion_input(DisplayHelper_getSurface(DisplayHelper_current()));
#ifdef NXT_ROM_STOCK
    if(PAD_justPressed(BTN_HOME)&&!(PWR_getDisplayOff()&&nxt.screen_off_controls)){
        PWR_setDisplayOff(0);
        if(!NxtStock_requestPanel())Toast_show(Nxt_text("Stock quick panel is unavailable","Bảng nhanh Stock chưa sẵn sàng"),2000);
    }
#endif
    /* Raw HOME remains available before screen-off and lock input filtering. */
    if(PAD_justPressed(BTN_MENU)){
        LOG_info("TrimUI logo MENU: stopping and closing full music session\n");
        nxt_before_poweroff();
        ModuleCommon_signalStop();ModuleCommon_requestQuit();
    }
    PWR_setScreenOffControls(nxt.screen_off_controls);
    PWR_displayFrameBegin();
    PWR_update(NULL,NULL,nxt_before_poweroff,NULL);
    Toast_tick();
}

void ModuleCommon_markSurfaceDrawn(void) {
    FrameState_markSurfaceDrawn(&frame);
}

void ModuleCommon_markLayerDrawn(void) {
    FrameState_markLayerDrawn(&frame);
}

void ModuleCommon_frameEnd(SDL_Surface* screen) {
    // A new audio sink (Bluetooth, USB DAC) moves the audio on the main thread
    Player_handleAudioSinkChange();
#ifdef NXT_ROM_STOCK
    if(NxtStock_panelActive()){GFX_sync();return;}
#endif

    FrameChange change = FrameState_take(&frame);
    if(NxtMotion_frame(screen,change==FRAME_CHANGED_SURFACE))change=FRAME_CHANGED_SURFACE;
    if (PWR_getDisplayOff()) { GFX_sync(); return; }
    switch (change) {
        case FRAME_CHANGED_SURFACE: GFX_flip(screen);  break;
        case FRAME_CHANGED_LAYERS:  PLAT_GPU_Flip();   break;
        case FRAME_CHANGED_NOTHING:                    break;
    }
    // A present that the platform does not hold to the display rate, such as
    // one with no window, would let the loop run free
    GFX_sync();
}

bool ModuleCommon_handleHIDVolume(USBHIDEvent hid_event) {
    if (hid_event != USB_HID_EVENT_VOLUME_UP && hid_event != USB_HID_EVENT_VOLUME_DOWN) {
        return false;
    }
    
    int delta=hid_event==USB_HID_EVENT_VOLUME_UP?1:-1;
    NxtVolume_setPercent(NxtVolume_getPercent()+delta);
#ifdef NXT_ROM_STOCK
    LOG_info("HID volume: event=%d sink=%d volume=%d/20\n",hid_event,GetAudioSink(),GetVolume());
#endif
    return true;
}
