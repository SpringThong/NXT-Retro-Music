#!/usr/bin/env bash
set -euo pipefail

nxt_kit_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ ${1:-} == --help ]]; then
  printf '%s\n' 'bash bootstrap-wsl.sh' 'Tạo dự án tại ~/projects/nxt-retro-music; cài công cụ Ubuntu và lấy Music Player gốc.' 'Có thể đặt NXT_PROJECT_DIR thành đường dẫn tuyệt đối khác.'
  exit 0
fi
[[ $# == 0 ]] || { printf 'Tham số không hợp lệ.\n' >&2; exit 2; }
[[ $(id -u) != 0 ]] || { printf 'Mở Ubuntu bằng tài khoản thường; không chạy script bằng sudo.\n' >&2; exit 1; }
[[ $(uname -m) == x86_64 ]] || { printf 'Bộ này dành cho WSL x86_64 của PC Windows.\n' >&2; exit 1; }
command -v apt-get >/dev/null || { printf 'Cần Ubuntu/Debian với apt-get.\n' >&2; exit 1; }
command -v sudo >/dev/null || { printf 'Không tìm thấy sudo.\n' >&2; exit 1; }
nxt_project="${NXT_PROJECT_DIR:-$HOME/projects/nxt-retro-music}"
[[ $nxt_project == /* && $nxt_project != / && $nxt_project != /mnt/* && $nxt_project != *[[:space:]]* ]] || { printf 'Dùng đường dẫn Linux tuyệt đối, không có khoảng trắng, ví dụ ~/projects/nxt-retro-music.\n' >&2; exit 1; }

printf '\nCài công cụ build trong Ubuntu. sudo sẽ hỏi mật khẩu Linux; khi gõ không hiện ký tự.\n'
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  git python3 build-essential pkg-config cmake ninja-build meson curl ca-certificates \
  xz-utils zip unzip file rsync \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
  libsamplerate0-dev libsqlite3-dev libzip-dev libgl1-mesa-dev

if [[ $nxt_kit_dir != "$nxt_project" ]]; then
  python3 "$nxt_kit_dir/update-project.py" "$nxt_project" --kit-only
fi
cd "$nxt_project"
mkdir -p logs .cache dist
nxt_origin='https://github.com/nborodikhin/nextui-music-player.git'
nxt_pin='f0e5e2ea2fea4f535d659b72aa1d2a8e969e5484'
if [[ -e upstream ]]; then
  if [[ ! -d upstream/.git ]]; then
    [[ -f upstream/src/nxt_text.c && -f SOURCE.json ]] || { printf 'Thư mục upstream không phải nguồn NXT đã đóng gói.\n' >&2; exit 1; }
    git init -q upstream
    git -C upstream config core.autocrlf false
    git -C upstream remote add origin "$nxt_origin"
    git -C upstream -c user.name='NXT Source Kit' -c user.email='source@localhost' add .
    git -C upstream -c user.name='NXT Source Kit' -c user.email='source@localhost' commit -qm 'NXT v1.0 bundled corresponding source'
  fi
  [[ $(git -C upstream remote get-url origin) == "$nxt_origin" ]] || { printf 'Origin của upstream khác bản nền đã chọn; dừng để giữ dữ liệu.\n' >&2; exit 1; }
  printf 'Dùng lại mã nguồn upstream hiện có, không reset hoặc đổi nhánh.\n'
else
  git init -q upstream
  git -C upstream config core.autocrlf false
  git -C upstream remote add origin "$nxt_origin"
  git -C upstream fetch --depth 1 origin "$nxt_pin"
  git -C upstream checkout -b nxt-nextui FETCH_HEAD
fi
if [[ ! -f upstream/NextUI/workspace/all/common/api.h ]]; then
  (cd upstream && ./dev deps fetch nextui)
fi
python3 tools/apply-source.py
chmod +x tools/*.sh bootstrap-wsl.sh
printf '\nĐã chuẩn bị dự án: %s\n' "$nxt_project"
printf '%s\n' 'Bước tiếp theo:' "cd \"$nxt_project\"" 'make nextui'
