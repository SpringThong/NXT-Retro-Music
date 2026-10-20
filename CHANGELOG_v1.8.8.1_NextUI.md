# NXT Retro Music v1.8.8.1 — NextUI

- LyricsFix2: hỗ trợ plainLyrics, tìm lời cover theo tên gốc với thời gian ước lượng; báo rõ thiếu lời gốc/lỗi mạng, thử lại nguồn lời.
- LyricsFix2: snapshot môi trường riêng cho curl, đóng các FD của audio/session trong helper, bỏ I/O idle và ghi trạng thái HTTP/spawn/curl vào log khi lỗi.
- LyricsFix2: phục hồi endpoint còn sót khi tiến trình cũ đã chết và khóa app đã được nhả; không dừng phiên nhạc còn sống. Chưa xác nhận đã hết lỗi crash trên thiết bị.
- Build bổ sung: chỉ lời dịch trong màn hình Y dùng chữ nghiêng; lời gốc và Now Playing giữ kiểu chữ hiện tại. Font nghiêng riêng, tái sử dụng cache.
- Song ngữ ngay trên Now Playing: hai hàng cố định cùng một mốc LRC, lời gốc + bản dịch; đủ 50 theme, giữ màn hình Y.
- Dịch theo nhóm nhỏ, giảm số tiến trình/kết nối. Google Cloud dùng mảng q; MyMemory gom các câu ngắn và chỉ nhận nhóm có đủ ranh giới dòng.
- Tự quay lại phần chưa dịch khi lỗi mạng; lỗi một dòng không bỏ cả bài. Có backoff khi giới hạn dịch vụ, không gọi dồn dập.
- Worker ưu tiên thấp và I/O idle; tạo curl bằng posix_spawn, UI nhận kết quả bằng trylock khi có thay đổi; đổi Bản dịch/Song ngữ không dịch lại.
- Cache v1.8.8 tiếp tục dùng được; lưu ít lần hơn. Giữ âm thanh, Bit-Perfect/Automatic, Bluetooth/DAC, 50 theme, sắc nét chữ, hồ sơ, phím SELECT và timer thoát app.

Dịch cần Wi-Fi và phụ thuộc hạn mức/tốc độ dịch vụ. Không thể đảm bảo dịch toàn bài khi hết quota. Chưa đo nhiệt độ/pin hoặc nghe thử trên máy thật.
