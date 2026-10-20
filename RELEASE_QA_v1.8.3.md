# NXT Retro Music v1.8.3 — kết quả kiểm tra Stock / NextUI

## Thay đổi đã build

Hết hẹn giờ, app lưu vị trí/cài đặt, dừng mọi nguồn nhạc và thoát toàn bộ phiên
foreground/background. Module cleanup giải phóng âm thanh, online workers, LED,
Wi-Fi transfer, database và session socket. Trả việc ngủ máy về firmware; không
ghi trực tiếp vào `/sys/power/state`, không thêm timer daemon hay dùng SIGKILL.

## Các kiểm tra đã thực hiện

- **Hẹn giờ:** production `nxt_config.c` và `nxt_session.c`, clock/playback fixtures,
  ASan/UBSan; đúng thời điểm hết hạn, chỉ kích hoạt một lần, hủy, thay hẹn giờ,
  tràn đồng hồ 32 bit, pause/screen-off, radio/podcast, online/background exit,
  lưu thiết lập và dọn session/status. Host chặn sockets, nên transport là fixture
  rõ ràng; state machine/cleanup là mã thực. Stock và NextUI đều thoát toàn bộ.
- **Stock:** adapter thật với các tiến trình Linux kiểm thử. Cờ stay_awake/stay_alive
  do NXT sở hữu được gỡ, cờ ứng dụng khác được giữ, keymon do NXT tạm dừng được
  trả lại, keymon đã dừng trước đó được giữ nguyên. Protocol này cũng khớp launcher
  musicplayer trong firmware Stock chính thức đã khóa SHA-256.
- **Cài đặt:** đọc/lưu/load config thực, giữ lựa chọn âm thanh, LED, hình nền,
  chế độ hiệu năng; fallback an toàn khi dữ liệu lỗi. Không đổi đường dữ liệu cá nhân.
- **Bluetooth/âm thanh:** production discovery, private ALSA config/parser và PCM
  reopen, fixtures SDL/BlueZ; chọn A2DP thay vì gamepad/SCO, đổi loa/BT/DAC, sửa
  sink cũ, retry transport, local/Spotify/SoundCloud PCM khác 0, idle không mở lại
  audio; thực thi mixer helper với mute/volume cho cả hai ROM. ASan/UBSan.
- **ARM64:** frontend và backend Stock đã chạy qua QEMU với rootfs TrimUI Brick Pro
  chính thức v1.1.1-20260717 (SHA-256 trong STOCK_SOURCE.json). Kiểm tra loader,
  --help/--build-info/--stock-info, SDL, tài nguyên, Spotify/SoundCloud khởi động
  và thoát, yt-dlp. Frontend NextUI cũng chạy --help/--build-info với thư viện
  payload của mình và dependency rootfs Stock; đây không phải một phiên NextUI OS.
- **OTA từng ROM:** worker và installer thật, nguyên ZIP v1.8.3, giả lập bản đang
  cài v1.8.2; Drive-confirmation fixture, từ chối sai ROM/SHA/size/HTML, download,
  stage/install, giữ profile/credentials/hình nền, backup bản cũ, hủy và thu hồi child.
- **Bộ tạo nguồn OTA:** Node VM với DOM/download fixtures; tạo JSON/.txt đúng ROM,
  kiểm tra hash nguyên ZIP, từ chối link sai, nội dung tự chứa và CSP không mạng.
- **Bộ cài:** hai ZIP đúng version/ROM, ELF ARM64 khớp build, 36 theme, 300 icon,
  180 emoji 3D, 14 ngôn ngữ/IME, quyền thực thi, CRC; không có dữ liệu đăng nhập,
  thư mục hệ thống hoặc copy USB-C.
- **Giữ v1.8.2:** đối chiếu byte với source NX Redux v1.8.2 hiện tại. Chỉ bốn
  file frontend đổi (`nxt.h`, `nxt_config.c`, `nxt_session.c`, `nxt_screens.c`).
  Mã render theme, decoder, DSP, Bluetooth/DAC, LED, online, Wi-Fi và keyboard
  cùng backend/asset giữ nguyên. Locale chỉ thêm hướng dẫn/tên phiên bản mới.

## Giới hạn thực tế

Chưa chạy app mới trên Brick Pro vật lý, chưa đăng nhập tài khoản online mới,
chưa đo pin/nhiệt, thử DAC/tai nghe vật lý hoặc xác nhận suspend/wakeup thực.
Nguồn OTA công khai của nhà phát hành chưa được cập nhật trong tác vụ này;
cung cấp các bộ để đăng. Deep sleep phụ thuộc firmware/cài đặt và điều kiện hệ
thống như sạc USB, ứng dụng khác hoặc cờ giữ máy thức còn tồn tại.

NX Redux v1.8.2 không được build lại trong tác vụ này. Báo cáo/binary NX cũ và các
kết quả v1.8.2 đi kèm source được giữ làm lịch sử, không phải kết quả v1.8.3.

Kiểm tra nhanh trên máy: hẹn giờ 1 phút khi phát, pause và phát nền; app đóng,
nhạc dừng, máy ngủ theo thiết lập; đánh thức và kiểm tra vị trí/cài đặt còn nguyên.
Chi tiết chứng cứ: `BUILD_REPORT_v1.8.3_Stock.json`, `BUILD_REPORT_v1.8.3_NextUI.json`
và các file `logs/v183-*.json` được tham chiếu trong báo cáo.
