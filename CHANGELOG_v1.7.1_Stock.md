# NXT Retro Music v1.7.1 — Stock

- Dựng lại 23 bố cục máy nghe nhạc theo demo 24 giao diện đã duyệt; Original NXT giữ màu RGB, hình nền và lựa chọn ảnh vuông/đĩa than. Tên bài, lời, phổ, tiến độ, phím và thông số âm thanh được đặt trong cửa sổ riêng của từng kiểu máy.
- LED đang bật: đổi màu RGB, hiệu ứng hoặc chế độ sẽ lập trình RGB trước rồi ghi lại hiệu ứng để driver nhận màu mới. Không cần tự tắt/bật LED. Giữ cơ chế kiểm tra đọc lại, thử lại khi driver lỗi và trả quyền LED cho ROM khi thoát.
- EQ dùng icon thể loại + tên: Flat, Pop, Rock, Jazz, Classical, Vocal, EDM, Acoustic, Bass Boost, Treble, Lo-Fi, Loudness. Giữ Tùy chỉnh 5 dải và trang Nâng cao.
- Rainbow hiển thị đồng thời 7 màu; Multicolor Mix xen kẽ nhiều màu riêng biệt. Hai màu RGB riêng vẫn được lưu. E-Ink dùng mực xám theo mẫu. Oscilloscope vẽ PCM thực và bỏ FFT không cần thiết.
- Khung máy được dựng và cache một lần cho giao diện đang dùng. Cuộn băng/VU/CD cập nhật tối đa 20 lần/giây; các màn hình khác 10 lần/giây; E-Ink 4 lần/giây; tạm dừng 1 lần/giây. Phổ và đĩa than dùng lớp riêng. Tắt màn hình/ẩn app không làm việc vẽ/FFT; tốc độ đọc phím được giữ.
- Không ghi log mỗi bước hoạt ảnh LED, tránh ghi thẻ liên tục. Giữ nhạc thẻ, Drive, Spotify, SoundCloud, lời, chất lượng nguồn, loa/tai nghe/Bluetooth/DAC, PCM, shuffle/repeat, yêu thích và hẹn giờ.

Kiểm tra bằng mã native ở 1024×768 và 640×480, ASan/UBSan, điều khiển giả lập và driver sysfs giả lập. Bản phát hành chưa được đo nhiệt/độ trễ trên thiết bị thật trong phiên này.
