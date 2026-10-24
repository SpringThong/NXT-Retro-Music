# NXT Retro Music v1.10.1

## v1.10.1 — Phím B và thư viện Spotify QR / Back navigation and QR library

- **B ở Now Playing/Đang phát luôn về menu chính**, kể cả màn hình bài nghe dở khi vừa mở app và lúc pause. Giữ nguyên trạng thái phát/tạm dừng; không hỏi thoát ở đây.
- **B tại menu chính**: đang phát thì chọn nghe nền hoặc dừng nhạc và thoát hẳn; đang pause/dừng/chưa phát thì thoát hẳn ngay. Trong hộp thoại: A xác nhận, X ở lại menu, B thoát hẳn.
- **Spotify QR**: dùng token Login5 của chính phiên vừa xác thực thay cho đường Mercury keymaster cũ. Không cần nhập JSON PC trước để app thử mở thư viện. Cách nhập JSON PC vẫn được giữ làm dự phòng. Lỗi không lấy được token và lỗi API từ chối quyền được phân biệt rõ; không đổi tài khoản/client ID sau lỗi.
- Giữ bộ đệm online v1.10 và toàn bộ theme/font/tài nguyên, DSD/DoP, EQ, dịch/cache lời, hồ sơ, CPU/LED, đầu ra âm thanh và ba link OTA chính thức.

English: Back from every shared Now Playing/startup card returns to the main menu without stopping or pausing audio. Only Back at the root menu exits: playing prompts for background or close; paused/stopped closes the app. QR catalog requests reuse the same authenticated session's cached/refreshable Login5 token instead of Mercury keymaster. PC JSON login remains optional fallback. Actual Spotify service permissions, Premium and region requirements still apply.

Chưa kiểm tra tài khoản Spotify thật, máy thật, FPS, nhiệt độ hoặc pin. Cần thử QR trên máy sau khi cập nhật; không thể khẳng định mọi tài khoản được Spotify cấp mọi quyền chỉ dựa vào kiểm tra phần mềm. Chưa đăng gói hoặc thay nội dung các feed OTA đang hoạt động.

## Cài đặt và OTA

Chọn ZIP đúng Stock, NextUI hoặc NX Redux, giải nén vào gốc thẻ nhớ. Giữ dữ liệu người dùng; không chép lẫn các ROM. Gói OTA tương ứng có CHUAN_BI_OTA.html để tạo update.json. Đăng bản mới lên đúng ba file JSON cũ để giữ nguyên link.

Hướng dẫn đầy đủ: docs/HUONG_DAN_NXT_Retro_Music_v1.10.1.md. Các tài liệu phiên bản trước vẫn được giữ làm lịch sử.
