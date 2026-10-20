# NXT Retro Music v1.8.9 — NX Redux

## Thay đổi chính

- Thêm Groq, giữ MyMemory và Google Cloud. Đổi nguồn trong Dịch lời bài hát; không tự chuyển sang dịch vụ khác khi lỗi.
- Khóa Groq và Google lưu riêng trên thiết bị. Không kèm khóa dùng chung trong bản phát hành.
- Groq dịch tối đa 24 dòng / 2.400 byte lời trong một yêu cầu, kiểm tra ID để gắn đúng bản dịch với lời gốc. Khi model trả đầu ra bị cắt, app tự giảm nhóm rồi tiếp tục; một dòng vượt giới hạn vẫn giữ lời gốc. Câu lặp dùng lại bản dịch, không dịch lại.
- Lỗi Groq 429 chờ theo Retry-After/thông tin phục hồi nếu có, dự phòng 15 phút khi dịch vụ không trả thời gian. Hạn mức không được app nâng lên; không cam kết số bài mỗi giờ.
- Cache tăng lên tối đa 512 tệp bài / 16 MiB cho tất cả nguồn. Bản dịch cũ được giữ; nguồn/ngôn ngữ/model khác có cache riêng. Khi đầy, bản lưu cũ có thể bị loại bỏ.
- Stock, NextUI và NX Redux có nguồn OTA chính thức riêng. Nguồn đã lưu từ các bản cũ tự chuyển sang nguồn chính thức khi mở v1.8.9. Sau đó có thể nhập nguồn tùy chọn hoặc Khôi phục nguồn nhà phát hành.
- Giữ song ngữ trên Now Playing đủ 50 theme, chữ nghiêng chỉ ở hàng dịch trong màn hình Y, thời gian/độ trễ lời gốc, hồ sơ và chức năng xóa hồ sơ.
- Giữ pipeline phát nhạc, Bit-Perfect/Tự động, DAC/loa/3.5/Bluetooth, Spotify/SoundCloud/Drive, EQ, LED, phổ nhạc, Wi-Fi, hẹn giờ thoát app và độ sắc nét hiện có.
- v1.8.9 có bộ cài và OTA riêng cho Stock, NextUI và NX Redux.

## Bật Groq

1. Trên máy tính/điện thoại mở https://console.groq.com/keys, đăng nhập hoặc tạo tài khoản. Tạo API key riêng, giữ gói Free; không chọn nâng lên Developer trả phí nếu chỉ muốn dùng miễn phí.
2. Trong app: Cài đặt → Phát nhạc & âm thanh → Cách phát → Dịch lời bài hát. Cũng mở được từ START ở trình phát/màn hình Y.
3. Nhập **Khóa API Groq**, chọn **Dịch vụ dịch → Groq**, rồi **Hiển thị → Song ngữ** hoặc Bản dịch. Wi-Fi phải có kết nối Internet.
4. Model mặc định **openai/gpt-oss-20b**; có thể chọn GPT-OSS-120B hoặc Qwen 3.8 27B (preview). Model có thể đổi/ngừng ở phía Groq; lỗi model có trạng thái riêng, không gửi dồn dập.
5. Đích dịch theo ngôn ngữ app. Tự nhận diện gửi source=auto; có thể chọn ngôn ngữ lời thủ công.
6. Không gửi API key vào chat/log công khai. Tệp userdata/translation-service.json chứa khóa; không chia sẻ bản sao userdata. Trên thẻ FAT quyền Unix có thể không được bảo đảm.

## MyMemory và Google Cloud

Chọn nguồn trong cùng mục **Dịch vụ dịch**. MyMemory không cần khóa; Google Cloud tiếp tục dùng **Khóa Google Cloud** đã lưu và Cloud Translation Basic v2. Google có hạn mức/chi phí theo tài khoản của bạn. Đổi nguồn không xóa khóa hay cache của nguồn trước. Nhập khóa Groq không tự sửa khóa Google.

## Cơ chế dịch và mượt khi nghe

Một worker nền ưu tiên thấp xử lý mạng/JSON/cache; không dịch trong callback âm thanh hoặc hàm vẽ. Không thêm model AI chạy trên CPU TrimUI. Kết quả đến đâu hiện đến đó, lời gốc vẫn đọc được khi chưa có bản dịch. Ẩn app/tắt màn hình dừng gửi yêu cầu mới; mở lại thì tiếp tục. Đổi bài/hồ sơ/ngôn ngữ/model hủy kết quả của tác vụ cũ; đổi Bản dịch/Song ngữ dùng tiếp tác vụ.

