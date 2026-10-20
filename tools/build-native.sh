#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
if [[ ${1:-} == --help ]]; then
  printf '%s\n' 'bash tools/build-native.sh desktop|nextui' 'Build trực tiếp bằng make, không gọi Docker/Podman.'
  exit 0
fi
[[ $# == 1 && ( $1 == desktop || $1 == nextui ) ]] || { printf 'Chọn desktop hoặc nextui.\n' >&2; exit 2; }
if [[ $1 == nextui ]]; then
  python3 "$nxt_root/tools/build-nextui.py"
  python3 "$nxt_root/tools/sync-overlay.py"
  python3 "$nxt_root/tools/package-nextui.py"
  exit 0
fi
nxt_require_source
if [[ ! -f $nxt_src/.nxt-overlay-version ]]; then python3 "$nxt_root/tools/apply-source.py"; fi
nxt_start_log "$1"
bash "$nxt_root/tools/setup-audio.sh" "$1"
bash "$nxt_root/tools/setup-text.sh" "$1"
printf 'Nguồn: %s\n' "$(git -C "$nxt_src" rev-parse HEAD)"
git -C "$nxt_src" status --short
if [[ $1 == desktop ]]; then
  unset CROSS_COMPILE CROSS_TRIPLE SYSROOT PREFIX PREFIX_LOCAL PKG_CONFIG_SYSROOT_DIR PKG_CONFIG_LIBDIR
  bash "$nxt_src/NextUI/workspace/desktop/prepare_fake_sd_root.sh"
  make -C "$nxt_src/NextUI/workspace/desktop/libmsettings" build \
    CROSS_COMPILE=/usr/bin/ PREFIX=/usr PREFIX_LOCAL=/var/tmp/nextui
  make -C "$nxt_src/src" -j "$nxt_jobs" PLATFORM=desktop KIND=debug \
    CROSS_COMPILE=/usr/bin/ PREFIX=/usr PREFIX_LOCAL=/var/tmp/nextui "NXT_AUDIO_PREFIX=$nxt_root/.nxt-sdk/audio-desktop" "NXT_TEXT_PREFIX=$nxt_root/.nxt-sdk/text-desktop"
  mkdir -p /var/tmp/nextui/sdcard/{Music,Podcasts,.userdata/shared,.userdata/desktop}
  file "$nxt_src/bin/desktop/musicplayer.elf"
  printf '\nBản PC đã build xong. Chạy: make run\n'
else
  if [[ ! -f $nxt_sdk/.libraries-ready ]]; then
    bash "$nxt_root/tools/setup-sdk.sh"
  fi
  nxt_cross_env
  "$CC" --version
  make -C "$nxt_src/NextUI/workspace/tg5040/libmsettings" build
  nxt_native_ldflags='-Wl,--as-needed'
  if [[ ${NXT_KIND:-release} == release ]]; then nxt_native_ldflags+=' -s'; fi
  make -C "$nxt_src/src" -j "$nxt_jobs" PLATFORM=tg5040 KIND=${NXT_KIND:-release} "NXT_AUDIO_PREFIX=$nxt_root/.nxt-sdk/audio-nextui" "NXT_TEXT_PREFIX=$nxt_root/.nxt-sdk/text-nextui" \
    "MY_LDFLAGS=$nxt_native_ldflags -L$nxt_src/src/build/${NXT_KIND:-release}/tg5040 -L$PREFIX_LOCAL/lib -L$SYSROOT/usr/lib -L$SYSROOT/usr/lib/$CROSS_TRIPLE -L$nxt_src/NextUI/workspace/tg5040/libmsettings -Wl,-rpath-link,$SYSROOT/usr/lib:$SYSROOT/lib"
  python3 "$nxt_root/tools/package-nextui.py"
  "$CROSS_ROOT/bin/$CROSS_TRIPLE-readelf" -h "$nxt_src/bin/tg5040/musicplayer.elf"
  "$CROSS_ROOT/bin/$CROSS_TRIPLE-readelf" -d "$nxt_src/bin/tg5040/musicplayer.elf" | awk '/NEEDED/ { print }'
  printf '\nPak v1.1: %s\n' "$nxt_root/dist/NXT_Retro_Music_v1.1_NextUI.zip"
fi
