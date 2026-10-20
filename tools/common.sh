#!/usr/bin/env bash
set -euo pipefail
nxt_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
nxt_src="$nxt_root/upstream"
nxt_sdk="$nxt_root/.nxt-sdk/tg5040"
nxt_jobs="${NXT_JOBS:-4}"
nxt_build_tools="${NXT_BUILD_TOOLS:-$nxt_root/../.build-tools}"
if [[ -d $nxt_build_tools ]]; then
  export PYTHONPATH="$nxt_build_tools${PYTHONPATH:+:$PYTHONPATH}"
  export PATH="$nxt_build_tools/bin:$nxt_root/.host-tools/usr/bin:$PATH"
fi
[[ $nxt_jobs =~ ^[1-9][0-9]*$ ]] || { printf 'NXT_JOBS phải là số nguyên dương.\n' >&2; exit 2; }

nxt_require_source() {
  [[ -f $nxt_src/src/Makefile && -f $nxt_src/NextUI/workspace/all/common/api.h ]] || { printf 'Chưa có mã nguồn/phụ thuộc. Chạy bash bootstrap-wsl.sh trước.\n' >&2; exit 1; }
}

nxt_start_log() {
  mkdir -p "$nxt_root/logs"
  nxt_log="$nxt_root/logs/$(date -u +%Y%m%dT%H%M%SZ)-$1-$$.log"
  printf 'Log: %s\n' "$nxt_log"
  exec > >(tee -a "$nxt_log") 2>&1
  trap 'nxt_code=$?; if ((nxt_code != 0)); then printf "\nDừng do lỗi (mã %s). Gửi phần cuối log: %s\n" "$nxt_code" "$nxt_log" >&2; fi' EXIT
}

nxt_cross_env() {
  [[ -x $nxt_sdk/compiler/bin/aarch64-nextui-linux-gnu-gcc ]] || { printf 'Chưa có SDK. Chạy make sdk trước.\n' >&2; exit 1; }
  unset CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH LIBRARY_PATH PKG_CONFIG_PATH
  export CROSS_TRIPLE=aarch64-nextui-linux-gnu
  export CROSS_ROOT="$nxt_sdk/compiler"
  export SYSROOT="$CROSS_ROOT/$CROSS_TRIPLE/libc"
  export CROSS_COMPILE="$CROSS_ROOT/bin/$CROSS_TRIPLE-"
  export CC="${CROSS_COMPILE}gcc" CXX="${CROSS_COMPILE}g++" AR="${CROSS_COMPILE}ar"
  export AS="${CROSS_COMPILE}as" LD="${CROSS_COMPILE}ld" RANLIB="${CROSS_COMPILE}ranlib"
  export STRIP="${CROSS_COMPILE}strip"
  export PREFIX="$SYSROOT/usr" PREFIX_LOCAL="$nxt_sdk/local"
  export PKG_CONFIG_SYSROOT_DIR="$SYSROOT"
  export PKG_CONFIG_LIBDIR="$SYSROOT/usr/lib/pkgconfig:$SYSROOT/usr/share/pkgconfig"
  export UNION_PLATFORM=tg5040
  export PATH="$CROSS_ROOT/bin:$PATH"
}

nxt_run_aarch64() {
  local nxt_emulator="${NXT_QEMU_AARCH64:-qemu-aarch64}"
  if [[ -z ${NXT_QEMU_AARCH64:-} && -x $nxt_root/.host-tools/usr/bin/qemu-aarch64 ]]; then
    nxt_emulator="$nxt_root/.host-tools/usr/bin/qemu-aarch64"
  fi
  command -v "$nxt_emulator" >/dev/null || { printf 'Cần qemu-user (qemu-aarch64) để chạy QA ARM64.\n' >&2; return 1; }
  "$nxt_emulator" -L "$SYSROOT" "$@"
}
