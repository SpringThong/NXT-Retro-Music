# NXT Retro Music v1.8.2 — NX Redux / TrimUI Brick Pro

Lên ý tưởng và thực hiện: Nguyễn Xuân Thông — Spring Thông · Vibe Code.

- Bộ cài native ARM64 riêng cho NX Redux, xuất hiện ở Tools với biểu tượng NXT.
- Giữ 36 giao diện, hồ sơ/avatar, gợi ý cá nhân, LED đổi ngay, phổ nhạc, EQ/DSP,
  lời bài hát, playlist, nghe trên thẻ và các dịch vụ online của v1.8.2.
- Kết nối đúng API cài đặt của NX Redux; âm lượng nhạc và mute trong OSD có tác dụng
  với NXT. Không dùng cấu trúc shared memory cũ của NextUI để sửa dữ liệu NX Redux.
- Đầu ra Bluetooth dùng PCM riêng; USB DAC/loa dùng bộ trộn phù hợp với firmware.
  Tạm dừng trình phát hệ thống khi mở NXT để nhường đầu ra âm thanh.
- **Hết hẹn giờ: lưu vị trí/cài đặt, dừng nhạc và thoát toàn bộ NXT**, kể cả phát nền,
  tạm dừng hoặc đã tắt màn hình. Không để lại phiên NXT ngủ ngầm.
- Sau khi NXT thoát, NX Redux xử lý tắt màn hình/deep sleep theo cài đặt hệ thống.
  NXT không tự thay thời gian ngủ của firmware. Hủy hẹn giờ bằng 0 phút.
- OTA riêng: `rom=nxredux`, nguồn và bộ cài tách khỏi Stock/NextUI.
- Copy file qua Wi-Fi; USB-C tiếp tục dùng cho DAC. Các chế độ hiệu năng vẫn có đủ
  Tiết kiệm pin / Cân bằng / Hiệu năng.

Chỉ bản NX Redux nhận thay đổi thoát app khi hết hẹn giờ trong lượt phát hành này.
Stock/NextUI vẫn dừng nhạc như trước; chưa build lại hai bản đó.

Đối chiếu runtime NX Redux v2.0.0 (Brick Pro). Kiểm thử trên máy tính/QEMU và các
fixture của driver, không thay cho việc thử Bluetooth, DAC, deep sleep trên máy thật.
