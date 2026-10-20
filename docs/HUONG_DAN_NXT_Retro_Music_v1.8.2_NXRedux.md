# NXT Retro Music v1.8.2 — NX Redux

## Cài trên TrimUI Brick Pro

1. Giải nén `NXT_Retro_Music_v1.8.2_NXRedux.zip` trên máy tính.
2. Chép thư mục **Tools** vào gốc thẻ nhớ. Đường dẫn sau khi chép:
   `Tools/NXT Retro Music.pak/launch.sh`.
3. Mở tab **Tools → NXT Retro Music** trong NX Redux. Khởi động lại máy nếu vừa
   thay bản cài mà phiên NXT cũ vẫn chạy nền.
4. Giữ `.userdata/shared/nxt-retro-music/` để bảo toàn hồ sơ, playlist, EQ, hình nền,
   tài khoản và cài đặt. Không chép gói này vào `.system`.

Gói này dành cho Brick Pro (1024×768, tg5040); đã đối chiếu gói firmware NX Redux
v2.0.0 dành cho Brick Pro, trên nền firmware TrimUI tương ứng. Các bản NX Redux khác
chưa được thử trực tiếp. Bộ cài Stock/NextUI không phải gói dành cho NX Redux.

## Nghe và cá nhân hóa

- Nhạc trên thẻ: vào Thư viện, chọn thư mục nhạc và phát bài/playlist.
- Cá nhân: chọn hồ sơ; đổi avatar, giao diện, hình nền, LED và phổ nhạc tại hồ sơ.
- Nút X/Y/SELECT/START và các phím khác theo ánh xạ đang hiển thị của NXT; có thể
  sửa ở Cài đặt phím. Logo TrimUI mặt trước dừng và thoát hẳn app.
- Cài đặt → Hiệu năng thiết bị: chọn Tiết kiệm pin, Cân bằng hoặc Hiệu năng.
- Trong trình phát hoặc Tùy chọn nhanh, mở Thông tin bài hát & đầu ra để xem nguồn,
  chất lượng bài, PCM và DAC. Thông tin khả năng DAC không có nghĩa mọi bài phát
  ở đúng mức tối đa của DAC. USB trên NX Redux chia sẻ bộ trộn S16_LE của firmware.
- Bật Bluetooth/kết nối tai nghe trong NX Redux, rồi chọn Bluetooth trong NXT.
  Khi mở NXT, trình phát nhạc tích hợp của NX Redux được dừng để nhường PCM; không
  xóa playlist hay tài khoản của trình phát hệ thống.
- Spotify cần tài khoản Premium và đăng nhập bằng bộ Spotify_Login_PC đi kèm.
  SoundCloud và các nguồn online vẫn dùng cơ chế/quyền truy cập của v1.8.2.

## Hẹn giờ thoát app để tiết kiệm pin

1. Mở biểu tượng đồng hồ hoặc Tùy chọn nhanh → Hẹn giờ.
2. Chọn 15/30/45/60/90/120 phút hoặc nhập thời gian; 0 phút là hủy.
3. Hết giờ, NXT lưu vị trí/cài đặt, dừng mọi nguồn nhạc và **thoát toàn bộ tiến trình**.
   Áp dụng cả khi app đang phát nền, tạm dừng hoặc màn hình đã tắt.
4. NX Redux tiếp tục xử lý tắt màn hình và deep sleep theo cài đặt của hệ thống.
   Hãy bật thời gian tắt màn hình và suspend/deep sleep trong NX Redux nếu muốn
   máy tự ngủ; khi hệ thống đặt Never/Off, thoát NXT không tự bật lại chế độ ngủ.
5. Mở lại NXT khi muốn nghe tiếp. Hẹn giờ đã hết không tự kích hoạt lại.

App đóng qua quy trình dọn dẹp đầy đủ thay cho SIGKILL để bảo toàn dữ liệu và trả
âm thanh/LED cho hệ thống. Không có thêm tiến trình canh hẹn giờ sau khi NXT đóng.

## Chép nhạc qua Wi-Fi và OTA

Chỉ bật chia sẻ Wi-Fi khi cần chép file, dùng địa chỉ và PIN app hiển thị rồi tắt
chia sẻ sau khi xong. USB-C vẫn dành cho DAC; không có tính năng copy qua USB-C.

Mở **Thông tin & cập nhật → Cập nhật → Nguồn cập nhật**, nhập link TỆP `update.json`
công khai dành riêng cho NX Redux hoặc đọc link từ `.txt/.url`. Bộ OTA đi kèm giúp
tạo manifest và nguồn khi bạn đăng lên Drive. Không nhập link thư mục hoặc nguồn
Stock/NextUI. OTA kiểm tra ROM, thiết bị, phiên bản, dung lượng và SHA-256.

## Thử nhanh trên máy

Thử phát nhạc thẻ và online, chuyển loa/Bluetooth/DAC, chỉnh âm lượng và mute trong
OSD, đổi LED khi đang phát, chạy nền/mở lại. Đặt hẹn giờ tùy chỉnh 1 phút; thử lần
nữa khi chạy nền hoặc tạm dừng; xác nhận NXT đóng và máy ngủ theo thời gian đã đặt
trong NX Redux. Bấm Power để thức máy rồi mở NXT để kiểm tra vị trí/cài đặt.

Khi lỗi, gửi `Tools/NXT Retro Music.pak/NXT_Input_Log.txt` và phiên bản NX Redux.
Log trong app: Thông tin → xuất `NXT_App_Log.txt`.

## Build lại

Giải nén gói Source dành cho NX Redux, trên Ubuntu/WSL:

```bash
NXT_JOBS=3 make nxredux
```

SDK/compiler có checksum; lần đầu cần mạng để lấy SDK. Không cần Docker.
Mã NXT mới giữ GPL-3.0-only; mã nền và thư viện/font giữ giấy phép đi kèm.
