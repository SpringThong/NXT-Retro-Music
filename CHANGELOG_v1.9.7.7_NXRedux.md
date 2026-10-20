# NXT Retro Music v1.9.7.7

Bản tinh chỉnh giao diện cho Stock, NextUI và NX Redux, phát triển từ v1.9.7.6.

- Dựng lại 316 icon từ vector ở 192 px, lọc thu nhỏ có khử răng cưa; giữ nguyên logo màu và emoji.
- Nút Auralis lớn hơn vừa phải, căn đối xứng theo tâm màn hình. Trạng thái bật có nền sáng và icon tối, dễ nhận ra trên glass.
- Làm mịn góc bo, hình tròn và đường chéo; lọc ảnh bìa, khung theme, avatar, nhãn đĩa khi đổi kích thước. Giữ tỉ lệ và căn giữa trong khung.
- Sắp lại vùng tên bài/ảnh bìa Ocean Blue, nới vùng tên bài Boombox và giữ thông tin trong vùng nội dung khi bật song ngữ.
- Icon được lưu riêng theo tên và kích thước; ảnh/khung chỉ chuẩn bị khi đổi nguồn, kích thước hoặc tùy chọn. Không tăng độ phân giải render toàn app, xung CPU hay vòng lặp nền để đổi lấy độ nét.
- Giữ 51 themes, 60 fonts, dịch/cache 256 MiB, lời song ngữ, tô từng chữ, DSD/DoP, DAC/Bluetooth, EQ, Spotify/SoundCloud/Drive, hồ sơ và điều khiển khi tắt màn hình.
- Có ZIP cài đặt và bộ chuẩn bị OTA riêng cho ba ROM; giữ nguồn cập nhật chính thức. Chưa đăng gói lên Drive.

Ảnh được hiển thị theo độ phân giải thực của máy và chi tiết ảnh nguồn; ảnh nhỏ không tự trở thành ảnh HD thật. Chưa đo FPS, nhiệt độ hoặc pin trên thiết bị vật lý.

## English

Refined vector icons, native antialiasing, slightly larger and symmetric Auralis controls with unmistakable filled active states. Cached area-filtered covers, theme textures, avatars and vinyl labels preserve original detail and aspect ratio. Ocean Blue and Boombox metadata have safer bounds. Bounded caches avoid repeated scaling; opaque panel interiors retain fast fills. No full-app supersampling, raised CPU clocks or new background loops. All v1.9.7.6 listening features and ROM-specific official OTA feeds remain. Device FPS, thermals and battery life require physical testing.


## Tài liệu trước / Previous release notes

# NXT Retro Music v1.9.7.6

## v1.9.7.6 — Sửa nguồn SoundCloud công khai

- Cập nhật bộ phát native để nhận CDN HTTPS playback.media-streaming.soundcloud.cloud của SoundCloud; giữ kiểm tra tên miền, cổng, chuyển hướng và chứng chỉ TLS.

- Sửa bộ lọc quyền chặn nhầm bài có policy MONETIZE: bài công khai vẫn có thể cung cấp luồng AAC đầy đủ.
- Tiếp tục kiểm tra bài riêng tư, access preview/blocked, policy SNIP/BLOCK, streamable false và SUB_HIGH_TIER; không thêm quyền Go+ hoặc vượt giới hạn khu vực.
- Không còn khẳng định Go+/khu vực cho mọi lỗi không lấy được luồng phát. Log hiển thị đúng bản app và ROM.
- Giữ mọi tính năng nghe nhạc/lời/giao diện và nguồn OTA chính thức của v1.9.7.5. Không khôi phục bộ lọc giảm giọng.

Fixed the guest SoundCloud permission filter: MONETIZE public tracks can be playable. Preserve explicit preview/private/block/subscription checks and only use the existing supported full-length, unencrypted AAC/MP3 streams. No new login, account access or rights bypass.


## v1.9.7.5 — Nghe nhạc nguyên bản / Music listening

- Gỡ tính năng giảm giọng ca sĩ: không còn lọc stereo-centre, dòng Karaoke/giảm giọng, ký hiệu K hoặc phím tắt giảm giọng.
- L3 mặc định không gán. Mọi nút từng gán giảm giọng ở v1.9.7.4 được trả về Không gán; các chức năng nút khác giữ nguyên.
- Giữ lời song ngữ, dịch chữ nghiêng, hiệu ứng lời/tô từng chữ, 51 themes, 60 fonts, preview, điều khiển khi tắt màn hình và bản sửa chuyển trang.
- Giữ Normal/Bit-Perfect/Auto, DSD Native/DoP, Spotify/SoundCloud/Drive, EQ và các tính năng nghe nhạc hiện có.
- Giữ nguồn OTA chính thức riêng cho Stock, NextUI và NX Redux. Chưa đăng OTA lên Drive và chưa kiểm thử trên máy thật.

Removed the vocal-reduction experiment and its shortcuts/settings/status. Old action 22 is cleared instead of being remapped to Stop. All other listening, bilingual lyrics, visual effects, screen-off controls, source/output modes and official ROM OTA feeds remain.


- Bỏ hiệu ứng trượt toàn trang trong menu, lời toàn màn hình và khi trở về Đang phát. Trang mới hiện trực tiếp; không chụp/vẽ lại toàn màn hình để làm hiệu ứng.
- Ghép đầy đủ các lớp từ backbuffer sạch cho cả vẽ trang và cập nhật lớp chuyển động; không dùng lại nội dung khung cửa sổ sau lần Present trước.
- Giữ hiệu ứng chuyển lời, karaoke, đĩa than và phổ nhạc.
- Giữ Điều khiển khi tắt màn hình (Cài đặt → Phát nhạc & âm thanh → Cách phát), 51 giao diện, 60 font và các tính năng v1.9.7.2.
- Giữ ba nguồn OTA chính thức; không cần nhập lại nguồn.

Atomic page navigation replaces the whole-screen slide. Both full redraw and layer-only presentation compose from a clean window backbuffer in the same layer order. Lyric/vinyl/spectrum motion, dark-screen controls and all audio/online engines are retained.

Software checks only; no physical-device scan-out/FPS/temperature measurement and no live OTA feed publication.

ROM: NXRedux