Y hiển thị lời đầy đủ: bản dịch nghiêng, lời gốc giữ font. Now Playing hiển thị hai hàng cố định, không dùng chữ nghiêng cho hàng dịch. Nếu không có lời gốc thì chưa thể dịch; dùng LRC cạnh bài nhạc hoặc Thử dịch lại.

## OTA cho người nghe

Vào **Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm**. App tự kiểm tra JSON chính thức của đúng ROM. Có bản mới: Tải → chờ kiểm tra → Cài → mở lại app. Chỉ tự kiểm tra; không tự tải/cài khi chưa chọn. Chỉ cài ZIP dành cho đúng ROM Stock/NextUI/NX Redux.

Lần đầu và nâng từ bản cũ không cần nhập link. Nguồn custom được người dùng nhập rõ trong v1.8.9 sẽ giữ qua lần mở/nâng cấp; chọn **Nguồn cập nhật → Khôi phục nguồn nhà phát hành** để trở về nguồn chính thức.

## OTA cho người phát hành

Giải nén bộ OTA đúng ROM, mở CHUAN_BI_OTA.html. Đăng nguyên ZIP cài đặt lên Drive và chia sẻ công khai, điền link ZIP để tải update.json có version/ROM/size/SHA-256 chính xác. Trên đúng tệp JSON chính thức, dùng **Quản lý phiên bản → Tải phiên bản mới**; giữ nguyên ID/link. Không xóa rồi tạo JSON khác. Link ZIP có thể thay cho mỗi lần phát hành.

Nếu chính tệp JSON vẫn chứa bản cũ, người dùng vẫn chỉ thấy bản cũ dù app đã gắn đúng link. Cần cập nhật nội dung JSON mỗi lần phát hành. Không đăng templates còn placeholder. Bộ này chuẩn bị sẵn cho bạn đăng, chưa tự đăng lên Drive.

## Cài thủ công

Thoát hoàn toàn app trước khi chép đè. Stock: Apps/NXT_Retro_Music. NextUI: Tools/tg5040/NXT Retro Music.pak. NX Redux: Tools/NXT Retro Music.pak. Giữ .userdata, nhạc, cache và hồ sơ; không chép gói ROM khác.

## Kiểm chứng và giới hạn

Kiểm thử dùng máy build, HTTP/lời giả lập rõ ràng, sanitizer và QEMU ARM64. Thành công dịch trực tiếp với khóa Groq thật, chất lượng dịch lời, nhiệt độ, pin và độ trễ trên TrimUI cần kiểm tra trên thiết bị. Gói Free vẫn có hạn mức, không phải không giới hạn.

API tham khảo: https://console.groq.com/docs/api-reference ; https://console.groq.com/docs/structured-outputs ; https://console.groq.com/docs/rate-limits

## Nguồn OTA chính thức đã gắn

Stock: https://drive.google.com/file/d/1OUrxqAs2fOnjKeqZscGcMEiX5Yv3jP5m/view?usp=sharing

NextUI: https://drive.google.com/file/d/16OuNQmcxwLE3VTTN2bUyZ0yL2EiGFeuD/view?usp=sharing

Đã kiểm tra lại ngày 2026-10-13: nguồn Stock ghi rom=stock, nguồn NextUI ghi rom=nextui; cả hai hiện là v1.8.8.2, size và SHA khớp đúng gói cài trước đó của từng ROM. Khi đăng v1.8.9, người phát hành tải JSON mới tạo từ bộ OTA lên **chính từng tệp** bằng Quản lý phiên bản để giữ nguyên ID/link. Mỗi ZIP mới cần version/size/SHA/download_url đúng; không dùng lại JSON v1.8.8.2.

NX Redux: https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing

Đã kiểm tra trực tiếp: JSON hiện ghi rom=nxredux, device=trimui-brick-pro, version=1.8.8.2. Khi đăng v1.8.9, thay nội dung trên cùng ID bằng JSON mới tạo từ bộ OTA NX Redux.

## Cài riêng NX Redux

Giải nén ZIP NXRedux vào gốc thẻ, đặt app tại **Tools/NXT Retro Music.pak**. Mở từ Tools của NX Redux trên TrimUI Brick Pro. Giữ dữ liệu người dùng hiện có. Không dùng gói NextUI thay cho gói này.
