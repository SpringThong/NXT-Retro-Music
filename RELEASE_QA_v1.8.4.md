# NXT Retro Music v1.8.4 — Kiểm tra phát hành

## Các phần đã kiểm tra

- Ba frontend C native compile đầy đủ cho Stock, NextUI, NX Redux bằng GCC 8.3.0/SDK đã khóa SHA-256. ELF AArch64 nạp được bằng QEMU/pinned libc; --build-info và --help nhận đúng v1.8.4/từng ROM. Backend Spotify/SoundCloud khởi động và nhận quit qua IPC thật; chưa đăng nhập tài khoản.
- Decoder ARM64 thật: 13 mẫu WAV/FLAC/ALAC ở 44.1/48/96/192 kHz, mono/stereo, nguồn 16/24/32 bit. Dữ liệu PCM nguyên bản và vị trí tua đối chiếu từng byte với fixture gốc; nguồn MP3 bị từ chối trong native integer mode.
- Chế độ Thường: 10 định dạng WAV/MP3/FLAC/AAC/ALAC/Vorbis/Opus, giải mã/tua bằng FFmpeg ARM64; DSP bypass/ReplayGain, 10/15/31-band EQ và limiter vẫn qua kiểm tra.
- Worker Bit-Perfect C thật dùng simulator ALSA/decoder rõ ràng: S16_LE/S24_3LE/S24_LE/S32_LE, sample bytes, partial write/EAGAIN, tạm dừng không gửi PCM, EOF/nghỉ, tua/tiếp tục, mất DAC, bận DAC, Hz không phù hợp, nguồn không phù hợp, không fallback plug/dmix/loa, thoát worker. ASan/UBSan qua; simulator không chứng nhận phần cứng.
- UI production code render tại 640×480 và 1024×768: 36 giao diện/lời nguyên câu, giữ NXT gốc/ảnh nền/album và nút, 300 icon/180 emoji, 105 phối phổ và 6 hàm màu LED. SELECT release đóng preset mà không đổi EQ, đóng toàn bộ mục con, đóng màn hình bypass. Lưu bài/vị trí theo hồ sơ, giữ file thiếu, mở Now Playing trống, khôi phục tạm dừng, tự phát tùy chọn và không ghi đè vị trí khi seek đang chờ. Audio/ROM input trong kiểm tra UI là fixture.
- Config production của cả ba ROM: migration từ dữ liệu cũ, mặc định English/Cân bằng/Thường/tạm dừng, lưu/load Bit-Perfect và tự phát, giữ chế độ thiết bị khi đổi hồ sơ.
- Timer/config/session C của cả ba ROM: deadline/hủy/thay timer/wrap, tạm dừng/tắt màn hình, radio/podcast, lưu vị trí/cài đặt rồi đóng cả phiên nền. Không thêm SIGKILL, lệnh ép suspend hoặc daemon.
- Bluetooth production helpers trong chế độ Thường: route A2DP, private ALSA config, reopen/retry, mixer/unmute và PCM local/Spotify/SoundCloud vẫn hoạt động với transport fixture. Firmware settings source không thay đổi.
- Đối chiếu v1.8.3: 2.074 file source/assets/runtime không đổi, gồm renderer/themes/fonts/icon/emoji/locales/IME và backend online; 23 file native cũ thay đổi có chủ đích, cộng các module mới.
- Ba ZIP cài: CRC, ELF AArch64, executable bits, đúng cây Apps/Tools và ROM metadata. Bộ OTA chứa chính xác ZIP cài và SHA-256 của ROM đó.
- OTA worker/installer production kiểm tra với ZIP đầy đủ: nâng từ fixture v1.8.3, Drive confirmation, size/SHA sai bị từ chối, tải/stage/cài, backup app cũ, giữ userdata/appearance/last-playback, từ chối khác ROM/HTML, hủy và thu hồi child process. Nguồn tải trong bài thử là fixture, không phải link công khai.

## Giới hạn

Không có TrimUI/DAC/tai nghe thật trong môi trường build. Chưa thử âm thanh USB/Bluetooth trên thiết bị, chưa đo pin/nhiệt hoặc ngủ/thức. Chưa thử streaming đăng nhập và chưa tự đăng nguồn OTA công khai. Bit-Perfect trên máy chỉ báo sẵn sàng sau khi hw PCM mở đúng Hz/kênh/format/precision; DAC bận hoặc không phù hợp sẽ dừng, không hạ chất lượng âm thầm. Việc vào deep sleep do chính sách ROM sau khi NXT đóng.

Bằng chứng JSON/log và hash native source/binary/gói cài/OTA nằm trong BUILD_REPORT_v1.8.4_<ROM>.json cùng logs/v184-*. Các báo cáo phiên bản cũ được giữ như tài liệu lịch sử và không được dùng thay cho bằng chứng bản này.
