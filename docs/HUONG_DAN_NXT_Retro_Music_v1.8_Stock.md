# NXT Retro Music v1.8 — Big Update

Nguyễn Xuân Thông · Spring Thông · Vibe Code

## Cài đặt và nâng cấp — Stock

Chọn đúng bản **Stock** hoặc **NextUI**. Dừng hẳn phiên phát nền 1.7.1 trước khi chép bản mới; có thể khởi động lại máy để kết thúc phiên cũ. Giải nén ZIP vào gốc thẻ nhớ, giữ đúng cấu trúc thư mục. Stock đặt app ở `Apps/NXT_Retro_Music`; NextUI đặt ở `Tools/tg5040/NXT Retro Music.pak`. Mở từ Apps / Tools của ROM tương ứng.

Dữ liệu hồ sơ, playlist, lịch sử, ảnh riêng và đăng nhập vẫn nằm trong thư mục dữ liệu của ROM. Bản cài không chứa tài khoản hay nhạc mẫu. Lần mở đầu trên dữ liệu mới mặc định tiếng Anh; dữ liệu cũ giữ ngôn ngữ đã chọn.

## Hồ sơ và nhạc hôm nay

Màn hình mở app hiển thị logo đã lưu của từng hồ sơ. Chọn người nghe, rồi chọn tâm trạng, thể loại và nước muốn nghe. Mục **Personal / Cá nhân → For you / Dành cho bạn** có nghe tiếp, bài quen, bài trong thẻ chưa nghe, bài lâu chưa nghe, cùng lối mở gợi ý bài và playlist Spotify/SoundCloud. Gợi ý online lấy từ danh mục dịch vụ khi bạn mở; vẫn cần Wi-Fi và quyền truy cập phù hợp. Quốc gia được đưa vào truy vấn gợi ý, không phải bộ lọc địa lý tuyệt đối.

Đổi lại lựa chọn ở **Settings → Profiles & controls → Today's listening choices**. Mỗi hồ sơ giữ riêng logo, giao diện và lựa chọn nghe. Thay logo ở **Your avatar**: 300 icon, 180 emoji 3D, chữ riêng hoặc ảnh JPG/PNG trên thẻ. Lựa chọn avatar pixel cũ chuyển sang icon; mục chọn pixel được bỏ.

## 36 giao diện và ảnh album

Vào **Settings → Appearance & LEDs → 36 full interfaces**. D-pad chọn, A áp dụng, B hủy. Giữ nguyên mã giao diện 0–23 và thêm 12 giao diện 24–35. Original NXT giữ ảnh vuông/đĩa than và hình nền riêng; các giao diện máy nghe còn lại dùng khung toàn màn hình với ảnh album cân đối.

Đang phát hiển thị số bài hiện tại/tổng hàng đợi, ngẫu nhiên, lặp Tắt/Một/Tất cả, thời gian hẹn tắt, âm lượng, lời bài hát và gợi ý nút. Thông tin codec, tần số, bit depth/bitrate xuất hiện theo dữ liệu nguồn có thật; thông tin PCM/DAC là đầu ra và được hiển thị riêng. Tên bài dài cuộn sau một khoảng dừng ngắn.

## Phím điều khiển mặc định

| Nút | Khi đang phát |
|---|---|
| A | Phát / tạm dừng |
| B | Về menu |
| X | Danh sách đang phát |
| Y | Lời toàn màn hình |
| SELECT | EQ / DSP |
| START | Tùy chọn nhanh |
| L1 / R1 | Bài trước / bài tiếp |
| L2 | Bật / tắt phổ nhạc |
| R2 | Yêu thích |
| Giữ trái / phải | Tua |
| SELECT + lên / xuống | Âm lượng |

Đổi nút trong **Profiles & controls → Button shortcuts**. Các gợi ý trên giao diện đọc theo cấu hình nút đang phát. Ngẫu nhiên, lặp và hẹn giờ mở qua nút Tùy chọn nhanh đã gán. Hẹn tắt có 15/30/45/60/90/120 phút, tùy chỉnh và hủy; thời gian vẫn chạy khi tạm dừng.

## Nhạc trong thẻ và playlist

Chọn thư mục nhạc trong **Library & files → Music folder**, rồi mở Library. Thẻ có thể chứa nhạc và file LRC cùng tên để hiện lời. Playlist hỗ trợ tạo, chỉnh và nhập/xuất M3U/M3U8. Hàng đợi X cho biết thứ tự bài; tùy chọn lặp áp dụng sau khi đến cuối hàng đợi thật.

## Nghe online

**Online music** giữ Spotify, SoundCloud và Google Drive. Với Spotify, dùng luồng đăng nhập QR / công cụ `Spotify_Login_PC` đi kèm; quyền phát và quyền thư viện được kiểm tra riêng. SoundCloud dùng cấu hình API/quyền truy cập của bạn; app lọc bản preview và bài không được phép phát. Drive phát từ nguồn/link được hỗ trợ. Các dịch vụ có thể giới hạn theo tài khoản, vùng hoặc API; không tạo bài/playlist giả khi dịch vụ không trả dữ liệu.

