# NXT Retro Music v1.9.3 — unified source

Một bộ source cho **Stock, NextUI và NX Redux** trên TrimUI Brick Pro. Ba frontend dùng chung mã hiện tại, cùng các sửa đổi v1.9.2 và thư viện tăng dần v1.9.3; mỗi ROM giữ tích hợp riêng và link OTA chính thức. Tất cả ba binary/fingerprint trong source này được đồng bộ với cùng mã nguồn.

Đọc **BUILD_v1.9.3.md** để chuẩn bị Linux/WSL x86_64, build và tìm ZIP cài/OTA. Lệnh nhanh:

```sh
python3 tools/build-release.py --rom all --check
python3 tools/build-release.py --rom all
```

Chọn riêng `--rom stock`, `--rom nextui` hoặc `--rom nxredux` khi chỉ cần một ROM. SDK ghim checksum tải tự động lần đầu; các dependency/frontend input và runtime nền đã giữ trong source. Không cần ghép source cũ, không cần Docker.

Tài liệu dùng app: docs/HUONG_DAN_NXT_Retro_Music_v1.9.3_<ROM>.md; thư viện: docs/LIBRARY_v1.9.3.md; DSD: docs/DSD_v1.9.3_<ROM>.md. Xem RELEASE_QA_v1.9.3.md và BUILD_REPORT_v1.9.3_<ROM>.json cho phạm vi kiểm tra. Báo cáo/tài liệu bản cũ được giữ làm lịch sử.

Không có khóa API hay dữ liệu người dùng trong source. Giữ giấy phép vendor/GPL/LGPL và relink theo tài liệu kèm theo. Link OTA có sẵn; publisher vẫn đăng ZIP cài rồi thay nội dung cùng update.json trên Drive, giữ ID. Không dùng ZIP source làm gói cài app hay payload OTA.
