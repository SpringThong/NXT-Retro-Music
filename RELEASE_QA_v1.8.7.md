# NXT Retro Music v1.8.7 — kiểm tra phát hành

## Kiểm tra phần mềm hiện tại

- Hai đường vào trang thay đổi (Tính năng & thay đổi và phím X từ Thông tin ứng dụng) hiển thị v1.8.7 trong ảnh render native tiếng Việt/Anh ở 640×480 và 1024×768. Nội dung phân biệt thay đổi mới với tính năng giữ từ v1.8.5–1.8.6. Nhãn dùng NXT_VERSION để tránh lệch phiên bản sau này.
- 50 tên/ảnh/geometry; 14 ảnh mới khác nhau. Vùng album/tên bài/phổ nhạc mới không chồng nhau, ảnh vừa khung. Chữ chính, chữ phụ và màu nhấn trên nền/panel của theme mới có tỷ lệ tương phản ít nhất 4,5:1. Build-time generator lặp lại cho cùng bytes.
- Trang cuối có hai lựa chọn, wrap trái/phải và lên/xuống giữ cột; nhãn Đang chọn vừa ô. Config/save/load đủ 50 ID trên ba ROM, hồ sơ đọc được ID 49, giá trị ngoài miền được giới hạn an toàn. Giữ ID 0–35 của v1.8.6.
- Giữ pipeline độ nét, frame budgets, cache chữ/ảnh, xử lý ẩn/tắt màn hình và worker ghi nền của v1.8.6. Khung mới tải/scale khi chọn rồi cache; không tải 50 ảnh vào RAM khi phát. Benchmark trong log là máy host, không phải FPS/nhiệt của TrimUI.

