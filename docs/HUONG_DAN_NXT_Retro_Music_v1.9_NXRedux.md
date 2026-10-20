# NXT Retro Music v1.9 — NX Redux

## Cài đặt

Thoát hẳn app trước khi chép. Giải nén ZIP vào gốc thẻ để có Tools/NXT Retro Music.pak, hoặc dùng OTA. Giữ nguyên .userdata, hồ sơ, playlist và cache bản dịch.

Mở app một lần để cài phần icon Tools. Về Tools hoặc khởi động lại máy nếu icon chưa đổi. Phần icon chỉ thay launcher đúng mã SHA của NX Redux v2.1.2 trên Brick Pro; launcher khác được bỏ qua. Bản gốc nằm trong integration/backup. Script integration/install-tools-icon.sh --restore phục hồi bản gốc đã lưu nếu launcher hiện tại vẫn đúng bản NXT.

## Hiệu năng và LED

Cài đặt → Hiệu năng thiết bị → Chế độ hiệu năng. Tiết kiệm pin / Cân bằng / Hiệu năng giới hạn tối đa lần lượt 1008 / 1608 / 2000 MHz theo driver. Đây là giới hạn tối đa, không ép CPU luôn chạy ở mức đó. Cân bằng là mặc định. Khi nghe nền, ROM tự quản lý CPU để không can thiệp game.

Cá nhân → Hồ sơ → Tùy chỉnh giao diện cá nhân → LED. Bật LED rồi chọn màu/RGB, độ sáng hoặc hiệu ứng. Màu áp dụng ngay, không cần tắt/bật lại.

## Bảng Home

Khi NXT đang phát, bấm nút Home bên hông để mở bảng của ROM. Ô nhạc nhận thông tin của NXT; phát/tạm dừng và trước/tiếp dùng hàng đợi hiện tại. Mở lại NXT từ Tools để chọn nhạc hoặc sửa hàng đợi. Khi NXT thoát, dịch vụ nhạc mặc định có thể khởi động lại bình thường.

## Nhập khóa Groq bằng TXT

1. Tạo groq-api-key.txt trên máy tính, ghi duy nhất khóa Groq đầy đủ vào file.
2. Chép vào Music ở gốc thẻ, hoặc Tools/NXT Retro Music.pak cạnh launch.sh. Có thể chép vào Music qua tính năng Wi-Fi hiện có.
3. Trong Dịch lời bài hát, chọn Dịch vụ dịch → Groq; chọn Khóa API Groq → Nhập từ TXT.
4. Chọn model rồi bật Song ngữ hoặc Bản dịch.

App giữ TXT gốc; bạn có thể xóa file khỏi thẻ sau khi nhập. Khóa đã lưu nằm trong dữ liệu riêng của app, không hiện đầy đủ và không ghi vào log. Google Cloud giữ khóa riêng; MyMemory vẫn dùng được.

## OTA

Thông tin & cập nhật → Cập nhật phần mềm. Nguồn NX Redux chính thức giữ nguyên: https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing

Người phát hành mở CHUAN_BI_OTA.html trong bộ OTA trên máy tính, điền link ZIP công khai mới để tạo update.json đúng phiên bản, kích thước và SHA. Trên chính file JSON cũ của Drive, dùng Quản lý phiên bản → Tải phiên bản mới để giữ nguyên ID/link. Không đăng JSON mẫu còn chỗ điền link. Hồ sơ, vị trí nghe và cache bản dịch giữ nguyên khi nâng cấp.

Đã kiểm tra build và fixture; Home/CPU/LED cùng mức pin, nhiệt độ, âm thanh và deep sleep cần thử trên Brick Pro thật.


DSD v1.9: đọc docs/DSD_v1.9_NXRedux.md kèm gói cài. Cần DAC USB và driver Native DSD hoặc DAC hỗ trợ DoP. Giữ bit gốc, không chuyển PCM, không phát qua loa/Bluetooth.
