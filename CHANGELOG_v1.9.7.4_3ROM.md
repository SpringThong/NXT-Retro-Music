# NXT Retro Music v1.9.7.4

## Karaoke / giảm giọng trực tiếp

- Bật/tắt ngay bằng L3 nếu phím này trước đó chưa được gán. Giữ phím L3 đã gán chức năng khác; có thể đổi trong Cài đặt phím. START → dòng đầu Karaoke cũng bật/tắt ngay và trở về trình phát. Hoạt động cả khi xem lời toàn màn hình và khi bật Điều khiển khi tắt màn hình; khóa nút vẫn được tôn trọng.
- K ở thanh trạng thái báo đang bật. Tùy chỉnh tại Cài đặt → Phát nhạc & âm thanh → Cách phát → Karaoke / giảm giọng.
- Mạnh: loại phần âm ở giữa hai kênh stereo, xuất nhạc nền mono để không bị triệt tiêu khi phát loa máy. Có thể mất cả nhạc cụ ở giữa, còn tiếng bè/vang hoặc giọng trộn rộng. Giữ âm trầm: giữ phần trầm ở giữa, nhưng có thể còn nhiều giọng hơn. Không bảo đảm xóa sạch giọng ca sĩ. Đây không phải tách AI.
- Nguồn mono tự giữ bản gốc. Một đoạn chỉ có giọng ở giữa trong bài đã có âm stereo vẫn tiếp tục được lọc.
- Không cần PC, mạng, API hay tải mô hình; không ghi đè tệp nhạc. Phiên app mới mặc định tắt hiệu ứng.
- Khi đang ở đường PCM Thường, bật/tắt không tải lại bài hay gọi tua. Khi phải chuyển Bit-Perfect ↔ Thường, dùng cơ chế đổi đường âm thanh giữ vị trí và trạng thái phát/tạm dừng; DAC có thể ngắt nhẹ khi mở lại đầu ra. Chế độ âm thanh đã chọn được giữ và khôi phục khi tắt.
- DSD Native/DoP giữ nguyên: không chuyển DSD sang PCM để lọc. Không cho bật hiệu ứng trên DSD đang chọn; nếu đã bật rồi chuyển sang DSD thì bỏ qua xử lý.
- Giữ sửa lỗi chuyển trang, điều khiển màn hình tắt và các tính năng v1.9.7.3. Ba nguồn OTA chính thức không đổi.

## English

L3 toggles live Karaoke/vocal reduction when previously unassigned; existing custom mapping is retained. START → first row also toggles without leaving the current song. Settings → Playback & audio → Playback → Karaoke / vocal reduction provides Strong / Preserve bass. A K status indicates enabled. New app sessions start with the effect off.

This is lightweight stereo-centre cancellation, not AI stems. Strong outputs dual mono and removes centre instruments too; reverberation, backing/wide vocals may remain. Mono sources retain the original. No PC, API, network, model or file modification.

PCM uses Normal while enabled and restores the saved sound policy when disabled. Existing Normal/online playback is not reopened; raw-PCM transport changes retain track position/state but may briefly interrupt the DAC. Original DSD is bypassed and never converted. All v1.9.7.3 features and official OTA feeds are retained.

Software/synthetic-signal checks only; no real-song listening, physical TrimUI, FPS/thermal/battery or live OTA publication test.

ROM: 3ROM
