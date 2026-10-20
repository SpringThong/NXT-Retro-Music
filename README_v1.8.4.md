# NXT Retro Music v1.8.4 — Stock / NextUI / NX Redux

Một bộ source chung, ba frontend ARM64 và ba bộ OTA riêng cho TrimUI Brick Pro. Giao diện đã được duyệt ở demo; bản này triển khai bằng C native, không kèm trình duyệt/webview.

Mới: SELECT mở/đóng EQ; USB DAC Bit-Perfect dùng lossless integer PCM/exclusive hw, không DSP/resample/software gain; vào thẳng phiên Now Playing gần nhất và tạm dừng mặc định; không có câu hỏi tâm trạng khi khởi động. Giữ các tính năng đang hoạt động từ v1.8.3 và adapter NX Redux.

## Build

Ubuntu/WSL cần gcc, Python và mạng ở lần đầu tải toolchain được khóa SHA-256. Từ thư mục gốc:

```bash
NXT_JOBS=3 make stock
NXT_JOBS=3 make nextui
NXT_JOBS=3 make nxredux
python3 tools/package-source.py
```

Builder compile toàn bộ frontend source cho từng ROM; Stock/NextUI dùng settings library tương ứng, NX Redux dùng libnxt-nxredux-settings.so với adapter riêng. Giữ nguồn runtime/backends/fonts đã khóa trong SOURCE.json và nguồn kèm theo. Output ở dist/stock-release, dist/nextui-release, dist/nxredux-release và dist/release. Dùng đúng bản ROM; NX Redux có Tools phẳng, NextUI có Tools/tg5040.

## Kiểm tra và tài liệu

Xem RELEASE_QA_v1.8.4.md, BUILD_REPORT_v1.8.4_<ROM>.json và logs/v184-* cho bằng chứng. Các script QA dùng mô phỏng rõ ràng cho phần cứng chưa có ở host; không coi mock/ARM64 loader là chứng nhận DAC/tai nghe thật. Chưa đo pin/nhiệt hoặc ngủ/thức trên máy cho bản này.

Hướng dẫn cài/sử dụng: docs/HUONG_DAN_NXT_Retro_Music_v1.8.4.md và các bản riêng ROM. Mỗi bộ OTA có trang tạo JSON offline và hướng dẫn đăng nguồn công khai. Không tự đăng/publish nguồn OTA trong bước build.

Ý tưởng và thực hiện: Nguyễn Xuân Thông — Spring Thông. Vibe code. PayPal tùy chọn: thongkhung2@gmail.com. Giấy phép và nguồn của các thành phần được giữ trong thư mục licenses và source.
