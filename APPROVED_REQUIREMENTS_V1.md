# NXT Retro Music — yêu cầu điều chỉnh cho v1.0

**Ngày:** 05/10/2026  
**Thiết bị:** TrimUI Brick Pro, hệ điều hành NextUI  
**Trạng thái:** đã ghi nhận yêu cầu và cách dùng L2 do người dùng làm rõ; đang ở bước duyệt yêu cầu trước khi sửa code và build.

Người dùng xác nhận bản đã build chạy gần như tốt trên Brick Pro thật. Đợt này xử lý năm nhóm thay đổi dưới đây. Các chức năng đã thống nhất trước đó tiếp tục là nền của ứng dụng, đặc biệt font dự phòng offline cho tên bài và lời Hàn/Nhật/Trung/Thái.

Tài liệu này bổ sung cho yêu cầu hiện có; các quy định mới về L2, R2, nguồn, About và icon thiết bị dưới đây được ưu tiên khi có khác biệt với bảng nút hoặc mô tả cũ.

## 1. Nhấn nút nguồn tắt màn hình, nhạc vẫn chạy

**Lỗi người dùng báo:** khi ứng dụng đang mở, kể cả tại Now Playing, nhấn nút nguồn để tắt màn hình khiến nhạc cũng tắt.

**Hành vi cần đạt:**

- Nhấn ngắn nút nguồn khi giao diện app đang ở phía trước để tắt màn hình; âm thanh tiếp tục phát, giữ đúng bài và vị trí.
- Nhấn nút nguồn lần nữa để bật màn hình và trở lại màn đang sử dụng.
- Tắt màn hình là tắt hiển thị/đèn nền; không đưa hệ thống vào trạng thái ngủ làm ngừng xử lý âm thanh của phiên này.
- Hẹn giờ dừng, chuyển bài tự động, EQ/DSP và các tác vụ nền của app vẫn hoạt động. Hết giờ thì dừng theo chính sách hẹn giờ đã chốt.
- Nếu bài đang tạm dừng thì vẫn tạm dừng khi tắt/bật màn hình; không tự phát lại hoặc khởi động lại bài.
- Áp dụng tại Now Playing và các màn hình khác trong app: menu, thư viện, playlist, cài đặt, lời, EQ và About.
- Nút nguồn vẫn dùng được khi khóa thao tác nhạc. Thao tác đánh thức màn hình không đồng thời phát sinh thao tác điều khiển nhạc.
- Thao tác giữ nút nguồn để tắt máy vẫn tuân theo hệ điều hành. Việc lấy nút nguồn của game khi giao diện app đã ở nền không được mở rộng trong đợt này.

Bổ sung giải thích nhấn ngắn nút nguồn vào Hướng dẫn sử dụng để phân biệt tắt màn hình với tắt máy.

## 2. L2 đổi màu phổ và bật/tắt theo chu kỳ

**Cách dùng người dùng đã làm rõ:**

> L2 nhấn 1 lần thì đổi 1 màu phổ, cho tới khi hết màu phổ thì tắt và lặp lại như vậy.

**Quy tắc:**

- Tại Now Playing, mỗi lần nhấn L2 chuyển đúng một bước:
  **Màu 1 → Màu 2 → … → Màu cuối → Tắt phổ → Màu 1 → …**
- Khi đang ở trạng thái Tắt, nhấn L2 bật lại phổ với màu đầu của chu kỳ.
- Dùng nhấn đơn; không cần giữ nút hoặc tổ hợp để thực hiện chu kỳ này.
- Cách dùng được chốt là đổi màu và trạng thái bật/tắt; kiểu phổ đang chọn được giữ khi chỉ đổi màu.
- Phổ tiếp tục phản ánh tín hiệu âm thanh thật; đổi màu hoặc tắt/bật phổ không làm gián đoạn nhạc.
- Lưu màu hiện tại và trạng thái bật/tắt để khôi phục khi mở lại app.

**Dòng hướng dẫn cần thêm:** “L2: đổi màu phổ; hết màu thì tắt, nhấn tiếp để bật lại từ màu đầu.”

## 3. R2 thêm/bỏ yêu thích bài đang nghe

- Tại Now Playing, nhấn R2 thêm bài đang nghe vào Yêu thích; nhấn lại để bỏ Yêu thích.
- Có hiệu lực khi bài đang phát hoặc tạm dừng.
- Cập nhật ngay biểu tượng trái tim, danh sách Yêu thích và trạng thái đã lưu.
- Hiện thông báo ngắn tương ứng: “Đã thêm vào Yêu thích” / “Đã bỏ Yêu thích”.
- Thao tác không đổi bài, không tua và không thay đổi trạng thái phát/tạm dừng.
- Khi không có bài hiện tại, không tạo mục Yêu thích rỗng.

**Dòng hướng dẫn cần thêm:** “R2: thêm/bỏ Yêu thích bài đang nghe.”

L2 và R2 là bổ sung mới cho bảng nút. Hướng dẫn English/Tiếng Việt cần cập nhật và phản ánh ánh xạ thực tế nếu người dùng đổi nút.

## 4. About tối giản, phiên bản v1.0 và thông tin ủng hộ

About chỉ có **một mục nội dung có thể chọn: Hướng dẫn sử dụng**. A mở hướng dẫn, B quay lại. Các dòng thông tin còn lại là văn bản, không có khung chọn hoặc mũi tên mở trang.

