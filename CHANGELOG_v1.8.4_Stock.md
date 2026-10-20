# NXT Retro Music v1.8.4 — Stock

- SELECT mở EQ; nhấn SELECT lần nữa để đóng, kể cả trong tùy chọn EQ con. B vẫn quay lại. Chỉ mở/thoát không áp dụng preset.
- Thêm chế độ Bit-Perfect cho DAC USB: PCM số nguyên lossless đi trực tiếp tới ALSA hw ở Hz, số kênh và độ chính xác tương thích với nguồn. Bỏ qua EQ/DSP, limiter, ReplayGain, âm lượng phần mềm, đổi tốc độ và crossfade; giữ các thiết lập chế độ Thường.
- DAC không hỗ trợ, đang bận hoặc bị rút: tạm dừng và báo trạng thái; không tự resample hay chuyển sang loa. Nguồn lossy/Spotify/SoundCloud/radio/podcast dùng chế độ Thường.
- Mở app vào thẳng Now Playing với hồ sơ gần nhất, bài và vị trí đã lưu. Mặc định tạm dừng; tùy chọn Tự phát ở Phát nhạc & âm thanh → Cách phát. Không hỏi tâm trạng/thể loại/quốc gia khi mở; các lựa chọn vẫn nằm trong Cá nhân.
- Lưu phiên nghe theo hồ sơ, gồm nguồn Local/Drive/Spotify/SoundCloud. Nếu file mất, giữ thông tin để chọn bài khác; không âm thầm phát bài đầu danh sách.
- Phát nhạc & âm thanh chia rõ Đầu ra, Chế độ âm thanh, EQ/DSP, Cách phát, DAC/âm lượng và Thông tin bài hát/đầu ra. START tập trung thao tác thường dùng, đặt Đầu ra/Chế độ âm thanh đầu danh sách.
- Giữ 36 giao diện, NXT gốc, album/đĩa than, lời, ngẫu nhiên/lặp lại, số bài, hồ sơ, các nguồn nhạc, Bluetooth, LED đổi ngay, phổ nhạc, chuyển file Wi-Fi và 14 ngôn ngữ.
- Giữ hẹn giờ lưu tiến trình rồi thoát toàn bộ app cả khi phát nền; hệ thống ROM quyết định ngủ theo cài đặt của máy.
- Worker giải mã nghỉ bằng condition variable khi không có việc, đầy đệm hoặc đã hết bài; Bit-Perfect cũng nghỉ khi tạm dừng. Không thêm daemon canh hẹn giờ.
- Bộ cài và bộ OTA riêng cho Stock, NextUI, NX Redux, có kiểm tra ROM/máy/version/dung lượng/SHA-256 và giữ dữ liệu cá nhân.

Kiểm tra phần mềm gồm PCM nguyên bản, native ARM64, giao diện/điều khiển và hồi quy. DAC/tai nghe thật, OTA qua link công khai, pin/nhiệt và ngủ/thức cần thử trên máy; không tuyên bố đã đo những phần này.
