#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/../tools/common.sh"
nxt_cross_env
nxt_audio="$nxt_root/.nxt-sdk/audio-nextui"
nxt_text="$nxt_root/.nxt-sdk/text-nextui"
"$CC" -O2 -D_GNU_SOURCE -I"$nxt_src/src" -I"$SYSROOT/usr/include/SDL2" -I"$nxt_src/NextUI/workspace/all/common" -I"$nxt_src/NextUI/workspace/tg5040/platform" -I"$nxt_audio/include" -I"$nxt_text/include/SDL2" -I"$nxt_text/include" -I"$nxt_text/include/harfbuzz" -I"$nxt_text/include/fribidi" \
 "$nxt_root/qa/test_native_audio.c" "$nxt_src/src/nxt_decoder.c" "$nxt_src/src/nxt_dsp.c" "$nxt_src/src/nxt_text.c" -L"$SYSROOT/usr/lib" \
 "$nxt_audio/lib/libavformat.a" "$nxt_audio/lib/libavcodec.a" "$nxt_audio/lib/libswresample.a" "$nxt_audio/lib/libavutil.a" -lSDL2_image "$nxt_text/lib/libSDL2_ttf.a" "$nxt_text/lib/libharfbuzz.a" "$nxt_text/lib/libfreetype.a" "$nxt_text/lib/libutf8proc.a" "$nxt_text/lib/libfribidi.a" -lstdc++ -lSDL2 -lm -lz -lpthread -o "$nxt_root/qa/test_native_audio.arm64"
nxt_run_aarch64 "$nxt_root/qa/test_native_audio.arm64" "$nxt_root/qa/media"
