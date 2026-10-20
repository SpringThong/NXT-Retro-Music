# NXT Retro Music v1.8.9.1 — NX Redux

- Icon NXT trong Tools cho launcher chính thức NX Redux v2.1.2. Có kiểm tra mã SHA, sao lưu launcher gốc và hỗ trợ khôi phục. Launcher khác được giữ nguyên.
- Ba chế độ hiệu năng thực sự điều chỉnh CPUFreq: Tiết kiệm pin tối đa 1008 MHz; Cân bằng 1608 MHz; Hiệu năng 2000 MHz theo bảng xung driver. Governor tự điều chỉnh theo tải; app nhường quyền cho ROM khi nghe nền hoặc thoát. Không thay điện áp hay bảo vệ nhiệt.
- Bảng Home nhận tên bài, nghệ sĩ, album, ảnh bìa, tiến độ và hàng đợi của NXT. Điều khiển phát/tạm dừng, trước/tiếp cho nhạc trong thẻ, Spotify và SoundCloud. Thanh âm lượng nhạc dùng gain nhạc của ROM. Dịch vụ nhạc mặc định được shutdown nhẹ nhàng khi NXT mở để tránh tranh PCM; có thể khởi động lại sau khi NXT thoát.
- LED áp dụng màu ngay: đúng hiệu ứng static của driver, nhường animation native khi NXT điều khiển và phục hồi trạng thái khi thoát. Giữ RGB, 16 màu, gradient, rainbow và các hiệu ứng hiện có.
- Nhập Groq API key từ groq-api-key.txt trong Music hoặc cạnh launch.sh. Hỗ trợ UTF-8/BOM, CRLF và UTF-16 LE/BE; giữ nguyên file gốc, không ghi khóa vào log. Giữ MyMemory, Google Cloud và các model Groq.
- Giữ link OTA NX Redux chính thức, xác minh SHA/kích thước/ROM và giữ dữ liệu người dùng khi cập nhật.

Giữ 50 theme và tài nguyên sắc nét của v1.8.9; các đường giải mã, EQ/DSP, Bit-Perfect, Bluetooth/DAC/3.5 mm, hồ sơ, nhớ bài đang nghe, song ngữ, cache bản dịch, Wi-Fi và hẹn giờ thoát app được giữ lại.

Phạm vi kiểm tra: build ARM64, bộ nạp và các fixture CPU/TXT/LED/OTA/hẹn giờ. Giao thức Home chạy ARM64 với transport socket mô phỏng do giới hạn môi trường. Chưa thử trực tiếp Home, LED, âm thanh, nhiệt độ, pin hoặc deep sleep trên máy thật.
