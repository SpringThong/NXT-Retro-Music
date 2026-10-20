# NXT Retro Music v1.8.3 — Stock / NextUI

Một bộ mã nguồn chung, hai bản native riêng cho TrimUI Brick Pro. Giữ các tính
năng của v1.8.2, thêm hẹn giờ lưu tiến trình và thoát hẳn NXT cả khi phát nền,
tạm dừng hoặc tắt màn hình. Firmware tiếp tục điều khiển tắt màn hình/ngủ theo
thiết lập hệ thống; không có lệnh ép suspend hay tiến trình canh hẹn giờ mới.

## Build và OTA

Trên Ubuntu/WSL, lần đầu cần mạng để tải compiler/SDK đã khóa SHA-256. Không cần Docker.

```bash
NXT_JOBS=3 make stock
NXT_JOBS=3 make nextui
```

Mỗi lệnh build frontend từ source, cập nhật overlay và tạo bộ cài cùng bộ đăng
OTA tương ứng trong `dist/stock-release/` hoặc `dist/nextui-release/`.
Không dùng bộ cài hoặc nguồn OTA của ROM này cho ROM còn lại.

Hai bản dùng cùng thư viện nhạc, backend online, theme, font và dữ liệu bàn phím
của v1.8.2. Chỉ bốn file C/header của frontend được sửa cho hẹn giờ và thông tin
v1.8.3; kiểm tra byte có trong `logs/v183-baseline-preserved.json`.

## Cài đặt và hẹn giờ

- Stock: chép thư mục **Apps** của bộ cài vào gốc thẻ nhớ.
- NextUI: chép thư mục **Tools** của bộ cài vào gốc thẻ nhớ.
- Khi nâng từ bản đang chạy nền, khởi động lại máy một lần rồi mở bản mới.
- Đặt hẹn giờ bằng biểu tượng đồng hồ/Tùy chọn nhanh. 0 phút hủy hẹn giờ.
- Hết giờ, app lưu tiến trình/cài đặt, dừng nhạc và đóng cả phiên chạy nền.
- NextUI: bật **Screen timeout** và **Suspend timeout** trong cài đặt hệ thống.
- Stock: bật tính năng tắt màn hình/ngủ tự động tương ứng của firmware. App gỡ
  cờ giữ máy thức do mình sở hữu và trả keymon về trạng thái trước khi mở NXT.

Giữ thư mục dữ liệu cá nhân ngoài app để giữ hồ sơ, playlist, ảnh nền, EQ và đăng
nhập. Đóng app giúp giảm xử lý nền; deep sleep và lượng pin tiết kiệm phụ thuộc
firmware, cài đặt, sạc USB và các ứng dụng khác còn chạy.

## Kiểm tra

`RELEASE_QA_v1.8.3.md` và hai `BUILD_REPORT_v1.8.3_*.json` ghi kết quả kiểm tra thực tế.
Đã kiểm tra hẹn giờ/controller, phục hồi cài đặt, cleanup cờ Stock, Bluetooth/ALSA,
loader ARM64 trên rootfs Stock chính thức, OTA và bộ tạo nguồn cập nhật.
Chưa thử Bluetooth/DAC/deep sleep trên Brick Pro vật lý hoặc đo pin/nhiệt.

Hướng dẫn theo ROM trong `docs/HUONG_DAN_NXT_Retro_Music_v1.8.3_Stock.md` và
`docs/HUONG_DAN_NXT_Retro_Music_v1.8.3_NextUI.md`.

Gói vẫn giữ source, binary và báo cáo NX Redux v1.8.2 làm dữ liệu lịch sử. Chúng
chưa được build lại cho v1.8.3; target NX Redux sẽ báo phiên bản chưa chuẩn bị.
Gói NX Redux v1.8.2 đã phát hành trước đó vẫn dùng được như cũ.

Giữ nguyên giấy phép đi kèm của mã nền, thư viện, font và các thành phần NXT.
