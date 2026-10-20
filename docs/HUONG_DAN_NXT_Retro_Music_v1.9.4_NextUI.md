# NXT Retro Music v1.9.4 — Auralis & fine volume

- Thêm **Auralis Themes** (theme số 51), giữ nguyên 50 theme trước. Bìa album phủ đầy màn hình theo đúng tỉ lệ, có lớp tối giúp lời dễ đọc. Chọn trong Cá nhân → Chỉnh sửa giao diện cá nhân → Bộ giao diện.
- Thêm **MiSans Semibold · Xiaomi / NX Redux** vào bộ font, giữ nguyên 16 font cũ. Auralis sử dụng MiSans. Giấy phép và ghi công Xiaomi tại licenses/MiSans.txt.
- Đang phát và Y xem lời đầy đủ hiển thị cặp lời gốc / bản dịch **in nghiêng**, phân biệt Đang hát và Tiếp theo. Mỗi câu nằm trong một hàng, co vừa chiều rộng; không chạy ngang. Chuyển dòng dọc có thể tắt bằng tùy chọn chuyển động giao diện. Hiệu ứng theo mốc câu LRC, không tự suy đoán mốc từng từ.
- Phím âm lượng máy, SELECT + lên/xuống, nút âm lượng gán lại và HID DAC đều chỉnh từng **1%**, nhớ riêng mức của loa/3,5 mm, Bluetooth và DAC. Giữ tương thích cấu hình âm lượng cũ. Phần cứng có số nấc mixer hữu hạn vẫn phụ thuộc DAC/ROM.
- Bỏ lớp gợi ý phím hệ thống bị chồng lên trình phát khi chỉnh âm lượng; giữ thông báo âm lượng của app.
- Giữ toàn bộ tính năng v1.9.3: thư viện có chỉ mục, quét tăng dần, hồ sơ/tiếp tục bài, 256 MiB cache dịch ngoại tuyến, Groq/MyMemory/Google, nhập Groq TXT, Spotify/SoundCloud/Drive, EQ/DSP, Bit-Perfect, DSF/DFF Native/DoP, Bluetooth/DAC/3,5 mm, LED/phổ, chế độ CPU, Wi-Fi, hẹn giờ thoát app và tích hợp riêng từng ROM.
- Nền album/lớp tối chỉ xử lý khi thay ảnh, kích thước hoặc màn hình; dùng cache chữ và giới hạn nhịp vẽ theo chế độ hiệu năng. Không thêm xử lý vào luồng giải mã/âm thanh. Nhiệt độ, pin và DAC thực tế cần kiểm tra trên máy.
- Giữ 3 link OTA chính thức đã nhúng. Khi phát hành, đăng ZIP cài đúng ROM rồi thay nội dung update.json tại **cùng ID Drive**. Người dùng chỉ cần bấm Kiểm tra cập nhật. Không dùng ZIP OTA kit/source làm payload cài app.

# NXT Retro Music v1.9.4 — NextUI

## Thư viện nhanh v1.9.4

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

Giữ đầy đủ sửa đổi DSD/DoP, tên DAC và log của v1.9.2; thêm thư viện tăng dần v1.9.4 cho NextUI. Dùng đúng ZIP dành cho ROM.

- Giữ nguyên dữ liệu/hồ sơ/cache 256 MiB, Groq TXT, CPU/LED/phím nền riêng của ROM, nguồn nhạc và OTA.
- DSD không tự chuyển PCM. Native cần driver ROM; DoP cần DAC thật sự hỗ trợ đúng tần số truyền.
- Mặc định và lựa chọn DSD cũ được giữ. Muốn thử Native rồi DoP: Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra → Đầu ra DSD USB → Tự động, xác nhận DoP.
- DSD64 qua DoP cần 176.4 kHz, DSD128 cần 352.8 kHz, DSD256 cần 705.6 kHz. DAC hỗ trợ DSD256 Native không đồng nghĩa DoP DSD256. Không hạ DSD tier hay chuyển PCM ngầm.
- Tên USB có thể là tên bộ điều khiển/OEM. Chọn Tên hiển thị DAC → TRN Black Pearl hoặc tự nhập. App lưu tên cho bộ VID/PID/manufacturer/product bạn xác nhận, không suy đoán model/tính năng từ chip hay tên TE-C. Hai thiết bị có descriptor giống hệt sẽ dùng chung nhãn.
- Log DSD giữ lỗi ở bước đã mở được phần cứng, không bị các cổng không tồn tại ghi đè. Native/DoP ghi rõ format, tần số truyền, bước thất bại. Trạng thái không còn kết luận mọi lỗi đều do thiếu DAC tương thích.
- Thông tin, hướng dẫn & cập nhật: START xuất NXT_App_Log.txt, Y xuất NXT_Device_Log.txt gồm VID/PID, descriptor, kernel, stream formats, trạng thái DSD. Giữ DAC cắm khi xuất. File ở Tools/tg5040/NXT Retro Music.pak cạnh launch.sh.

Cài đặt: thoát hẳn phiên nghe, giải nén ZIP ở gốc thẻ; giữ nguyên dữ liệu người dùng. OTA giữ nguyên link NextUI đã tích hợp. Người dùng không cần nhập link; người phát hành vẫn cần cập nhật nội dung chính file update.json cũ trên Drive.

Bản sửa cần thử lại với Black Pearl và DFF thật. Các kiểm thử phần mềm dùng mô phỏng ALSA, không thay thế xác nhận phần cứng hay đo nhiệt/pin. Nếu Native vẫn lỗi, log mới sẽ xác định driver thiếu format hay thiết lập bị từ chối; app không thể tạo khả năng Native DSD mà kernel ROM không hỗ trợ.

Nguồn OTA chính thức đi kèm: https://drive.google.com/file/d/16OuNQmcxwLE3VTTN2bUyZ0yL2EiGFeuD/view?usp=sharing

Không xóa dữ liệu người dùng khi cài đè. Chọn hồ sơ rồi khôi phục Now Playing như trước. Giữ 50 giao diện, dịch song ngữ/cache offline, Groq/MyMemory/Google, Spotify/SoundCloud/Drive, EQ/Bit-Perfect, Bluetooth/3.5 mm/DAC, chép Wi-Fi và hẹn giờ thoát app.
