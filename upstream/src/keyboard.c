#include "keyboard.h"
#include "defines.h"
#include "api.h"
#include "display_helper.h"
#include "help_screen.h"
#include "module_common.h"
#include "keyboard_map.h"
#include "utf8.h"
#include "ui_keyboard.h"
#include "ui_utils.h"
#include "nxt.h"
#include "nxt_telex.h"
#include "nxt_ime.h"
#include "nxt_text.h"
#include <stdio.h>
#include <ctype.h>

#include <stdlib.h>
#include <string.h>

// How long A is held on a character key before its alternates come up.
#define KEYBOARD_HOLD_MS 250

static Uint32 navigation_due[4];
static bool navigation_repeat(int b){int i=b==BTN_UP?0:b==BTN_DOWN?1:b==BTN_LEFT?2:3;Uint32 now=SDL_GetTicks();if(!NxtPad_isPressed(b)){navigation_due[i]=0;return false;}if(NxtPad_justPressed(b)||!navigation_due[i]){navigation_due[i]=now+210;return true;}if((Sint32)(now-navigation_due[i])>=0){navigation_due[i]=now+45;return true;}return false;}
typedef struct {
    const char *prompt;
    char text[KEYBOARD_MAX_INPUT];
    size_t text_limit; // bytes the text may grow to, terminator aside
    int text_version; // bumped on every edit, so a redraw sees one
    int show_setting;
    const Keyboard *keyboard;
    const KeyboardGeometry *geometry;
    int layout_index;
    const KeyboardLayout *current_layout;
    int row, col;
    const Key *current_key; // the key at row and col
    ShiftState shift;
    bool pressed; // the key under the cursor is held down
    bool picking_variant; // the alternates of the current key are up
    int current_variant; // the alternate being picked, while they are
    const char *current_char; // what that key types now, "" where it types nothing
    NxtTelex telex;
    char ime_raw[KEYBOARD_MAX_INPUT],ime_prefix[KEYBOARD_MAX_INPUT];
} KeyboardState;


static void prepare_ui_state(const KeyboardState *in, KeyboardUiState *out) {
    out->title = in->prompt;
    out->show_setting = in->show_setting;
    out->text = in->text;

    out->row = in->row;
    out->col = in->col;
    out->layout = in->current_layout;
    out->lang_label = in->current_layout->name;
    out->shift = in->shift;
    out->pressed = in->pressed;
    out->picking_variant = in->picking_variant;
    // Canonical while the alternates are down, so a stale pick is not a redraw
    out->current_variant = in->picking_variant ? in->current_variant : 0;
}

void Keyboard_init(void) {
    UIKeyboard_init();
}

// Number of keys in the raw.
static int row_length(const KeyboardState *state, int row) {
    const KeyboardGeometry *g = state->geometry;
    if (row < 0 || row >= g->rows) return 0;

    int length = 0;
    while (g->keys[row][length]) length++;
    return length;
}

// Update current key and char, must be called after any change that affects
// key/char selection (cursor, layout, shift change etc)
static void update_current_key(KeyboardState *state) {
    state->current_key = state->geometry->keys[state->row][state->col];

    const Key *key = state->current_key;
    if (key->action == KEY_SPACE) {
        state->current_char = " ";
        return;
    }
    if (key->action != KEY_TEXT) {
        state->current_char = "";
        return;
    }

    int variant = state->picking_variant ? state->current_variant : 0;
    state->current_char = KeyboardMap_char(state->current_layout, key,
                                           state->shift != SHIFT_OFF, variant);
}

static void next_layout(KeyboardState *state) {
    state->layout_index = (state->layout_index + 1) % state->keyboard->layout_count;
    state->current_layout = state->keyboard->layouts[state->layout_index];
    NxtTelex_reset(&state->telex,state->text);
    snprintf(state->ime_prefix,sizeof(state->ime_prefix),"%s",state->text);state->ime_raw[0]=0;
    update_current_key(state);
}

static void set_shift(KeyboardState *state, ShiftState shift) {
    state->shift = shift;
    update_current_key(state);
}

static void tap_shift(KeyboardState *state) {
    set_shift(state, (state->shift == SHIFT_OFF) ? SHIFT_ONCE : SHIFT_OFF);
}

