#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
[[ ${1:-} == nextui || ${1:-} == desktop ]] || exit 2
nxt_text_target=$1
nxt_text_prefix="$nxt_root/.nxt-sdk/text-$1"
nxt_text_build="$nxt_root/.cache/build/text-$1"
mkdir -p "$nxt_text_prefix/include/SDL2" "$nxt_text_prefix/include/harfbuzz" "$nxt_text_prefix/lib" "$nxt_text_build"
if [[ -f $nxt_text_prefix/.ready ]]; then exit 0; fi
for nxt_input in 'freetype:freetype-2.13.2.tar.gz' 'harfbuzz:harfbuzz-8.3.0.tar.gz' 'sdl-ttf:SDL2_ttf-2.24.0.tar.gz' 'utf8proc:utf8proc-2.9.0.tar.gz' 'fribidi:fribidi-1.0.13.tar.xz'; do
  nxt_folder="${nxt_input%%:*}"; nxt_archive="${nxt_input#*:}"
  if [[ ! -d $nxt_root/.cache/src/$nxt_folder ]]; then
    (cd "$nxt_root/sources" && sha256sum -c "$nxt_archive.sha256")
    mkdir -p "$nxt_root/.cache/src/$nxt_folder"
    tar -xf "$nxt_root/sources/$nxt_archive" --strip-components=1 -C "$nxt_root/.cache/src/$nxt_folder"
  fi
done

if [[ $1 == nextui ]]; then
  nxt_cross_env
  nxt_text_inc="$SYSROOT/usr/include"
  nxt_text_cmake=(-DCMAKE_TOOLCHAIN_FILE="$nxt_root/tools/toolchain.cmake")
else
  export CC=/usr/bin/gcc CXX=/usr/bin/g++ AR=/usr/bin/ar RANLIB=/usr/bin/ranlib
  nxt_text_inc="${NXT_HOST_PREFIX:-/usr}/include"
  if [[ -z ${NXT_HOST_PREFIX:-} && -d $nxt_root/.host-tools/usr/include/SDL2 ]]; then nxt_text_inc="$nxt_root/.host-tools/usr/include"; fi
  nxt_text_cmake=()
fi
cmake -S "$nxt_root/.cache/src/freetype" -B "$nxt_text_build/freetype" "${nxt_text_cmake[@]}" \
  -DCMAKE_INSTALL_PREFIX="$nxt_text_prefix" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
  -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_ZLIB=ON
cmake --build "$nxt_text_build/freetype" --parallel "$nxt_jobs"
cmake --install "$nxt_text_build/freetype"
"$CXX" -O2 -std=c++11 -fPIC -fno-exceptions -fno-rtti -DHAVE_FREETYPE \
  -I"$nxt_text_prefix/include/freetype2" -I"$nxt_root/.cache/src/harfbuzz/src" \
  -c "$nxt_root/.cache/src/harfbuzz/src/harfbuzz.cc" -o "$nxt_text_build/harfbuzz.o"
"$AR" rcs "$nxt_text_prefix/lib/libharfbuzz.a" "$nxt_text_build/harfbuzz.o"
cp "$nxt_root"/.cache/src/harfbuzz/src/*.h "$nxt_text_prefix/include/harfbuzz/"
"$CC" -O2 -fPIC -DTTF_USE_HARFBUZZ=1 -I"$nxt_text_inc" -I"$nxt_text_inc/x86_64-linux-gnu" -I"$nxt_text_inc/SDL2" \
  -I"$nxt_text_prefix/include/freetype2" -I"$nxt_text_prefix/include/harfbuzz" \
  -c "$nxt_root/.cache/src/sdl-ttf/SDL_ttf.c" -o "$nxt_text_build/ttf.o"
"$AR" rcs "$nxt_text_prefix/lib/libSDL2_ttf.a" "$nxt_text_build/ttf.o"
cp "$nxt_root/.cache/src/sdl-ttf/SDL_ttf.h" "$nxt_text_prefix/include/SDL2/"
"$CC" -O2 -fPIC -DUTF8PROC_STATIC -c "$nxt_root/.cache/src/utf8proc/utf8proc.c" -o "$nxt_text_build/utf8proc.o"
"$AR" rcs "$nxt_text_prefix/lib/libutf8proc.a" "$nxt_text_build/utf8proc.o"
cp "$nxt_root/.cache/src/utf8proc/utf8proc.h" "$nxt_text_prefix/include/"
cat > "$nxt_text_build/native.ini" <<'INI'
[binaries]
c = '/usr/bin/gcc'
ar = '/usr/bin/ar'
strip = '/usr/bin/strip'
INI
nxt_text_meson=()
if [[ $1 == nextui ]]; then
  cat > "$nxt_text_build/cross.ini" <<INI
[binaries]
c = '$CC'
ar = '$AR'
strip = '$STRIP'
[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'
INI
  nxt_text_meson=(--cross-file="$nxt_text_build/cross.ini")
fi
meson setup "$nxt_text_build/fribidi" "$nxt_root/.cache/src/fribidi" \
  --native-file="$nxt_text_build/native.ini" "${nxt_text_meson[@]}" \
  --prefix="$nxt_text_prefix" --libdir=lib --buildtype=release --default-library=static \
  -Ddocs=false -Dtests=false -Dbin=false
meson compile -C "$nxt_text_build/fribidi" -j "$nxt_jobs"
meson install -C "$nxt_text_build/fribidi"
printf '%s\n' 'FreeType 2.13.2; HarfBuzz 8.3.0; SDL2_ttf 2.24.0; utf8proc 2.9.0; FriBidi 1.0.13' > "$nxt_text_prefix/.ready"
