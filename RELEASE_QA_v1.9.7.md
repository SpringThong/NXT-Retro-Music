# Kiểm chứng phát hành v1.9.7

Baseline: NXT_Retro_Music_v1.9.6_Source.zip, SHA-256 `0de030e258af8662be53b983e36602ad259694232458e69f5d85eaf61994482b`.

## Build và bảo toàn

- Biên dịch mới cả ba frontend ARM64 từ source chung bằng GCC 8.3.0, O2: Stock, NextUI, NX Redux.
- `BUILD_INPUTS_v1.9.7.json` của từng ROM chứa hash source/header và binary thực tế; BUILD_REPORT liên kết hash binary, installer, OTA-kit và mọi bằng chứng dưới đây.
- So baseline: kiểm tra 827 đầu vào source/header, chỉ 14 tệp cũ được sửa trong phạm vi lời/giao diện/config/chuyển bài, thêm hai tệp helper phân tách từ. Giữ nguyên 676 tài nguyên, 50 bảng màu đầu, 51 tên theme, 22 chỉ số font và byte hai backend online.
- Loader `--build-info`/`--help` chạy được cho ba binary ARM64 qua QEMU và SDK đã ghim, báo đúng ROM/v1.9.7. Không thay thế test boot máy thật.

## Kiểm tra native và hồi quy

| Phạm vi | Bằng chứng trong logs | Kết quả |
| --- | --- | --- |
| Renderer SDL thật, 51 theme/22 font, hai độ phân giải, song ngữ/nghiêng, 10 hiệu ứng, màu/đậm/viền trên bìa sáng | v197-ui-test.json, v197-ui-test.log, ảnh qa/screens-v1.9.7 | PASS, ASan/UBSan; audio/input được mô phỏng |
| Căn tâm ba nút và bốn icon phụ; prev/next ngẫu nhiên và tuần tự cho browser/playlist/Spotify/SoundCloud | v197-shuffle-controls.json | PASS các hàm production, tâm icon theo rect thực ở 640/1024 |
| Ghép từ thiếu dấu/thiếu token, CJK, ASR token lỗi, từ không khớp, rollback cache hỏng, WAV16k, đồng ý gửi âm thanh, 429, quá cỡ, hủy/thu hồi con, cache offline không cần khóa và retry bỏ qua cache | v197-karaoke-test.json | PASS, ASan/UBSan; PCM và HTTP fixture, chưa gọi Groq thật |
| Enhanced LRC thật, API provenance và ước lượng offline, giữ lời gốc, offset, mốc dòng/từ, không đổi isSynced cho plaintext, 800 dữ liệu parser ngẫu nhiên; chuyển trang | v197-motion-lyrics.json | PASS, ASan/UBSan |
| Nhịp motion mục tiêu16ms, FFT giới hạn riêng, đứng yên/paused/ẩn/tắt màn và tràn tick | v197-frame-budget.json | PASS logic production, không phải đo FPS máy |
| Config mới/cũ, 10 hiệu ứng, các màu/đậm/viền/cỡ tên bài/ước lượng lưu–đọc và profile defaults; ba chế độ hiệu năng | v197-config-test.json | PASS cả ba ROM |
| Âm lượng thật từng nấc và đường phím/keymon/HID, giữ volume riêng từng đầu ra | v197-volume-test.json, v197-volume-keys-test.json | PASS ba ROM, mixer/ROM fixture |
| Chỉ mục thư viện warm không probe/ghi lại, thay đổi tăng dần, debounce Wi-Fi, hủy, card mất, thư viện vào lại không quét | v197-index.json, v197-library-flow-nxredux.json | PASS, ASan/UBSan |
| DSF/DFF và tám kiểu packing byte-for-byte, worker seek/pause/removal, policy Normal/Manual/Auto, rút DAC, loa/jack/Bluetooth/online | v197-dsd-test-stock.json, -nextui.json, -nxredux.json | PASS 576 trường hợp parser/packing mỗi ROM và policy/worker; DAC/ALSA fixture |
| Installer/OTA-kit CRC, SHA, quyền chạy, nguồn OTA đúng ROM, không credentials/userdata, assets/backend giữ byte, kit chứa đúng installer | v197-package-qa-*.json | PASS ba ROM |
| Worker OTA thật, bản đang cài fixture1.9.6 → manifest1.9.7, Drive confirm, sai ROM/HTML/size/SHA bị chặn, download/stage/install/backup, giữ userdata/wallpaper, hủy không treo process | v197-ota-qa-*.json | PASS cả ba ROM, ASan/UBSan; mạng Drive fixture |

## Giới hạn

Chưa test trên TrimUI thật, chưa đo FPS/nhiệt độ/pin, chưa phát với DAC/Bluetooth thật trong lần build này, chưa xác nhận độ khớp tiếng hát bằng Groq thật. Nhận diện ca sĩ có thể sai hoặc lệch. Từ nội suy/ước lượng có nhãn ƯỚC LƯỢNG; Enhanced LRC thực được ưu tiên. Có thể tắt ước lượng. Chưa đăng manifest/ZIP lên ba link Drive chính thức.

Khi test thiết bị: kiểm tra tâm nút trên ROM đang dùng; màu/đậm/viền trên bìa sáng và nhiều chi tiết; lyric thường/bilingual/full; mốc Enhanced LRC so với ước lượng và Groq; L1/R1 khi bật/tắt ngẫu nhiên; volume, DAC/DSD, Bluetooth, timer và OTA. Không cần xóa dữ liệu để nâng cấp.