static void tap_caps(KeyboardState *state) {
    set_shift(state, (state->shift == SHIFT_LOCKED) ? SHIFT_OFF : SHIFT_LOCKED);
}

static void open_variants_panel(KeyboardState *state) {
    state->picking_variant = true;
    state->current_variant = 0;
    update_current_key(state);
}

static void close_variants_panel(KeyboardState *state) {
    state->picking_variant = false;
    update_current_key(state);
}

static void set_current_variant(KeyboardState *state, int variant) {
    state->current_variant = variant;
    update_current_key(state);
}

static void move_vertical(KeyboardState *state, int step) {
    int target_row = state->row + step;
    if (target_row < 0 || target_row >= state->geometry->rows) {
        return;
    }

    int target_col = (step > 0) ? state->current_key->col_down : state->current_key->col_up;
    if (target_col == KEY_NO_COLUMN) return;

    int length = row_length(state, target_row);
    if (target_col >= length) return;

    // The column led to may hold a gap, which the cursor cannot sit on
    const Key *const*keys = state->geometry->keys[target_row];
    while (target_col < length - 1 && keys[target_col]->action == KEY_SPACER) target_col++;
    while (target_col > 0 && keys[target_col]->action == KEY_SPACER) target_col--;

    state->row = target_row;
    state->col = target_col;
    update_current_key(state);
}

static void move_horizontal(KeyboardState *state, int step) {
    const Key *const*keys = state->geometry->keys[state->row];
    int length = row_length(state, state->row);
    if (length <= 0) return;

    for (int attempt = 0; attempt < length; attempt++) {
        state->col = (state->col + step + length) % length;
        if (keys[state->col]->action != KEY_SPACER) break;
    }

    update_current_key(state);
}

#include "nxt_keyboard_ime.inc"
// Add a character (whole UTF-8 only), if it fits.
static void append_text(KeyboardState *state, const char *addition) {
    if(ime_language(state)>=0){if(!strcmp(addition," "))ime_convert(state);else ime_append(state,addition);return;}
    if(!strcmp(state->current_layout->name,"TELEX")) {
        if(NxtTelex_append(&state->telex,state->text,state->text_limit+1,addition))
            state->text_version++;
        return;
    }
    size_t length = strlen(state->text);
    size_t addition_length = strlen(addition);
    if (addition_length == 0) return;
    if (length + addition_length > state->text_limit) return;

    memcpy(state->text + length, addition, addition_length + 1);
    state->text_version++;
}

static void backspace(KeyboardState *state) {
    if(ime_language(state)>=0){ime_backspace(state);return;}
    if(!strcmp(state->current_layout->name,"TELEX")) {
        if(NxtTelex_backspace(&state->telex,state->text,state->text_limit+1))
            state->text_version++;
        return;
    }
    const char *p=state->text,*last=p;while(*p){last=p;p=NxtText_nextCluster(p);}
    if(last==p)return;state->text[last-state->text]=0;
    state->text_version++;
}

