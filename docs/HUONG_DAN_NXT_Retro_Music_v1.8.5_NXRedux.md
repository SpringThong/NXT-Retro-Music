# NXT Retro Music v1.8.5 — thay đổi và cách dùng

- Giữ font, tài nguyên và độ phân giải v1.8.4. Cache ảnh album không có alpha; ảnh alpha vẫn dùng đường vẽ nguyên bản để giữ đúng pixel. Giảm nhịp phổ nhạc ở Cân bằng/Tiết kiệm pin; không đổi độ nét chữ, tần số âm thanh hay độ trễ phím. Lưu JSON/fsync và nén ảnh phiên nghe qua worker nền có giới hạn, chụp đúng hồ sơ và ghi hết khi thoát.
- NextUI: ba mức CPUFreq lấy từ bảng xung được driver hỗ trợ. Mục tiêu trần: Tiết kiệm pin 1008 MHz; Cân bằng 1608 MHz; Hiệu năng 2000 MHz, không vượt giới hạn ROM/phần cứng. Ưu tiên governor schedutil/ondemand; nếu chỉ hỗ trợ userspace thì đặt xung mục tiêu thật. Hạ cả mức tối thiểu nếu ROM khóa 2GHz. Theo dõi CPU có dòng trạng thái áp dụng hoặc lỗi. Xung tức thời có thể thấp hơn trần; app trả lại chính sách cũ khi ẩn/thoát, không đổi điện áp hay bảo vệ nhiệt. Stock/NX Redux tiếp tục theo chính sách CPU của ROM; các mức vẫn giảm/tăng tải hiệu ứng.
- Cài đặt → Phát nhạc & âm thanh chỉ còn Cách phát và Âm thanh & đầu ra. Chọn thiết bị, chế độ âm thanh và âm lượng trong Âm thanh & đầu ra; chỉnh EQ bằng SELECT trong trình phát.
- Chế độ Thường: đầu ra chia sẻ, EQ/DSP và âm lượng phần mềm như trước. Bit-Perfect: PCM số nguyên lossless nguyên bản qua DAC USB, không EQ/limiter/ReplayGain/resample/crossfade/phần mềm tăng giảm âm lượng; chỉnh âm lượng trên DAC. Chỉ hiển thị sẵn sàng sau khi mở đúng PCM.
- Tự động: khi DAC USB được chọn/kết nối, thử PCM nguyên bản. Nếu nguồn/DAC không hỗ trợ hoặc DAC bận thì chuyển Thường, báo rõ và không thử lại liên tục trên cùng nguồn/DAC. MP3 và nguồn Spotify/SoundCloud/radio/podcast dùng Thường. Khi rút DAC hoặc chọn loa/3,5mm/Bluetooth, chuyển Thường, vẫn giữ lựa chọn Tự động. Tùy chọn Khi rút DAC → Tạm dừng/Chuyển loa vẫn được giữ; nếu tạm dừng, bấm Play để nghe tiếp ở đầu ra mới. Không gọi Bluetooth là Bit-Perfect.
- Mở app → chọn hồ sơ → Now Playing của hồ sơ đó, đúng bài và vị trí lưu gần nhất. Mặc định tạm dừng; A phát, B về menu. Tâm trạng/thể loại/quốc gia chỉ chọn trong Cá nhân, không hỏi khi khởi động. Giữ tùy chọn tự phát nếu người dùng đã bật.
- Giữ 36 giao diện, NXT gốc/ảnh nền, 14 ngôn ngữ, icon/emoji, LED đổi tức thì, Wi-Fi copy file, Spotify/SoundCloud, phím tắt và timer thoát trọn phiên app. Deep sleep do ROM quyết định sau khi app đóng.
- Ba gói cài và OTA riêng cho Stock/NextUI/NX Redux. Không dùng nhầm ROM. OTA publisher kit chứa ZIP cài, SHA-256 và CHUAN_BI_OTA.html; tải ZIP và update.json đã tạo lên Drive công khai rồi dùng nguồn đúng ROM. Chưa tự đăng nguồn công khai.

## Cài đặt và kiểm tra trên máy

Sao lưu userdata trước khi nâng cấp. Đóng app trước khi chép gói; giải nén giữ đúng cây Apps (Stock), Tools/tg5040 (NextUI), Tools (NX Redux). Không xóa userdata/các hồ sơ. Các gói dành cho TrimUI Brick Pro/tg5040; chưa xác nhận original Brick.

1. NextUI: theo dõi CPU, lần lượt chọn ba mức khi đang phát; kiểm tra cả trạng thái trần/governor và xung tức thời. Báo kèm ROM version và dòng lỗi nếu sysfs bị từ chối hoặc ROM ghi đè.
2. Chọn Tự động. Nghe FLAC/WAV/ALAC bằng DAC phù hợp, kiểm tra trạng thái PCM. Thử MP3/DAC bận, rút cắm USB, chuyển 3,5mm/loa/Bluetooth và Play/Pause. Giảm âm lượng DAC trước khi thử Bit-Perfect vì không có âm lượng phần mềm.
3. Mỗi hồ sơ mở lại đúng bài/vị trí; B quay menu. So độ trễ thao tác, nhiệt/pin sau cùng thời lượng nghe và cùng độ sáng với v1.8.4.
4. Thử timer đóng app và chính sách ngủ của ROM; OTA cần link công khai thật và kết nối Internet.

Không có thiết bị vật lý trong môi trường build: kiểm tra phần mềm/mô phỏng không thay cho đo nhiệt/pin hoặc nghe DAC/Bluetooth thật.

## Gói dành cho NXRedux

Giải nén ở gốc thẻ; app nằm tại `Tools/NXT Retro Music.pak`. Đóng app trước khi cập nhật. Không chép gói dành cho ROM khác. Các hướng dẫn tính năng đầy đủ v1.8.4 giữ trong bộ source là tài liệu nền; dùng thay đổi v1.8.5 ở trên cho vị trí menu và luồng mở app mới.
