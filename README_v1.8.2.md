# NXT Retro Music v1.8.2 · Stock / NextUI

v1.8.2 có bản sửa Bluetooth NextUI được đối chiếu bằng log thật và kiểm thử hồi quy với mã v1.8.1. Xem BLUETOOTH_CHECK_v1.8.2.md. Dùng gói cài và OTA có hash trong BUILD_REPORT_v1.8.2_Stock.json / BUILD_REPORT_v1.8.2_NextUI.json.

Native C/SDL ARM64 cho TrimUI Brick Pro, dựa trên source v1.8.1 đã cài trên máy của người dùng. Khôi phục trình phát NXT gốc theo v1.8/ảnh tham chiếu; thêm ba chế độ hiệu năng cho phần hiển thị; đổi tên Tùy chọn giao diện gốc; giữ các phần phát nhạc đang hoạt động tốt.

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
```

Compiler/sysroot ghim SHA-256 trong SOURCE.json; libraries và backend ARM64 giữ nguyên. Source ZIP bỏ SDK/cache/host tools. Bộ OTA chứa ZIP cài nguyên bản và trang offline tạo update.json từ link công khai; nhà phát hành cần đăng ZIP/JSON đúng ROM.

## Kiểm chứng

RELEASE_QA_v1.8.2.md và BUILD_REPORT_v1.8.2_Stock.json / BUILD_REPORT_v1.8.2_NextUI.json ghi kết quả kiểm tra hiện tại cùng SHA binary/gói cài. Chạy QA rồi tools/finalize-v182-reports.py, tools/package-source.py để freeze source. Không dùng báo cáo phiên bản cũ để khẳng định bản mới đã đạt.

QA dùng dữ liệu kiểm thử cho bộ điều khiển, cảm biến, tài khoản và tải Drive; không đo nhiệt, pin, DAC/Bluetooth vật lý hoặc tải nguồn công khai của bạn. Decoder/DSP/PCM/LED, online workers, Wi-Fi, IME và input loop được đối chiếu byte với source v1.8.1. Điều khiển đầu ra và âm lượng có sửa cho Bluetooth, được kiểm thử riêng với parser ALSA và selector thật.

Hướng dẫn cài nằm trong docs/HUONG_DAN_NXT_Retro_Music_v1.8.2_Stock.md và docs/HUONG_DAN_NXT_Retro_Music_v1.8.2_NextUI.md.

Ý tưởng và thực hiện: Nguyễn Xuân Thông · Spring Thông · Vibe Code.
