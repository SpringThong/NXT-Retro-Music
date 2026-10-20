## Thư viện nhanh v1.9.7.6

Mở app vẫn chọn hồ sơ rồi vào Now Playing. Danh sách nhạc trong Thư viện lấy ngay từ chỉ mục đã lưu, không chờ đọc thông tin từng bài.

- Sau khi vào app, một lượt kiểm tra nhẹ chạy nền sau khoảng 1,5 giây; chỉ duyệt tên và thuộc tính tệp để phát hiện thay đổi. Vào lại Thư viện/Yêu thích/Thêm bài trong cùng phiên không khởi động lượt quét mới.
- Nhạc không đổi không bị mở lại để đọc tag và không ghi lại dấu seen. Nhạc mới hoặc sửa đổi mới được đọc thông tin; bài thực sự bị xóa được gỡ khỏi chỉ mục. App không xóa tệp nhạc.
- Khi nâng từ bản cũ, tên bài/nghệ sĩ và yêu thích đã lưu vẫn dùng ngay; lần kiểm tra đầu bổ sung dấu thời gian vào bảng phụ theo từng đợt mà không đọc lại tag của bài không đổi. Từ lần sau, thư viện không đổi không tạo thay đổi ghi vào chỉ mục.
- Chép bằng máy tính: mở lại app để tự nhận nhạc mới. Chép qua Wi-Fi trong app: chỉ báo thay đổi sau khi tệp cài vào thư mục thành công; nhiều tệp được gom thông báo. Tệp .part/ẩn và liên kết tượng trưng được bỏ qua.
- Nếu chép qua công cụ khác trong lúc app đang chạy, dùng START trong Thư viện → Kiểm tra nhạc mới. Không chạy vòng quét thẻ liên tục.
- START → Quét lại toàn bộ thông tin: đọc lại tag mọi bài trong thư mục đang chọn, vẫn giữ danh sách cũ khi cập nhật. Cài đặt → Thư mục nhạc → Quét lại thư viện cũng dùng lượt kiểm tra đầy đủ này. Dùng khi sửa tag nhưng công cụ vẫn giữ nguyên dấu thời gian/dung lượng hoặc chỉ mục chưa đúng.
- Công việc nền ưu tiên thấp, ghi tối đa 16 thay đổi/đợt, không giữ khóa ghi khi đọc tag. Tạm dừng khi ẩn app, tắt màn hình hoặc đang chép tệp Wi-Fi; tiếp tục khi giao diện hoạt động lại.
- Khi đổi thư mục hoặc tùy chọn gồm thư mục con, danh sách được lọc đúng phạm vi ngay. Dữ liệu các thư mục đã xem và yêu thích được giữ. Nếu thẻ/thư mục mất hoặc lượt duyệt bị hủy/lỗi, không xóa chỉ mục hàng loạt theo phần duyệt dở.
- Danh sách All/Folders/Artists/Albums giữ lại khi đứng yên; không đọc lại thư mục mỗi giây. Chỉ tải lại khi đổi mục, tìm/sắp xếp, điều hướng thư mục hoặc có thay đổi dữ liệu.

Lần đầu chưa có chỉ mục, thư viện sẽ dần xuất hiện trong lúc tạo chỉ mục. Dấu thời gian tệp không phát hiện được mọi thay đổi cố tình giữ nguyên thuộc tính; mục quét lại toàn bộ là cách kiểm tra lại.

Log app có dòng library/index: checked/decoded/writes/commits. Với thư viện đã lập chỉ mục và không đổi, decoded=0, writes=0, commits=0 cho phần cập nhật chỉ mục; các dữ liệu nghe nhạc/hồ sơ khác vẫn lưu bình thường.

