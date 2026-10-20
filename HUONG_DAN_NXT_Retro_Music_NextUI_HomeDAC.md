# NXT Retro Music v1.0 — NextUI BottomDAC

Dành cho TrimUI Brick Pro, hệ điều hành NextUI. Giữ giao diện FooterFix, font, ảnh album, chức năng nghe nhạc và dữ liệu của bản NextUI đã chốt.

## Cài đặt

1. Tắt hẳn máy qua menu NextUI để kết thúc phiên app cũ; tắt màn hình không đủ.
2. Giải nén gói cài, chép đè `NXT Retro Music.pak` vào đúng thư mục Tools đang chứa app trên thẻ nhớ (thường là `Tools/tg5040/`).
3. Giữ nguyên thư mục nhạc và `.userdata`. Khởi động máy, mở app như trước.

## Nút logo TrimUI

Nhấn một lần nút có logo TrimUI khi đang mở app: dừng nhạc, lưu vị trí nghe và thoát hoàn toàn về NextUI. Không tiếp tục nghe nền dù tùy chọn giữ phiên đang bật. Áp dụng cho màn Đang phát, Thư viện, Cài đặt và màn khác; Nút logo (MENU trong NextUI) được đọc trước khi lọc khóa phím/tắt màn hình.

Nút HOME riêng bên hông không được gán thoát.

App thoát qua quy trình dọn dẹp: đóng player, trả đầu ra/LED và xóa trạng thái phiên. Không dùng SIGKILL. Nút B giữ chức năng quay lại và nút nguồn nhấn ngắn vẫn tắt/bật màn hình.

## Thông số DAC thực tế

Màn Đang phát hiển thị hai phần riêng:

- Thông tin file: codec, bit-depth nếu có, sample rate và bitrate.
- Dưới tên DAC: ví dụ **DAC thực tế: 48 kHz · 32-bit PCM**, lấy từ `hw_params` của card USB đang được app dùng, không lấy giới hạn tối đa trong quảng cáo DAC hoặc thông tin file.

32-bit ở đây là độ rộng mẫu PCM của luồng gửi tới DAC. Nó không biến nhạc 16/24-bit thành bản ghi có thêm chi tiết. Không đo được tín hiệu analog sau DAC hoặc chất lượng tai nghe. Nếu file 192 kHz qua mixer 48 kHz, dòng DAC sẽ hiện 48 kHz. Bản này không đổi đường âm thanh hoặc bật chế độ bit-perfect.

Nếu thiết bị/driver chưa cung cấp thông số PCM đang mở, hiện **DAC thực tế: chưa xác nhận**; không suy đoán từ chất lượng file. Dòng tự cập nhật theo lần dò phần cứng khoảng 0,5 giây. Không hiện dòng này cho loa/3,5 mm/Bluetooth.

## Kiểm tra trên máy thật

- Phát nhạc qua Black Pearl + Delci: kiểm tra tên DAC và dòng PCM thực tế.
- Thử file 44,1/48/96 kHz; thông số DAC có thể giữ 48 kHz nếu mixer thực sự đang chạy 48 kHz.
- Rút/cắm DAC, phát/tạm dừng, chuyển đầu ra để kiểm tra dòng thông tin.
- Nhấn nút logo TrimUI ở Đang phát, Thư viện, Cài đặt; thử khi khóa phím hoặc màn hình tắt. Nhạc phải dừng và app đóng hoàn toàn.
- Mở lại để kiểm tra vị trí nghe, ảnh album và LED.

Đã build release ELF ARM64 và kiểm tra mã sản xuất với đầu vào giả lập: nút logo, lưu vị trí, khóa phím/tắt màn hình, PCM/định dạng/card USB và trường hợp thiếu thông số. Không có Brick Pro hoặc Black Pearl thật để xác nhận thao tác vật lý. Các ảnh/kiểm tra GUI trong thư mục QA cũ là từ bản FooterFix trước cập nhật này.

Nếu nút logo vẫn không phản hồi: thoát qua menu app rồi gửi file `NXT_Input_Log.txt` ngay trong `NXT Retro Music.pak`. Log ghi mã joystick đã nhận để đối chiếu firmware.

## Cổng USB dưới tích hợp

Vào **Cài đặt → Âm thanh & DAC → Cổng USB dưới**, nhấn A và chọn:

- **Nghe DAC**: rút cáp sạc/PC khỏi cổng dưới trước khi áp dụng. App bật USB host; sau đó cắm DAC, chọn đầu ra DAC nếu app chưa tự chuyển.
- **Chế độ sạc**: rút DAC, chọn chế độ này rồi cắm sạc. App chuyển cổng về USB device như trạng thái sạc ban đầu; không tự bật chia sẻ thẻ nhớ.

Chỉ đổi chế độ khi nhấn A. Nếu chế độ đã được chọn thì không reset cổng. Không tự bật host khi mở app, không flash firmware hay đổi cấu hình khởi động. Nếu host mất sau khi khởi động lại, chọn Nghe DAC một lần nữa. Khi đổi từ host về device, app tạm dừng nếu đang dùng đầu ra USB.

Công cụ sử dụng đúng cơ chế Tina đã hoạt động với Black Pearl của bạn: đọc action node usb_host/usb_device để chuyển chế độ. Việc hiển thị trạng thái chỉ đọc otg_role, không đọc action node. Bản tích hợp đã build ARM64 và kiểm tra bằng sysfs giả lập; cần thử trên Brick Pro thật. Không khẳng định mọi DAC hoặc vừa sạc vừa nghe qua hub đều hoạt động; phụ thuộc thiết bị, driver USB Audio và nguồn cấp.

## BottomDAC Fix1
Đọc mỗi nút USB bằng một lần read, không dùng fread lặp lại. Cho phép yêu cầu chuyển cổng khi trạng thái ban đầu chưa xác định. Khi kernel chưa xác nhận chế độ mới, thông báo “Đã gửi yêu cầu”, không báo thành công. Xem trạng thái ở Cổng USB dưới; nếu chưa chuyển, thoát bằng nút logo và gửi NXT_Input_Log.txt. Không tự chuyển cổng khi khởi động hoặc trong vòng cập nhật. Bản tích hợp này cần test thực tế trên máy.
