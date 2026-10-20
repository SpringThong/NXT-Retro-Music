#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
nxt_cross_env
mapfile -d '' nxt_stock_objects < <(find "$nxt_root/stock/relink/obj" -type f -name '*.o' -print0 | sort -z)
nxt_stock_expected=$(python3 -c 'import json,sys; print(len(json.load(open(sys.argv[1]))))' "$nxt_root/stock/frontend-sources.json")
[[ ${#nxt_stock_objects[@]} -eq "$nxt_stock_expected" ]] || { printf 'Missing Stock frontend objects.\n' >&2; exit 1; }
mkdir -p "$nxt_root/dist/stock-relinked"
"$CC" "${nxt_stock_objects[@]}" -s -Wl,--as-needed \
  -L"$nxt_root/stock/bin/tg5040" -L"$nxt_root/relink/lib" -L"$SYSROOT/usr/lib" \
  -Wl,-rpath-link,"$SYSROOT/usr/lib:$SYSROOT/lib" \
  "$nxt_root/relink/lib/libopus.a" "$nxt_root/relink/lib/libmbedtls.a" \
  "$nxt_root/relink/lib/ffmpeg/libavformat.a" "$nxt_root/relink/lib/ffmpeg/libavcodec.a" \
  "$nxt_root/relink/lib/ffmpeg/libswresample.a" "$nxt_root/relink/lib/ffmpeg/libavutil.a" \
  -lSDL2_image "$nxt_root/relink/lib/text/libSDL2_ttf.a" \
  "$nxt_root/relink/lib/text/libharfbuzz.a" "$nxt_root/relink/lib/text/libfreetype.a" \
  "$nxt_root/relink/lib/text/libutf8proc.a" "$nxt_root/relink/lib/text/libfribidi.a" \
  -lstdc++ -lmsettings "$nxt_root/relink/lib/libsamplerate.a" "$nxt_root/relink/lib/libzip.a" \
  -lsqlite3 -lm -lpthread -ldl -lz -lSDL2 -lGLESv2 -lfdk-aac -lasound \
  -o "$nxt_root/dist/stock-relinked/musicplayer.elf"
printf 'Stock relinked: %s\n' "$nxt_root/dist/stock-relinked/musicplayer.elf"
