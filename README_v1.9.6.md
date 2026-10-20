# NXT Retro Music v1.9.6

Bản chung cho TrimUI Brick Pro: Stock, NextUI, NX Redux. Chọn đúng ZIP cho ROM. Giữ tính năng và dữ liệu v1.9.5, không cần xóa app hoặc hồ sơ cũ.

## Thay đổi v1.9.6

- Auralis theo ảnh mẫu: nền xám than `#24292F`, chữ/ô chọn `#DADADA`, chữ và icon trong ô chọn `#080909`. HarmonyOS Sans là font theo mẫu, áp dụng cho menu và lời; bản dịch vẫn nghiêng. Font này là lựa chọn đối chiếu gần mẫu, không khẳng định nhận diện chính xác font trong ảnh.
- Bỏ Bộ font chữ trùng ở Giao diện cá nhân. Đổi font tại Cá nhân → Giao diện cá nhân → Tùy chọn giao diện gốc → Bộ font. Chọn font mới sẽ dùng font đó; bật lại Font theo ảnh mẫu trong Tùy chỉnh Auralis nếu muốn dùng HarmonyOS Sans riêng cho Auralis.
- 6 hiệu ứng chuyển lời: Tắt, Trượt lên, Mờ nhẹ, Trượt + mờ, Lướt từ phải, Nổi nhẹ. Chọn tại Tùy chọn giao diện gốc → Hiệu ứng chuyển lời, hoặc Tùy chỉnh Auralis. Lời gốc và dịch nghiêng đi cùng nhau trên Now Playing và Full lời. Tắt hiệu ứng không xóa mốc karaoke.
- Menu chọn phản hồi ngay, không hòa chữ/ô chọn cũ và mới toàn màn hình. Chuyển trang dùng một ảnh RAM và sao chép điểm ảnh, bỏ pha trộn alpha toàn màn. Giữ cache chữ/bìa/icon, giới hạn phổ riêng và ngừng vẽ khi ẩn/tắt màn.
- Âm lượng vẫn 0..100 từng nấc thực, không chỉ đổi số hiển thị. Có cơ chế hòa giải bước thô trùng từ keymon của ROM quanh thao tác âm lượng. Nút bấm, phím gán và HID giữ cách dùng cũ; mức âm lượng lưu riêng theo đầu ra. Cần test thực trên ROM/DAC vì keymon và mixer của từng bản ROM khác nhau.

## Karaoke theo giọng hát

Có hai nguồn mốc chữ: Enhanced LRC có mốc từng từ thật (dùng offline), hoặc **Groq Whisper Large v3** nhận diện âm thanh. Mốc theo câu đơn thuần không cho biết thời gian từng chữ; app không chia đều câu để giả khớp tiếng hát.

Vào Full lời bằng Y → START → Karaoke theo giọng hát · Groq. Cũng có tại Tùy chọn giao diện gốc → Karaoke theo giọng hát.

1. Nhập khóa Groq như trước (hỗ trợ TXT). Không phải đổi model dịch chữ GPT đang dùng: Whisper là tác vụ riêng cho âm thanh.
2. Chọn Tạo mốc từng chữ cho bài này. Đọc thông báo và chọn Đồng ý nếu muốn gửi âm thanh bài hát đến Groq. Mặc định Hủy; chưa đồng ý thì không tải âm thanh lên.
3. Hoặc bật Tự phân tích bài trên thẻ sau khi đồng ý. Mặc định Tắt, lưu theo hồ sơ. Tự động chỉ phân tích bài chưa có mốc/cache; không tự thử lại liên tục khi lỗi/hết hạn mức.
4. Chờ phân tích ở nền ưu tiên thấp. App tạo WAV mono 16 kHz để phân tích, không đổi đường phát nhạc, Bit-Perfect hay DSD. Đổi bài/hủy thì kết quả cũ không được ghép sang bài mới. Khi ẩn/tắt màn, bước chuẩn bị âm thanh tạm dừng.
5. Chỉ gắn mốc khi chữ nhận diện khớp với lời hiện có. Kết quả lưu riêng theo tệp nhạc để lần sau dùng offline, không cần khóa/mạng để đọc cache. Không ghi đè tệp nhạc hoặc .lrc gốc. Bộ nhớ đệm mốc karaoke giới hạn riêng 16 MiB/256 tệp; cache dịch vẫn 256 MiB như trước.

**Giới hạn cần biết:** nhận diện tiếng hát bằng AI có thể lệch thời gian hoặc không nhận ra lời, nhất là bè, rap, hòa âm dày, bản cover và nhiều ngôn ngữ. Không đảm bảo mọi bài đều có karaoke khớp tuyệt đối. Với lời theo câu, chỉ những câu khớp mới được bổ sung mốc. Với lời chưa đồng bộ, chỉ áp dụng khi ghép đủ các câu. Không tạo lời mới hoặc ghi đè lời đang dùng. Từng từ theo cách Groq tách từ; không chia giả một từ thành từng âm tiết khi thiếu mốc thật.

Phân tích AI hiện chỉ cho tệp âm thanh trên thẻ đọc được bằng decoder thường, tối đa **10 phút**. Không ghi lại Spotify/SoundCloud/Drive đang phát và không phân tích DSD theo đường raw. Các nguồn đó vẫn tô chữ nếu lời có Enhanced LRC. Whisper dùng hạn mức âm thanh riêng; lỗi 429 cần chờ dịch vụ phục hồi. Chi phí phụ thuộc gói tài khoản Groq, không tự đăng ký/nâng cấp gói. Khóa không đưa vào tham số tiến trình/log; tệp WAV tạm riêng tư được xóa khi xong hoặc hủy.

Ví dụ Enhanced LRC UTF-8 cùng tên bài nhạc:

```text
[00:10.000]<00:10.000>Đường <00:10.650>về <00:11.100>nhà<00:12.000>
```

Các mốc ví dụ cần được căn đúng bản thu. Đây không phải phân tích giọng hát chạy liên tục trên CPU TrimUI. Không hứa 60 FPS/nhiệt độ khi chưa đo trên thiết bị.

## Cài đặt và OTA

Thoát hẳn NXT, giải nén ZIP đúng ROM đè lên thẻ. Giữ hồ sơ/đăng nhập/cache. Stock: Apps/NXT_Retro_Music; NextUI: Tools/tg5040/NXT Retro Music.pak; NX Redux: Tools/NXT Retro Music.pak.

Ba link OTA chính thức đã có sẵn, người dùng chỉ cần Kiểm tra cập nhật. Người phát hành tải ZIP app lên Drive, dùng CHUAN_BI_OTA.html trong gói OTA tạo update.json, rồi thay nội dung cùng file update.json đang dùng để giữ ID chính thức. Không dùng ZIP source hoặc ZIP OTA-kit làm payload. Chưa tự đăng cập nhật lên các link trực tuyến.

Giữ DSD Native/DoP, Bit-Perfect/Tự động, DAC/loa/3.5mm/Bluetooth, EQ/DSP, Spotify/SoundCloud/Drive, Groq/MyMemory/Google, cache dịch offline 256 MiB, chỉ mục thư viện tăng dần, chép Wi-Fi, 51 theme/22 font, LED, hồ sơ, hẹn giờ thoát app và tích hợp ROM.

Ý tưởng & thực hiện: Nguyễn Xuân Thông · Spring Thông · Vibe Code.
