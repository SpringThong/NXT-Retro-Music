# NXT Retro Music v1.5 — Stock

Chuyển thẳng v1.3 → v1.5, bỏ qua v1.4.

- Tối đa 16 hồ sơ người nghe: biệt danh, biểu tượng, yêu thích trong thẻ, playlist, bookmark SoundCloud, resume, EQ/giao diện/nút riêng. Spotify dùng chung tài khoản trên máy.
- Dành cho bạn: nghe tiếp, bài quen, bài trong thẻ chưa nghe, lâu chưa nghe; lịch sử có phân trang.
- Thống kê hôm nay / 7 ngày / tháng, nguồn nhạc, lượt nghe, top bài/nghệ sĩ, chi tiết theo ngày; tính từ PCM tạo ra, không cộng tua, tạm dừng, chờ dữ liệu. Có tắt ghi theo hồ sơ.
- 16 màu và RGB/HEX tự chọn; nền mẫu, nhập JPG/JPEG/PNG từ thẻ. Chữ, nền, vùng chọn, hộp thoại và bàn phím đổi đồng bộ tối/sáng; kiểm tra tương phản và nền đọc chữ riêng.
- Google Drive OTA công khai: nguồn update.json nhập/dùng lại, tải nền, kiểm tra phiên bản/ROM/máy, dung lượng/SHA-256, ZIP và ARM64, giữ app cũ, dữ liệu cá nhân ngoài app. Có hủy tải và log theo bước.
- Log chung xoay vòng 1 MiB; START trong Thông tin xuất NXT_App_Log.txt. Hướng dẫn/changelog v1.5 có ngay trong app.
- Stock Bluetooth tự chuyển khi A2DP vừa sẵn sàng; đổi thiết bị nạp lại đầu ra; ngắt Bluetooth/DAC quay về đầu ra còn có. Tùy chọn tự Bluetooth và tự DAC độc lập, giữ lựa chọn tay khi kết nối không đổi.
- Giữ Spotify/SoundCloud đã hoạt động, DAC Volume Fix1, Home Stock, Wi-Fi/pin, 16 nút, Telex, thứ tự tự nhiên, đĩa than/kim, Drive progressive cache/tải trước bài sau, hàng đợi online nhiều trang.

## Phạm vi kiểm tra

Frontend ARM64 được build từ source, relink đối chiếu binary, payload kiểm tra ZIP/ELF/mode/resource. Kiểm tra native SQLite, routing, stats, màu/Telex, OTA/worker bằng dữ liệu và I/O fixture; chạy payload trên rootfs Stock chính thức qua QEMU. Không có TrimUI/Bluetooth/DAC/tài khoản nhà phát hành kết nối trực tiếp trong môi trường build.

Không sửa thêm LED theo yêu cầu bỏ qua. OTA chưa có link phát hành thật; người phát hành cần cấu hình update.json trên Drive. Hình nền nhập từ thẻ; chưa có trình duyệt tải ảnh. Hồ sơ cục bộ chưa tách tài khoản Spotify. Radio/podcast chưa vào thống kê.
