#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
[[ ${1:-} != --help ]] || { printf '%s\n' 'Chuẩn bị GCC 8.3 + sysroot TG5040 và build hai thư viện tĩnh; không dùng Docker.'; exit 0; }
nxt_start_log sdk
python3 "$nxt_root/tools/sdk-assets.py"
nxt_cross_env
"$CC" --version
printf 'Sysroot compiler: %s\n' "$("$CC" -print-sysroot)"
mkdir -p "$PREFIX_LOCAL/lib" "$PREFIX_LOCAL/include" "$nxt_root/.cache/src"
if [[ -f $nxt_sdk/.libraries-ready && -f $PREFIX_LOCAL/lib/libzip.a && -f $PREFIX_LOCAL/lib/libsamplerate.a ]]; then
  printf 'Các thư viện ARM64 đã sẵn sàng.\n'
  exit 0
fi
nxt_fetch_library() {
  local nxt_name=$1 nxt_url=$2 nxt_ref=$3
  local nxt_archive=""
  if [[ $nxt_name == samplerate ]]; then nxt_archive="$nxt_root/sources/libsamplerate-0.2.2-source.tar.gz"; fi
  if [[ $nxt_name == libzip ]]; then nxt_archive="$nxt_root/sources/libzip-1.10.1-source.tar.gz"; fi
  if [[ ! -e $nxt_root/.cache/src/$nxt_name && -f $nxt_archive ]]; then
    (cd "$nxt_root/sources" && sha256sum -c "$(basename "$nxt_archive").sha256")
    mkdir -p "$nxt_root/.cache/src/$nxt_name"
    tar -xzf "$nxt_archive" -C "$nxt_root/.cache/src/$nxt_name"
    printf '%s\n' "$nxt_ref" > "$nxt_root/.cache/src/$nxt_name/.nxt-release"
  fi
  if [[ -f $nxt_root/.cache/src/$nxt_name/.nxt-release && $(cat "$nxt_root/.cache/src/$nxt_name/.nxt-release") == "$nxt_ref" ]]; then return; fi
  if [[ ! -e $nxt_root/.cache/src/$nxt_name ]]; then
    git clone --depth 1 --branch "$nxt_ref" "$nxt_url" "$nxt_root/.cache/src/$nxt_name"
  fi
  [[ -d $nxt_root/.cache/src/$nxt_name/.git ]] || { printf 'Thư mục thư viện không phải Git checkout.\n' >&2; exit 1; }
  [[ $(git -C "$nxt_root/.cache/src/$nxt_name" describe --tags --exact-match HEAD) == "$nxt_ref" ]] || { printf 'Thư viện không ở phiên bản đã chọn.\n' >&2; exit 1; }
}
nxt_fetch_library samplerate https://github.com/libsndfile/libsamplerate.git 0.2.2
nxt_fetch_library libzip https://github.com/nih-at/libzip.git v1.10.1
# CMake 4 rejects dependency projects that request policy compatibility < 3.5.
# Use its documented external compatibility override for these pinned sources.
cmake -S "$nxt_root/.cache/src/samplerate" -B "$nxt_sdk/build-samplerate" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_TOOLCHAIN_FILE="$nxt_root/tools/toolchain.cmake" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX_LOCAL" -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DLIBSAMPLERATE_EXAMPLES=OFF
cmake --build "$nxt_sdk/build-samplerate" --parallel "$nxt_jobs"
cmake --install "$nxt_sdk/build-samplerate"
cmake -S "$nxt_root/.cache/src/libzip" -B "$nxt_sdk/build-libzip" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_TOOLCHAIN_FILE="$nxt_root/tools/toolchain.cmake" \
  -DCMAKE_INSTALL_PREFIX="$PREFIX_LOCAL" -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF \
  -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF -DBUILD_OSSFUZZ=OFF \
  -DENABLE_BZIP2=OFF -DENABLE_LZMA=OFF -DENABLE_ZSTD=OFF \
  -DENABLE_OPENSSL=OFF -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF
cmake --build "$nxt_sdk/build-libzip" --parallel "$nxt_jobs"
cmake --install "$nxt_sdk/build-libzip"
printf 'libsamplerate 0.2.2; libzip v1.10.1; static ARM64\n' > "$nxt_sdk/.libraries-ready"
printf '\nSDK sẵn sàng. Bước tiếp theo: make nextui\n'
