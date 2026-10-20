# NXT Retro Music v1.8.2

Ý tưởng và thực hiện: Nguyễn Xuân Thông · Spring Thông · Vibe Code.

- Sửa Bluetooth: chọn đúng tai nghe A2DP, giữ cấu hình đầu ra riêng của app, làm mới PCM khi đổi đầu ra và đặt âm lượng kèm unmute. Đã đối chiếu lỗi từ log v1.8.1 và tái hiện bằng selector gốc; bản sửa vượt kiểm thử hồi quy. Gói phát hành được build lại từ mã có bản sửa.

- Khôi phục trình phát NXT gốc theo bố cục v1.8: thấy hình nền dưới UI, ảnh album/đĩa than bên trái, thông tin bài và DAC bên phải, lời một hàng, phổ nhạc, thanh tiến trình, số bài và hai hàng nút/phím có nhãn như cũ.
- Giữ 35 theme còn lại và các điều khiển icon của v1.8.1. Giữ chữ lời nguyên câu, không chạy ngang.
- Cài đặt → Hiệu năng thiết bị → Chế độ hiệu năng: Tiết kiệm pin, Cân bằng, Hiệu năng. Áp dụng ngay, lưu qua lần mở tiếp theo và không bị đổi theo hồ sơ. Mặc định Cân bằng.
- Các chế độ điều chỉnh nhịp vẽ, FFT và đĩa than. Giữ nguyên xử lý âm thanh, EQ/DSP, DAC/PCM, nhịp đọc phím và chính sách CPU/nhiệt của firmware. Không ép xung.
- Cá nhân → Hồ sơ → Giao diện cá nhân: đổi Tinh chỉnh NXT thành Tùy chọn giao diện gốc. Giữ toàn bộ tùy chọn gốc, LED và phổ nhạc.
- Giữ phát nhạc trong thẻ, Spotify/SoundCloud/Drive, hồ sơ, gợi ý nhạc, lịch sử, 14 ngôn ngữ và bàn phím, LED đổi tức thì, phát nền, chép file Wi-Fi và DAC USB.
- Hướng dẫn trong app theo đúng vị trí menu mới; sửa mục Wi-Fi lẫn sang hướng dẫn ngôn ngữ.
- OTA v1.8.2 riêng cho Stock/NextUI, kiểm tra ROM/version/size/SHA-256 trước cài, sao lưu app cũ và giữ dữ liệu cá nhân. Có bộ tạo update.json offline để nhà phát hành đăng lên Drive.