## Chép qua Wi-Fi

1. Máy và điện thoại/máy tính kết nối cùng mạng Wi-Fi.
2. Mở **Settings → Library & files → Wi-Fi file transfer**.
3. Mở địa chỉ hiện trên máy bằng trình duyệt và nhập PIN 6 số của phiên.
4. Chọn file, nhấn Upload files; xem tiến trình. File vào thư mục nhạc đã chọn.
5. X trên máy hủy file đang chép; B đóng chuyển file. Trình duyệt có nút Cancel.

Nhận nhạc, LRC, TXT, JPG/PNG và M3U/M3U8; giới hạn 2 GiB mỗi file. File trùng tên được thêm số, không ghi đè. File chép dở được dọn khi hủy. Máy chỉ mở máy chủ khi đang ở màn hình chuyển file.

## Chép qua USB-C

Mở **Library & files → USB-C file transfer → Start**. App dừng phát và đóng các file, rồi chuyển sang màn hình USB Storage của firmware. Gắn cáp USB-C có truyền dữ liệu vào cổng hỗ trợ USB device; nếu đang dùng DAC/USB host, luồng chuyển sẽ chuyển về USB device.

Chép file trên máy tính rồi **Eject/Safely remove** ổ thẻ trước khi trở lại màn hình máy. App dùng unmount bình thường; thẻ còn bận thì chuyển file dừng. Nếu ROM thiếu USB Storage của firmware, màn hình báo và bạn có thể dùng Wi-Fi. Luồng đã được đối chiếu với firmware Brick Pro 1.1.1-20260717; thao tác dây/cổng thực tế cần test trên thiết bị của bạn.

## Ngôn ngữ và bàn phím

**Settings → Language & keyboard** có English, tiếng Việt, Français, Русский, 한국어, 日本語, ไทย, 简体中文, 繁體中文, Deutsch, Español, Português (Brasil), Italiano và Bahasa Indonesia. Bàn phím mở theo ngôn ngữ đã chọn; ô nhập link, token/PIN và HEX dùng Latin.

Telex tiếng Việt: `tieengs Vieetj` → tiếng Việt. Hàn dùng bàn phím hai bộ, ghép âm tiết tự động. Nhật dùng Romaji → Kana; Trung dùng Pinyin không dấu thanh. Với Nhật/Trung, X/Space mở danh sách ứng viên Kanji/Hán tự, A chọn và SELECT xác nhận ô nhập. Y chuyển layout, L1 Shift, R1 Caps; bàn phím Latin có lựa chọn ký tự có dấu khi giữ phím. Font và từ điển nằm trên thẻ, dùng được offline. Có **Test keyboard** để thử trước.

## EQ, LED và hiệu năng

SELECT mở 12 preset EQ và Custom với năm thanh; giữ các mục DSP/PEQ nâng cao. LED đổi màu, chế độ và hiệu ứng trong cài đặt sẽ áp dụng ngay, không cần tắt/bật lại.

Nếu muốn giảm việc vẽ, tắt Animations hoặc Spectrum, dùng E-Ink, hoặc tắt màn hình khi nghe. App giữ cache có giới hạn, chỉ giữ một khung giao diện đang dùng và không vẽ/FFT khi ẩn hay tắt màn hình. Giao diện cơ học tối đa 20 lần vẽ/giây, giao diện thường 10 và E-Ink 4; xử lý phím không bị giới hạn theo nhịp vẽ. Mức nóng và độ mượt cuối cùng phụ thuộc máy, nguồn nhạc, Wi-Fi và DAC; chưa có số đo nhiệt độ trên máy thật trong bộ kiểm thử này.

## Hướng dẫn và thông tin trong app

Hướng dẫn chia mục nhạc trong thẻ, online, hồ sơ, phím, giao diện/LED, chuyển file, bàn phím và phát nền. Cài đặt gồm sáu nhóm, X tìm, Y xóa tìm. About ghi lên ý tưởng và thực hiện bởi Nguyễn Xuân Thông, biệt danh Spring Thông, giữ Vibe Code. PayPal: **thongkhung2@gmail.com**; giữ thông tin MoMo và Vietcombank hiện có.

## Test nhanh bản mới trên máy

Kiểm tra logo hồ sơ lúc vào app; chọn ba bước nhạc hôm nay; đổi vài giao diện sáng/tối với bài có tên dài và ảnh album; thử X/Y/SELECT/START, lặp/ngẫu nhiên/hẹn giờ; đổi LED ngay; chép một file Wi-Fi rồi hủy một file; thử USB-C sau khi Eject. Mở Device performance để xem cảm biến máy thực sự hỗ trợ. Gửi tên giao diện và lỗi cụ thể nếu cần tinh chỉnh tiếp.

Build đã kiểm tra bằng compiler ARM64, render SDL native, kiểm thử bộ gõ/luồng chuyển file và fixtures LED/đầu ra. Các kiểm thử không thay cho phát nhạc có tài khoản, cáp USB và đo nhiệt độ trên Brick Pro thật.
