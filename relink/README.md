# Relink NXT Retro Music v1.0 ARM64

`obj/` chứa object ứng dụng **release v1.0** đã dùng cho bản cài. `lib/ffmpeg/` và `lib/text/` chứa static library tương ứng; các thư viện Opus/mbedTLS/libsamplerate/libzip và libmsettings cũng kèm. Đây là object ARM64, không phải bản PC.

Sau khi chuẩn bị compiler/sysroot bằng `bash tools/setup-sdk.sh`, chạy từ gốc dự án:

```bash
bash tools/relink-nextui.sh
```

Để thay dependency LGPL bằng bản đã sửa có API/ABI tương thích, build ARM64 rồi chỉ định thư mục static library:

```bash
NXT_FFMPEG_LIB_DIR=/absolute/path/to/ffmpeg/lib \
NXT_TEXT_LIB_DIR=/absolute/path/to/text/lib \
bash tools/relink-nextui.sh
```

Thư mục FFmpeg cần libavformat.a/libavcodec.a/libswresample.a/libavutil.a. Thư mục text cần libSDL2_ttf.a/libharfbuzz.a/libfreetype.a/libutf8proc.a/libfribidi.a; có thể sao chép các library không sửa từ `relink/lib/text/`.

Kết quả: `dist/relinked/musicplayer.elf`; thay file `bin/tg5040/musicplayer.elf` trong Pak. Có thể sửa/rebuild toàn bộ bằng source tại `upstream/`, `overlay/` và archive tại `sources/`. Configure flags ở `tools/setup-audio.sh` và `tools/setup-text.sh`; FFmpeg không bật GPL/nonfree. Không có hạn chế ngăn việc sửa, debug hay thay thư viện LGPL.
