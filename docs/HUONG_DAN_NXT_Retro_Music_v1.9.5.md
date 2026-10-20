# NXT Retro Music v1.9.5

Dành cho TrimUI Brick Pro. Giữ tính năng của v1.9.4 và bổ sung các thay đổi đã duyệt trong demo Black Glass.

- Auralis: màu đen trung tính ở toàn bộ menu, cài đặt và cửa sổ con; nút trình phát nhỏ, hiệu ứng kính, chỉ hiện icon. Vẫn đủ danh sách, lời, EQ, tùy chọn, ngẫu nhiên, lặp, hẹn giờ, yêu thích, phổ, âm lượng và điều khiển phát.
- Cá nhân → Giao diện cá nhân → Tùy chỉnh Auralis: hai độ sáng bìa Đang phát / Full lời độc lập. Chỉ hiện mục này khi chọn Auralis; lưu riêng theo người nghe; không thay độ sáng màn hình.
- 22 font: giữ đúng 17 vị trí cũ, thêm Huawei HarmonyOS Sans, HarmonyOS Sans Condensed, OPPO Sans 4.0, Roboto Condensed và Roboto Slab. Xiaomi MiSans được giữ. Có mục Bộ font chữ dùng cho mọi theme, xem trước và A lưu / B hủy. Font dự phòng đa ngôn ngữ và bản dịch in nghiêng được giữ.
- Karaoke từ dữ liệu Enhanced LRC có mốc từng tiếng. Câu đang hát và câu tiếp theo phân biệt rõ; lời gốc và bản dịch đi cùng nhau. Chỉ có mốc câu thì hiển thị theo câu, không tự suy đoán từng tiếng. Bản dịch luôn in nghiêng.
- Chuyển menu toàn app dùng hai ảnh giao diện lưu trong RAM, hiệu ứng ngắn có mục tiêu 60 Hz. Nhịp 60 Hz cũng dành cho chuyển lời và mốc karaoke đang chạy; chữ, hình bìa, icon được cache. Khi đứng yên giảm vẽ, tắt màn / ẩn app ngừng phần hình. FFT/phổ vẫn có giới hạn riêng, CPU vẫn có ba chế độ. Không tăng xung CPU bắt buộc để tuyên bố đạt 60 FPS.
- Full lời không vẽ phổ, không đổi lựa chọn phổ đã lưu. Khi trở về Now Playing, dùng lại lựa chọn đó.
- Âm lượng giữ điều khiển thực từng 1 mức 0..100 và lưu riêng theo đầu ra, hiển thị số nấc không kèm %. Độ chính xác thực của DAC vẫn phụ thuộc mixer và driver.
- Bìa và chữ dài được thu/phóng có làm mịn và cache; nguồn bìa online mới yêu cầu ảnh 900×900 thay ảnh 300×300. Ảnh nhúng/ảnh đã có giữ chất lượng nguồn. Không thể tạo chi tiết HD thật từ ảnh nhỏ; màn hình máy hiển thị theo độ phân giải thực của nó.

## Cài đặt

Chọn đúng ZIP theo ROM, thoát hẳn NXT rồi giải nén đè lên thẻ. Giữ nguyên dữ liệu người nghe, đăng nhập, thư viện và cache; không cần xóa app cũ. Không dùng một ZIP cho ba ROM.

## Lời karaoke

Đặt tệp UTF-8 `.lrc` cùng tên, cùng thư mục với bài nhạc. Ví dụ `Music/BaiHat.flac` và `Music/BaiHat.lrc`:

```text
[00:10.000]<00:10.000>Đường <00:10.650>về <00:11.100>nhà<00:12.000>
[00:12.500]<00:12.500>Tiếng <00:13.100>nhạc <00:13.700>dịu <00:14.200>dàng<00:15.000>
```

Các mốc là ví dụ, cần căn đúng bản thu. Thẻ `<mm:ss.xxx>` có sẵn xác định đoạn đang hát; thẻ kết thúc cuối câu xác định lúc kết thúc tiếng cuối. Không có các mốc này thì app vẫn dùng LRC theo câu. App không phân tích giọng hát hay tự chấm điểm; không tạo lời/mốc từng tiếng từ âm thanh.

Y mở Full lời; ↑↓ cuộn, Y bám lời, X đổi cỡ, L1/R1 chỉnh lệch, START mở dịch, A dừng/phát, B về. Theo ánh xạ phím đã tùy chỉnh nếu khác mặc định.

## Giữ chức năng

DSF/DFF DSD Native/DoP nguyên bản, Bit-Perfect / Thường / Tự động, DAC/loa/3,5 mm/Bluetooth và thông tin đầu ra; EQ/DSP; Spotify, SoundCloud, Google Drive; Groq/MyMemory/Google và nhập Groq TXT; cache dịch 256 MiB ngoại tuyến; chỉ mục nhạc tăng dần; chép tệp Wi-Fi; 51 theme, avatar, hồ sơ, thống kê; LED đổi ngay; hẹn giờ lưu vị trí rồi thoát hẳn; tích hợp riêng của mỗi ROM. Native DSD vẫn cần cả DAC và driver ROM tương thích, không chuyển ngầm thành PCM.

## OTA

Ba nguồn chính thức được giữ riêng theo ROM. Người nghe chỉ vào Thông tin & cập nhật → Cập nhật phần mềm → Kiểm tra cập nhật. Bộ OTA đi kèm dành cho người phát hành: tải ZIP cài nguyên bản lên Drive, mở CHUAN_BI_OTA.html để tạo update.json với đúng link ZIP, rồi thay nội dung cùng tệp update.json đang dùng bằng Quản lý phiên bản, giữ ID. Không đăng tệp mẫu còn placeholder và không đăng ZIP source/OTA-kit làm payload.

## Kiểm tra và giới hạn

Xem RELEASE_QA_v1.9.5.md và BUILD_REPORT của từng ROM cho kết quả thực tế. Kiểm tra phần mềm/renderer không thay thế nghe thử trên máy. 60 Hz là mục tiêu điều phối khi chuyển động, không phải cam kết đã đo được 60 FPS trên mọi ROM. Nhiệt, pin, Bluetooth/DAC thật và dịch vụ tài khoản cần bạn test trên thiết bị.

Ý tưởng & thực hiện: Nguyễn Xuân Thông · Spring Thông · Vibe Code.
Font MiSans thuộc Xiaomi; HarmonyOS Sans thuộc Huawei; OPPO Sans thuộc OPPO; các font Google giữ giấy phép đi kèm. Tệp font mới nguyên bản, không đổi glyph; bản quyền/giấy phép đầy đủ ở `licenses/`.
