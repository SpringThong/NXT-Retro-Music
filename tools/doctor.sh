#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
printf 'NXT Retro Music — kiểm tra WSL/native build\n'
printf 'Máy host: %s\n' "$(uname -m)"
if [[ -r /etc/os-release ]]; then . /etc/os-release; printf 'Linux: %s\n' "$PRETTY_NAME"; fi
for nxt_command in git python3 gcc make cmake pkg-config curl file; do
  if command -v "$nxt_command" >/dev/null; then printf '[OK] %s\n' "$nxt_command"; else printf '[THIẾU] %s\n' "$nxt_command"; fi
done
printf 'WSLg: DISPLAY=%s; WAYLAND_DISPLAY=%s\n' "${DISPLAY:-chưa có}" "${WAYLAND_DISPLAY:-chưa có}"
if command -v pkg-config >/dev/null; then
  for nxt_library in sdl2 SDL2_image SDL2_ttf samplerate sqlite3 libzip; do
    if pkg-config --exists "$nxt_library"; then printf '[OK] %s %s\n' "$nxt_library" "$(pkg-config --modversion "$nxt_library")"; else printf '[THIẾU] %s\n' "$nxt_library"; fi
  done
fi
if [[ -x $nxt_sdk/compiler/bin/aarch64-nextui-linux-gnu-gcc ]]; then
  "$nxt_sdk/compiler/bin/aarch64-nextui-linux-gnu-gcc" --version | head -n 1
else
  printf 'SDK ARM64 chưa cài; make nextui sẽ chuẩn bị SDK.\n'
fi
df -h "$nxt_root"
