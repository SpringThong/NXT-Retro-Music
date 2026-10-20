# NXT Retro Music v1.8.8 — NXRedux

## Thay đổi chính
- Thêm **dịch lời theo ngôn ngữ app**: Tắt / Bản dịch / Song ngữ.
- Now Playing hiện một câu tĩnh. Song ngữ ghép lời gốc và bản dịch ở màn hình lời toàn màn hình; giữ mốc LRC và độ trễ gốc.
- Một worker ưu tiên thấp, một yêu cầu mạng tại một thời điểm, dùng lại câu lặp và bản dịch đã lưu. Không thêm công việc vào callback âm thanh.
- Ẩn app/tắt màn hình sẽ tạm dừng dịch online. Đổi bài/ngôn ngữ/hồ sơ hủy tác vụ cũ; getter không đưa bản dịch cũ lên lời mới ngay cả trước tick tiếp theo.
- Lỗi mạng/hạn mức vẫn hiện lời gốc. Có cooldown và thử lại thủ công, không retry theo mỗi khung.
- MyMemory mặc định dùng công khai không cần khóa, có hạn mức. Google Cloud Basic v2 là tùy chọn với khóa riêng.
- Giữ 50 giao diện, độ nét, hồ sơ → Now Playing, EQ/SELECT, CPUFreq NextUI, loa/3,5mm/Bluetooth/DAC, Bit-Perfect/Tự động, online, LED đổi ngay, Wi-Fi transfer và timer thoát trọn app của v1.8.7.
- Hai trang thay đổi, changelog, hướng dẫn và OTA riêng của ba ROM đều ghi v1.8.8.

## Bật dịch lời
1. Kết nối Wi-Fi trong cài đặt ROM. Chọn ngôn ngữ app ở Cài đặt → Ngôn ngữ.
2. Mở **Cài đặt → Phát nhạc & âm thanh → Cách phát → Dịch lời bài hát**. Có thể mở ở cuối menu START trong Now Playing, hoặc bấm START trong màn hình lời.
3. Đổi Hiển thị sang Bản dịch hoặc Song ngữ. Mặc định Tắt để giữ hành vi cũ và không gửi lời đi trước khi bạn bật.
4. Ngôn ngữ đích tự theo app, có 14 lựa chọn. Ngôn ngữ lời có thể tự nhận diện hoặc chọn thủ công. MyMemory nhận diện dựa theo chữ; lời Latin, Kanji hoặc trộn ngôn ngữ có thể cần chọn lại nguồn. Google Cloud tự nhận diện bằng dịch vụ khi chọn Tự nhận diện.
5. Chờ dịch ở nền; nhạc tiếp tục và câu chưa dịch vẫn hiện lời gốc. Trạng thái & cách dùng hiện tiến độ; Thử dịch lại tiếp tục các câu còn thiếu.
6. Nếu chọn Google Cloud: bật Cloud Translation API cho dự án của bạn và nhập khóa API riêng. Bàn phím nhập khóa luôn Latin, khóa không hiện lại trong menu và chỉ lưu trên máy. Dịch vụ dùng hạn mức/chi phí của tài khoản Google Cloud của bạn; app không nhúng khóa dùng chung.

## Lời, cache và hiệu năng
Dịch dựa trên lời có sẵn: .lrc cạnh file nhạc, lời nhúng, cache hoặc lời nguồn online cung cấp. Bài chưa có lời sẽ chờ; tính năng này không nhận dạng giọng hát từ âm thanh. Lời chưa có mốc thời gian vẫn ghi nhận là không đồng bộ.

Cache nhận diện bằng SHA-256 của toàn bộ thứ tự lời, nguồn, đích và dịch vụ. Câu lặp được dùng lại; lần phát sau dùng các câu đã lưu khi offline. Chưa có cache thì lần đầu cần mạng. Cache giới hạn 96 tệp/16 MiB trong hoạt động bình thường, tối đa 512 KiB mỗi tệp. Input/output/phản hồi mạng cũng được giới hạn. Ghi JSON ở worker bằng tệp tạm rồi đổi tên; không ghi hay fsync trong paint path.

