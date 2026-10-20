#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/../tools/common.sh"
nxt_cross_env
nxt_text="$nxt_root/.nxt-sdk/text-nextui"
"$CC" -O2 -D_GNU_SOURCE -DUSE_SDL2 -ffunction-sections -fdata-sections -Wl,--gc-sections \
 -I"$nxt_src/src" -I"$SYSROOT/usr/include/SDL2" -I"$nxt_src/NextUI/workspace/all/common" -I"$nxt_src/NextUI/workspace/tg5040/platform" \
 -I"$nxt_text/include/SDL2" -I"$nxt_text/include" -I"$nxt_text/include/harfbuzz" -I"$nxt_text/include/fribidi" \
 "$nxt_root/qa/test_native_persistence.c" "$nxt_root/overlay/src/nxt_presets.c" "$nxt_root/overlay/src/playlist_m3u.c" "$nxt_root/overlay/src/nxt_text.c" \
 "$nxt_text/lib/libutf8proc.a" -lm -o "$nxt_root/qa/test_native_persistence.arm64"
nxt_run_aarch64 "$nxt_root/qa/test_native_persistence.arm64"
