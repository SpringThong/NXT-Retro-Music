# QA bản v1.0

Các test `test_native_text.c`, `test_native_audio.c`, `test_native_persistence.c` được cross-compile ARM64 và chạy bằng QEMU với sysroot TG5040. Log đã đạt nằm ở `logs/arm-*-test.log`. Đây là kiểm tra mã ARM64 trong giả lập userspace, không phải kiểm tra âm thanh/GPU/DAC/LED trên máy thật.

```bash
sudo apt-get install qemu-user
bash tools/setup-sdk.sh
bash tools/setup-audio.sh nextui
bash tools/setup-text.sh nextui
bash qa/run-arm-tests.sh
bash qa/run-arm-audio.sh
bash qa/run-arm-persistence.sh
```

`media/` chứa tín hiệu sine thử tự tạo, codec khác nhau, metadata và lời Unicode thử. Không sử dụng bản thu nhạc thương mại. Test text cần chạy từ thư mục `upstream/` để tìm `res/fonts/`; script đã xử lý đường dẫn này. Test audio so metadata thực, giải mã, PCM hữu hạn và tua. Test persistence kiểm tra định dạng preset, hồ sơ đầu ra, dữ liệu không hợp lệ và playlist UTF-8/đường dẫn tương đối/tên dài.

`*.control` và ảnh `screens/` ghi thao tác giao diện native SDL trên bản desktop debug 1024×768 trong môi trường kiểm tra. Các script capture lưu đường dẫn tuyệt đối của môi trường build; nếu dùng lại trên WSL, đổi đường dẫn `screenshot(...)` sang thư mục QA của bạn và chuẩn bị thẻ giả tại `/var/tmp/nextui/sdcard/`. Binary bản cài ARM64 release không có kênh `--test-control`.

Ảnh `help-mapping.png` kiểm tra đổi A/X: hướng dẫn cập nhật X thành phát/tạm dừng và A thành hàng đợi. Ảnh `appearance-large.png` kiểm tra JetBrains Mono ở cỡ giao diện 120%. Các ảnh `*-final.png` là giao diện chốt với Noto Sans 100%; log `gui-final.log` xác nhận bài tạm dừng và giữ vị trí 554 ms khi chuyển màn.

Kiểm tra trên phần cứng còn lại: khởi động trên NextUI, phát/tua thực, nút vật lý và giữ SELECT, USB DAC/Bluetooth, mix nhạc với game, hẹn giờ/tiết kiệm điện, LED và khôi phục cấu hình hệ thống. Các điều này chưa có kết quả thực tế cho bản 0.3.0.

## Kiểm tra cập nhật v1.0

`run-v1-gui.py` kiểm tra Power, L2/R2, khóa, biểu tượng jack/DAC và chụp About/hướng dẫn. `run-v1-lifecycle.py` kiểm tra khởi động lại, giữ kiểu/màu phổ, chuyển bài khi tắt màn hình, hẹn giờ thật 60 giây và giữ Power. Chạy lần lượt trên binary **desktop debug** sau khi build; các script chỉ ghi thẻ thử tại `/var/tmp/nextui/sdcard/`. Test đầu reset dữ liệu app trong thẻ thử. `NXT_QA_OFFLINE=1` chặn tra cứu ảnh bìa và metadata trong bản debug; release không có hook này. Log mới nằm tại `logs/v1-*.log`.

ARM64 font / codec / persistence đã chạy lại: `logs/arm-text-v1.log`, `arm-audio-v1.log`, `arm-persistence-v1.log`. Bản 0.3.0 đã có phản hồi chạy trên Brick Pro từ người dùng; kết quả phần cứng v1.0 còn chờ thử trực tiếp.

## Stock Controls & Drive Fix2 — historical

