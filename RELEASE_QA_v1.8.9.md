# Kiểm chứng v1.8.9 — Stock, NextUI và NX Redux

Groq đã tích hợp; MyMemory và Google Cloud được giữ. Stock/NextUI đã biên dịch ARM64 và chạy --build-info/--help bằng QEMU; helper nghe nhạc giữ nguyên và kiểm tra thoát. NX Redux cũng đã biên dịch ARM64 và qua loader --build-info/--help bằng QEMU.

Production worker + fake HTTP fixture, ASan/UBSan: giữ riêng khóa, đổi nguồn dùng lại cache, 14 đích, tiếng Anh/Trung/Hàn/Nhật, nhóm 48 dòng thành 2 yêu cầu, ID đảo thứ tự, từ chối dòng thiếu/trùng/ID sai/đầu ra bị cắt, lỗi khóa/model/429, Retry-After, hạn mức ngày xuyên đổi bài, pause nền/tắt màn hình, kết quả cũ không phát ra sau chuyển bài, thoát và thu hồi helper.

Native UI 50 theme ở 640×480 và 1024×768, song ngữ Now Playing và chỉ chữ dịch nghiêng trong Y, phím/menu/giao diện/khởi động/hồ sơ. Kiểm thử xóa hồ sơ dùng SQLite thật và kiểm tra giữ dữ liệu chung.

2.124 tệp nguồn âm thanh/CPU/render/hồ sơ/tài nguyên/font/online backend giữ nguyên từ source v1.8.8.2. Chỉ thay nguồn dịch, OTA, số phiên bản và thông tin thay đổi; thêm nxt_groq.inc. Không có phép đo nhiệt/pin trên TrimUI.

OTA production worker với ZIP cài đặt đầy đủ cuối cùng: mô phỏng cài từ 1.8.8.2 lên 1.8.9, nguồn cũ tự chuyển, nguồn tùy chọn giữ, khôi phục nguồn chính thức, Google Drive confirmation, hash/size/ROM sai bị từ chối, sao lưu app cũ, giữ userdata/khóa/cache/hồ sơ, hủy và thu hồi helper. Ba ZIP cuối đã được đối chiếu nguồn chính thức đúng ROM, version, ELF, CRC, size/SHA, khóa riêng không kèm và HTML tạo manifest. Luồng tải vẫn dùng HTTP fixture; chưa tự đăng lên Drive.

Chưa kiểm tra Groq trực tiếp với API key thật. Chưa kiểm tra nhiệt/pin/âm thanh/độ trễ trên máy thật.

Nguồn Stock: https://drive.google.com/file/d/1OUrxqAs2fOnjKeqZscGcMEiX5Yv3jP5m/view?usp=sharing

Nguồn NextUI: https://drive.google.com/file/d/16OuNQmcxwLE3VTTN2bUyZ0yL2EiGFeuD/view?usp=sharing

Kiểm tra trực tiếp lại ngày 2026-10-13: cả hai trả JSON v1.8.8.2, đúng package/device/ROM; size/SHA Stock khớp report 1.8.8.2 Stock (111651374 byte, 891fd587...), NextUI khớp report NextUI (109290224 byte, 67fda595...). Người dùng đã sửa nội dung Stock trên cùng ID sau lần kiểm tra trước. Khi phát hành, cần đăng ZIP v1.8.9 và JSON mới lên cùng từng tệp; chưa tự đăng lên Drive.

Bổ sung kiểm thử Groq đầu ra bị cắt: tự giảm nhóm và phục hồi, không lặp mãi nhóm quá lớn; đầu ra một dòng vượt giới hạn giữ lời gốc và dừng thử tự động dòng đó. ASan/UBSan.

Kiểm thử NxtOTA_init với nguồn lấy từ chính ZIP cuối: lần đầu, nguồn cũ v1.5, nguồn lưu sai ROM, nguồn tùy chọn mới, khôi phục nguồn chính thức và đổi link chính thức trong bản sau. Không gửi mạng trong thử nghiệm khởi tạo; khóa dịch và đăng nhập giữ nguyên.

## Bổ sung NX Redux v1.8.9

Nguồn chính thức: https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing. Kiểm tra trực tiếp trả đúng package/device, rom=nxredux, version=1.8.8.2; cần đăng JSON mới khi phát hành.

Gói NX Redux dùng Tools/NXT Retro Music.pak, launcher và adapter firmware riêng. Adapter và nguồn adapter byte-identical với v1.8.8.2. QEMU chạy chính ELF release v1.8.9; fixture launcher kiểm tra cold start/reattach không nhân đôi app, nhả PCM firmware, giữ dữ liệu và từ chối sai thiết bị/ROM. Audio fixture dùng đúng ALSA parser, A2DP discovery, đóng PCM trước cấu hình lại, speaker/Bluetooth/DAC handoff, local/Spotify/SoundCloud PCM, retry transport và gain/mute NX Redux; không phải thử tai nghe/DAC vật lý.

Native UI NX Redux: 50 theme tại 640×480 và 1024×768, song ngữ và italic chỉ trong Y, điều khiển/hồ sơ/performance, ASan/UBSan. Timer fixture kiểm tra hết giờ thoát toàn bộ app kể cả nền/tắt màn hình, lưu vị trí và tùy chọn. Firmware tự xử lý idle/suspend sau app thoát; chưa đo deep sleep.

Production OTA NX Redux cài chính ZIP cuối qua HTTP fixture, giữ hồ sơ/khóa/cache/wallpaper, backup app cũ, kiểm tra ROM/size/hash, Google Drive confirmation và cancellation. NxtOTA_init dùng nguồn từ ZIP cuối, kiểm tra lần đầu/nguồn cũ/custom/khôi phục/thay link chính thức. Source chung chứa đủ ba native build + input fingerprint + QA. Các bộ cài Stock/NextUI đã giao giữ nguyên SHA-256.
