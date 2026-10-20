# NXT Retro Music v1.8.4 — Hướng dẫn chung

Dành cho TrimUI Brick Pro / tg5040. Ba gói khác nhau:

| ROM | Gói cài | Thư mục |
|---|---|---|
| Stock | NXT_Retro_Music_v1.8.4_Stock.zip | Apps/NXT_Retro_Music |
| NextUI | NXT_Retro_Music_v1.8.4_NextUI.zip | Tools/tg5040/NXT Retro Music.pak |
| NX Redux | NXT_Retro_Music_v1.8.4_NXRedux.zip | Tools/NXT Retro Music.pak |

Thoát hoàn toàn bản cũ, giải nén vào gốc thẻ và chép đè đúng thư mục app. Giữ dữ liệu người dùng và thư mục nhạc. Nếu bản cũ còn chạy nền, khởi động lại một lần trước khi mở bản mới. Đọc hướng dẫn ROM nằm trong từng bộ cài để xem đường dẫn dữ liệu cụ thể.


## Mới trong v1.8.4

App tự dùng hồ sơ gần nhất và vào thẳng **Now Playing**. Mặc định không tự phát; bấm **A** để nghe tiếp, **B** về menu. Đổi hồ sơ, tâm trạng, thể loại và quốc gia trong **Cá nhân**. Bật tự phát ở **Cài đặt → Phát nhạc & âm thanh → Cách phát → Tự phát khi mở app** nếu muốn. Bài, vị trí và ảnh album gần nhất được lưu riêng theo hồ sơ; thiếu file sẽ giữ thông tin và cho chọn bài khác.

**SELECT** mở EQ; nhấn **SELECT lần nữa** đóng EQ mà không chọn preset. **B** vẫn quay lại. Trong Bit-Perfect, EQ chỉ hiện thông báo bypass và giữ nguyên preset cũ.

### Bit-Perfect

1. Cắm DAC USB và chọn **Đầu ra âm thanh → USB DAC**. Giữ chức năng USB host/DAC của ROM như bản trước.
2. Mở **Cài đặt → Phát nhạc & âm thanh → Chế độ âm thanh → Bit-Perfect**, hoặc **START → Chế độ âm thanh**.
3. Chọn nhạc lossless số nguyên tương thích. WAV PCM, FLAC và ALAC 16/24-bit đã được đối chiếu PCM; WAV PCM 32-bit cũng đã được kiểm tra. Mono/stereo giữ nguyên số kênh. Nguồn Drive phải thực sự cung cấp tệp lossless.
4. Xem trạng thái và thông số nguồn/đầu ra tại **START → Thông tin bài hát & đầu ra**. “Sẵn sàng” là đã mở đúng PCM; “PCM USB nguyên bản” là đang phát qua đường này. Nếu không mở được DAC, app tạm dừng và báo lý do.
5. Chỉnh âm lượng trên DAC/tai nghe có điều khiển phần cứng. Âm lượng phần mềm và EQ/DSP, limiter, ReplayGain, tốc độ, crossfade đều được bỏ qua. App giữ các giá trị chế độ Thường để dùng lại sau.
6. Chọn **Thường** khi dùng loa máy, Bluetooth, nguồn lossy, Spotify, SoundCloud, radio hoặc podcast. Thoát ứng dụng/game khác nếu DAC báo bận. App không tự chuyển sang loa hoặc đổi Hz trong Bit-Perfect.

Độ sâu bit của **container PCM** là cách truyền dữ liệu số, không phải chứng nhận độ phân giải analog của DAC. Khả năng mở Hz/format còn phụ thuộc DAC, driver và ROM. Màn hình báo “yêu cầu/tạm dừng/chưa sẵn sàng” không có nghĩa đã phát Bit-Perfect thành công.

### Bố cục và thao tác

Năm mục Cài đặt vẫn là **Phát nhạc & âm thanh; Thư viện & tệp; Hiệu năng thiết bị; Cài đặt phím; Ngôn ngữ**. Âm thanh gồm: Đầu ra, Chế độ âm thanh, EQ/DSP, Cách phát, Kết nối DAC & âm lượng, Thông tin bài hát & đầu ra.

START mở lần lượt: Đầu ra; Chế độ âm thanh; Hẹn giờ; Ngẫu nhiên; Lặp lại; Thông tin bài/đầu ra; Yêu thích; Lời; Tắt màn hình; Dừng và lưu; Phát nền; Dừng và đóng app. Phím tùy chỉnh vẫn áp dụng trong trình phát.

Hẹn giờ hết sẽ lưu rồi đóng toàn bộ NXT; deep sleep phụ thuộc cài đặt ROM. Không ép suspend và không dùng SIGKILL để thoát app bình thường. Giữ chế độ Cân bằng hoặc Tiết kiệm pin, tắt màn hình khi chỉ nghe để giảm xử lý giao diện; chưa có số đo pin/nhiệt trên thiết bị cho v1.8.4.

## OTA

Mỗi bộ NXT_OTA_Google_Drive_v1.8.4_<ROM>.zip chứa đúng ZIP cài, CHUAN_BI_OTA.html, công cụ Python, SHA-256 và hướng dẫn đăng. Mở trang HTML offline trên máy tính, nhập link ZIP HTTPS công khai, xuất update.json. Đăng JSON lên nguồn ổn định và nhập nguồn đúng ROM trong app. Không đăng JSON mẫu chưa thay link; không sửa giả phiên bản để ép cập nhật.