Worker nghỉ bằng condition variable khi rảnh, chỉ một curl đang dịch. Khi lỗi mạng, job mới nghỉ 60 giây; lỗi quota/quyền nghỉ 15 phút. Thử lại thủ công cho phép thử mới. HTTP có timeout, phản hồi bị giới hạn và child được hủy/reap khi đổi tác vụ hoặc thoát app.

Giữ pipeline chữ/ảnh, cache font/album, nhịp vẽ và chính sách CPU v1.8.7. Dịch không thay PCM, EQ/DSP/Bit-Perfect hoặc âm lượng. Chưa đo nhiệt/pin/độ trễ trên TrimUI; không thể suy ra phép đo thiết bị từ benchmark host.

Dịch máy có thể mất ý thơ, tiếng lóng hoặc tên riêng. Lời gốc luôn còn và có thể tắt dịch bất cứ lúc nào.

## Dữ liệu
Khi bật dịch, chỉ nội dung câu lời và ngôn ngữ nguồn/đích được gửi tới dịch vụ chọn. Không gửi âm thanh, đường dẫn bài, nghệ sĩ hoặc token Spotify/SoundCloud. Khóa Google Cloud chỉ gửi cho API Google Cloud qua stdin của curl; không có trong argv/log/cache lời. Tệp translation-service.json chỉ nằm ở userdata, quyền 0600 và không đưa vào bộ cài/source.

## Cài đặt và OTA
Đóng app rồi giải nén gói đúng ROM ở gốc thẻ, giữ đúng cây ZIP và userdata:
- Stock: Apps/NXT_Retro_Music
- NextUI: Tools/tg5040/NXT Retro Music.pak
- NX Redux: Tools/NXT Retro Music.pak

Target là Brick Pro/tg5040; original Brick chưa xác nhận. Chọn hồ sơ trước rồi Now Playing mở bài/vị trí nghe dở, mặc định tạm dừng; A nghe tiếp, B về menu. SELECT mở EQ, SELECT lần nữa đóng mà không áp preset. Timer lưu vị trí và thoát trọn NXT; ROM điều khiển ngủ sâu sau đó.

Giải nén bộ NXT_OTA_Google_Drive_v1.8.8_<ROM>.zip, mở CHUAN_BI_OTA.html, nhập link ZIP công khai để tạo update.json. Đăng ZIP/JSON đúng ROM lên Drive và dùng nguồn tương ứng. Không đăng JSON còn placeholder. OTA kiểm tra ROM/phiên bản/dung lượng/SHA-256, giữ userdata gồm cache dịch và tệp cấu hình riêng. Chưa tự xuất bản nguồn công khai.

## Kiểm tra và giới hạn
Kiểm tra native/ASan/UBSan cho worker/curl/cache, parser lời gốc, renderer 50 theme và song ngữ, controller/config/profile; hồi quy audio/PCM/volume/Bluetooth/online/CPUFreq/timer và OTA full package. Máy chủ dịch thành công trong QA là fixture; ảnh minh họa dùng phản hồi fixture tự viết, không phải chứng nhận chất lượng dịch live.

Môi trường thử API với các câu mẫu tự viết trực tiếp trả HTTP 429; chưa xác nhận dịch live thành công và chưa có khóa Google Cloud thật. Chưa thử TrimUI/DAC/Bluetooth vật lý hay đo nhiệt/pin/latency/sleep. Hạn mức và chất lượng dịch phụ thuộc nhà cung cấp.

## Tài liệu dịch vụ
- [MyMemory API](https://mymemory.translated.net/doc/spec.php)
- [MyMemory usage limits](https://mymemory.translated.net/doc/usagelimits.php)
- [Google Cloud Basic v2](https://docs.cloud.google.com/translate/docs/reference/rest/v2/translate)
- [Google Cloud setup](https://docs.cloud.google.com/translate/docs/setup)
- [Google Cloud pricing](https://cloud.google.com/products/translate/pricing)

Hướng dẫn và báo cáo các phiên bản cũ trong source là lịch sử; bản hiện tại là v1.8.8.
