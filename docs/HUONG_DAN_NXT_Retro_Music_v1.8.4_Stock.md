# Cài NXT Retro Music v1.8.4 — Stock

Dành cho TrimUI Brick Pro chạy Stock. Dùng gói `NXT_Retro_Music_v1.8.4_Stock.zip`.

1. Trong app cũ, dừng nhạc và thoát hẳn, kể cả phiên phát nền.
2. Giải nén gói vào gốc thẻ nhớ, giữ nguyên thư mục `Apps`. Chép đè thư mục app `Apps/NXT_Retro_Music`.
3. Giữ thư mục nhạc, playlist và dữ liệu cá nhân của bản cũ. Gói cài không chứa dữ liệu người dùng và không ghi đè các thư mục hệ thống/ROM.
4. Mở NXT Retro Music; Thông tin ứng dụng phải hiện v1.8.4. App tự mở hồ sơ gần nhất; kiểm tra thư mục nhạc nếu vị trí thẻ thay đổi.

## Vị trí các mục mới

- Cài đặt: Phát nhạc & âm thanh; Thư viện & tệp; Hiệu năng thiết bị; Cài đặt phím; Ngôn ngữ.
- Cá nhân → Hồ sơ → Giao diện cá nhân: Bộ sưu tập giao diện; Tùy chọn giao diện gốc; Đèn LED; Phổ nhạc.
- Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm: Kiểm tra cập nhật; Hướng dẫn cập nhật.
- Hỗ trợ phát triển/PayPal: Thông tin ứng dụng. PayPal: thongkhung2@gmail.com.

## Trình phát và âm thanh

L1 bài trước · A phát/tạm dừng · R1 bài tiếp · X danh sách · Y lời toàn màn hình · SELECT mở/đóng EQ · START tùy chọn · R2 yêu thích · L2 phổ nhạc · B quay lại. Trong chế độ Thường, SELECT + ↑↓ chỉnh âm lượng. Nếu bạn đổi phím, nhãn trên player lấy từ cấu hình đang dùng.

Ba icon ngẫu nhiên, lặp lại và hẹn giờ đổi trạng thái theo thiết lập; chỉnh trong START → Tùy chọn. Lặp lại một bài có số 1 trên icon. Hẹn giờ bật có nền màu. Không cần bật/tắt LED để áp dụng màu/chế độ mới.

NXT gốc hiện chất lượng nguồn và đầu ra ở cột phải; các theme còn lại giữ hai dòng thông tin riêng. START → Thông tin bài hát & đầu ra xem định dạng, Hz, bit, bitrate, số kênh, DAC và PCM. Một số nguồn online không cung cấp bitrate hoặc độ sâu bit; app chỉ hiện thông số đã có, không tự gán chất lượng. DAC chưa xác nhận PCM sẽ hiện trạng thái chưa xác nhận.

## Chế độ hiệu năng

Cài đặt → Hiệu năng thiết bị → Chế độ hiệu năng. A áp dụng, B quay lại. Dấu ✓ chỉ lựa chọn đang dùng; app lưu chế độ toàn thiết bị, kể cả khi đổi hồ sơ.

- **Tiết kiệm pin:** giảm nhịp vẽ và hiệu ứng, phù hợp khi muốn giảm việc xử lý giao diện.
- **Cân bằng:** mặc định, giữ ngân sách hiển thị như v1.8.1 để nghe nhạc hằng ngày.
- **Hiệu năng:** tăng nhịp vẽ cho chuyển động mượt hơn, có thể dùng pin nhiều hơn.

Ba chế độ điều chỉnh phần hiển thị, không đổi tần số CPU, xử lý âm thanh hay định dạng đầu ra DAC. Khi tạm dừng, giao diện chỉ cập nhật 1 Hz; E-Ink tối đa 4 Hz. Tắt màn hình/ẩn app không vẽ hoặc FFT. Phần Theo dõi thiết bị vẫn hiển thị số đo cảm biến như trước. Mức nóng, thời lượng pin và độ mượt thực tế cần kiểm tra trên máy.

## Trình phát NXT gốc

