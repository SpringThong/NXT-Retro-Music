# NXT Retro Music v1.3 — Stock

Build **Stock · LED Driver Fix3**.

Fix3: log máy thật xác nhận hiệu ứng 3 là sniff, 4 là static. Đọc bảng hiệu ứng của driver để chọn đúng chế độ cố định; kiểm tra RGB/chế độ sau khi ghi, thử lại lỗi và giữ màu khi Stock ghi đè. Giữ 16 màu mẫu, RGB tự chọn, độ sáng theo nhạc và khôi phục LED gốc khi thoát.

Fix3: Thông tin → Y thêm bảng hiệu ứng và mã static vào NXT_Device_Log.txt; dùng mã phiên mới để tránh gắn lại phiên app cũ. Giữ nguyên bộ hỗ trợ Spotify/SoundCloud từ Fix2 và dữ liệu đăng nhập. Kiểm tra bằng log driver thật và fixture phần mềm; chưa xác nhận màu trực tiếp trên thiết bị sau bản sửa.

Fix2: DAC khởi tạo chậm vẫn được thử lại sau 5 giây, với khoảng thử 2 giây; ghi cấu hình lỗi được thử lại mà không chuyển nhầm âm lượng/đầu ra. LED chỉ ghi nhận áp màu thành công khi ghi được, thử lại lỗi và áp lại sau khi Stock thay màu/chế độ.

Fix2: nguồn đăng nhập Spotify theo thao tác PC/QR gần nhất; nhập PC không còn bị QR cũ ưu tiên. Kết nối lại giữ lựa chọn. Ghi loại lỗi AP/Login5, giữ lịch sử thử và không cắt làm mới OAuth chỉ vì timeout kiểm tra thư viện.

Fix2: Thông tin → Y xuất NXT_Device_Log.txt cho trạng thái PCM/đầu ra/LED; nhật ký không chứa token. Bộ cài, hướng dẫn và source cập nhật đồng bộ; kiểm tra ARM64/giả lập, chưa xác nhận trên máy/tài khoản thật.

Fix1: tắt animation LED Stock khi nhận điều khiển RGB, khôi phục animation khi thoát; chuyển loa ↔ DAC trong phiên bằng cách nạp lại ALSA trước khi mở thiết bị và retry USB chưa sẵn sàng; Spotify/SoundCloud tiếp tục phát khi đổi nguồn.

Fix1: mặc định tự chuyển sang DAC, rút DAC về loa (migration một lần, giữ thay đổi thủ công sau đó). Giữ fix gain PCM cho nút âm lượng DAC.

Fix1: nhập PC giữ file gốc và lưu bản riêng an toàn; nhận client ID OAuth hợp lệ được cấp quyền; PC/QR phải cùng tài khoản. Xác thực AP/Login5 quyết định kết nối phát; lỗi API thư viện hiển thị riêng. Phát link bài hát Spotify qua phiên đã xác thực khi thư viện bị giới hạn. Cooldown API không chặn làm mới token OAuth.

Thanh trạng thái: nhận Wi-Fi Stock, phần trăm nằm trong biểu tượng pin.

Hướng dẫn sử dụng/Spotify/SoundCloud dạng văn bản cuộn; gọn cài đặt Nhạc online và Phát nhạc; đổi nhãn Phát nhạc dưới nền/Phát nền.

Gán chức năng Đang phát cho toàn bộ 16 nút: âm lượng, ánh sáng, tua, trộn/lặp, tắt màn hình/phát nền/dừng. Đổi nút menu/bàn phím riêng và khôi phục mặc định.

Ghép Spotify: xác thực quyền phát riêng với API thư viện, giữ lần ghép đã lưu, báo lỗi từng bước và không thử đăng nhập vô hạn. Tôn trọng Retry-After và giới hạn quyền máy chủ.

Spotify/SoundCloud: nối kết quả khi cuộn, tải thêm hàng đợi gần cuối; không giới hạn ở trang 20 bài. Vẫn áp dụng giới hạn kết quả của dịch vụ.

Drive: đệm tạm trên thẻ theo tiến trình; dùng lại dữ liệu khi tua; tải trước bài kế và chuẩn bị giải mã khi đủ dữ liệu.

LED: định dạng RGB đúng driver, 16 màu mẫu, chế độ cố định/theo nhạc, bảng RGB tự chỉnh có xem trước.

Bàn phím: đệm font/chữ trên phím/hướng dẫn, giữ hướng lặp nhanh hơn; giữ Telex tiếng Việt và ABXY.

Giữ lại: DAC Volume Fix1, Home task manager Stock, đĩa than/kim quay, phiên nền và thứ tự thư mục tự nhiên.

Không có thiết bị vật lý trong môi trường build. Các kiểm tra dùng fixture/giả lập được ghi rõ trong báo cáo; cần xác nhận trên máy cho Wi-Fi, LED, DAC thật và tài khoản Spotify.
