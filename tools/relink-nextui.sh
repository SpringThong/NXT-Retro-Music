#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
if [[ ! -f $nxt_sdk/.libraries-ready ]]; then bash "$nxt_root/tools/setup-sdk.sh"; fi
nxt_cross_env
nxt_ffmpeg_lib="${NXT_FFMPEG_LIB_DIR:-$nxt_root/relink/lib/ffmpeg}"
nxt_text_lib="${NXT_TEXT_LIB_DIR:-$nxt_root/relink/lib/text}"
mapfile -d '' nxt_relink_objects < <(find "$nxt_root/relink/obj" -type f -name '*.o' -print0 | sort -z)
[[ ${#nxt_relink_objects[@]} -gt 0 ]] || { printf 'Thiếu object để relink.\n' >&2; exit 1; }
mkdir -p "$nxt_root/dist/relinked"
"$CC" "${nxt_relink_objects[@]}" -Wl,--as-needed \
  -L"$nxt_root/relink/lib" -L"$SYSROOT/usr/lib" -L"$SYSROOT/usr/lib/$CROSS_TRIPLE" \
  -Wl,-rpath-link,"$SYSROOT/usr/lib:$SYSROOT/lib" \
  "$nxt_root/relink/lib/libopus.a" "$nxt_root/relink/lib/libmbedtls.a" \
  "$nxt_ffmpeg_lib/libavformat.a" "$nxt_ffmpeg_lib/libavcodec.a" \
  "$nxt_ffmpeg_lib/libswresample.a" "$nxt_ffmpeg_lib/libavutil.a" \
  -lSDL2_image "$nxt_text_lib/libSDL2_ttf.a" "$nxt_text_lib/libharfbuzz.a" \
  "$nxt_text_lib/libfreetype.a" "$nxt_text_lib/libutf8proc.a" "$nxt_text_lib/libfribidi.a" -lstdc++ -lmsettings \
  "$nxt_root/relink/lib/libsamplerate.a" "$nxt_root/relink/lib/libzip.a" \
  -lsqlite3 -lm -lpthread -ldl -lz -lSDL2 -lGLESv2 -lfdk-aac -lasound \
  -o "$nxt_root/dist/relinked/musicplayer.elf"
"${CROSS_COMPILE}readelf" -h "$nxt_root/dist/relinked/musicplayer.elf"
printf 'Đã relink: %s\n' "$nxt_root/dist/relinked/musicplayer.elf"
