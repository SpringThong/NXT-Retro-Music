# NXT Retro Music v1.8.2 — kết quả kiểm tra

**Bản phát hành đã build lại sau sửa Bluetooth.** ELF, ZIP cài và bộ OTA Stock/NextUI đều được tạo từ mã nguồn hiện tại. Kết quả kiểm tra được đối chiếu với đúng hash ELF, input build và gói phát hành. Chi tiết Bluetooth nằm trong `BLUETOOTH_CHECK_v1.8.2.md` và `logs/v182-nextui-bluetooth-test.json`.

Build native C/SDL AArch64 cho TrimUI Brick Pro: Stock và NextUI, dựa trên source v1.8.1. Các kết quả dưới đây được chạy cho mã và gói v1.8.2; báo cáo SHA-256 riêng nằm trong `BUILD_REPORT_v1.8.2_Stock.json` và `BUILD_REPORT_v1.8.2_NextUI.json`.

## Các thay đổi đã kiểm tra

- NXT gốc dùng bố cục trình phát v1.8 theo ảnh tham chiếu: nền xuyên qua UI, ảnh album/đĩa than trái, thông tin nguồn/DAC phải, lời, phổ nhạc, tiến trình, số bài và các phím có nhãn như cũ. 35 theme khác giữ bố cục v1.8.1. Ảnh album ngang/dọc giữ tỉ lệ và nằm giữa khung cũ.
- Ba chế độ Tiết kiệm pin / Cân bằng / Hiệu năng trong Cài đặt → Hiệu năng thiết bị. Đổi chế độ áp dụng ngay; A lưu, B hủy; lỗi lưu trả lại chế độ cũ. Mặc định Cân bằng.
- Chạy hàm cấu hình thật trên hai ROM: cấu hình v1.8.1 không có trường mới sẽ dùng Cân bằng và giữ tùy chọn EQ/giao diện/LED; cả ba chế độ lưu và đọc lại; đổi hồ sơ không đổi chế độ toàn thiết bị; dữ liệu sai kiểu, ngoài khoảng hoặc số lẻ dùng Cân bằng.
- Tên Tùy chọn giao diện gốc xuất hiện trong hub và trang tùy chọn; nhãn chế độ mới có trong 14 bộ ngôn ngữ. Menu cài đặt vẫn có năm nhóm; hướng dẫn Wi-Fi không lẫn nội dung ngôn ngữ.

## Hiệu năng phần hiển thị

Các chế độ chỉ điều chỉnh ngân sách công việc hiển thị. Không đổi decoder, callback âm thanh, EQ/DSP, nhịp đọc phím hoặc chính sách CPU/nhiệt của firmware. Việc chọn và mở lại đầu ra PCM được sửa riêng để khắc phục Bluetooth.

| Công việc | Tiết kiệm pin | Cân bằng | Hiệu năng |
|---|---:|---:|---:|
| Vẽ player thông thường | ≥125 ms | ≥100 ms | ≥50 ms |
| Vẽ theme chuyển động cơ khí | ≥100 ms | ≥50 ms | ≥33 ms |
| FFT/phổ nhạc | ≥67 ms | ≥33 ms | ≥33 ms |
| Đĩa than/xem trước theme | ≥100 ms | ≥50 ms | ≥33 ms |
| Player tạm dừng | ≥1000 ms | ≥1000 ms | ≥1000 ms |
| E-Ink | ≥250 ms | ≥250 ms | ≥250 ms |
| Ẩn app/tắt màn hình | Không vẽ/FFT | Không vẽ/FFT | Không vẽ/FFT |

Hàm policy và hàm frame production chạy với vòng xử lý phím 60 Hz trong test: player thường vẽ 8/9/16 lần, theme cơ khí 9/16/21 lần, FFT 13/21/21 lần trong một giây theo ba chế độ. Các khoảng trên là giới hạn nhịp, không phải cam kết FPS trên máy thật. Kiểm tra có chuyển chế độ ngay, thức lại và tràn bộ đếm uint32; ASan/UBSan đạt.

## Kiểm tra gói, giao diện và OTA

