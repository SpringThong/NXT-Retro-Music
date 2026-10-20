# NXT Retro Music v1.9 — NX Redux

- Phát DSD nguyên bản từ DSF/DFF DSD thô nội bộ, mono/stereo, DSD64/128/256/512 thuộc họ 44,1 hoặc 48 kHz nếu DAC/driver hỗ trợ.
- Mặc định Native DSD: chỉ mở ALSA hw có đúng định dạng DSD, tốc độ và số kênh. DoP là lựa chọn riêng phải xác nhận DAC hỗ trợ; không suy đoán DoP từ khả năng PCM thông thường.
- Không giải mã DSD thành PCM, resample, EQ/DSP, ReplayGain, âm lượng phần mềm hay crossfade. Không chạy phân tích phổ/FFT DSD, giảm tải CPU và bộ nhớ với đọc file theo khối giới hạn.
- Rút DAC tạm dừng DSD, không phát dữ liệu thô qua loa, 3,5 mm hoặc Bluetooth. Khi chuyển sang PCM/online, dùng lại chính sách Thường/Bit-Perfect/Tự động đã lưu.
- Thông tin bài hát và đầu ra hiển thị DSD nguồn, Native/DoP, tần số gốc và tần số khung DoP. Giữ ảnh album ID3, lời LRC ngoài file, hàng đợi và vị trí nghe dở.
- Copy Wi-Fi nhận .dsf/.dff. Giữ Groq TXT, CPU, Home/task manager, LED, Tools icon và các tính năng v1.8.9.1 đang có.
- OTA v1.9 cho NX Redux dùng link update.json chính thức cũ. Stock/NextUI vẫn là v1.8.9, chưa build thêm.

Giới hạn: cần DAC USB và driver/ROM thật sự tương thích. Không DST nén, SACD ISO, streaming DSD URL, hay chuyển DSD sang PCM/đầu ra analog/BT. Nút âm lượng máy dùng được khi DAC có mixer phần cứng, còn lại dùng âm lượng trên DAC. Phần mềm đã có kiểm thử và build ARM64; DAC, dropouts, nhiệt độ và pin cần kiểm tra trên máy thật.

Sau khi NX ổn mới tích hợp DSD và Groq TXT vào NextUI/Stock. Không chuyển các tích hợp riêng CPU/LED/Home/icon NX sang hai ROM đó.