Chọn NXT gốc trong Bộ sưu tập giao diện. Trình phát trở lại bố cục v1.8 với hình nền thấy dưới bảng trong suốt, ảnh album/đĩa than bên trái, thông tin và DAC bên phải, lời, phổ nhạc, số bài và phím tắt có nhãn.

Tùy chọn giao diện gốc giữ các tùy chọn sáng/tối, màu, font, kích cỡ chữ, hình nền, lớp phủ, ảnh album/đĩa than, chuyển động, icon và độ trễ lời. 35 theme khác giữ cách hiển thị của v1.8.1.

## OTA v1.8.4

Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm → Kiểm tra cập nhật. Nguồn cập nhật là link tệp update.json HTTPS công khai, riêng theo ROM. Máy v1.8.1–v1.8.3 sẽ thấy v1.8.4 khi nhà phát hành cập nhật nguồn đó.

Bộ phát hành OTA kèm đúng ZIP cài, trang CHUAN_BI_OTA.html tạo JSON và tệp nguồn offline, cùng hướng dẫn đăng. Đăng ZIP cài trước, tạo và đăng JSON sau. Không dùng ZIP bộ OTA hoặc mã nguồn làm ZIP cài. Hướng dẫn cụ thể ở HUONG_DAN_DANG_OTA.md trong bộ OTA.

## Chép tệp qua Wi-Fi

Cài đặt → Thư viện & tệp → Chuyển file qua Wi-Fi. Máy và điện thoại/máy tính dùng cùng mạng; mở địa chỉ hiện trên máy, nhập PIN sáu chữ số và chọn tệp. B đóng máy chủ, X hủy tệp đang chép. File đã có được giữ nguyên; file chưa chép xong được dọn. Quét lại thư viện sau khi chép nhạc. Không còn chép tệp qua USB-C; DAC USB và tùy chọn host/sạc vẫn được giữ.

## Kiểm tra sau cài

Thử nhạc trong thẻ, nguồn online bạn đã đăng nhập, EQ, DAC rút/cắm, LED đổi màu/chế độ, hồ sơ cũ, X/Y/SELECT/START và Wi-Fi. Độ mượt/nhiệt trên máy thật cần đánh giá trực tiếp với file nhạc và DAC của bạn; bản phát hành đã kiểm tra native nhưng không có máy thật trong môi trường build.

Nếu cần gửi chẩn đoán: vào Thông tin, hướng dẫn & cập nhật, START xuất `NXT_App_Log.txt`, Y xuất `NXT_Device_Log.txt`.

## Hẹn giờ dừng nhạc và thoát app — v1.8.4

Chọn biểu tượng đồng hồ hoặc Tùy chọn nhanh → Hẹn giờ, đặt số phút. 0 phút là hủy.
Hết giờ, NXT lưu vị trí nghe và cài đặt, dừng mọi nguồn nhạc rồi đóng toàn bộ app,
kể cả khi phát nền, tạm dừng hoặc màn hình đã tắt. Hẹn giờ hết không tự chạy lại
khi mở app; lựa chọn phát nền và các thiết lập cá nhân được giữ nguyên.

App đóng qua quy trình dọn dẹp, trả âm thanh/LED và gỡ cờ giữ máy thức do NXT sở
hữu. Không dùng SIGKILL và không có tiến trình canh hẹn giờ còn chạy sau khi đóng.
Đóng app giúp giảm phần xử lý nền; không đồng nghĩa máy vào deep sleep ngay lập
tức. Hệ điều hành quyết định việc tắt màn hình/suspend theo thiết lập của máy.

Trong Stock, kiểm tra cài đặt tắt màn hình/ngủ tự động của firmware. NXT chỉ gỡ
các cờ stay_awake/stay_alive do chính NXT tạo, giữ cờ của ứng dụng khác và trả
keymon về trạng thái trước khi mở app. Không ghi trực tiếp vào /sys/power/state.

Thử hẹn giờ tùy chỉnh 1 phút khi đang phát, khi tạm dừng và khi phát nền. Xác nhận
nhạc/app đều dừng; chờ máy ngủ theo thiết lập hệ thống, đánh thức và mở lại NXT
để kiểm tra vị trí nghe. Khởi động lại máy một lần nếu bản v1.8.2 cũ còn chạy nền
sau khi chép bộ cài mới. Chưa kiểm tra pin, nhiệt và deep sleep trên máy thật.


