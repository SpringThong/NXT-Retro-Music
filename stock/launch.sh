#!/bin/sh
# NXT Retro Music v1.9.3 / official TrimUI Brick Pro firmware.
NXT_APP_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)" || exit 1
cd "$NXT_APP_DIR" || exit 1
NXT_STOCK_DATA="/mnt/SDCARD/.userdata/nxt-stock"
export DEVICE=brickpro PLATFORM=tg5040 TRIMUI_MODEL="TrimUI Brick Pro"
export SYSTEM_PATH="$NXT_APP_DIR/compat"
export USERDATA_PATH="$NXT_STOCK_DATA/system"
export SHARED_USERDATA_PATH="$NXT_STOCK_DATA/shared"
export LOGS_PATH="$NXT_STOCK_DATA/logs"
export NXT_STOCK_RUNTIME_DIR="/tmp/nxt-retro-music-stock-$(id -u)"
export NXT_SESSION_SOCKET="$NXT_STOCK_RUNTIME_DIR/session.sock"
# ALSA configuration belongs to this app; /root/.asoundrc stays untouched.
export HOME="$NXT_STOCK_DATA/home"
mkdir -p "$USERDATA_PATH" "$SHARED_USERDATA_PATH/nxt-retro-music" \
    "$LOGS_PATH" "$HOME" "$NXT_STOCK_RUNTIME_DIR" || exit 1
export LD_LIBRARY_PATH="$NXT_APP_DIR/bin/tg5040:/usr/trimui/lib:${LD_LIBRARY_PATH:-}:/usr/lib:/lib"
export SDL_VIDEODRIVER=mali SDL_AUDIODRIVER=alsa LIBASOUND_THREAD_SAFE=0
NXT_BINARY="$NXT_APP_DIR/bin/tg5040/musicplayer.elf"
NXT_SESSION_PID=""
NXT_LAUNCH_PID=""

nxt_stock_recover() {
    # Only resume processes that this app stopped; preserve pre-stopped daemons.
    for NXT_MARKER in "$NXT_STOCK_RUNTIME_DIR"/keymon-*; do
        [ -f "$NXT_MARKER" ] || continue
        NXT_KEYMON_PID=${NXT_MARKER##*keymon-}
        case "$NXT_KEYMON_PID" in ''|*[!0-9]*) continue ;; esac
        if [ "$(cat "/proc/$NXT_KEYMON_PID/comm" 2>/dev/null)" = keymon ]; then
            kill -CONT "$NXT_KEYMON_PID" 2>/dev/null || :
        fi
        rm -f "$NXT_MARKER"
    done
    for NXT_FLAG in stay_awake stay_alive; do
        NXT_MARKER="$NXT_STOCK_RUNTIME_DIR/owned-$NXT_FLAG"
        if [ "$NXT_MARKER" -ef "/tmp/$NXT_FLAG" ]; then rm -f "/tmp/$NXT_FLAG"; fi
        rm -f "$NXT_MARKER"
    done
}

nxt_stock_finish() {
    if [ -n "$NXT_SESSION_PID" ] && kill -0 "$NXT_SESSION_PID" 2>/dev/null; then
        return
    fi
    nxt_stock_recover
    cp "$LOGS_PATH/nxt-retro-music.txt" "$NXT_APP_DIR/NXT_Input_Log.txt" 2>/dev/null || :
}

nxt_stock_signal() {
    if [ -n "$NXT_SESSION_PID" ]; then
        kill -TERM "$NXT_SESSION_PID" 2>/dev/null || :
        NXT_STOP_WAIT=0
        while kill -0 "$NXT_SESSION_PID" 2>/dev/null && [ "$NXT_STOP_WAIT" -lt 50 ]; do
            sleep 0.1
            NXT_STOP_WAIT=$((NXT_STOP_WAIT + 1))
        done
        kill -0 "$NXT_SESSION_PID" 2>/dev/null && kill -KILL "$NXT_SESSION_PID" 2>/dev/null
        [ -z "$NXT_LAUNCH_PID" ] || wait "$NXT_LAUNCH_PID" 2>/dev/null
    fi
    nxt_stock_recover
    exit 1
}
trap nxt_stock_finish EXIT
trap nxt_stock_signal INT TERM

"$NXT_BINARY" --build-info > "$LOGS_PATH/nxt-launch.txt" 2>&1 || exit 1
"$NXT_BINARY" --attach-current >/dev/null 2>> "$LOGS_PATH/nxt-launch.txt"
NXT_ATTACH_RESULT=$?
if [ "$NXT_ATTACH_RESULT" -eq 1 ]; then
    # A crash can leave recovery records; clean them before the next session.
    nxt_stock_recover
    "$NXT_BINARY" --session > "$LOGS_PATH/nxt-retro-music.txt" 2>&1 &
    NXT_LAUNCH_PID=$!
    NXT_SESSION_PID=$NXT_LAUNCH_PID
elif [ "$NXT_ATTACH_RESULT" -ne 0 ]; then
    cp "$LOGS_PATH/nxt-launch.txt" "$NXT_APP_DIR/NXT_Launch_Log.txt" 2>/dev/null || :
    exit "$NXT_ATTACH_RESULT"
fi
NXT_WAIT=0
while [ "$NXT_WAIT" -lt 300 ]; do
    if [ -f "$NXT_SESSION_SOCKET.state" ]; then
        read NXT_VISIBILITY NXT_STATE_PID < "$NXT_SESSION_SOCKET.state"
        if [ -z "$NXT_LAUNCH_PID" ]; then
            case "$NXT_STATE_PID" in ''|*[!0-9]*) ;; *) NXT_SESSION_PID=$NXT_STATE_PID ;; esac
        fi
        if [ "$NXT_VISIBILITY" = VISIBLE ] &&
           { [ -z "$NXT_LAUNCH_PID" ] || [ "$NXT_STATE_PID" = "$NXT_LAUNCH_PID" ]; }; then
            NXT_SESSION_PID=$NXT_STATE_PID
            break
        fi
    fi
    if [ -n "$NXT_LAUNCH_PID" ] && ! kill -0 "$NXT_LAUNCH_PID" 2>/dev/null; then
        wait "$NXT_LAUNCH_PID"; exit 1
    fi
    NXT_WAIT=$((NXT_WAIT + 1))
    sleep 0.1
done
if [ "$NXT_WAIT" -ge 300 ]; then
    [ -z "$NXT_LAUNCH_PID" ] || kill -TERM "$NXT_LAUNCH_PID" 2>/dev/null || :
    exit 1
fi
# Return to stock Apps once the UI hides. Playback and its idle inhibitors
# remain owned by the music process; reopening this app attaches to it.
while [ -f "$NXT_SESSION_SOCKET.state" ]; do
    read NXT_VISIBILITY NXT_STATE_PID < "$NXT_SESSION_SOCKET.state"
    [ "$NXT_VISIBILITY" = VISIBLE ] || break
    kill -0 "$NXT_SESSION_PID" 2>/dev/null || break
    sleep 0.1
done



exit 0
