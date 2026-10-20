# Kiểm chứng phát hành v1.9.7.1

Baseline: NXT_Retro_Music_v1.9.7_Source.zip, SHA-256 `d5c42040e044661361ac9061e3ebfd61bd665941d12e9095e893cf14e10bf33e`.

## Phạm vi

Thêm 38 họ font được phép phân phối, tổng 60 lựa chọn. Trình chỉnh native có xem trước Now Playing/Full lời trên bìa hiện tại với lời mẫu song ngữ, màu/RGB/đậm/viền/bóng/cỡ chữ/hiệu ứng, bản nháp Áp dụng/Hủy. Giữ 22 ID font cũ và mọi tài nguyên cũ. Không thay mã karaoke nhận diện, pipeline âm thanh, DSD, dịch mạng, thư viện, điều khiển hay các backend online.

## Build và bằng chứng

- Cả ba frontend ARM64 Stock/NextUI/NX Redux được biên dịch mới bằng GCC 8.3.0, O2. BUILD_INPUTS từng ROM ghi hash source/header và ELF; BUILD_REPORT gắn hash ELF, ZIP cài, OTA-kit cùng bằng chứng kiểm tra.
- `logs/v1971-preservation.json`: so source chung với baseline, chỉ sửa trong phạm vi version/font/giao diện/preview. 676 tài nguyên cũ, 50 palette cũ, 51 tên theme, 22 ID font và byte hai backend được giữ.
- `logs/v1971-font-assets.json`: 38 font mới, 69 tài sản gốc; kiểm tra SHA/license OFL/commit nguồn và cmap tiếng Việt cho cả regular/italic đóng gói. SF Pro không kèm. Danh sách/provenance ở docs/FONTS_v1.9.7.1.*.

| Phạm vi kiểm tra | Bằng chứng logs | Kết quả |
| --- | --- | --- |
| SDL thật ở 1024×768 và 640×480; 51 theme, 60 font, song ngữ/nghiêng; hình qa/screens-v1.9.7.1 | v1971-ui-test.json | ASan/UBSan, input/audio fixtures |
| Bản nháp: mỗi khung input/nền kiểm tra cấu hình chưa đổi; Hủy không lưu; Áp dụng chỉ đổi trường mong muốn; lỗi lưu giữ bản nháp; RGB chính xác; font nhảy 10/quay vòng; X đổi hai chế độ ở 51 theme; lời mẫu không đổi Lyrics revision | v1971-ui-test.log, v1971-ui-test.json | PASS |
| 60 font lưu/đọc trong config và hồ sơ, giới hạn chỉ số; thiết lập màu/cỡ chữ và brightness; migration | v1971-config-test.json | PASS cả ba ROM, ASan/UBSan |
| Âm lượng từng nấc, phím/keymon/HID; ngẫu nhiên trước/sau; tâm icon | v1971-volume-test.json, v1971-volume-keys-test.json, v1971-shuffle-controls.json | PASS fixtures |
| Enhanced LRC, từ ước lượng, parser, motion; worker karaoke, cache offline, hủy, 429 | v1971-motion-lyrics.json, v1971-karaoke-test.json | PASS, ASan/UBSan; HTTP/PCM fixtures |
| Nhịp vẽ mục tiêu 16 ms, idle/paused/ẩn/tắt màn, giới hạn FFT | v1971-frame-budget.json | PASS logic, không phải đo FPS |
| Chỉ mục tăng dần và vào lại thư viện không quét; dữ liệu cache/nóng/lạnh | v1971-index.json, v1971-library-flow-nxredux.json | PASS |
| 576 parser/packing DSD mỗi ROM, worker, seek/pause/removal và policy Normal/Manual/Auto/online | v1971-dsd-test-*.json | PASS, DAC/ALSA fixtures |
| Binary ARM64 thật --build-info/--help đúng version và ROM | v1971-loader-*.json | PASS QEMU + SDK |
| ZIP/CRC/SHA/quyền chạy, đúng ROM/feed, font/tài nguyên/license, không userdata/credentials; OTA-kit chứa đúng ZIP | v1971-package-qa-*.json | PASS cả ba ROM |
| OTA worker thật với bản đang cài fixture 1.9.7 → 1.9.7.1; Drive confirm, sai ROM/HTML/size/SHA, backup/install, giữ userdata/cache, hủy | v1971-ota-qa-*.json | PASS ASan/UBSan, Drive fixture |

## Giới hạn thực tế

Chưa chạy phiên bản này trên TrimUI thật, chưa đo FPS/nhiệt độ/pin hoặc thử DAC/Bluetooth thật. Chưa gọi Groq trực tiếp hoặc kiểm tra độ khớp giọng ca sĩ. Bản này không sửa độ khớp karaoke v1.9.7; chuyển động mẫu trong preview chỉ mô phỏng hình thức và có nhãn LỜI MẪU. Không cam kết mọi bài khớp tiếng hát hay 60 FPS cố định.

OTA-kit là bộ công cụ cho người phát hành. Chưa thay JSON hoặc ZIP trên ba link Drive chính thức. Cài đúng ROM rồi test chọn font, chỉnh màu/kiểu chữ hai chế độ, Áp dụng/Hủy, hồ sơ, âm lượng, DSD/Bluetooth và OTA trên máy.