## Mới trong v1.8.4

App tự dùng hồ sơ gần nhất và vào thẳng **Now Playing**. Mặc định không tự phát; bấm **A** để nghe tiếp, **B** về menu. Đổi hồ sơ, tâm trạng, thể loại và quốc gia trong **Cá nhân**. Bật tự phát ở **Cài đặt → Phát nhạc & âm thanh → Cách phát → Tự phát khi mở app** nếu muốn. Bài, vị trí và ảnh album gần nhất được lưu riêng theo hồ sơ; thiếu file sẽ giữ thông tin và cho chọn bài khác.

**SELECT** mở EQ; nhấn **SELECT lần nữa** đóng EQ mà không chọn preset. **B** vẫn quay lại. Trong Bit-Perfect, EQ chỉ hiện thông báo bypass và giữ nguyên preset cũ.

### Bit-Perfect

1. Cắm DAC USB và chọn **Đầu ra âm thanh → USB DAC**. Giữ chức năng USB host/DAC của ROM như bản trước.
2. Mở **Cài đặt → Phát nhạc & âm thanh → Chế độ âm thanh → Bit-Perfect**, hoặc **START → Chế độ âm thanh**.
3. Chọn nhạc lossless số nguyên tương thích. WAV PCM, FLAC và ALAC 16/24-bit đã được đối chiếu PCM; WAV PCM 32-bit cũng đã được kiểm tra. Mono/stereo giữ nguyên số kênh. Nguồn Drive phải thực sự cung cấp tệp lossless.
4. Xem trạng thái và thông số nguồn/đầu ra tại **START → Thông tin bài hát & đầu ra**. “Sẵn sàng” là đã mở đúng PCM; “PCM USB nguyên bản” là đang phát qua đường này. Nếu không mở được DAC, app tạm dừng và báo lý do.
5. Chỉnh âm lượng trên DAC/tai nghe có điều khiển phần cứng. Âm lượng phần mềm và EQ/DSP, limiter, ReplayGain, tốc độ, crossfade đều được bỏ qua. App giữ các giá trị chế độ Thường để dùng lại sau.
6. Chọn **Thường** khi dùng loa máy, Bluetooth, nguồn lossy, Spotify, SoundCloud, radio hoặc podcast. Thoát ứng dụng/game khác nếu DAC báo bận. App không tự chuyển sang loa hoặc đổi Hz trong Bit-Perfect.

Độ sâu bit của **container PCM** là cách truyền dữ liệu số, không phải chứng nhận độ phân giải analog của DAC. Khả năng mở Hz/format còn phụ thuộc DAC, driver và ROM. Màn hình báo “yêu cầu/tạm dừng/chưa sẵn sàng” không có nghĩa đã phát Bit-Perfect thành công.

### Bố cục và thao tác

Năm mục Cài đặt vẫn là **Phát nhạc & âm thanh; Thư viện & tệp; Hiệu năng thiết bị; Cài đặt phím; Ngôn ngữ**. Âm thanh gồm: Đầu ra, Chế độ âm thanh, EQ/DSP, Cách phát, Kết nối DAC & âm lượng, Thông tin bài hát & đầu ra.

START mở lần lượt: Đầu ra; Chế độ âm thanh; Hẹn giờ; Ngẫu nhiên; Lặp lại; Thông tin bài/đầu ra; Yêu thích; Lời; Tắt màn hình; Dừng và lưu; Phát nền; Dừng và đóng app. Phím tùy chỉnh vẫn áp dụng trong trình phát.

Hẹn giờ hết sẽ lưu rồi đóng toàn bộ NXT; deep sleep phụ thuộc cài đặt ROM. Không ép suspend và không dùng SIGKILL để thoát app bình thường. Giữ chế độ Cân bằng hoặc Tiết kiệm pin, tắt màn hình khi chỉ nghe để giảm xử lý giao diện; chưa có số đo pin/nhiệt trên thiết bị cho v1.8.4.
