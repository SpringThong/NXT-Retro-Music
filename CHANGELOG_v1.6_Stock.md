# NXT Retro Music v1.6 — Stock

## Các thay đổi chính

- **Kho hình nền chung**: Khám phá, Thể loại, Yêu thích, Đã tải và Ảnh trên thẻ. Không còn mục nhập ảnh nằm tách riêng.
- **Ảnh online từ Wallhaven**: bộ ảnh chọn lọc và ảnh mới, chỉ nhận SFW, không yêu cầu người dùng nhập API key. Tám nhóm: Retro & Pixel, Anime & Minh họa, Game, Thiên nhiên, Thành phố & Neon, Vũ trụ, Âm nhạc, Tối giản & Trừu tượng.
- **Ảnh từ thẻ JPG/PNG**: duyệt thư mục ngay trong kho, sắp tên tự nhiên 1, 2, 9, 10… rồi A–Z. Mỗi thư mục hiển thị tối đa 512 ảnh/thư mục con; có thông báo khi vượt giới hạn.
- **Xem thử trên menu thật**: chỉnh độ phủ ô và sáng/tối trước khi áp dụng; hủy khôi phục nền cũ. Giữ điều chỉnh lớp phủ riêng để nhìn rõ ảnh và chữ.
- **Riêng từng hồ sơ**: nền đang dùng và tối đa 128 hình nền yêu thích được lưu theo người nghe. Các tệp ảnh tải về dùng chung, dùng được offline.
- **Tải nhẹ hơn**: tải danh sách và ảnh thu nhỏ trước; chỉ tải ảnh đầy đủ khi chọn tải/xem thử. Tải và thu nhỏ ở tác vụ nền; giữ tối đa sáu ảnh thu nhỏ đang xem trong RAM. Cache ảnh thu nhỏ tối đa 96 tệp hoặc 24 MiB; ảnh đã tải/đang dùng được giữ lại.
- **OTA v1.6 riêng cho Stock**, dùng ZIP cài đầy đủ, kiểm tra ROM, dung lượng và SHA-256. Giữ dữ liệu nghe và nguồn OTA đã lưu ngoài thư mục app.
- Bổ sung hướng dẫn và thay đổi v1.6 ngay trong **Thông tin, hướng dẫn & cập nhật**; log ghi các bước lỗi kho hình nền.

## Giữ từ v1.5.2

Các phần nghe nhạc Local, Spotify, SoundCloud, Drive, cá nhân hóa, thống kê, LED, pin/Wi-Fi, DAC/Bluetooth và phím của từng ROM được kế thừa từ bản v1.5.2 đã được chủ dự án xác nhận hoạt động trên cả hai ROM. v1.6 không thay backend Spotify/SoundCloud hoặc driver âm thanh/LED.

## Phạm vi kiểm tra

Build ARM64 cho Stock và NextUI; kiểm tra parser, lưu ảnh, duyệt thẻ, giao diện native ở 640×480 và 1024×768, tác vụ ảnh và gói OTA bằng mã kiểm tra thực tế. Máy tính không thay thế thử nghiệm phần cứng; v1.6 cần chủ dự án kiểm tra lại trên máy trước khi public.

Ảnh Wallhaven do tác giả tương ứng sở hữu, phù hợp dùng làm nền cá nhân. NXT không đóng gói lại kho ảnh của Wallhaven. Dịch vụ có thể tạm giới hạn truy cập; nền có sẵn và ảnh trên thẻ vẫn sử dụng được.
