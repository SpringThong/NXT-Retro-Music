# NXT Retro Music v1.9.7.5

## v1.9.7.5 — Nghe nhạc nguyên bản / Music listening

- Gỡ tính năng giảm giọng ca sĩ: không còn lọc stereo-centre, dòng Karaoke/giảm giọng, ký hiệu K hoặc phím tắt giảm giọng.
- L3 mặc định không gán. Mọi nút từng gán giảm giọng ở v1.9.7.4 được trả về Không gán; các chức năng nút khác giữ nguyên.
- Giữ lời song ngữ, dịch chữ nghiêng, hiệu ứng lời/tô từng chữ, 51 themes, 60 fonts, preview, điều khiển khi tắt màn hình và bản sửa chuyển trang.
- Giữ Normal/Bit-Perfect/Auto, DSD Native/DoP, Spotify/SoundCloud/Drive, EQ và các tính năng nghe nhạc hiện có.
- Giữ nguồn OTA chính thức riêng cho Stock, NextUI và NX Redux. Chưa đăng OTA lên Drive và chưa kiểm thử trên máy thật.

Removed the vocal-reduction experiment and its shortcuts/settings/status. Old action 22 is cleared instead of being remapped to Stop. All other listening, bilingual lyrics, visual effects, screen-off controls, source/output modes and official ROM OTA feeds remain.


Dành cho TrimUI Brick Pro: **Stock, NextUI và NX Redux**. Nâng từ v1.9.7, giữ tính năng, dữ liệu và các nguồn OTA riêng. Chọn đúng ZIP cho ROM đang dùng.

## Điểm mới

- **60 lựa chọn font:** thêm 38 họ chữ có giấy phép phân phối và đầy đủ dấu tiếng Việt. Giữ nguyên chỉ số 22 font cũ và dự phòng chữ Trung, Nhật, Hàn, Thái. Có Be Vietnam Pro, Geist, Public Sans, Plus Jakarta Sans, Lexend, Quicksand, Playfair Display, Newsreader…
- **Xem trước trực tiếp:** chỉnh màu lời đang hát/chưa hát/bản dịch, đậm, viền, bóng, cỡ chữ và 10 hiệu ứng trên bìa của bài đang nghe. Với Auralis có thêm màu/cỡ tên bài, màu ca sĩ và độ sáng bìa từng màn.
- **Bản nháp có Áp dụng/Hủy:** nhạc tiếp tục theo thiết lập đã lưu trong lúc chỉnh. Không ghi cấu hình khi di chuyển thanh chỉnh. Lời mẫu song ngữ có nhãn riêng; không thay lời thật, không gửi mạng hay ghi cache dịch.

## Cách chỉnh và xem trước

Bộ font vẫn ở **Cá nhân → Giao diện cá nhân → Tùy chọn giao diện gốc → Bộ font**; không có mục trùng ở ngoài.

Màu/kiểu chữ: **Tùy chọn giao diện gốc → Lời & thông tin bài hát**, hoặc **Tùy chỉnh Auralis → Lời & thông tin bài hát** khi dùng Auralis. Hiệu ứng chuyển lời mở cùng trình xem trước.

- **↑↓:** chọn thuộc tính; **←→:** đổi giá trị/màu có sẵn.
- **Y** tại thuộc tính màu: mở chỉnh RGB ngay trên màn hình. **↑↓** chọn R/G/B, **←→** ±1, **L1/R1** ±16, **Y** trở lại danh mục.
- **X:** đổi xem trước **Now Playing / Full lời**. Bản dịch mẫu luôn nghiêng.
- **SELECT:** chạy lại hiệu ứng mẫu; không mở EQ trong trình chỉnh.
- **A – Áp dụng:** lưu thay đổi; nếu lưu lỗi, giữ bản nháp để thử lại. **B – Hủy:** giữ nguyên thiết lập cũ.
- Khi chọn font: **↑↓/←→** chuyển font, **L1/R1** nhảy 10 lựa chọn. X/A/B tương tự trên.

Lời xem trước được ghi rõ **LỜI MẪU · XEM TRƯỚC** để kiểm tra hình thức, không phải kết quả nhận diện giọng hát. Bản này giữ cơ chế karaoke v1.9.7; không sửa độ khớp tiếng hát hay cam kết đồng bộ cho mọi bài. Font SF Pro/San Francisco của Apple không được đóng gói do điều kiện giấy phép; các font giữ đúng tên thật. Xem danh sách và nguồn tại `docs/FONTS_v1.9.7.5.md`.

## Nghe nhạc và thao tác

Khi mở app, chọn hồ sơ rồi vào Now Playing của lần trước. Mặc định chưa tự phát; có thể bật tự phát trong cài đặt. B về menu, A phát/dừng; L1/R1 chuyển bài trong Now Playing, X danh sách, Y Full lời, SELECT EQ, START tùy chọn (theo gán phím hiện tại). Nút ngẫu nhiên/lặp lại/hẹn giờ giữ trạng thái cũ. Hết giờ hẹn sẽ lưu trạng thái và thoát app; firmware quyết định thời gian ngủ của máy.

