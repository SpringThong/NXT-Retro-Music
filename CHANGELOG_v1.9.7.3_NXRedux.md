# NXT Retro Music v1.9.7.3

- Bỏ hiệu ứng trượt toàn trang trong menu, lời toàn màn hình và khi trở về Đang phát. Trang mới hiện trực tiếp; không chụp/vẽ lại toàn màn hình để làm hiệu ứng.
- Ghép đầy đủ các lớp từ backbuffer sạch cho cả vẽ trang và cập nhật lớp chuyển động; không dùng lại nội dung khung cửa sổ sau lần Present trước.
- Giữ hiệu ứng chuyển lời, karaoke, đĩa than và phổ nhạc.
- Giữ Điều khiển khi tắt màn hình (Cài đặt → Phát nhạc & âm thanh → Cách phát), 51 giao diện, 60 font và các tính năng v1.9.7.2.
- Giữ ba nguồn OTA chính thức; không cần nhập lại nguồn.

Atomic page navigation replaces the whole-screen slide. Both full redraw and layer-only presentation compose from a clean window backbuffer in the same layer order. Lyric/vinyl/spectrum motion, dark-screen controls and all audio/online engines are retained.

Software checks only; no physical-device scan-out/FPS/temperature measurement and no live OTA feed publication.

ROM: NXRedux
