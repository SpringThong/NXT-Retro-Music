# NXT Retro Music v1.8.8 — QA phát hành

- Ba frontend ELF AArch64 mới; pinned GCC/sysroot; QEMU --build-info/--help đúng ROM và v1.8.8. Tất cả fingerprint input C/h/inc/binary/installer/OTA được xác minh.
- Worker/curl/cache production qua ASan/UBSan với server/lyrics/ROM fixtures: default Off không thread/request, dedup, cache/offline, 14 đích, English/CJK/manual nguồn, chunk UTF-8 ≤480 byte, lỗi HTTP/quota/JSON/quyền/oversize, cooldown qua đổi bài và retry.
- Hủy kết quả cũ, getter từ chối lời mới trước tick kế tiếp, pause khi ẩn/tắt màn, giữ lời gốc; hủy/reap child <700 ms trong fixture. Test cả trường hợp stdin của GUI đã đóng.
- Google v2 key FAKE qua stdin, không trong argv/log; file 0600. Không nhúng key thật. MyMemory và Google Cloud dùng API đã ghi trong hướng dẫn.
- Parser lời gốc production: revision chỉ đổi khi dữ liệu đổi; thay hai lời có cùng số câu vẫn invalidates; UTF-8, mốc LRC/offset và plain unsynced giữ nguyên.
- Renderer production ba ROM ở 640×480/1024×768: 50 player dịch một câu, full-screen song ngữ và controller vào từ Playback/START. Font/layout/album/vinyl/RGB cũ giữ. Bản dịch trong ảnh là fixture tự viết.
- Config/save/load/migration/profile cả ba ROM: mode 0/1/2, source 0..14, hồ sơ cũ Off, 50 ID theme; giữ chế độ thiết bị/EQ/LED/position.
- Integer decoder ARM64: 13 mẫu lossless/seek đối chiếu bytes, MP3 không raw. Normal 10 format + EQ/DSP/ReplayGain/limiter. Production Bit-Perfect worker với simulator qua format/partial writes/EAGAIN/pause/EOF/seek/lỗi/quit.
- Audio policy/core, volume/controller/callback Stock, private Bluetooth route/reopen/unmute và nonzero PCM qua fixtures. Spotify/SoundCloud C/IPC và Drive decoder/FILE transport cũng qua.
- Đối chiếu source v1.8.7: giữ 50 theme/font/icon/emoji/locale/IME/stations/licence, module audio/CPUFreq/writer/startup và backend runtime. Config/UI/lyrics/lifecycle được sửa có chủ đích cho dịch.
- CPUFreq/disk writer, timer/config/session qua regression. Cleanup translator có trong production main; ROM điều khiển ngủ sau khi thoát.
- Installer/OTA ba ROM: tree/metadata/executable bits/CRC, đúng installer/hash. OTA production thử nâng từ fixture v1.8.7, Drive confirmation, size/hash/ROM/HTML sai, stage/install/backup, giữ userdata gồm cache dịch/key FAKE, cancel/reap.

## Giới hạn
Môi trường thử câu mẫu dịch live trả HTTP 429; chưa chứng nhận dịch live thành công hay Google Cloud thật. Protocol/cache/UI fixtures không phải đo chất lượng dịch, nhiệt hoặc pin.

Chưa thử TrimUI/DAC/Bluetooth thật, chưa đo latency/nhiệt/pin hoặc chứng nhận sleep/wakeup. OTA public chưa được đăng. Brick Pro/tg5040 là target; original Brick chưa xác nhận.

Bằng chứng: logs/v188*, BUILD_REPORT_v1.8.8_<ROM>.json và BUILD_INPUTS_v1.8.8.json. Các báo cáo cũ là lịch sử.
