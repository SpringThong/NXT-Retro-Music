#!/bin/sh
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$DIR" || exit 1
PLATFORM="${PLATFORM:-tg5040}"
export PLATFORM
export LD_LIBRARY_PATH="$DIR:$DIR/bin:$DIR/bin/$PLATFORM:${LD_LIBRARY_PATH:-}:/usr/bin"
export LIBASOUND_THREAD_SAFE=0
export NXT_SESSION_SOCKET="${NXT_SESSION_SOCKET:-/tmp/nxt-retro-music-spotify-$(id -u).sock}"
LOGDIR="${LOGS_PATH:-${SHARED_USERDATA_PATH:-/tmp}/nxt-retro-music-spotify}"
mkdir -p "$LOGDIR"
BIN="$DIR/bin/$PLATFORM/musicplayer.elf"
NXT_LAUNCH_PID=""
"$BIN" --attach-current >/dev/null 2>&1
NXT_ATTACH_RESULT=$?
if [ "$NXT_ATTACH_RESULT" -eq 1 ]; then
    "$BIN" --session > "$LOGDIR/nxt-retro-music.txt" 2>&1 &
    NXT_LAUNCH_PID=$!
elif [ "$NXT_ATTACH_RESULT" -ne 0 ]; then exit "$NXT_ATTACH_RESULT"
fi
# Wait for the existing session to recover its display, or the new one to start.
NXT_WAIT=0
while [ "$NXT_WAIT" -lt 300 ]; do
    if [ -f "$NXT_SESSION_SOCKET.state" ]; then
        read NXT_VISIBILITY NXT_SESSION_PID < "$NXT_SESSION_SOCKET.state"
        if [ "$NXT_VISIBILITY" = VISIBLE ] && { [ -z "$NXT_LAUNCH_PID" ] || [ "$NXT_SESSION_PID" = "$NXT_LAUNCH_PID" ]; }; then break; fi
    fi
    if [ -n "$NXT_LAUNCH_PID" ] && ! kill -0 "$NXT_LAUNCH_PID" 2>/dev/null; then exit 1; fi
    NXT_WAIT=$((NXT_WAIT + 1))
    sleep 0.1
done
[ "$NXT_WAIT" -lt 300 ] || exit 1
# The shell returns to NextUI once the frontend hides; the audio process stays.
while [ -f "$NXT_SESSION_SOCKET.state" ]; do
    read NXT_VISIBILITY NXT_SESSION_PID < "$NXT_SESSION_SOCKET.state"
    [ "$NXT_VISIBILITY" = VISIBLE ] || break
    kill -0 "$NXT_SESSION_PID" 2>/dev/null || break
    sleep 0.1
done

# Keep the finished session log accessible beside the Pak for device testing.
cp "$LOGDIR/nxt-retro-music.txt" "$DIR/NXT_Input_Log.txt" 2>/dev/null || :


