# NXT Retro Music v1.10 — NextUI

- Spotify QR: use the approved playback session to request catalog scopes; PC JSON is optional. Use a friendly profile name instead of exposing an opaque account ID. Provider account, Premium, scope and regional restrictions still apply.
- Spotify search: trim empty queries, keep search pages within the 1,000-item API window, and retain the 10-result search limit.
- Online audio: bounded worker-owned PCM queues for Spotify/SoundCloud, startup/refill buffering, audible-clock progress, drain the Spotify tail before advancing, and short click-suppression ramps. Google Drive decoding primes/refills its PCM queue. No network reads in the online audio callback.
- B in Now Playing or the main menu: while playing, choose background playback or stop and close. When paused/stopped, close the process. X in the prompt returns to the main menu; nested settings/library Back behavior remains available.
- Keep the official Stock, NextUI and NX Redux OTA feed URLs unchanged. Existing themes, fonts, outputs, DAC/DSD, EQ, lyrics/translation cache and profiles are retained.

## Tiếng Việt

- Đăng nhập Spotify bằng QR dùng phiên đã được xác thực để lấy quyền thư viện; không phụ thuộc file JSON PC có sẵn. Hiển thị tên thân thiện; giữ các giới hạn thực tế của dịch vụ và yêu cầu Premium.
- Tìm Spotify: bỏ khoảng trắng, chặn truy vấn rỗng, giới hạn trang tìm kiếm hợp lệ và giữ mức 10 kết quả mỗi lượt.
- Spotify/SoundCloud: bộ đệm PCM có giới hạn, luồng nhận riêng, nạp trước/nạp lại khi mạng gián đoạn, giữ phần cuối bài trước khi chuyển, tiến độ theo âm thanh đã phát. Drive có nạp trước và nạp lại PCM. Thêm ramp ngắn để giảm tiếng tách khi thiếu dữ liệu.
- B ở Now Playing/menu chính: đang phát thì hỏi nghe nền hay thoát hẳn; đang pause/dừng thì thoát hẳn. X trong hộp thoại về menu chính. B trong các trang con vẫn quay lại để thao tác cài đặt và thư viện.
- Giữ nguyên ba nguồn OTA chính thức và các chức năng hiện có.

Live Spotify account authorization, real network conditions, physical controls and ROM audio outputs require device testing. Buffering cannot compensate indefinitely for a connection slower than the audio stream. No measured temperature, battery or FPS claim is made.
