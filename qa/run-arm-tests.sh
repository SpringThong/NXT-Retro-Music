#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/../tools/common.sh"
nxt_cross_env
nxt_text="$nxt_root/.nxt-sdk/text-nextui"
"$CC" -O2 -I"$nxt_src/src" -I"$nxt_text/include/SDL2" -I"$nxt_text/include" -I"$nxt_text/include/harfbuzz" -I"$nxt_text/include/fribidi" -I"$SYSROOT/usr/include/SDL2" \
 "$nxt_root/qa/test_native_text.c" "$nxt_src/src/nxt_text.c" -L"$SYSROOT/usr/lib" -lSDL2_image \
 "$nxt_text/lib/libSDL2_ttf.a" "$nxt_text/lib/libharfbuzz.a" "$nxt_text/lib/libfreetype.a" "$nxt_text/lib/libutf8proc.a" "$nxt_text/lib/libfribidi.a" -lSDL2 -lstdc++ -lm -lz -lpthread -o "$nxt_root/qa/test_native_text.arm64"
cd "$nxt_src"
nxt_run_aarch64 "$nxt_root/qa/test_native_text.arm64" "$nxt_root/qa/screens"
