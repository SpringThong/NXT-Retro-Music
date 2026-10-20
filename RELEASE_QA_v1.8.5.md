# NXT Retro Music v1.8.5 — kiểm tra phát hành

## Phạm vi và kết quả phần mềm

- Ba frontend native AArch64 được build mới bằng SDK/GCC 8.3 đã khóa SHA-256, không dùng lại ELF v1.8.4. Danh sách toàn bộ đầu vào, hash ELF và báo cáo riêng từng ROM đi kèm.
- QEMU/pinned libc nạp ELF thật của cả ba ROM: `--build-info`/`--help` nhận đúng v1.8.5 và ROM. Backend Spotify/SoundCloud được giữ nguyên, chạy và nhận quit qua IPC thật; không chứng nhận streaming đăng nhập.
- CPUFreq production trên cây sysfs giả lập: ba mức 1008/1608/2000 MHz lấy từ bảng tần số; hạ mức tối thiểu đang khóa 2GHz, chọn governor động/userspace, tôn trọng trần phần cứng, trả chính sách cũ khi ẩn/thoát, không ghi đè giới hạn đã đổi bởi app khác. Trường hợp governor không hỗ trợ trả lỗi. Đây không phải chứng nhận driver NextUI trên máy người dùng.
- Worker ghi file production dưới ASan/UBSan: coalesce snapshot mới nhất, đường dẫn từng hồ sơ độc lập, PNG sở hữu bản sao pixel, xóa có thứ tự, lỗi ghi được báo, thoát đợi ghi hết và có thể khởi động lại. JSON fsync và PNG compression không chạy trong UI tick.
- Controller âm thanh production: ba lựa chọn Thường/Bit-Perfect/Tự động, USB attach/detach, raw sẵn sàng trước UI snapshot, lỗi exact PCM fallback một lần, không lặp mở lại DAC, đổi bài/card thử lại, online/Bluetooth/loa dùng Thường. Mô phỏng đầu ra và state được ghi rõ.
- Kiểm tra ghép controller với player core production: đóng SDL trước raw, giữ vị trí/trạng thái khi USB→Thường, Play sau khi worker Thường nạp bài, lỗi exact PCM quay về Thường tại cùng vị trí và không retry liên tục. PCM/decoder phần cứng trong phép thử này là fixture.
- Decoder ARM64 thật: 13 mẫu WAV/FLAC/ALAC, 44.1/48/96/192 kHz, mono/stereo, 16/24/32 bit; PCM nguyên bản và seek đối chiếu từng byte. MP3 bị từ chối ở native integer mode. Chế độ Thường: 10 định dạng và EQ/DSP/ReplayGain/limiter vẫn qua test.
- Worker Bit-Perfect production, simulator ALSA/decoder dưới ASan/UBSan: bốn format integer PCM, byte equality, partial write/EAGAIN, pause, EOF/idle, seek, mất/bận DAC, Hz/nguồn không phù hợp và thoát worker.
- UI production của ba ROM: 36 theme tại 640×480 và 1024×768, câu lời tĩnh, SELECT mở/đóng EQ, NXT gốc/ảnh nền/album, 300 icon/180 emoji, 14 ngôn ngữ/IME, 105 phối phổ và 6 hàm LED. Pixel của cache ảnh opaque giống đường SDL gốc; alpha/color-key vẫn dùng đường gốc vì hai blitter làm tròn khác nhau. Không thay font hoặc hạ độ phân giải.
- Kiểm tra phím chọn hồ sơ ở cold start không hỏi mood/genre/country; các câu hỏi vẫn hoạt động khi mở riêng Cá nhân. Now Playing lưu bài/vị trí theo hồ sơ, thiếu file giữ thông tin, mặc định tạm dừng, tự phát tùy chọn, seek đang chờ không ghi đè vị trí.
- Config cả ba ROM migrate/lưu/reload chế độ hiệu năng và audio_mode, giữ EQ/hình nền/LED; không để hồ sơ ghi đè chế độ thiết bị. Timer đóng trọn phiên app, giữ settings/session, không SIGKILL hay ép suspend.
- Bluetooth helpers/player/msettings production trong Thường: route A2DP và speaker/Bluetooth/DAC reopen/retry, volume/unmute, private ALSA config, PCM local/Spotify/SoundCloud không bằng zero; transport fixture, không tai nghe vật lý.
- Đối chiếu source v1.8.4: 2.083 file source/tài nguyên/runtime không đổi, 20 file native cũ đổi có chủ đích, cùng module mới. Giữ nguyên fonts, theme assets, icon/emoji, locale/IME, backend online và msettings.
- Ba ZIP cài được kiểm tra CRC, metadata/ROM, ELF và quyền executable. Ba publisher kit chứa chính xác ZIP cài tương ứng và SHA-256. OTA worker/installer production thử ZIP đầy đủ từ fixture v1.8.4: size/hash sai, khác ROM/HTML bị từ chối, Drive confirmation, stage/install/backup, giữ userdata/hình nền/bài dở, hủy/reap child process.

## Giới hạn cần thử trên máy

Chưa có TrimUI/DAC/tai nghe thật: chưa đo nhiệt, pin, độ trễ phím, chưa chứng nhận CPU driver/sysfs, analog DAC/Bluetooth, ngủ/thức. Benchmark renderer trên host không phải FPS thiết bị. Chưa đăng nhập dịch vụ streaming và chưa đăng OTA công khai. Deep sleep do ROM quyết định sau khi NXT đóng. Các kiểm tra phần mềm không bảo đảm máy luôn mát hoặc mọi DAC hỗ trợ Bit-Perfect.

Gói dành cho TrimUI Brick Pro/tg5040; chưa xác nhận original TrimUI Brick. Giảm âm lượng trên DAC trước khi thử Bit-Perfect. Lưu ý lỗi sysfs/DAC có trạng thái rõ; không gắn nhãn Bit-Perfect cho âm thanh đã xử lý.

Bằng chứng hiện tại: `logs/v185-*`, `BUILD_REPORT_v1.8.5_<ROM>.json`, `BUILD_INPUTS_v1.8.5.json`. Các báo cáo cũ là lịch sử, không thay cho kết quả v1.8.5.
