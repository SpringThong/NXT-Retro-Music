#!/bin/sh
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$DIR" || exit 1
PLATFORM="${PLATFORM:-tg5040}"
if [ "$PLATFORM" != tg5040 ] || [ "${DEVICE:-}" != brickpro ]; then
    printf '%s\n' 'NXT v1.9.7.7 NX Redux: this build requires TrimUI Brick Pro.' > "$DIR/NXT_Input_Log.txt"
    exit 1
fi
NXT_VERSION_FILE="${SYSTEM_PATH:-}/version.txt"
[ -r "$NXT_VERSION_FILE" ] || NXT_VERSION_FILE="${SDCARD_PATH:-/mnt/SDCARD}/.system/version.txt"
NXT_FIRMWARE_VERSION=""
[ ! -r "$NXT_VERSION_FILE" ] || read -r NXT_FIRMWARE_VERSION < "$NXT_VERSION_FILE"
case "$NXT_FIRMWARE_VERSION" in NXRedux*|NX\ Redux*) ;; *)
    printf '%s\n' 'NXT v1.9.7.7 NX Redux: launch this Pak from NX Redux Tools.' > "$DIR/NXT_Input_Log.txt"
    exit 1 ;;
esac
NXT_NXREDUX_SETTINGS_LIB="${SYSTEM_PATH:-}/lib/libmsettings.so"
if [ ! -r "$NXT_NXREDUX_SETTINGS_LIB" ]; then
    printf '%s\n' 'NXT: NX Redux firmware settings library is unavailable.' > "$DIR/NXT_Input_Log.txt"
    exit 1
fi
export PLATFORM NXT_NXREDUX_SETTINGS_LIB
export LD_LIBRARY_PATH="$DIR:$DIR/bin:$DIR/bin/$PLATFORM:${LD_LIBRARY_PATH:-}:/usr/bin"
export LIBASOUND_THREAD_SAFE=0
export NXT_SESSION_SOCKET="${NXT_SESSION_SOCKET:-/tmp/nxt-retro-music-nxredux-$(id -u).sock}"
LOGDIR="${SHARED_USERDATA_PATH:-/tmp}/nxt-retro-music/logs"
mkdir -p "$LOGDIR" || exit 1
# Gracefully shut down the ROM music service, preserving its saved queue.
# NXT serves its Home protocol while active, releasing it on normal exit.
NXT_SYSTEM_CTL="${SYSTEM_PATH:-}/bin/musicplayerctl.elf"
if [ -x "$NXT_SYSTEM_CTL" ]; then
    if command -v timeout >/dev/null 2>&1; then timeout 2 "$NXT_SYSTEM_CTL" shutdown >/dev/null 2>&1 || :
    else "$NXT_SYSTEM_CTL" shutdown >/dev/null 2>&1 || :; fi
fi
sh "$DIR/integration/install-tools-icon.sh" > "$LOGDIR/nxt-tools-icon.txt" 2>&1 || :
BIN="$DIR/bin/$PLATFORM/musicplayer.elf"
NXT_LAUNCH_PID=""
"$BIN" --attach-current >/dev/null 2>&1
NXT_ATTACH_RESULT=$?
if [ "$NXT_ATTACH_RESULT" -eq 1 ]; then
    "$BIN" --session > "$LOGDIR/nxt-retro-music-nxredux.txt" 2>&1 &
    NXT_LAUNCH_PID=$!
elif [ "$NXT_ATTACH_RESULT" -ne 0 ]; then exit "$NXT_ATTACH_RESULT"
fi
NXT_WAIT=0
while [ "$NXT_WAIT" -lt 300 ]; do
    if [ -f "$NXT_SESSION_SOCKET.state" ]; then
        read NXT_VISIBILITY NXT_SESSION_PID < "$NXT_SESSION_SOCKET.state"
        if [ "$NXT_VISIBILITY" = VISIBLE ] && { [ -z "$NXT_LAUNCH_PID" ] || [ "$NXT_SESSION_PID" = "$NXT_LAUNCH_PID" ]; }; then break; fi
    fi
    if [ -n "$NXT_LAUNCH_PID" ] && ! kill -0 "$NXT_LAUNCH_PID" 2>/dev/null; then
        cp "$LOGDIR/nxt-retro-music-nxredux.txt" "$DIR/NXT_Input_Log.txt" 2>/dev/null || :
        exit 1
    fi
    NXT_WAIT=$((NXT_WAIT+1));sleep 0.1
done
[ "$NXT_WAIT" -lt 300 ] || exit 1
while [ -f "$NXT_SESSION_SOCKET.state" ]; do
    read NXT_VISIBILITY NXT_SESSION_PID < "$NXT_SESSION_SOCKET.state"
    [ "$NXT_VISIBILITY" = VISIBLE ] || break
    kill -0 "$NXT_SESSION_PID" 2>/dev/null || break
    sleep 0.1
done
cp "$LOGDIR/nxt-retro-music-nxredux.txt" "$DIR/NXT_Input_Log.txt" 2>/dev/null || :
