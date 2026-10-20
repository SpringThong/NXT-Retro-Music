# Kiểm tra Bluetooth — NXT Retro Music v1.8.2

**Trạng thái: đã build lại Stock và NextUI v1.8.2 với bản sửa Bluetooth. Gói cài và bộ đăng OTA dùng ELF mới, đã qua kiểm tra hồi quy và đối chiếu hash. Chưa thử tai nghe Bluetooth trên máy thật.**

Máy được người dùng xác nhận chạy **NextUI**. Bluetooth kết nối và đã chọn làm đầu ra nhưng cả nhạc trên thẻ và nhạc online đều không có tiếng.

## Đối chiếu log máy ngày 11/10/2026

Đã đọc cả `NXT_Device_Log.txt` và `NXT_Input_Log.txt` do người dùng gửi, từ bản **v1.8.1 NextUI**:

| Trường trong log | Giá trị | Ý nghĩa |
|---|---|---|
| Audio requested | 1 | App muốn dùng Bluetooth |
| Audio selected | 0 | Đầu ra logic thực tế vẫn là mặc định/loa hoặc tai nghe dây |
| Route write OK / pending | 0 / 1 | Áp dụng cấu hình Bluetooth thất bại, còn chờ xử lý |
| PCM ready / retry pending | 1 / 0 | PCM cũ vẫn mở; chưa có lỗi mở PCM Bluetooth vì route chưa chuyển thành công |
| USB card | -1 | Không có DAC USB trong lần ghi log này |
| ALSA config / NXT shared PCM | 1 / 1 | File cấu hình tồn tại và đang mang cấu hình do NXT tạo |
| EDIFIER W820NB Plus A2DP | 74%, [on] ở cả hai kênh | Mixer A2DP tồn tại; log không cho thấy thiết bị đang mute tại thời điểm ghi |

Input log ghi `SetAudioSink(0)` rồi `Audio: output configuration pending, sink=1`. Điều này khớp với trạng thái trong device log: app yêu cầu Bluetooth nhưng đường xuất chưa được áp dụng, PCM cũ vẫn sẵn sàng.

Đối chiếu mã gốc v1.8.1: app viết cấu hình loa/DAC vào cùng file `.asoundrc` do NextUI theo dõi; nhánh Bluetooth chỉ đọc file đó và chấp nhận nếu thấy `bluealsa`. Sau khi cấu hình bị thay bằng đường loa, chọn Bluetooth không tự tạo lại route. Đã tái hiện thất bại này bằng selector v1.8.1 nguyên bản với file loa cũ; bản sửa dùng cấu hình riêng và chuyển thành công sang PCM A2DP trong kiểm thử.

Đây là bằng chứng lỗi chuyển đầu ra trong app. Mixer A2DP có mặt chưa chứng minh âm thanh Bluetooth thật sau sửa đã phát được; việc đó vẫn cần cài bản sửa và thử trên thiết bị.

Các dòng `[CFG] Unable to open settings file, cant write` xuất phát từ shim có sẵn chặn app ghi đè cấu hình hệ thống NextUI. Không đổi shim này trong bản sửa Bluetooth.

## Lỗi tìm thấy trong mã

- Trước khi sửa, đường NextUI không chọn PCM riêng và không xóa cache cấu hình ALSA khi mở lại âm thanh như đường Stock. Vì vậy lựa chọn Bluetooth có thể vẫn dùng PCM loa được nạp trước đó.
- Bộ chọn đầu ra đọc file cấu hình âm thanh chung do firmware quản lý, trong khi helper NextUI có sẵn đường cấu hình riêng và thông tin thiết bị A2DP nhưng chưa được dùng ở đây.
- Lệnh đặt âm lượng Bluetooth chưa bỏ trạng thái mute. Bước giảm âm lượng lúc khởi động để tránh tiếng bụp cũng tác động lên thiết bị Bluetooth.

Log thực tế đã xác nhận trạng thái yêu cầu Bluetooth nhưng route thất bại. Phần unmute và giảm âm lượng khởi động là xử lý bổ sung; log mixer 74%, [on] cho thấy mute không phải bằng chứng nguyên nhân của lần lỗi này.

## Bản sửa

1. Dùng thông tin thiết bị A2DP của monitor NextUI có sẵn, chọn PCM và mixer theo địa chỉ tai nghe. Thiết bị Bluetooth chỉ có chức năng tay cầm không được coi là đầu ra âm thanh.
2. Ghi cấu hình ALSA riêng của app bằng file tạm rồi đổi tên. Không ghi đè file do audiomon của NextUI quản lý. Xóa định nghĩa PCM cũ trước khi thêm cấu hình mới để tránh lỗi gộp kiểu dữ liệu.
3. Khi đổi đầu ra, đóng thiết bị SDL cũ, xóa cache ALSA, chọn `nxt_music_output`, rồi mở lại. Giữ cơ chế thử lại hiện có khi transport chưa sẵn sàng.
4. Đặt âm lượng A2DP kèm `unmute`; trạng thái mute của hệ thống vẫn cho mức âm lượng 0. Chỉ thực hiện giảm âm lượng chống bụp trên đường loa/tai nghe dây lúc khởi động.
5. Thêm trạng thái PCM, lỗi mở PCM cuối cùng và trạng thái chọn cấu hình riêng vào log thiết bị của bản sửa.

