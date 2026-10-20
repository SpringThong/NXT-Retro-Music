# NXT Retro Music v1.9.7.5

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
