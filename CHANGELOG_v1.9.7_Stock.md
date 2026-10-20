# NXT Retro Music v1.9.7

Dành cho TrimUI Brick Pro: **Stock, NextUI và NX Redux**. Giữ tính năng và dữ liệu v1.9.6; chọn đúng gói cho ROM đang dùng.

## Các thay đổi chính

- **Auralis cân đối hơn:** ba nút trước – phát/dừng – sau nằm giữa màn hình; bốn icon phụ chia đều hai bên, cùng tâm dọc. Icon được căn theo kích thước điểm ảnh thực ở cả 1024×768 và 640×480.
- **Dễ đọc trên bìa album:** tên bài lớn hơn (mặc định 50 đơn vị thiết kế, chỉnh 38–60), tự chia tối đa hai dòng khi dài; màu tên bài và ca sĩ riêng. Lời đang hát mặc định vàng, lời chưa hát trắng dịu, dịch xanh nhạt và luôn nghiêng. Có màu tùy chọn RGB/HEX, in đậm, viền 0–3 và bóng chữ, lưu theo từng hồ sơ.
- **10 lựa chọn chuyển lời:** Tắt, Trượt lên, Mờ nhẹ, Trượt + mờ, Lướt từ phải, Nổi nhẹ, Lướt từ trái, Hạ nhẹ, Lướt chéo, Nhún nhẹ. Lời gốc và bản dịch chuyển cùng nhau.
- **Karaoke bớt bỏ câu:** ghép theo từ và bỏ qua khác dấu thay vì đòi cả câu giống nguyên văn. Nhận diện thiếu một từ không còn bắt buộc loại toàn bộ câu. Bỏ qua token ASR lỗi/không có thời lượng; giữ các mốc hữu ích. Thử lại thủ công sẽ nhận diện mới thay vì chỉ đọc lại cache cũ.
- **Karaoke ước lượng offline:** mặc định bật để câu thiếu mốc chữ vẫn tô tiến dần. Có nhãn **ƯỚC LƯỢNG**, có thể tắt. Ưu tiên mốc Enhanced LRC thật; AI nhận diện không phải lúc nào cũng chính xác. Không cam kết khớp giọng mọi bài.
- **L1/R1 theo ngẫu nhiên:** bật ngẫu nhiên thì cả bài trước và bài sau đều chọn ngẫu nhiên trong hàng đợi hiện tại; tắt thì theo thứ tự như trước. Áp dụng đường chuyển bài trên thẻ/Drive, Spotify và SoundCloud.

## Chỉnh giao diện và lời

Cá nhân → Giao diện cá nhân → **Tùy chỉnh Auralis → Lời & thông tin bài hát**. Cũng mở được cùng bảng tại **Tùy chọn giao diện gốc → Lời & thông tin bài hát**.

Chọn màu rồi A để mở RGB/HEX; trái/phải đổi nhanh màu có sẵn. Các mục còn lại chỉnh trái/phải hoặc A. Có Đặt lại màu và kiểu chữ. Màu tên bài/ca sĩ và cỡ tên bài áp dụng trình phát Auralis; màu/đậm/viền/bóng lời áp dụng các giao diện lời native. Bộ font vẫn chỉ nằm tại Tùy chọn giao diện gốc → Bộ font; không thêm lại mục trùng ở Giao diện cá nhân.

Chọn hiệu ứng tại Tùy chỉnh Auralis hoặc Tùy chọn giao diện gốc → Hiệu ứng chuyển lời. Độ sáng bìa Now Playing và Full lời vẫn chỉnh riêng, chỉ hiện với Auralis. Menu Auralis giữ nền xám than và ô chọn theo bản trước.

## Nghe nhạc và thao tác

Khi mở app, chọn hồ sơ rồi vào Now Playing của lần trước. Mặc định chưa tự phát; có thể bật tự phát trong cài đặt. B về menu, A phát/dừng; L1/R1 chuyển bài trong Now Playing, X danh sách, Y Full lời, SELECT EQ, START tùy chọn (theo gán phím hiện tại). Nút ngẫu nhiên/lặp lại/hẹn giờ giữ trạng thái cũ. Hết giờ hẹn sẽ lưu trạng thái và thoát app; firmware quyết định thời gian ngủ của máy.

Full lời: ↑↓ cuộn, Y đổi bám lời/cuộn tay, X cỡ chữ, L1/R1 chỉnh lệch lời, START dịch/karaoke, A phát/dừng, B về. Phổ nhạc không vẽ chồng lên Full lời. Chế độ song ngữ hiển thị lời gốc và bản dịch nghiêng cùng nhau.

## Karaoke và dịch offline

