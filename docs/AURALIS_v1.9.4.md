# NXT Retro Music v1.9.4 — Auralis & fine volume

- Thêm **Auralis Themes** (theme số 51), giữ nguyên 50 theme trước. Bìa album phủ đầy màn hình theo đúng tỉ lệ, có lớp tối giúp lời dễ đọc. Chọn trong Cá nhân → Chỉnh sửa giao diện cá nhân → Bộ giao diện.
- Thêm **MiSans Semibold · Xiaomi / NX Redux** vào bộ font, giữ nguyên 16 font cũ. Auralis sử dụng MiSans. Giấy phép và ghi công Xiaomi tại licenses/MiSans.txt.
- Đang phát và Y xem lời đầy đủ hiển thị cặp lời gốc / bản dịch **in nghiêng**, phân biệt Đang hát và Tiếp theo. Mỗi câu nằm trong một hàng, co vừa chiều rộng; không chạy ngang. Chuyển dòng dọc có thể tắt bằng tùy chọn chuyển động giao diện. Hiệu ứng theo mốc câu LRC, không tự suy đoán mốc từng từ.
- Phím âm lượng máy, SELECT + lên/xuống, nút âm lượng gán lại và HID DAC đều chỉnh từng **1%**, nhớ riêng mức của loa/3,5 mm, Bluetooth và DAC. Giữ tương thích cấu hình âm lượng cũ. Phần cứng có số nấc mixer hữu hạn vẫn phụ thuộc DAC/ROM.
- Bỏ lớp gợi ý phím hệ thống bị chồng lên trình phát khi chỉnh âm lượng; giữ thông báo âm lượng của app.
- Giữ toàn bộ tính năng v1.9.3: thư viện có chỉ mục, quét tăng dần, hồ sơ/tiếp tục bài, 256 MiB cache dịch ngoại tuyến, Groq/MyMemory/Google, nhập Groq TXT, Spotify/SoundCloud/Drive, EQ/DSP, Bit-Perfect, DSF/DFF Native/DoP, Bluetooth/DAC/3,5 mm, LED/phổ, chế độ CPU, Wi-Fi, hẹn giờ thoát app và tích hợp riêng từng ROM.
- Nền album/lớp tối chỉ xử lý khi thay ảnh, kích thước hoặc màn hình; dùng cache chữ và giới hạn nhịp vẽ theo chế độ hiệu năng. Không thêm xử lý vào luồng giải mã/âm thanh. Nhiệt độ, pin và DAC thực tế cần kiểm tra trên máy.
- Giữ 3 link OTA chính thức đã nhúng. Khi phát hành, đăng ZIP cài đúng ROM rồi thay nội dung update.json tại **cùng ID Drive**. Người dùng chỉ cần bấm Kiểm tra cập nhật. Không dùng ZIP OTA kit/source làm payload cài app.

