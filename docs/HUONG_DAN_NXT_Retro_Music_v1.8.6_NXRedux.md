# NXT Retro Music v1.8.6

## Thay đổi

- Bỏ thông báo ép chỉnh âm lượng trên DAC và bỏ chặn thao tác khi đang dùng Bit-Perfect. SELECT + lên/xuống, phím +/- trên máy, phím âm lượng đã gán và HID của DAC đều dùng chung bộ điều khiển âm lượng. Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra cũng chỉnh được các mức âm lượng.
- Khi DAC có mixer âm lượng phần cứng: áp dụng mức trên DAC, giữ PCM nguyên bản. Không thêm gain vào worker Bit-Perfect. Nếu DAC không cung cấp mixer phù hợp hoặc từ chối lệnh, app chuyển sang Thường để dùng gain phần mềm; giữ lựa chọn chế độ đã lưu và hiển thị đúng trạng thái đầu ra, không gắn nhãn Bit-Perfect cho PCM đã giảm âm lượng bằng phần mềm. Không hiện lại thông báo ép dùng nút DAC.
- Chọn Spotify/SoundCloud/radio/podcast sẽ tạm dùng Thường để nghe online, kể cả khi trước đó đang phát Bit-Perfect. Giữ phiên helper và hàng đợi khi chuyển chế độ. Các URI Google Drive, file cache của nguồn Drive và HTTP/HTTPS cũng dùng Thường; không thử mở raw USB cho những nguồn này.
- Chế độ Tự động: nguồn online dùng Thường; khi về file lossless nội bộ và DAC tương thích, thử lại Bit-Perfect. Lựa chọn Thường/Bit-Perfect/Tự động đã lưu không bị đổi khi chọn nguồn. Bit-Perfect thủ công vẫn giữ nguyên nguyên tắc từ chối PCM nội bộ không tương thích; ngoại lệ nguồn online tạm dùng Thường để phát được.
- Cài đặt → Thư viện & tệp chỉ giữ Thư mục nhạc và Chuyển file qua Wi-Fi. Bỏ Hệ thống/Wi-Fi tại mục này; vẫn giữ chức năng copy file Wi-Fi.
- Giữ tài nguyên/fonts, renderer, 36 theme và độ phân giải v1.8.5. Giữ CPUFreq NextUI, cache ảnh/chữ, ghi thẻ ở nền, EQ/SELECT, Bluetooth/loa/3,5mm, LED đổi ngay, hồ sơ → Now Playing, 14 ngôn ngữ và timer thoát toàn bộ app. Không thêm tác vụ quét mixer/network mỗi UI tick.

## Cài đặt

Dùng gói đúng ROM, đóng app rồi giải nén ở gốc thẻ, giữ cây thư mục của ZIP. Sao lưu userdata trước khi nâng cấp, không xóa hồ sơ hoặc thông tin nghe dở.

- Stock: Apps/NXT_Retro_Music.
- NextUI: Tools/tg5040/NXT Retro Music.pak.
- NX Redux: Tools/NXT Retro Music.pak.

Đây là gói cho TrimUI Brick Pro/tg5040; chưa xác nhận original TrimUI Brick. Mở app chọn hồ sơ, vào Now Playing của hồ sơ đó; mặc định tạm dừng, A nghe tiếp, B về menu. Chỉnh EQ bằng SELECT; SELECT lần nữa đóng mà không áp preset.

## Âm thanh và cách thử

1. Trong Âm thanh & đầu ra chọn Tự động. Dùng file FLAC/WAV/ALAC nội bộ với DAC phù hợp, kiểm tra trạng thái Bit-Perfect và thông tin PCM thực tế.
2. Chỉnh volume bằng máy, bằng phím đã gán, HID hoặc nút DAC. Kiểm tra có thay đổi âm lượng thật và không xuất hiện thông báo ép dùng DAC. Với DAC thiếu mixer, trạng thái Thường là đúng vì đang dùng gain phần mềm.
3. Từ bài Bit-Perfect, chọn Spotify/SoundCloud/Google Drive. Kiểm tra phát, lời, pause, tua, tiếp bài và phát nền; trạng thái phải là Thường. Chuyển lại file nội bộ, chế độ Tự động có thể dùng lại Bit-Perfect nếu DAC/PCM phù hợp.
4. Rút/cắm DAC, chuyển loa/3,5mm/Bluetooth, thử Play. Giữ lựa chọn Khi rút DAC → Tạm dừng/Chuyển loa của bạn. Giảm âm lượng DAC trước khi chuyển sang PCM nguyên bản vì mức phần mềm và mức phần cứng có thể khác nhau.
5. NextUI: thử Tiết kiệm pin/Cân bằng/Hiệu năng, xem giới hạn và xung CPU thực tế. Mục tiêu trần 1008/1608/2000 MHz được điều chỉnh theo bảng xung và giới hạn ROM; báo lỗi nếu driver không cho áp dụng. Stock/NX Redux vẫn để ROM quản lý CPU, các chế độ điều chỉnh tải hiển thị.
6. So độ trễ, pin/nhiệt với v1.8.5 ở cùng độ sáng, nguồn nhạc và thời lượng. Timer kết thúc sẽ lưu phiên rồi đóng NXT; ngủ sâu do chính sách ROM quyết định.

## OTA của ba ROM

Mỗi ROM có bộ NXT_OTA_Google_Drive_v1.8.6_<ROM>.zip riêng, chứa chính xác ZIP cài và SHA-256. Giải nén bộ đúng ROM, mở CHUAN_BI_OTA.html, nhập link ZIP công khai để tạo update.json. Đăng ZIP/update.json lên Drive công khai rồi dùng nguồn đúng ROM trong app. Không đăng JSON còn placeholder hoặc dùng gói khác ROM. Chưa tự đăng nguồn OTA lên mạng.

## Kiểm tra và giới hạn

Có kiểm tra phần mềm/native UI, controller âm thanh, các đường phím volume, PCM integer, decoder ARM64, helper IPC giả lập và OTA full-package. Chưa đăng nhập/nghe streaming bằng tài khoản thật trong môi trường build; chưa thử DAC/mixer/Bluetooth trên máy thật và chưa đo nhiệt/pin/độ trễ thực tế. Chi tiết xem RELEASE_QA_v1.8.6.md và bộ QA.

Các hướng dẫn/báo cáo phiên bản cũ trong source là lịch sử. Chỉ ảnh kiểm thử của phiên bản hiện tại được đóng gói để giảm dung lượng source; tài nguyên runtime được giữ nguyên.

Gói này dành riêng cho NXRedux.