- Biên dịch bằng compiler ARM64 đã ghim; hai ELF trả version v1.8.2 và đúng ROM. QEMU user-mode với SDK libc tải/thoát frontend và backend; Stock còn chạy với rootfs firmware chính thức và kiểm tra tài nguyên/SDL, startup backend, yt-dlp.
- UI production chạy ASan/UBSan với dữ liệu bộ điều khiển, cảm biến và âm thanh mô phỏng trên cả hai ROM: 36 home/player ở 1024×768 và 640×480; lời một hàng có thay đổi khi đổi câu, không tự chạy ngang; gallery apply/cancel; hồ sơ/login; font/album preview; EQ; menu; chất lượng nguồn và DAC/PCM độc lập. Đã xem ảnh render NXT gốc, chế độ hiệu năng và ảnh album fit để kiểm tra bố cục.
- Gói cài đúng ELF/version/ROM, CRC và quyền thực thi; đủ 36 theme, 300 icon, 180 emoji 3D, 14 locale và IME offline. Không chứa tài khoản/dữ liệu cá nhân hoặc chức năng chép file USB-C.
- So sánh byte với source v1.8.1 xác nhận decoder, callback âm thanh, DSP, LED, session/background, worker và binary Spotify/SoundCloud, Wi-Fi, IME, keyboard và nhịp input giữ nguyên. Các file điều khiển đầu ra và âm lượng được sửa cho Bluetooth và kiểm tra riêng bằng hàm thật.
- Log máy v1.8.1 NextUI cho thấy requested=1, selected=0, route write failed/pending trong khi PCM cũ vẫn sẵn sàng. Test với selector v1.8.1 nguyên bản tái hiện trạng thái này; bản sửa chọn A2DP bằng cấu hình app riêng và giữ nguyên `.asoundrc` firmware. Parser ALSA thật, phát callback với mẫu local/Spotify/SoundCloud mô phỏng, chuyển loa/Bluetooth/DAC, đổi tai nghe, transport chậm, sink firmware cũ, idle không mở PCM liên tục và unmute A2DP ở cả hai ROM đều đạt ASan/UBSan. Không thay mute hệ thống hoặc shim bảo vệ settings của NextUI.
- Kiểm thử phần cứng mô phỏng xác nhận HID âm lượng DAC, retry PCM và LED đổi màu/chế độ ngay không cần tắt bật vẫn hoạt động.
- OTA chạy worker và installer production với đúng ZIP v1.8.2, mô phỏng app đang cài v1.8.1: nhận bản mới, xác nhận tải Drive, từ chối sai ROM/dung lượng/SHA-256/HTML, tải/stage/cài, giữ trạng thái sẵn sàng, backup app cũ, hủy và thu hồi tiến trình. Hồ sơ, tài khoản và hình nền ngoài app giữ nguyên byte. Không thử tải bằng tài khoản Drive thật.
- Bộ phát hành OTA có ZIP cài nguyên bản, HTML offline tạo update.json và tệp nguồn, hướng dẫn, công cụ Python và checksum. SHA/size trỏ tới ZIP cài bên trong; JSON có placeholder chỉ nằm ở templates. Node VM với DOM/download giả lập kiểm tra sinh JSON, nguồn TXT, link sai và SHA của ZIP thật. Đây không phải kiểm tra trình duyệt có render.

## Lệnh tái kiểm tra

```sh
python3 qa/test_v182_config.py
python3 qa/test_v182_performance.py
python3 qa/test_v182_baseline.py
python3 qa/test_v182_nextui_bluetooth.py
python3 qa/test_v182_led_driver.py
python3 qa/test_stock_runtime.py
NXT_TEST_ROM=NXT_ROM_STOCK python3 qa/test_v182_ui.py
NXT_TEST_ROM=NXT_ROM_NEXTUI python3 qa/test_v182_ui.py
NXT_TEST_ROM=stock python3 qa/test_v182_arm64.py
NXT_TEST_ROM=nextui python3 qa/test_v182_arm64.py
python3 qa/test_v182_packages.py
python3 qa/test_v182_stock_rootfs.py
NXT_TEST_ROM=stock python3 qa/test_v182_ota.py
NXT_TEST_ROM=nextui python3 qa/test_v182_ota.py
node qa/test_v182_publisher_generator.cjs
python3 tools/finalize-v182-reports.py
```

Hai UI test dùng cùng stage tài nguyên nên chạy lần lượt. Test baseline cần ZIP source v1.8.1 ở `../v182-input/`. Các file `logs/v182-*` là kết quả hiện tại; log và báo cáo v1.8/v1.8.1 chỉ để đối chiếu lịch sử.

## Phạm vi cần thử trên máy

Chưa đo FPS, nhiệt độ hoặc pin trên thiết bị vật lý; chưa thử DAC/Bluetooth vật lý hay phát bằng tài khoản online đã đăng nhập. Chưa đăng ZIP/JSON lên nguồn OTA công khai của nhà phát hành. Khi đăng, dùng bộ đúng ROM và `CHUAN_BI_OTA.html`, nhập link ZIP công khai để tạo JSON rồi cập nhật nguồn JSON ổn định của bạn.
