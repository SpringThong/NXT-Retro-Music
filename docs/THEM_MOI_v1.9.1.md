# v1.9.1 — Stock, NextUI và NX Redux

**Cache lời dịch 256 MB trên thẻ nhớ**, không còn giới hạn 512 bản. Đây là dung lượng tệp JSON đã lưu, không phải 256 MB RAM. App chỉ đọc lời bài hiện tại, giữ hash và format cache cũ; không cần dịch lại những bài đã có cache đúng ngôn ngữ/dịch vụ/model. Đổi các lựa chọn đó dùng cache riêng.

Dọn cache chạy ở worker nền ưu tiên thấp, bộ nhớ tên file giới hạn khoảng 22 KB, nhường CPU/SD theo nhóm. Chỉ xóa các bản đã lưu cũ khi vượt dung lượng, bảo vệ bài hiện tại. Bản dịch chưa đủ vẫn giữ các câu đã dịch; offline chỉ hiển thị phần đã lưu. Giữ nguyên .userdata và thư mục cache khi cài/OTA.

**DSD nguyên bản trên cả ba ROM:** chép DSF hoặc DFF không nén vào thư mục nhạc (Wi-Fi copy cũng nhận), gắn/chọn USB DAC. Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra → Đầu ra DSD USB. Mặc định Native DSD cần driver hỗ trợ DSD thật; chỉ chọn DoP khi biết DAC hỗ trợ DoP ở tốc độ yêu cầu. DoP giữ bit DSD trong khung truyền PCM, không giải mã thành âm thanh PCM. Hỗ trợ mono/stereo DSD64–512 nếu DAC/driver/cổng USB đáp ứng. Không DST, SACD ISO, streaming DSD URL hay DSD ra loa/3,5 mm/Bluetooth.

DSD bỏ qua EQ/DSP, resample, ReplayGain, crossfade, âm lượng phần mềm và FFT. Nút âm lượng máy dùng được nếu DAC có mixer phần cứng, còn lại chỉnh trên DAC. Rút DAC tạm dừng DSD; chuyển sang PCM/online dùng lại chính sách âm thanh đã lưu. Xem thông tin bài hát & đầu ra để kiểm tra Native/DoP và tần số; DAC có màn hình nên báo DSD.

**Groq TXT trên cả ba ROM:** lưu khóa đầy đủ trong **groq-api-key.txt**, đặt tại **Music/groq-api-key.txt** trên thẻ hoặc cạnh **launch.sh** trong thư mục app. Vào Dịch lời → Khóa API Groq → Nhập từ TXT. Không gửi/chia sẻ file khóa với người khác. MyMemory và Google Cloud vẫn được giữ.

**Bài chưa có lời:** bật hiển thị lời; app ưu tiên .lrc cạnh bài, lời nhúng, cache lời gốc rồi tìm trên LRCLIB bằng tên/nghệ sĩ/thời lượng. Khi có lời và bật dịch, tự dịch sang ngôn ngữ app. LRC khớp được dùng theo thời gian; plain text chỉ là lời không đồng bộ. Không tìm được thì báo trạng thái, có thể thêm .lrc thủ công hoặc thử lại sau khi kiểm tra Wi-Fi/tên bài. Đây là tìm lời văn bản đã có; không nhận dạng giọng hát từ audio. Cần mạng lần đầu tìm/dịch; sau khi đã lưu lời gốc và bản dịch, có thể dùng offline.

Các tích hợp CPU/LED/Home/icon riêng NX vẫn chỉ ở NX; Stock/NextUI giữ cách điều khiển ROM của chính mình. Giữ độ nét, giao diện và tính năng phát nhạc có sẵn. Build/kiểm thử phần mềm không thay thế đo nhiệt/pin/độ mượt và test DAC trên máy thật.

**OTA:** dùng đúng bộ OTA cho ROM đang dùng. Mở CHUAN_BI_OTA.html, nhập link ZIP mới công khai, tạo update.json rồi thay nội dung file JSON Drive cũ của đúng ROM, giữ nguyên ID. App giữ link chính thức đã gắn, tự nhận 1.9.1 sau khi bạn đăng feed. Không đăng JSON mẫu còn chỗ REPLACE_WITH_PUBLIC_ZIP_FILE_ID.
