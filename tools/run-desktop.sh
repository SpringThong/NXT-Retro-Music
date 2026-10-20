#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
nxt_require_source
[[ -x $nxt_src/bin/desktop/musicplayer.elf ]] || { printf 'Chạy make desktop trước.\n' >&2; exit 1; }
if [[ -z ${DISPLAY:-} && -z ${WAYLAND_DISPLAY:-} && ${SDL_VIDEODRIVER:-} != offscreen ]]; then
  printf 'WSLg chưa có DISPLAY/WAYLAND_DISPLAY. Mở Ubuntu trực tiếp trong Windows để chạy giao diện.\n' >&2
  exit 1
fi
mkdir -p /var/tmp/nextui/sdcard/{Music,Podcasts,.userdata/shared,.userdata/desktop}
export PLATFORM=desktop
export USERDATA_PATH=/var/tmp/nextui/sdcard/.userdata/desktop
export SHARED_USERDATA_PATH=/var/tmp/nextui/sdcard/.userdata/shared
export SYSTEM_PATH=/var/tmp/nextui/sdcard/.system/desktop
export LD_LIBRARY_PATH="/var/tmp/nextui/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$nxt_src"
printf '%s\n' 'Nhạc thử: /var/tmp/nextui/sdcard/Music' 'Phím mũi tên: di chuyển; S: A/xác nhận; A: B/quay lại; Enter: Start/trợ giúp, giữ để thoát.'
exec ./bin/desktop/musicplayer.elf "$@"
