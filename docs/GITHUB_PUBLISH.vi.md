# Đưa NXT Retro Music lên GitHub

Bộ README này dành cho source **v1.9.7.7** và không thay đổi code hay bộ cài đã build.

## 1. Chép README vào đúng chỗ

Giải nén `NXT_Retro_Music_v1.9.7.7_Source.zip`. Mở thư mục source có `SOURCE.json`, `tools/`, `upstream/`, `stock/`, `nxredux/`.

Chép **nội dung** bộ README vào ngay thư mục đó:

| File/thư mục | Công dụng |
| --- | --- |
| `README.md` | Trang giới thiệu chính bằng tiếng Anh. Thay README gốc đang ghi phiên bản cũ. |
| `README.vi.md` | Bản tiếng Việt, có liên kết qua lại với bản tiếng Anh. |
| `docs/screenshots/` | 5 ảnh native dùng trong cả hai README. Cần tải thư mục này lên cùng README để ảnh hiện. |
| `docs/GITHUB_PUBLISH.vi.md` | Ghi chú này dành cho người quản lý dự án. |

Giữ hai README ở cùng cấp. Không chép thêm một lớp thư mục `github/` bao bên ngoài.

## 2. Tạo repository và đưa source vào

Có thể đặt tên repository là **`NXT-Retro-Music`** và mô tả ngắn:

> Native music player for TrimUI Brick Pro — 51 themes, bilingual lyrics, USB DAC/DSD, online sources and extensive personalization. A vibe-coded project by Spring Thông.

Đưa thư mục source đã giải nén lên bằng Git hoặc GitHub Desktop. Bộ source có nhiều file và một số file vượt giới hạn tải lên qua giao diện web, nên cách kéo thả từng file trên trình duyệt sẽ bất tiện.

README hiện không gắn tên tài khoản GitHub giả, URL repository giả hoặc nút tải dẫn tới nơi chưa tồn tại. Sau khi có repository, bạn có thể thêm link Releases thật và video YouTube demo của mình ở đầu README.

## 3. Đưa ZIP phát hành vào Releases

Tạo release/tag cho **`v1.9.7.7`**, đính kèm:

- 3 ZIP cài app: Stock, NextUI, NXRedux.
- 3 ZIP chuẩn bị OTA tương ứng nếu muốn người khác quản lý nguồn riêng.
- ZIP source đầy đủ nếu repository được sắp gọn và cần cung cấp lại snapshot build.

Không commit nguyên ZIP source gần 500 MB hay các ZIP cài vào lịch sử Git. Theo [GitHub Docs về file lớn](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github), tải file trên web có giới hạn 25 MiB/file và Git thông thường chặn file lớn hơn 100 MiB; GitHub Releases là nơi phù hợp để phân phối các gói lớn.

Nếu làm một repository gọn hơn, phải mô tả cách lấy lại các thư viện/tài nguyên/helper cần cho build. Không xóa `relink/lib`, `sources`, font, dữ liệu IME hoặc helper binary rồi vẫn coi đó là source build đầy đủ. SDK, cache build và thư mục đầu ra `dist/` có thể được tạo lại theo quy trình build.

## 4. Giữ giấy phép và dữ liệu đăng nhập tách biệt

Giữ thông báo của NXT, upstream, NextUI và các thư viện/font/icon. Các file NXT có SPDX `GPL-3.0-only`; trình phát upstream có MIT; thư mục `upstream/licenses/` chứa các giấy phép liên quan. Không đổi toàn bộ repository thành MIT chỉ vì file LICENSE của player gốc là MIT.

Chỉ đưa mã nguồn, cấu hình mẫu và công cụ vào repo. API key thật, `spotify-credentials.json`, `credentials.json`, `translation-service.json`, file `groq-api-key.txt` đang dùng và dữ liệu cá nhân trên thẻ cần được giữ riêng. Đọc lại log trước khi đăng trong issue.

## 5. OTA vẫn cần cập nhật riêng

Đăng release GitHub không tự cập nhật 3 nguồn OTA Google Drive đang nằm trong app.

Nếu tiếp tục dùng các nguồn đó: đăng ZIP cài đúng ROM, dùng `CHUAN_BI_OTA.html` của bộ OTA tạo `update.json`, rồi thay nội dung chính file JSON trên Drive đang được theo dõi. Giữ nguyên ID/link để máy người dùng tiếp tục tìm được bản mới.

## Ảnh minh họa và các thông tin đã đối chiếu

Ảnh trong `screenshots/` lấy từ render native của source v1.9.7.7 với bìa/lời mẫu, không phải ảnh chụp phần cứng. README ghi đúng điều này.

Nội dung đã đối chiếu với các bảng/menu/đường chạy hiện tại: 51 theme, 60 font, 14 ngôn ngữ, gán phím, EQ/PEQ, Groq TXT, luồng PC+QR Spotify, Drive, thư viện tăng dần, DSD, chế độ âm thanh, hiệu năng, hồ sơ và OTA. Giới hạn dịch vụ, độ khớp mốc chữ và nhiệt độ/FPS không được mô tả như bảo đảm cho mọi máy/bài hát.