static char *open_keyboard(const char *prompt, size_t max_bytes, bool latin) {
    DisplayContext *display = DisplayHelper_current();

    size_t limit = KEYBOARD_MAX_INPUT - 1;
    if (max_bytes > 0 && max_bytes < limit) limit = max_bytes;

    const Keyboard *keyboard = KeyboardMap_get();
    if (keyboard->layout_count <= 0) return NULL;

    int initial_layout=latin?1:nxt.language==0?1:nxt.language==1?0:nxt.language;
    if(initial_layout<0||initial_layout>=keyboard->layout_count)initial_layout=1;
    // Opens on "a": start of the home row, one step from caps and shift
    memset(navigation_due,0,sizeof(navigation_due));
    KeyboardState state = {
        .prompt = prompt,
        .text_limit = limit,
        .keyboard = keyboard,
        .geometry = keyboard->geometry,
        .layout_index = initial_layout,
        .current_layout = keyboard->layouts[initial_layout],
        .row = keyboard->home_row,
        .col = 1,
        .shift = SHIFT_OFF,
        .pressed = false,
    };
    update_current_key(&state);
    NxtTelex_reset(&state.telex,"");

    bool confirmed = false;
    bool done = false;

    // A character key types on release, so holding it can bring up its
    // alternates instead
    bool holding = false;

    // Set once a hold has done its work, so the release does not act again
    bool hold_consumed = false;

    // A direction pressed while a held key would do something unwanted takes
    // the press back: the way out of an A landing on enter or cancel by
    // accident. Typing a character or a space is not worth undoing.
    bool cancelled = false;

    // A button whose press went to a dialog must not act here when it comes
    // back up: closing the controls help with X is not a shift
    bool swallow_buttons = false;
    uint32_t held_since = 0;

    // The states are compared bytewise, so both must start out zeroed, padding
    // included.
    KeyboardUiState ui_state;
    KeyboardUiState last_ui_state;
    memset(&ui_state, 0, sizeof ui_state);
    memset(&last_ui_state, 0, sizeof ui_state);
    // The keyboard is a screen inside a module, thus the title of the module
    // starts again, in its own mode, when the keyboard returns
    bool outer_title_defer = ScreenTitle_start(false);

    // text version tracking is needed because keyboard UI state equality is shallow
    int last_text_version = -1;

    // Except a frame the keyboard does not own: an overlay leaves the screen
    // needing a redraw the state cannot tell us about
    bool force_render = false;

    while (!done) {
        ModuleCommon_frameBegin();
        SDL_Surface *const screen = DisplayHelper_getSurface(display);

        GlobalInputResult global = ModuleCommon_handleGlobalInput(screen, &state.show_setting,
                                                                  HELP_KEYBOARD);
        if (global.should_quit) {
            // The app is quitting; the caller sees it on its own next frame.
            // Nothing was confirmed, so the exit below returns NULL.
            break;
        }
        if (global.input_consumed) {
            // The overlay drew over the keyboard, or animated: draw it again
            // once the state alone would not say so
            if (global.dirty) force_render = true;
            swallow_buttons = true;
            ModuleCommon_frameEnd(screen);
            continue;
        }

        // Wait for the pad to come up before reading it again
        if (swallow_buttons) {
            if (NxtPad_isPressed(BTN_A) || NxtPad_isPressed(BTN_B) || NxtPad_isPressed(BTN_X) ||
                NxtPad_isPressed(BTN_Y) || NxtPad_isPressed(BTN_SELECT)) {
                ModuleCommon_frameEnd(screen);
                continue;
            }
            swallow_buttons = false;
        }

        state.pressed = NxtPad_isPressed(BTN_A) && !cancelled;

        const KeyboardLayout *layout = state.current_layout;
        const Key *key = state.current_key;
        int variant_count = KeyboardMap_charCount(layout, key);

        // A held button owns the pad, except while the alternates are up, where
        // left and right pick between them
        bool button_held = NxtPad_isPressed(BTN_A) || NxtPad_isPressed(BTN_B) ||
                           NxtPad_isPressed(BTN_X) || NxtPad_isPressed(BTN_Y) ||
                           NxtPad_isPressed(BTN_SELECT);

        if (state.picking_variant) {
            if (navigation_repeat(BTN_LEFT) && state.current_variant > 0) {
                set_current_variant(&state, state.current_variant - 1);
            } else if (navigation_repeat(BTN_RIGHT) && state.current_variant < variant_count - 1) {
                set_current_variant(&state, state.current_variant + 1);
            }
        } else if (!button_held) {
            if (navigation_repeat(BTN_UP)) {
                move_vertical(&state, -1);
            } else if (navigation_repeat(BTN_DOWN)) {
                move_vertical(&state, 1);
            } else if (navigation_repeat(BTN_LEFT)) {
                move_horizontal(&state, -1);
            } else if (navigation_repeat(BTN_RIGHT)) {
                move_horizontal(&state, 1);
            }
        }

        bool cancellable = (key->action != KEY_TEXT && key->action != KEY_SPACE);

        if (holding && !cancelled && cancellable &&
            (NxtPad_justPressed(BTN_UP) || NxtPad_justPressed(BTN_DOWN) ||
             NxtPad_justPressed(BTN_LEFT) || NxtPad_justPressed(BTN_RIGHT))) {
            cancelled = true;
            hold_consumed = true;
        } else if (NxtPad_justRepeated(BTN_A) && !cancelled &&
                   key->action == KEY_BACKSPACE) {
            // The one key that works like the B button: it deletes on the way
            // down and keeps deleting while it is held. The press is still a
            // hold as far as the rest of the loop is concerned, so a direction
            // can call it off; its release has nothing left to do.
            if (NxtPad_justPressed(BTN_A)) {
                holding = true;
                hold_consumed = true;
                cancelled = false;
                held_since = SDL_GetTicks();
            }
            backspace(&state);
        } else if (NxtPad_justPressed(BTN_A)) {
            // Every other key acts on release, so a hold can mean something
            // else - the alternates on a character key, the lock on shift
            holding = true;
            hold_consumed = false;
            cancelled = false;
            held_since = SDL_GetTicks();
        } else if (holding && !NxtPad_isPressed(BTN_A)) {
            if (hold_consumed) {
                holding = false;
                cancelled = false;
                close_variants_panel(&state);
                ModuleCommon_frameEnd(screen);
                continue;
            }

            switch (key->action) {
                case KEY_TEXT:
                case KEY_SPACE:
                    append_text(&state, state.current_char);
                    if (state.shift == SHIFT_ONCE) set_shift(&state, SHIFT_OFF);
                    break;
                case KEY_CAPS:
                    tap_caps(&state);
                    break;
                case KEY_SHIFT:
                    tap_shift(&state);
                    break;
                case KEY_LANG:
                    next_layout(&state);
                    break;
                case KEY_ENTER:
                    confirmed = true;
                    done = true;
                    break;
                case KEY_CANCEL:
                    done = true;
                    break;
                case KEY_SPACER:
                case KEY_TAB:
                    // no-op
                    break;
                case KEY_BACKSPACE: // already deleted on the way down
                    break;
            }

            holding = false;
            close_variants_panel(&state);
        } else if (holding && !hold_consumed && !state.picking_variant &&
                   SDL_GetTicks() - held_since >= KEYBOARD_HOLD_MS) {
            // Once: reopening every frame would put the pick back on the first
            // alternate as fast as Left and Right could move it
            if (key->action == KEY_TEXT) {
                // Even a key with a single character opens, so the gesture is
                // the same wherever the cursor is
                open_variants_panel(&state);
            }
        } else if (NxtPad_justPressed(BTN_B) && state.text[0] == '\0') {
            // B backs out of an empty field, the way it leaves any other screen
            done = true;
        } else if (NxtPad_justRepeated(BTN_B)) {
            // Held down it keeps deleting, and stops at the empty field rather
            // than leaving through it
            if (state.text[0] != '\0') {
                backspace(&state);
            }
        } else if (NxtPad_justPressed(BTN_X)) {
            append_text(&state," ");
        } else if (NxtPad_justPressed(BTN_L1)) {
            tap_shift(&state);
        } else if (NxtPad_justPressed(BTN_R1)) {
            tap_caps(&state);
        } else if (NxtPad_justPressed(BTN_Y)) {
            next_layout(&state);
        } else if (NxtPad_justPressed(BTN_SELECT)) {
            confirmed = true;
            done = true;
        }

        if (done) break;

        prepare_ui_state(&state, &ui_state);

        if (force_render ||
            state.text_version != last_text_version ||
            !UIKeyboard_stateEquals(&ui_state, &last_ui_state)
        ) {
            UIKeyboard_render(screen, &ui_state);
            ModuleCommon_markSurfaceDrawn();

            memcpy(&last_ui_state, &ui_state, sizeof last_ui_state);
            last_text_version = state.text_version;
            force_render = false;
        }
        // A title that needs a surface frame asks for it through the redraw flag
        int title_frame = 0;
        ScreenTitle_frameEnd(&title_frame, true);
        if (title_frame) force_render = true;
        ModuleCommon_frameEnd(screen);
    }

    ScreenTitle_start(outer_title_defer);

    if (!confirmed || state.text[0] == '\0') return NULL;

    char *result = malloc(strlen(state.text) + 1);
    if (!result) return NULL;
    strcpy(result, state.text);
    return result;
}

char *Keyboard_open(const char *prompt,size_t max_bytes) {
    return open_keyboard(prompt,max_bytes,false);
}
char *Keyboard_openLatin(const char *prompt,size_t max_bytes) {
    return open_keyboard(prompt,max_bytes,true);
}
