#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/common.sh"
[[ $# == 1 && ( $1 == desktop || $1 == nextui ) ]] || exit 2
nxt_audio_target=$1
if [[ $nxt_audio_target == nextui && ! -f $nxt_sdk/.libraries-ready ]]; then bash "$nxt_root/tools/setup-sdk.sh"; fi
nxt_audio_src="$nxt_root/.cache/src/ffmpeg"
nxt_audio_build="$nxt_root/.cache/build/ffmpeg-$nxt_audio_target"
nxt_audio_prefix="$nxt_root/.nxt-sdk/audio-$nxt_audio_target"
nxt_audio_commit=f1e3a2bf7a2f2cde936d1ed97f09a26853d20125
if [[ -f $nxt_audio_prefix/.ready && $(cat "$nxt_audio_prefix/.ready") == "$nxt_audio_commit" ]]; then exit 0; fi
if [[ -f $nxt_root/sources/ffmpeg-n6.1.6.tar.gz && ! -e $nxt_audio_src ]]; then
  mkdir -p "$nxt_audio_src"
  (cd "$nxt_root/sources" && sha256sum -c ffmpeg-n6.1.6.tar.gz.sha256)
  tar -xzf "$nxt_root/sources/ffmpeg-n6.1.6.tar.gz" -C "$nxt_audio_src"
  printf '%s\n' "$nxt_audio_commit" > "$nxt_audio_src/.nxt-source-commit"
elif [[ ! -d $nxt_audio_src/.git && ! -f $nxt_audio_src/.nxt-source-commit ]]; then
  git clone --depth 1 --branch n6.1.6 https://github.com/FFmpeg/FFmpeg.git "$nxt_audio_src"
fi
if [[ -d $nxt_audio_src/.git ]]; then nxt_audio_actual=$(git -C "$nxt_audio_src" rev-parse HEAD);else nxt_audio_actual=$(cat "$nxt_audio_src/.nxt-source-commit");fi
[[ $nxt_audio_actual == "$nxt_audio_commit" ]] || { printf 'FFmpeg source không đúng commit đã chọn.\n' >&2; exit 1; }
mkdir -p "$nxt_audio_build" "$nxt_audio_prefix"
nxt_audio_cross=()
if [[ $nxt_audio_target == nextui ]]; then
  nxt_cross_env
  nxt_audio_cross=(--enable-cross-compile --arch=aarch64 --target-os=linux "--cross-prefix=$CROSS_COMPILE" "--sysroot=$SYSROOT")
fi
cd "$nxt_audio_build"
"$nxt_audio_src/configure" "${nxt_audio_cross[@]}" --prefix="$nxt_audio_prefix" \
  --disable-everything --disable-autodetect --disable-doc --disable-programs --disable-network \
  --disable-avdevice --disable-avfilter --disable-postproc --disable-swscale --disable-shared \
  --enable-static --enable-pic --enable-small --disable-debug --disable-x86asm \
  --enable-avformat --enable-avcodec --enable-avutil --enable-swresample \
  --enable-protocol=file --enable-demuxer=mov,mp3,wav,flac,ogg,aac \
  --enable-decoder=alac,aac,mp3float,flac,vorbis,opus,pcm_s16le,pcm_s24le,pcm_s32le,pcm_s16be,pcm_s24be,pcm_s32be,pcm_f32le,pcm_f64le,pcm_u8 \
  --enable-parser=aac,mpegaudio,flac,opus,vorbis --enable-zlib
make -j "$nxt_jobs"
make install
printf '%s\n' "$nxt_audio_commit" > "$nxt_audio_prefix/.ready"
