# DSD nguyên bản — v1.9 NX Redux

1. Chép **.dsf/.dff DSD thô** vào thư mục nhạc trên thẻ, hoặc dùng copy Wi-Fi. Quét lại thư viện nếu chưa thấy bài.
2. Gắn DAC USB vào cổng DAC và chọn **Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra → USB DAC**. Bật chế độ DAC cho cổng USB nếu cần.
3. Mở **Đầu ra DSD USB**. Mặc định **Native DSD** cần driver ALSA của ROM cung cấp đúng định dạng DSD của DAC.
4. Nếu DAC hỗ trợ **DoP** mà driver không có Native DSD, chọn DoP và xác nhận tương thích. DoP đóng gói **bit DSD gốc** trong khung PCM để truyền, không giải mã DSD thành âm thanh PCM. Không chọn cho DAC chỉ hỗ trợ PCM thường.
5. Phát bài, vào **Thông tin bài hát & đầu ra** xem DSD64/128/256/512, Native/DoP, tần số nguồn/khung truyền, DAC. DAC có màn hình nên hiện DSD khi nhận đúng.

DSD luôn dùng đường nguyên bản dù chế độ âm thanh đang lưu là Thường, Bit-Perfect hay Tự động. Khi chuyển bài PCM/online, chính sách đã lưu được dùng lại. EQ, ReplayGain, crossfade, âm lượng phần mềm và FFT không tác động lên DSD. Dùng âm lượng DAC; nút máy cũng dùng được nếu DAC có mixer phần cứng. Không tạo thông báo bắt buộc chỉnh âm lượng trên DAC.

Rút DAC sẽ tạm dừng DSD. Cắm lại, chọn USB DAC và bấm phát để nghe tiếp. Không đưa dữ liệu DSD thô ra loa máy, 3,5 mm hay Bluetooth. Hỗ trợ DSF và DFF không nén, mono/stereo, DSD64–512 nếu DAC/driver/cổng USB đáp ứng đúng tốc độ; không DST, SACD ISO hay streaming DSD trực tiếp từ URL. Nhạc tải về thẻ có thể nghe như file nội bộ. Giữ ảnh album ID3 và LRC ngoài file.

Đọc khối giới hạn, không nạp cả bài vào RAM, không chạy giải mã float/resample/FFT DSD. Giữ tài nguyên chữ/hình/giao diện v1.8.9.1. Build và kiểm thử phần mềm không xác nhận độ nóng, pin hoặc ổn định trên DAC thật.

## OTA

Mở **CHUAN_BI_OTA.html** trong bộ OTA trên máy tính, nhập link ZIP v1.9 công khai rồi tạo **update.json**. Thay nội dung file JSON cũ bằng Quản lý phiên bản → Tải phiên bản mới, giữ nguyên ID:

https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing

App sẽ tự nhận bản mới sau khi feed được đăng đúng. Không đăng JSON mẫu còn REPLACE_WITH_PUBLIC_ZIP_FILE_ID. Gói này chưa tự đăng/thay nội dung Google Drive.

NextUI và Stock vẫn v1.8.9; sau khi test NX ổn mới thêm DSD và Groq TXT cho hai ROM đó, không chuyển các tích hợp NX riêng.
