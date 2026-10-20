# NXT Retro Music v1.9.6 — kiểm tra phát hành

Build mới từ một source chung, ba binary ARM64 O2 riêng: Stock, NextUI, NX Redux. File báo cáo BUILD_REPORT_v1.9.6_<ROM>.json liên kết SHA-256 của binary, installer, OTA-kit và các kết quả bên dưới. SDK/compiler ghim phiên bản được giữ.

## Kết quả

- Renderer SDL thực + ASan/UBSan: 1024×768 và 640×480, đủ 50 theme cũ và Auralis; 22 font; bìa toàn màn, cặp lời gốc/dịch nghiêng, Full lời không phổ. Kiểm tra màu ô chọn Auralis #DADADA, font HarmonyOS, điều khiển chọn hiệu ứng và 6 mẫu chuyển lời.
- Parser Enhanced LRC/UTF-8: thẻ lặp, sắp mốc, offset, dữ liệu hỏng, quá 64 từ và 800 mẫu lỗi. API áp dụng mốc AI kiểm tra revision và không cho job cũ thay lời mới.
- Chuyển trang: một ảnh RAM, sao chép opaque, không alpha toàn màn; chọn menu hủy chuyển động ngay; idle/ẩn/tắt màn không chạy hiệu ứng; xử lý đồng hồ tràn. Player chỉ yêu cầu nhịp 16 ms khi có chuyển động, giữ giới hạn phổ riêng và giảm nhịp khi đứng yên/tạm dừng.
- Config: lưu/đọc lại ba ROM; 6 hiệu ứng, font tham chiếu, karaoke tự động mặc định tắt, độ sáng bìa riêng và mức âm lượng; giữ mặc định an toàn khi hồ sơ thiếu trường mới.
- Âm lượng: controller thật với mixer ROM/ALSA mô phỏng, từng nấc 85→86→87; mô phỏng keymon nhận phím trước/sau app và khi giữ phím; 10 sự kiện app tăng đúng 10 mức. Phím vật lý, SELECT+tổ hợp, phím gán và HID đều qua ba cấu hình ROM. Đây không phải đo âm lượng thực của loa/DAC.
- Karaoke Groq: worker/mapping/cache/transport thật + PCM/HTTP mô phỏng. Kiểm tra đồng ý gửi âm thanh (mặc định Hủy), tắt không upload, WAV mono 16 kHz, chỉ ghép nguyên văn sau chuẩn hóa, offset CJK, không ghép lời khác, cache hỏng phục hồi nguyên trạng, đọc cache không cần khóa, 429/giới hạn phản hồi/hủy tiến trình chậm, không thử lại dồn dập, đổi bài hủy kết quả cũ, không lấy tệp local cũ khi đang ở nguồn online. Không dùng tài khoản/âm thanh thật để gọi Groq.
- Thư viện: migration, mở từ chỉ mục lưu trước worker, không quét lại khi vào ra, kiểm tra tăng dần, zero writes/probes khi không thay đổi, mẫu 2048 bài. Luồng UI/scheduler kiểm tra thêm với fixture NX Redux; mã thư viện của các ROM được đối chiếu bảo toàn.
- DSD: mỗi ROM 576 trường hợp parser/đóng gói độc lập và worker/chính sách thật với ALSA mô phỏng; giữ raw DSF/DFF Native/DoP, không thay DSP/đường phát.
- Bảo toàn: đối chiếu 825 đầu vào frontend và 676 tài nguyên với source v1.9.5 có SHA-256 đã biết; chỉ 18 đầu vào frontend có thay đổi đã duyệt, thêm cặp nxt_karaoke.c/.h; giữ 50 bảng màu cũ, tên 51 theme, 22 chỉ số font, decoder/âm thanh/ROM adapter và backend online.
- QEMU với SDK ghim: cả ba ELF ARM64 chạy --build-info / --help, báo v1.9.6 và đúng ROM.
- ZIP cuối: CRC, quyền chạy, đúng đường dẫn ROM, tài nguyên/font/giấy phép, binary đúng SHA, không dữ liệu người dùng/khóa riêng, đúng một link OTA chính thức theo ROM, OTA-kit chứa chính xác installer.
- OTA worker/installer thực + endpoint mô phỏng, cả ba ROM: nâng từ fixture v1.9.5 lên gói v1.9.6, xác minh size/SHA/ROM, xác nhận Drive, sao lưu app cũ, giữ hồ sơ/vị trí nghe/đăng nhập/cache/thư viện/hình nền, từ chối HTML/sai ROM, hủy và thu hồi tiến trình.

## Phạm vi thực tế

Chưa kiểm tra trực tiếp thiết bị TrimUI, loa/DAC/Bluetooth thật, cảm giác phím với keymon ROM đang cài, FPS/nhiệt/pin và Groq nhận diện tiếng hát thật. 60 Hz là mục tiêu nhịp vẽ, không phải kết quả đo 60 FPS. Các kiểm tra PCM/HTTP/mixer dùng mô phỏng đã nêu, không phải nghe thử trên máy.

Karaoke AI chỉ dành cho tệp trên thẻ đọc bằng decoder thường, tối đa 10 phút; không ghi luồng online được bảo vệ, không phân tích đường raw DSD. Dùng Groq Whisper sau khi đồng ý gửi âm thanh, có hạn mức/chi phí theo tài khoản. Nhận diện tiếng hát có thể lệch hoặc không khớp; không đảm bảo mọi bài đều chạy từng chữ chính xác. Chỉ áp dụng các phần khớp, giữ lời gốc và thời gian còn lại. Cache AI giới hạn riêng 16 MiB / 256 tệp, cache dịch vẫn 256 MiB.

Gói OTA đã kiểm tra cục bộ; chưa đăng ZIP và chưa thay nội dung ba update.json trực tuyến. Người phát hành cần dùng CHUAN_BI_OTA.html, đăng đúng ZIP app và thay cùng tệp update.json để giữ ID.