Không thêm thread hoặc vòng quét Bluetooth mới. Monitor và cơ chế thử lại đã có được dùng lại. Decoder, callback trộn mẫu âm thanh, DSP, LED, worker nhạc online và nhịp đọc phím giữ nguyên.

## Kết quả kiểm thử

| Kiểm tra | Kết quả | Phạm vi |
|---|---|---|
| Mã v1.8.1 tái hiện requested=1 / selected=0 / route pending | Đạt | Selector v1.8.1 nguyên bản lấy từ source, cấu hình loa cũ mô phỏng; khớp trường trong log máy |
| Bản sửa thoát trạng thái cũ, giữ `.asoundrc` firmware | Đạt | Cấu hình riêng, parser ALSA thật, PCM SDL mô phỏng |
| Phân tích thiết bị A2DP, bỏ qua tay cầm | Đạt | Hàm C thật với thông điệp D-Bus mô phỏng |
| Cấu hình ALSA riêng và đổi Loa ↔ Bluetooth ↔ DAC | Đạt | Hàm ghi route thật, parser ALSA thật, thiết bị SDL mô phỏng |
| Đổi tai nghe, sửa sink firmware cũ, thử lại khi transport chậm | Đạt | Bộ điều khiển đầu ra thật |
| Tín hiệu nhạc thẻ, Spotify, SoundCloud qua callback chung | Đạt | Mẫu PCM đầu vào mô phỏng; không phải phiên đăng nhập/phát online thật |
| Âm lượng Bluetooth, unmute, mute hệ thống, mixer chưa sẵn sàng | Đạt | Helper msettings thật của cả Stock và NextUI; lệnh amixer được ghi nhận trong test |
| DAC, HID âm lượng và LED | Đạt | Fixture phần cứng; không thử DAC vật lý |
| Stock runtime và xác thực địa chỉ Bluetooth | Đạt | Hàm runtime thật, dữ liệu firmware mô phỏng |
| So sánh decoder/DSP/LED/account/session/IME/input với v1.8.1 | Đạt | So sánh byte; các file routing đã sửa được kiểm tra riêng |

Kiểm thử Bluetooth/âm lượng có ASan và UBSan. Lúc không đổi thiết bị, không mở lại PCM liên tục. Không đo nhiệt, pin hay độ trễ trên thiết bị vật lý.

Lệnh chạy lại:

```sh
python3 qa/test_v182_nextui_bluetooth.py
python3 qa/test_v182_led_driver.py
python3 qa/test_stock_runtime.py
python3 qa/test_v182_baseline.py
```

## Nếu cần lấy log lại khi thử bản sửa

Hai log ban đầu đã nhận và đối chiếu; hiện tại không cần gửi lại. Các bước dưới đây dành cho lần kiểm tra bản sửa trên thiết bị:

1. Kết nối tai nghe, chọn đầu ra Bluetooth, mở phát một bài khi lỗi đang xảy ra.
2. Vào **Thông tin, hướng dẫn & cập nhật**, bấm **Y** để lưu `NXT_Device_Log.txt`; bấm **START** để lưu `NXT_App_Log.txt`.
3. Thoát app để launcher ghi `NXT_Input_Log.txt`. Gửi các file log này; có thể lấy file qua chức năng chép file Wi-Fi hiện có.

Các log nằm trong thư mục app. Log bản v1.8.1 chưa có những trường chẩn đoán mới nhưng vẫn hữu ích để đối chiếu đầu ra, trạng thái PCM và lỗi SDL.

## Trạng thái gói phát hành

Theo yêu cầu tiếp tục build, đã tạo lại ELF, ZIP cài và bộ OTA cho cả Stock lẫn NextUI từ mã nguồn chứa bản sửa. Gói cài được đối chiếu với ELF mới; bộ OTA chứa đúng ZIP cài này. Hash, input build và kết quả kiểm tra tương ứng nằm trong `BUILD_REPORT_v1.8.2_Stock.json`, `BUILD_REPORT_v1.8.2_NextUI.json` và `RELEASE_QA_v1.8.2.md`. Những gói tạo trước bản sửa đã được thay thế trong bộ phát hành hiện tại.

Bộ OTA dành cho nhà phát hành tự đăng lên nguồn công khai. Chưa đăng thay người dùng và chưa xác nhận phát Bluetooth trên thiết bị vật lý với ELF v1.8.2 mới.

Tham khảo kỹ thuật: [tài liệu plugin PCM/mixer BlueALSA chính thức](https://github.com/arkq/bluez-alsa/blob/master/doc/bluealsa-plugins.7.rst); đối chiếu thêm cấu hình và plugin của rootfs Stock chính thức đang dùng trong test.
