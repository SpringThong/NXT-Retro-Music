# Kiểm chứng NXT Retro Music v1.8.1

Bản native C/SDL ARM64 cho TrimUI Brick Pro, hai biến thể Stock và NextUI. Demo v1.8.1 đã được người dùng duyệt trước khi build. Source nền là gói v1.8 người dùng đã cài. Báo cáo này chỉ mô tả các kiểm tra đã chạy trên source và gói cài hiện tại.

## Kết quả đạt

- Build cả hai ROM bằng compiler ARM64/sysroot ghim SHA256. Source fingerprint trước/sau build trùng nhau; BUILD_REPORT_v1.8.1_Stock.json và BUILD_REPORT_v1.8.1_NextUI.json đối chiếu SHA binary, source và installer thực tế.
- Renderer SDL production với ASan/UBSan, 36 player/home ở 1024×768 và 640×480. Đã xem trực tiếp ảnh chụp cả 36 theme, kiểm tra tương phản chữ/nền ≥4.5 và ảnh album/đĩa than.
- Controller flow production: năm nhóm cài đặt; bốn nhóm giao diện cá nhân; sửa hồ sơ; cập nhật phần mềm ở mục thứ hai; chỉnh font bộ gốc; ảnh album xem trước, Apply/Cancel; danh sách hồ sơ lúc mở và chuỗi tâm trạng/thể loại/quốc gia.
- Lời nguyên câu trong vùng chung của cả 36 theme: đổi câu làm đổi pixel; tăng đồng hồ 15 giây giữ nguyên pixel của cùng câu, không có marquee. Màn lời toàn màn hình dùng mỗi câu một hàng và co vừa bề rộng.
- Chất lượng nguồn và đầu ra/DAC có hai vùng độc lập. Fixture đổi codec/Hz/bitrate/số kênh chỉ làm đổi vùng nguồn; đổi DAC/PCM chỉ làm đổi vùng đầu ra. Dữ liệu đến từ TrackInfo và bộ giám sát phần cứng, thông số không có không được tự gán.
- 300 icon, 180 emoji 3D; 14 ngôn ngữ, bàn phím tương ứng, bộ gõ Telex/Hangul/Romaji/Pinyin, các tổ hợp phím và glyph. IME offline giữ nguyên dữ liệu gốc.
- 105 tổ hợp phổ nhạc, màu bảy màu/màu hỗn hợp được kiểm tra pixel native. Sáu hàm màu LED cùng driver Stock/NextUI: đổi màu/chế độ/effect ngay, không disable/enable LED; phục hồi trạng thái driver cũ khi thoát.
- Wi-Fi worker production với ASan/UBSan: PIN, UTF-8, tên/path không hợp lệ, tách header/body, không ghi đè/đi qua symlink, truyền 16 MiB theo luồng, hủy và đóng máy chủ dọn tệp dở. Tăng RSS worker được đo bằng 0 KiB trong fixture.
- Nhịp vẽ production: spectrum ≤31 lần/s ở vòng input 60 Hz; không FFT/vẽ khi màn hình tắt/ẩn; thức dậy và vòng tràn tick. Giữ các giới hạn redraw 20/10/4 Hz khi phát, 1 Hz khi tạm dừng. Đây là kiểm tra logic, không phải phép đo nhiệt trên máy.
- Byte comparison với source v1.8: decoder/player/DSP, DAC/PCM/LED/platform, cấu hình hồ sơ, session phát nền, Wi-Fi worker, keyboard/IME và Spotify/SoundCloud backend giữ nguyên. Chỉ bỏ phần USB xuất thẻ/chép tệp; code USB host/sạc và DAC được giữ.
- Receiver PCM Spotify và SoundCloud production: socket thật, resampling 44.1→48 kHz, clock theo audio đã tiêu thụ, pause, epoch seek cũ, clamp mẫu không hợp lệ, callback giới hạn và dọn sạch. Test nâng cấp/attach session cũ cũng đạt.
- QEMU ARM64 chạy chính các frontend cuối cùng và backend ready/quit, SDK libc. Stock còn được chạy với rootfs firmware chính thức: help/build-info, tài nguyên, SDL 2.30.8/GLES, backend Spotify/SoundCloud và yt-dlp đóng gói.
- Hai ZIP installer: CRC, executable modes, SHA frontend trùng binary đã build, đúng ROM/OTA source, version 1.8.1 ở config/state/pak, đủ theme/icon/emoji/font/IME. Không có helper USB chuyển file, thông tin đăng nhập hoặc dữ liệu người dùng; không ghi các thư mục ROM/hệ thống.
- OTA production worker/installer dùng đúng ZIP cuối: fixture bản đang cài v1.8 → v1.8.1, SHA/size/ROM, xác nhận tải Drive, stage/install/backup bản cũ, giữ nguyên hồ sơ/credentials/hình nền, hủy và từ chối HTML/ROM sai. URL phát hành trong bộ công cụ vẫn là chỗ điền; chưa đăng OTA công khai.

## Giới hạn

Không có TrimUI Brick Pro thật trong môi trường này. Chưa đo trực tiếp nhiệt, pin, độ mượt, Wi-Fi hoặc DAC vật lý. Kiểm tra renderer/driver dùng đầu vào điều khiển và phần cứng dạng fixture; QEMU chỉ kiểm tra loader/runtime, không mô phỏng GPU hay phần cứng. Không đăng nhập tài khoản Spotify/SoundCloud thật trong lần build này. Phần tài khoản/âm thanh của bản v1.8 được giữ nguyên và kiểm tra IPC/PCM tương ứng.

Các log và JSON hiện tại có tiền tố v181. Các báo cáo v1.8 và cũ hơn chỉ là lịch sử, không được dùng để khẳng định bản mới đã đạt.