Bộ kiểm tra mới: `test_stock_controls_drive.py` (Telex, parser, Unicode/natural
sort); `test_stock_input.py` (SELECT và hai analog qua SDL); `test_stock_keyboard.py`
(vòng lặp bàn phím A/B/X/Y, Shift/Caps/SELECT và ảnh phần mềm);
`test_stock_drive_io.py` (tải/hủy/bộ đệm/nhãn-đường dẫn thư mục bằng fixture);
`test_stock_drive_decode.py` (bộ giải mã ARM64 thật, tệp .part, tên Việt, PCM/tua
trong rootfs Stock chính thức). `test_stock_runtime.py` thêm giao thức bảng Stock,
ACK/timeout và kiểm tra keymon không chạy lại trong lúc mở bảng. Log nằm trong
`logs/stock-*-test.*`; ảnh bàn phím ở `screens-stock-controls/`.

`test_stock_drive_live.py` là phép thử mạng tùy chọn với thư mục/tệp văn bản công
khai của dự án gdown; không dùng tài khoản riêng và không khẳng định phát nhạc
Drive trên máy thật. Native QA cần GCC, SDL2/Freetype/HarfBuzz/FriBidi và Python/PIL;
`setup_render_spotify.py` chuẩn bị header/object font khi cần. QA ARM64 cần SDK,
rootfs Stock và QEMU. Xem `stock/README.md` cho phép thử DAC và rootfs.

## Stock Vinyl Player Fix1 — hiện tại

Các phép thử hiện tại được chạy cho frontend mới và được ghi trong `BUILD_REPORT_V1_2_STOCK.json`:

| Script | Phần mã được kiểm tra |
| --- | --- |
| `test_stock_record.py` | Renderer đĩa than và vòng cập nhật thực, phát/tạm dừng/tiếp tục, tắt màn/nền, đổi bìa/theme/kích thước, ASan/UBSan. |
| `test_stock_controls_drive.py` | Telex tiếng Việt, parser link Drive và natural sort. |
| `test_stock_input.py` | SELECT + hướng, sự kiện SDL của cả hai analog và D-pad. |
| `test_stock_keyboard.py` | Vòng lặp bàn phím A/B/X/Y, chỉ hai chế độ Telex/Latin, URL nguyên văn. |
| `test_stock_drive_ui.py` | Chọn bài/Nghe cả thư mục tạo hàng đợi đúng thứ tự và vị trí; M3U lưu URI luồng; refresh/hủy/lỗi ghi. |
| `test_stock_drive_queue.py` | Tự chuyển bài, lặp một/tất cả, shuffle và khôi phục hàng đợi. |
| `test_stock_decoder.py` | Decoder ARM64 với rootfs Stock: 10 định dạng, tua, metadata, resample và DSP/EQ/ReplayGain/limiter. |
| `test_stock_dac_volume.py` | 200 lần nút HID, gain PCM, route DAC và SDL/ALSA chính thức qua QEMU. |
| `test_stock_runtime.py` | Home mở bảng nhanh Stock, ACK/timeout, keymon, cleanup và BT validation. |
| `test_stock_session_upgrade.py` | Handshake phiên nền, từ chối bản cũ, reattach cùng bản, stale endpoint/PID, launcher Stock thực. |
| `test_stock_rootfs.py` | Payload ARM64 đã đóng gói: help/build-info/Stock-info, Spotify/SoundCloud ready/quit, yt-dlp. |

Ảnh giao diện ở `screens-stock-vinyl/`, ảnh bàn phím ở `screens-stock-controls/`. Các boundary curl/audio/UI của Drive và dữ liệu Spotify/DAC trong ảnh là fixture; không khẳng định kiểm tra phát online hoặc DAC thật. Socket transport dùng file fixture do môi trường từ chối socket. LeakSanitizer không dùng được do hạn chế `/proc/task`; có ASan/UBSan và kiểm tra giải phóng cache riêng. Log Drive tải/cache của Fix2 được chuyển vào `logs/history/Stock_Controls_Drive_Fix2/` và không được dùng xác nhận bản luồng hiện tại.

Sau khi đổi source, chạy lại các script liên quan, `make stock`, `bash tools/relink-stock.sh` và `python3 qa/test_stock_rootfs.py`, rồi `python3 tools/package-stock-source.py`. Không thay kết quả mô phỏng bằng tuyên bố thử phần cứng. Hướng dẫn PDF/DOCX 20 trang có nguồn nội dung trong `docs/manual-v1.2.json` và script `tools/create-manual.py`.