| Thông tin hiển thị | Nội dung |
| --- | --- |
| Tên ứng dụng | NXT Retro Music |
| Phiên bản | v1.0 |
| Dòng nhận diện | Vibe Code |
| Người lên ý tưởng | Nguyễn Xuân Thông |
| Phát triển | ChatGPT 6.1 Sol |
| Lời ủng hộ | Ủng hộ phát triển ứng dụng |
| MoMo | **0339838267** |
| Vietcombank / VCB | **0581000786219** |
| Mục chọn duy nhất | Hướng dẫn sử dụng |

- Trình bày gọn, dễ đọc trên màn hình 1024 × 768; nhóm thông tin tác giả và nhóm ủng hộ rõ ràng.
- Hiển thị đúng hai số trên, giữ số 0 ở đầu. Không tự thêm tên chủ tài khoản chưa được cung cấp.
- About dùng thông tin sản phẩm, tác giả và ủng hộ; không thêm mục nguồn tham khảo hay các trang phụ.
- Nhãn phiên bản được đồng bộ thành v1.0 ở About và metadata của bản phát hành mới.
- Mở/đóng About hoặc Hướng dẫn sử dụng vẫn giữ phiên nghe nhạc.

## 5. Icon tai nghe/DAC theo thiết bị thật

Icon tai nghe không được dùng làm biểu tượng đầu ra mặc định khi không cắm thiết bị.

| Trạng thái kết nối | Icon thiết bị cần hiển thị |
| --- | --- |
| Không cắm tai nghe 3,5 mm, không cắm DAC | Không hiện icon tai nghe hoặc DAC. |
| Cắm tai nghe vào cổng 3,5 mm | Hiện icon tai nghe. |
| Cắm DAC | Hiện icon DAC riêng; nhận diện thiết bị theo dữ liệu thực. |
| Cắm cả tai nghe 3,5 mm và DAC | Hiện trạng thái kết nối tương ứng của từng thiết bị; dòng đầu ra vẫn ghi đúng đầu ra đang dùng. |
| Kết nối Bluetooth | Dùng icon Bluetooth theo trạng thái kết nối; không dùng icon tai nghe 3,5 mm để thay thế. |

- Áp dụng cho các vị trí đang hiển thị trạng thái thiết bị, gồm thanh trạng thái và Now Playing.
- Cắm/rút thiết bị cập nhật ngay icon; rút 3,5 mm thì ẩn tai nghe, rút DAC thì ẩn DAC và bỏ thông tin DAC cũ.
- Icon kết nối và đầu ra đang sử dụng phải phản ánh đúng hai trạng thái tương ứng; không tự suy ra đầu ra đang dùng chỉ từ việc thiết bị được cắm.
- Dòng DAC tiếp tục theo dạng **DAC: <tên/model thực nhận diện được>**; không đoán model/chip.
- Chính sách chọn đầu ra và xử lý rút DAC đã chọn trong Cài đặt tiếp tục có hiệu lực.
- Logo tai nghe của ứng dụng vẫn là nhận diện thương hiệu. Yêu cầu ẩn/hiện này áp dụng cho **icon trạng thái kết nối**, không phải logo app.

## 6. Kiểm tra cần đạt trên Brick Pro thật

| Thử nghiệm | Kết quả cần đạt |
| --- | --- |
| Phát nhạc rồi nhấn nguồn tại Now Playing | Màn hình tắt; nhạc liên tục, không mất vị trí. |
| Nhấn nguồn tại menu, thư viện, playlist, settings, lời, EQ và About | Tắt/bật màn hình đúng; giữ phiên nhạc và màn đang dùng. |
| Tắt màn hình qua ít nhất một lần tự chuyển bài | Bài tiếp theo vẫn phát; bật màn hình hiện đúng bài/vị trí. |
| Đặt hẹn giờ rồi tắt màn hình | Hẹn giờ tiếp tục chạy và dừng đúng hạn. |
| Tạm dừng rồi tắt/bật màn hình | Vẫn tạm dừng và giữ vị trí. |
| Khóa phím rồi nhấn nguồn | Vẫn tắt/bật màn hình được; trạng thái khóa giữ nguyên. |
| Nhấn L2 qua toàn bộ chu kỳ và thêm một vòng | Mỗi lần chuyển một màu, sau màu cuối là Tắt, sau Tắt là màu đầu; không gián đoạn nhạc. |
| Nhấn R2 hai lần | Thêm rồi bỏ Yêu thích đúng bài; trái tim và danh sách cập nhật. |
| Mở About và duyệt bằng D-pad | Chỉ Hướng dẫn sử dụng là mục chọn; v1.0, MoMo và VCB đúng. |
| Cắm/rút riêng tai nghe 3,5 mm và DAC, rồi thử cắm cả hai | Icon theo đúng kết nối; đầu ra ghi đúng; không giữ icon/thông tin thiết bị đã rút. |
| Mở lại app | Cài đặt phổ, Yêu thích và các tùy chỉnh đã lưu được khôi phục. |
| Mở Hướng dẫn sử dụng ở English/Tiếng Việt | Có cách dùng nguồn, L2 và R2; ánh xạ nút đúng. |

**Bước tiếp theo:** người dùng xem bản yêu cầu; sau khi chốt, sửa trên mã nguồn của bản đang chạy và build gói NextUI v1.0.
