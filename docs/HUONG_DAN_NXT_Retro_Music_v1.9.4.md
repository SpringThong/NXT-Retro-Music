# NXT Retro Music v1.9.4 — Auralis & fine volume

- Thêm **Auralis Themes** (theme số 51), giữ nguyên 50 theme trước. Bìa album phủ đầy màn hình theo đúng tỉ lệ, có lớp tối giúp lời dễ đọc. Chọn trong Cá nhân → Chỉnh sửa giao diện cá nhân → Bộ giao diện.
- Thêm **MiSans Semibold · Xiaomi / NX Redux** vào bộ font, giữ nguyên 16 font cũ. Auralis sử dụng MiSans. Giấy phép và ghi công Xiaomi tại licenses/MiSans.txt.
- Đang phát và Y xem lời đầy đủ hiển thị cặp lời gốc / bản dịch **in nghiêng**, phân biệt Đang hát và Tiếp theo. Mỗi câu nằm trong một hàng, co vừa chiều rộng; không chạy ngang. Chuyển dòng dọc có thể tắt bằng tùy chọn chuyển động giao diện. Hiệu ứng theo mốc câu LRC, không tự suy đoán mốc từng từ.
- Phím âm lượng máy, SELECT + lên/xuống, nút âm lượng gán lại và HID DAC đều chỉnh từng **1%**, nhớ riêng mức của loa/3,5 mm, Bluetooth và DAC. Giữ tương thích cấu hình âm lượng cũ. Phần cứng có số nấc mixer hữu hạn vẫn phụ thuộc DAC/ROM.
- Bỏ lớp gợi ý phím hệ thống bị chồng lên trình phát khi chỉnh âm lượng; giữ thông báo âm lượng của app.
- Giữ toàn bộ tính năng v1.9.3: thư viện có chỉ mục, quét tăng dần, hồ sơ/tiếp tục bài, 256 MiB cache dịch ngoại tuyến, Groq/MyMemory/Google, nhập Groq TXT, Spotify/SoundCloud/Drive, EQ/DSP, Bit-Perfect, DSF/DFF Native/DoP, Bluetooth/DAC/3,5 mm, LED/phổ, chế độ CPU, Wi-Fi, hẹn giờ thoát app và tích hợp riêng từng ROM.
- Nền album/lớp tối chỉ xử lý khi thay ảnh, kích thước hoặc màn hình; dùng cache chữ và giới hạn nhịp vẽ theo chế độ hiệu năng. Không thêm xử lý vào luồng giải mã/âm thanh. Nhiệt độ, pin và DAC thực tế cần kiểm tra trên máy.
- Giữ 3 link OTA chính thức đã nhúng. Khi phát hành, đăng ZIP cài đúng ROM rồi thay nội dung update.json tại **cùng ID Drive**. Người dùng chỉ cần bấm Kiểm tra cập nhật. Không dùng ZIP OTA kit/source làm payload cài app.

# NXT Retro Music v1.9.4 — unified source

Một bộ source cho **Stock, NextUI và NX Redux** trên TrimUI Brick Pro. Ba frontend dùng chung mã hiện tại, cùng các sửa đổi v1.9.2 và thư viện tăng dần v1.9.4; mỗi ROM giữ tích hợp riêng và link OTA chính thức. Tất cả ba binary/fingerprint trong source này được đồng bộ với cùng mã nguồn.

Đọc **BUILD_v1.9.4.md** để chuẩn bị Linux/WSL x86_64, build và tìm ZIP cài/OTA. Lệnh nhanh:

```sh
python3 tools/build-release.py --rom all --check
python3 tools/build-release.py --rom all
```

Chọn riêng `--rom stock`, `--rom nextui` hoặc `--rom nxredux` khi chỉ cần một ROM. SDK ghim checksum tải tự động lần đầu; các dependency/frontend input và runtime nền đã giữ trong source. Không cần ghép source cũ, không cần Docker.

Tài liệu dùng app: docs/HUONG_DAN_NXT_Retro_Music_v1.9.4_<ROM>.md; thư viện: docs/LIBRARY_v1.9.4.md; DSD: docs/DSD_v1.9.4_<ROM>.md. Xem RELEASE_QA_v1.9.4.md và BUILD_REPORT_v1.9.4_<ROM>.json cho phạm vi kiểm tra. Báo cáo/tài liệu bản cũ được giữ làm lịch sử.

Không có khóa API hay dữ liệu người dùng trong source. Giữ giấy phép vendor/GPL/LGPL và relink theo tài liệu kèm theo. Link OTA có sẵn; publisher vẫn đăng ZIP cài rồi thay nội dung cùng update.json trên Drive, giữ ID. Không dùng ZIP source làm gói cài app hay payload OTA.