- Ba ELF native AArch64 build mới bằng GCC 8.3/SDK khóa SHA-256. Danh sách đầy đủ đầu vào, hash binary và báo cáo riêng từng ROM đi kèm. QEMU/pinned libc nạp các ELF thật với `--build-info`/`--help` đúng phiên bản/ROM; backend Spotify/SoundCloud nhận quit qua IPC thật nhưng chưa đăng nhập tài khoản thật.
- Volume controller production cả ba ROM dưới ASan/UBSan: chọn mixer PCM playback thay cho Mic/Capture, range/mute/all-channel writes, clamp 0–20, cache handle và đổi card, lỗi/missing mixer chuyển Thường với software gain; không sửa PCM trong worker Bit-Perfect. Mixer/driver/ROM là fixture, chưa chứng nhận DAC vật lý.
- Input C production cả ba ROM: SELECT + hướng, +/- máy, HID DAC và phím volume gán đều gọi volume controller khi raw mode đang bật, không bị chặn và không phát thông báo ép chỉnh trên DAC.
- Audio policy production: Thường/Bit-Perfect/Tự động, nguồn online tạm dùng Thường trong cả lựa chọn Manual/Automatic, URI Drive/HTTP/HTTPS/cache Drive không mở raw, trở lại file nội bộ có thể mở raw. Giữ lựa chọn mode đã lưu. Volume cần gain phần mềm không bị tick lập tức chuyển lại raw. Có test attach/detach, lỗi exact PCM chỉ fallback một lần, retry khi đổi bài/card, manual incompatibility và không lặp mở DAC.
- Ghép controller với player core production: đóng SDL trước raw, giữ vị trí/trạng thái khi raw→Thường, giữ vị trí để lưu resume khi chuyển sang nguồn mới, Play sau normal load, lỗi PCM fallback và USB/Bluetooth policy. Chuyển source với `reload=false` không gọi quit các provider đang chọn. Decoder/raw PCM trong phép thử core là fixture.
- Spotify/SoundCloud frontend C và audio policy thật với helper IPC giả lập: từ trạng thái Bit-Perfect chọn bài online, effective mode về Thường, giữ child/queue, gửi lệnh play, xử lý metadata/pause/seek/background và quit/reap. Các bài thử dùng thông tin FAKE, không xác thực tài khoản thật hoặc nghe máy chủ dịch vụ.
- Google Drive: ARM64 FFmpeg/NXT decoder và URI/metadata production, URI công khai giả lập được resolve từ JSON và giải mã sang normal float PCM, đọc/tua có dữ liệu; cached WAV/UTF-8 sidecar đọc và chuyển 44.1→48 kHz. Lớp HTTPS buffer được thay bằng FILE transport fixture; không gọi Google Drive thật trong bài thử.
- Decoder ARM64 thật: 13 mẫu lossless WAV/FLAC/ALAC ở 44.1/48/96/192 kHz, mono/stereo, 16/24/32 bit; integer PCM và seek đối chiếu từng byte; MP3 bị từ chối trong native integer mode. Normal mode vẫn qua 10 định dạng, EQ/DSP/ReplayGain/limiter.
- Bit-Perfect worker production với ALSA/decoder simulator: bốn format integer, byte equality, partial writes/EAGAIN, paused silence, EOF/idle, seek, mất/bận DAC, Hz/source không phù hợp và graceful quit. ASan/UBSan qua; worker không thêm gain phần mềm.
- UI production cả ba ROM: 50 theme, 640×480/1024×768, câu lời tĩnh, SELECT toggle, profile trước Now Playing và resume riêng hồ sơ, 300 icon/180 emoji, 14 ngôn ngữ, 105 phối phổ và 6 hàm LED. Thư viện & tệp chỉ có Thư mục nhạc/Chuyển file Wi-Fi; ảnh chụp native được kiểm tra. Pixel cache album opaque giống đường SDL gốc; alpha dùng đường gốc.
- Đối chiếu source v1.8.6: 2.101 file source/assets/runtime không đổi, 10 file cũ đổi có chủ đích (ELF NextUI và 9 file nguồn phiên bản/theme/ghi chú). Giữ nguyên 36 ảnh khung giao diện cũ, font, icon, emoji, ngôn ngữ, renderer chữ/album, CPUFreq, background writer, startup và các module âm thanh/volume/route. Backend và msettings source giữ nguyên; thêm 14 khung tĩnh mới.
- CPUFreq và disk writer production vẫn qua regression fixture: trần/support/governor/minimum/restore policy, coalesced profile snapshots, PNG owns pixels, ordered remove, write failure và quit drain. Config/timer/session cả ba ROM giữ chế độ thiết bị, EQ/appearance và đóng trọn app khi timer hết.
- Bluetooth production helpers/player/msettings trong Thường vẫn qua private ALSA route, reopen/retry/volume/unmute và nonzero local/Spotify/SoundCloud PCM với transport fixture. Không kiểm tra tai nghe vật lý.
- Ba installer ZIP: CRC, cây Apps/Tools, metadata đúng ROM/phiên bản, executable bits và ELF AArch64. Ba OTA publisher kit chứa chính xác installer và SHA-256. OTA worker/installer C thật thử full ZIP nâng từ fixture v1.8.6: Drive confirmation, hash/size sai, khác ROM/HTML, stage/install/backup, giữ userdata/hình nền/bài nghe dở, cancel/reap child.

## Giới hạn

Chưa thử trên TrimUI/DAC/Bluetooth vật lý, chưa đo nhiệt/pin/độ trễ, chưa chứng nhận mixer của mọi DAC hoặc hành vi sleep/wakeup. Chưa đăng nhập các dịch vụ streaming và chưa đăng nguồn OTA công khai. Các fixture/benchmark host không phải phép đo thiết bị. Ngủ sâu do ROM quyết định sau khi app đóng.

Gói dành cho TrimUI Brick Pro/tg5040, chưa xác nhận original TrimUI Brick. Gain phần mềm được ghi nhận là Thường; chỉ raw PCM phù hợp mới dùng nhãn Bit-Perfect. Giảm volume DAC trước khi đổi đường phát vì các mức hardware/software có thể khác nhau.

Bằng chứng: `logs/v187*`, `BUILD_REPORT_v1.8.7_<ROM>.json`, `BUILD_INPUTS_v1.8.7.json`. Báo cáo cũ là lịch sử. Source chỉ giữ ảnh QA của phiên bản hiện tại để giảm dung lượng, không cắt tài nguyên runtime.