App vẫn tìm lời gốc theo .lrc cạnh nhạc, lời nhúng/cache và nguồn tìm lời sẵn có. Enhanced LRC có `<mm:ss.xxx>` giữ mốc chữ thật. Với LRC chỉ có mốc câu hoặc lời thuần, chế độ ước lượng tô theo khoảng câu và độ dài chữ, **không phân tích giọng hát**. Tắt tại Lời & thông tin bài hát → Karaoke ước lượng offline hoặc Karaoke theo giọng hát → Ước lượng mốc chữ còn thiếu offline. Khi tắt, chỉ tô chữ nếu có mốc thật/AI đã lưu.

Muốn phân tích tiếng hát: Full lời → START → Karaoke theo giọng hát · Groq → Tạo mốc từng chữ. App hỏi đồng ý trước khi gửi âm thanh đến Groq Whisper Large v3. Tự phân tích bài trên thẻ mặc định tắt. Chỉ tệp local decoder đọc được, tối đa 10 phút; không ghi âm Spotify/SoundCloud/luồng Drive hoặc chuyển DSD raw để phân tích. Nhận diện chạy nền ưu tiên thấp, có hủy khi đổi bài, không đổi pipeline phát nhạc. Mốc đã lưu dùng offline, cache karaoke riêng 16 MiB/256 tệp. Một từ ASR chứa nhiều chữ hoặc đoạn nội suy đều đánh dấu ước lượng. Hát bè/rap/bản cover có thể nhận sai; cần kiểm tra độ khớp trên bài thực.

Dịch chữ giữ **Groq, MyMemory và Google Cloud**, riêng khóa từng dịch vụ. Khóa Groq có thể nhập bằng TXT theo hướng dẫn sẵn trong app. Bản dịch đã lưu dùng lại offline trong cache tối đa **256 MiB**. Không tự gửi âm thanh để dịch chữ. Hạn mức/chi phí do tài khoản dịch vụ; app không nâng gói. Trạng thái 429 cần chờ hạn mức phục hồi hoặc đổi nguồn.

## Các tính năng được giữ

- 51 theme, 22 lựa chọn font, bìa album toàn màn Auralis, song ngữ, ảnh nền, avatar/hồ sơ và tùy chỉnh riêng.
- Thư viện tăng dần, đọc nhanh chỉ mục đã lưu, yêu thích/danh sách; chép nhạc qua Wi-Fi.
- Spotify đăng nhập/ghép thiết bị theo hướng dẫn trong app (giữ yêu cầu tài khoản nguồn), SoundCloud, Google Drive, radio và podcast.
- Âm thanh thường/Bit-Perfect/Tự động, EQ/DSP, loa/3.5 mm/DAC/Bluetooth; DSD DSF/DFF Native hoặc DoP theo khả năng DAC/kernel và chế độ chọn. Không chuyển DSD thành PCM trong đường raw.
- Âm lượng từng nấc, lưu theo đầu ra; hiệu năng, LED đổi ngay, phím gán, ngôn ngữ/bàn phím, hồ sơ/thông tin tác giả và tích hợp riêng từng ROM.

## Hiệu năng và giới hạn kiểm chứng

Ảnh bìa, chữ, viền, icon dùng cache; mask viền dùng chung ngân sách LRU chữ 6 MiB, không vẽ lại viền nhiều lần mỗi khung. Chỉ tạo ước lượng cho dòng đang dùng, không worker/network/decoder cho tính năng này. Hiệu ứng ngắn và tô chữ đặt mục tiêu nhịp 16 ms, giữ giới hạn riêng cho FFT, trang tĩnh và khi ẩn/tắt màn. Chưa đo FPS, nhiệt độ hay pin trên máy thật; không cam kết 60 FPS cố định hoặc mọi nhận diện đều khớp. Các kiểm tra phần mềm và SHA được ghi trong RELEASE_QA/BUILD_REPORT kèm source.

## Cài và OTA

Thoát hẳn app. Giải nén ZIP đúng ROM đè lên thẻ, giữ dữ liệu cũ. Stock: `Apps/NXT_Retro_Music`; NextUI: `Tools/tg5040/NXT Retro Music.pak`; NX Redux: `Tools/NXT Retro Music.pak`.

Cả ba bản giữ link OTA chính thức, người dùng chỉ vào Kiểm tra cập nhật. Gói **NXT_OTA_Google_Drive** dành cho người phát hành: tải ZIP app đúng ROM lên Drive, mở CHUAN_BI_OTA.html và nhập link ZIP để tạo update.json, rồi thay nội dung file update.json chính thức đang dùng để giữ nguyên ID. Không dùng ZIP source/OTA-kit làm payload. Bản build không tự thay file trên Drive.

Ý tưởng & thực hiện: **Nguyễn Xuân Thông · Spring Thông · Vibe Code**.
