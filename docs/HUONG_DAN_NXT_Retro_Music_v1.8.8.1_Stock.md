# Hướng dẫn NXT Retro Music v1.8.8.1 — Stock

## Bật song ngữ ngay trên Đang phát

Cài đặt → Phát nhạc & âm thanh → Cách phát → Dịch lời bài hát → Hiển thị → Song ngữ. Cũng có trong START (Tùy chọn), và START khi đang xem lời đầy đủ bằng Y.

- Tắt: chỉ lời gốc, không gửi yêu cầu dịch.
- Bản dịch: một hàng dịch; chưa dịch xong thì hiện lời gốc.
- Song ngữ: hai hàng đồng thời ở Now Playing, hàng trên lời gốc và hàng dưới bản dịch, áp dụng cả NXT gốc và 49 theme còn lại. Câu đang chờ có “Đang dịch…”; không dùng cuộn ngang cho hai hàng lời.
- Y vẫn mở lời đầy đủ song ngữ: chỉ hàng dịch dùng chữ nghiêng, hàng gốc giữ kiểu chữ hiện tại. Ở chế độ Bản dịch, câu đã dịch trên màn hình Y cũng nghiêng; câu chưa dịch vẫn hiện lời gốc như cũ. Hai hàng trên Now Playing giữ kiểu chữ hiện tại. Thời gian LRC và độ trễ lời giữ nguyên.
- Đích dịch theo ngôn ngữ app. Nếu tự nhận diện sai (nhất là lời Latin/kanji hoặc nhiều ngôn ngữ), chọn Ngôn ngữ lời thủ công.

## Tốc độ và dịch còn thiếu

Mỗi lượt gom tối đa 6 câu ngắn, tổng không quá 480 byte UTF-8. Google Cloud gửi mảng q và nhận kết quả riêng từng câu. MyMemory chỉ chấp nhận nhóm nếu giữ đủ ranh giới dòng; mất ranh giới thì dùng từng câu để tránh ghép sai lời. Câu dài chia thành nhiều phần. Câu lặp dùng lại kết quả.

Lỗi mạng tự quay lại phần thiếu theo nhịp chờ tăng dần 2/10/30 giây; tiếp tục các câu khác trước. Hết hạn mức hoặc khóa bị từ chối chờ 15 phút, không gửi dồn dập. Có thể chọn Thử dịch lại sau khi kiểm tra Wi-Fi/khóa/hạn mức. Không thể bảo đảm dịch đủ toàn bài khi dịch vụ hết quota hoặc không cung cấp bản dịch.

Bản dịch v1.8.8 trên thẻ được giữ và dùng lại khi offline. Dịch mới cần Wi-Fi. Ẩn app/tắt màn hình tạm dừng yêu cầu mới và tiếp tục khi quay lại, để giảm tải và tiết kiệm pin. Dữ liệu đang dịch không bị gửi lại khi chỉ đổi Bản dịch ↔ Song ngữ. Đổi bài, hồ sơ, ngôn ngữ hoặc nguồn dịch hủy tác vụ cũ.

## Dịch vụ dịch

MyMemory mặc định, không cần khóa, có giới hạn của nhà cung cấp. Google Cloud tùy chọn: cần khóa API của chính bạn đã bật Cloud Translation Basic; hạn mức và chi phí phụ thuộc tài khoản. Nhập khóa trong mục Dịch lời → Khóa Google Cloud, sau đó chọn dịch vụ Google Cloud. Khóa không kèm trong bản phát hành; không đưa vào dòng lệnh, log hay cache lời. Trên thẻ FAT quyền tệp Unix có thể không được áp dụng; không chia sẻ userdata chứa khóa.

Chỉ gửi nội dung lời khi bật dịch, không gửi âm thanh hoặc token nghe nhạc. Dịch máy có thể chưa sát nghĩa thơ/lời hát.

## Cài đúng bản và OTA

- Stock: giải nén ZIP vào gốc thẻ, app ở Apps/NXT_Retro_Music.
- NextUI: Tools/tg5040/NXT Retro Music.pak.
- NX Redux: Tools/NXT Retro Music.pak.

Giữ thư mục userdata, hồ sơ, lời/cache, wallpaper và nhạc. Chọn ZIP đúng ROM. Gói OTA riêng từng ROM chứa ZIP nguyên bản + CHUAN_BI_OTA.html và hướng dẫn đăng. Mở HTML trên máy tính, điền link ZIP Drive công khai để tạo update.json, đăng JSON rồi cập nhật nguồn OTA trong app. Không đăng các JSON mẫu còn placeholder. Gói này chưa tự đăng lên nguồn OTA công khai.

## Tính năng giữ lại

50 theme, chữ/ảnh sắc nét, hồ sơ trước Now Playing, bài/vị trí nghe dở, EQ bằng SELECT, Normal/Bit-Perfect/Automatic, online tự về Normal, DAC/Bluetooth/3.5/loa, âm lượng trên máy hoặc DAC, LED, phổ nhạc, timer dừng và thoát app, chuyển file Wi-Fi, đa ngôn ngữ và bàn phím, playlist/yêu thích được giữ như v1.8.8.

## Kiểm chứng

Worker/HTTP/cache/UI/audio/OTA được kiểm thử bằng fixture và QEMU ARM64. Không đo nhiệt độ/pin hay xác nhận hết giật trên máy TrimUI thật. Kết quả dịch trực tiếp và tốc độ nhà cung cấp cần thử với Wi-Fi/tài khoản của bạn.

Tham khảo: https://docs.cloud.google.com/translate/docs/reference/rest/v2/translate ; https://mymemory.translated.net/doc/spec.php

## Build LyricsFix2

Phân biệt “Đang tìm lời gốc”, “Không tìm thấy lời gốc” và trạng thái dịch. App chỉ dịch được khi có lời gốc. Hỗ trợ cả plainLyrics khi không có LRC; lời cover tìm qua tên cơ bản sẽ dùng thời gian ước lượng, không mượn mốc đồng bộ của bản khác. Thiếu lời có thể thêm file .lrc cạnh nhạc. Nút Thử dịch lại cũng thử tìm lại lời gốc nếu lần tìm trước đã kết thúc.

Log mới ghi [NXT lyrics]/[NXT translation] và mã HTTP/curl/spawn, không ghi lời hoặc khóa dịch. Nếu vẫn thoát khi chuyển bài, gửi NXT_Input_Log.txt sau lần lỗi; không thể kết luận hết crash chỉ bằng kiểm thử trên máy build.

Vẫn là v1.8.8.1, nhận diện nội bộ LyricsFix2. Đóng hẳn app trước khi chép đè. OTA từ v1.8.8 được kiểm thử; nếu đã cài v1.8.8.1 cũ cần chép đè vì số phiên bản giống nhau. Gói OTA cần đăng lại ZIP, size và SHA tương ứng.