Full lời: ↑↓ cuộn, Y đổi bám lời/cuộn tay, X cỡ chữ, L1/R1 chỉnh lệch lời, START dịch/karaoke, A phát/dừng, B về. Phổ nhạc không vẽ chồng lên Full lời. Chế độ song ngữ hiển thị lời gốc và bản dịch nghiêng cùng nhau.

## Karaoke và dịch offline

App vẫn tìm lời gốc theo .lrc cạnh nhạc, lời nhúng/cache và nguồn tìm lời sẵn có. Enhanced LRC có `<mm:ss.xxx>` giữ mốc chữ thật. Với LRC chỉ có mốc câu hoặc lời thuần, chế độ ước lượng tô theo khoảng câu và độ dài chữ, **không phân tích giọng hát**. Tắt tại Lời & thông tin bài hát → Karaoke ước lượng offline hoặc Karaoke theo giọng hát → Ước lượng mốc chữ còn thiếu offline. Khi tắt, chỉ tô chữ nếu có mốc thật/AI đã lưu.

Muốn phân tích tiếng hát: Full lời → START → Karaoke theo giọng hát · Groq → Tạo mốc từng chữ. App hỏi đồng ý trước khi gửi âm thanh đến Groq Whisper Large v3. Tự phân tích bài trên thẻ mặc định tắt. Chỉ tệp local decoder đọc được, tối đa 10 phút; không ghi âm Spotify/SoundCloud/luồng Drive hoặc chuyển DSD raw để phân tích. Nhận diện chạy nền ưu tiên thấp, có hủy khi đổi bài, không đổi pipeline phát nhạc. Mốc đã lưu dùng offline, cache karaoke riêng 16 MiB/256 tệp. Một từ ASR chứa nhiều chữ hoặc đoạn nội suy đều đánh dấu ước lượng. Hát bè/rap/bản cover có thể nhận sai; cần kiểm tra độ khớp trên bài thực.

Dịch chữ giữ **Groq, MyMemory và Google Cloud**, riêng khóa từng dịch vụ. Khóa Groq có thể nhập bằng TXT theo hướng dẫn sẵn trong app. Bản dịch đã lưu dùng lại offline trong cache tối đa **256 MiB**. Không tự gửi âm thanh để dịch chữ. Hạn mức/chi phí do tài khoản dịch vụ; app không nâng gói. Trạng thái 429 cần chờ hạn mức phục hồi hoặc đổi nguồn.

## Các tính năng được giữ

- 51 theme, 60 lựa chọn font, bìa album toàn màn Auralis, song ngữ, ảnh nền, avatar/hồ sơ và tùy chỉnh riêng.
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

Gói cài hiện tại: **NextUI**, `NXT_Retro_Music_v1.9.7.5_NextUI.zip`.


# NXT Retro Music v1.9.7.5

- Sửa giao diện gốc Đang phát bị dịch ngang/nhảy mỗi lần vẽ lại: hiệu ứng vào trang chỉ chạy khi chuyển trang.
- Thêm Cài đặt → Phát nhạc & âm thanh → Cách phát → Điều khiển khi tắt màn hình. Bật để dùng các nút mà màn hình vẫn tắt. Bấm Nguồn để bật sáng; giữ SELECT vẫn khóa/mở khóa nút. Mặc định tắt, giữ cách bấm nút bất kỳ để đánh thức như trước.
- Khi tắt màn hình, không vẽ lại Now Playing; nhạc, hàng đợi và các nút vẫn xử lý.
- Giữ toàn bộ tính năng, 51 giao diện, 60 font, dịch/cache, audio/DAC/DSD, tài khoản và ba nguồn OTA chính thức của v1.9.7.1.

Fixes repeated page-entry animation in Original Now Playing. Optional Screen-off controls keeps button mappings and volume active without waking the backlight. Power wakes; hold SELECT to lock/unlock. Default off retains any-key wake.

No physical-device FPS/thermal/DAC measurements or live OTA publication have been performed for this patch.


# NXT Retro Music v1.9.7.5

- Bỏ hiệu ứng trượt toàn trang trong menu, lời toàn màn hình và khi trở về Đang phát. Trang mới hiện trực tiếp; không chụp/vẽ lại toàn màn hình để làm hiệu ứng.
- Ghép đầy đủ các lớp từ backbuffer sạch cho cả vẽ trang và cập nhật lớp chuyển động; không dùng lại nội dung khung cửa sổ sau lần Present trước.
- Giữ hiệu ứng chuyển lời, karaoke, đĩa than và phổ nhạc.
- Giữ Điều khiển khi tắt màn hình (Cài đặt → Phát nhạc & âm thanh → Cách phát), 51 giao diện, 60 font và các tính năng v1.9.7.2.
- Giữ ba nguồn OTA chính thức; không cần nhập lại nguồn.

Atomic page navigation replaces the whole-screen slide. Both full redraw and layer-only presentation compose from a clean window backbuffer in the same layer order. Lyric/vinyl/spectrum motion, dark-screen controls and all audio/online engines are retained.

Software checks only; no physical-device scan-out/FPS/temperature measurement and no live OTA feed publication.
