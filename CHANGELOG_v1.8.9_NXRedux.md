# NXT Retro Music v1.8.9 — NX Redux

- Thêm Groq, giữ MyMemory và Google Cloud. Đổi nguồn trong Dịch lời bài hát; không tự chuyển sang dịch vụ khác khi lỗi.
- Khóa Groq và Google lưu riêng trên thiết bị. Không kèm khóa dùng chung trong bản phát hành.
- Groq dịch tối đa 24 dòng / 2.400 byte lời trong một yêu cầu, kiểm tra ID để gắn đúng bản dịch với lời gốc. Câu lặp dùng lại bản dịch, không dịch lại.
- Lỗi Groq 429 chờ theo Retry-After/thông tin phục hồi nếu có, dự phòng 15 phút khi dịch vụ không trả thời gian. Hạn mức không được app nâng lên; không cam kết số bài mỗi giờ.
- Cache tăng lên tối đa 512 tệp bài / 16 MiB cho tất cả nguồn. Bản dịch cũ được giữ; nguồn/ngôn ngữ/model khác có cache riêng. Khi đầy, bản lưu cũ có thể bị loại bỏ.
- Stock, NextUI và NX Redux có nguồn OTA chính thức riêng. Nguồn đã lưu từ các bản cũ tự chuyển sang nguồn chính thức khi mở v1.8.9. Sau đó có thể nhập nguồn tùy chọn hoặc Khôi phục nguồn nhà phát hành.
- Giữ song ngữ trên Now Playing đủ 50 theme, chữ nghiêng chỉ ở hàng dịch trong màn hình Y, thời gian/độ trễ lời gốc, hồ sơ và chức năng xóa hồ sơ.
- Giữ pipeline phát nhạc, Bit-Perfect/Tự động, DAC/loa/3.5/Bluetooth, Spotify/SoundCloud/Drive, EQ, LED, phổ nhạc, Wi-Fi, hẹn giờ thoát app và độ sắc nét hiện có.
- v1.8.9 có bộ cài và OTA riêng cho Stock, NextUI và NX Redux.


- NX Redux dùng đường dẫn Tools/NXT Retro Music.pak, bộ kết nối cài đặt firmware và launcher riêng; hẹn giờ vẫn thoát toàn bộ app.
- Nguồn OTA NX Redux chính thức: https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing
