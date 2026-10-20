# NXT Retro Music v1.8.1 · Stock / NextUI

Source native C/SDL ARM64 cho TrimUI Brick Pro, dựa trên source v1.8 đã cài trên máy của người dùng. Demo đã duyệt: https://nxt-music-v181-demo.bold-eft-4749.chatgpt.site

## Build

```sh
python3 tools/sdk-assets.py
NXT_JOBS=3 python3 tools/build-stock.py
NXT_JOBS=3 python3 tools/build-nextui.py
python3 tools/package-stock.py
python3 tools/package-nextui.py
NXT_PACKAGE_ROM=stock python3 tools/package-ota-kit.py
NXT_PACKAGE_ROM=nextui python3 tools/package-ota-kit.py
python3 tools/sync-overlay.py
python3 tools/package-source.py
```

Compiler/sysroot được ghim SHA256 trong SOURCE.json; audio/text static libraries và backend ARM64 từ v1.8 được giữ nguyên. Không cần Docker. Cache SDK, host tools và build được bỏ khỏi Source ZIP.

## Kiểm chứng

RELEASE_QA_v1.8.1.md và BUILD_REPORT_v1.8.1_Stock.json / BUILD_REPORT_v1.8.1_NextUI.json ghi kết quả hiện tại cùng SHA của binary/gói cài. Không dùng báo cáo bản cũ để khẳng định bản mới đã đạt. QA dùng fixture cho bộ điều khiển, cảm biến và tài khoản; không tuyên bố đã đo nhiệt trên máy thật.

Hướng dẫn cài ở docs/HUONG_DAN_NXT_Retro_Music_v1.8.1_Stock.md và docs/HUONG_DAN_NXT_Retro_Music_v1.8.1_NextUI.md.

# NXT Retro Music v1.8.1

Ý tưởng và thực hiện: Nguyễn Xuân Thông · Spring Thông · Vibe Code.

- 36 giao diện được thu gọn theo demo đã duyệt, nền toàn màn hình, ảnh album giữ tỉ lệ; bộ gốc có ảnh bìa hoặc đĩa than xem trước ngay khi chọn.
- Trình phát dùng icon và tên phím, có số bài đang phát/tổng bài. Ngẫu nhiên, lặp lại và hẹn giờ dùng icon có trạng thái. X mở danh sách; Y mở lời; SELECT mở EQ; START mở tùy chọn.
- Lời hiển thị nguyên câu trên một hàng, co vừa chiều rộng và không chạy ngang, cả trên player và màn lời toàn màn hình.
- Thông tin nguồn: định dạng, tần số lấy mẫu, độ sâu bit, bitrate, số kênh khi nguồn cung cấp. Đầu ra hiển thị riêng tên DAC/loa/tai nghe/Bluetooth và thông số PCM mà bộ âm thanh xác nhận. Không dùng thông số nguồn để suy ra thông số DAC.
- START → Thông tin bài hát & đầu ra có thông tin chi tiết.
- Cài đặt gồm: Phát nhạc & âm thanh; Thư viện & tệp; Hiệu năng thiết bị; Cài đặt phím; Ngôn ngữ.
- Cá nhân → Hồ sơ → Giao diện cá nhân gồm: Bộ sưu tập giao diện; Tinh chỉnh NXT; Đèn LED; Phổ nhạc. Tinh chỉnh NXT giữ các bảng màu, RGB, font, chữ, hình nền, lớp phủ, ảnh album, chuyển động, icon menu và độ trễ lời của bộ gốc.
- Thông tin ứng dụng đứng đầu, Cập nhật phần mềm đứng thứ hai và gom kiểm tra OTA với hướng dẫn cập nhật. Thông tin ủng hộ chỉ ở trang thông tin ứng dụng.
- Chỉ chép tệp qua Wi-Fi. Cổng USB vẫn giữ chế độ host/sạc và DAC; không còn chức năng xuất thẻ nhớ qua USB-C.
- Giữ phần phát nhạc, EQ/DSP, Spotify/SoundCloud, hồ sơ, gợi ý theo tâm trạng/thể loại/quốc gia, dữ liệu lịch sử, 14 ngôn ngữ và bàn phím, LED đổi tức thì, chế độ phát nền của v1.8.
- Giữ nhịp vẽ 1 Hz khi dừng/tạm dừng, 10 Hz thường, 20 Hz giao diện cơ khí, 4 Hz E-Ink; cache hình/chữ có giới hạn. Khi tắt màn hình/ẩn app không vẽ giao diện hoặc FFT.
