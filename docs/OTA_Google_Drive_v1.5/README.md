# Tạo OTA trên Google Drive cho NXT Retro Music Stock

Đây là công cụ/mẫu cấu hình, chưa phải nguồn OTA trực tuyến có sẵn.

1. Tải ZIP chính thức lên Drive; bật **Ai có đường liên kết**. Sao chép link tệp ZIP.
2. Chạy Python 3 để tạo update.json (không cần API key, service account hay token):

```sh
python3 make-ota-manifest.py --zip NXT_Retro_Music_v1.5_Stock.zip --url 'LINK_HTTPS_ZIP_CONG_KHAI' --version 1.5 --out update.json
```

Có thể thêm `--notes CHANGELOG_v1.5_Stock.md`. Công cụ kiểm tra config ROM/máy/phiên bản và ZIP CRC, tính dung lượng/SHA-256 đúng từ file. Nếu làm phiên bản sau, dùng ZIP và version của phiên bản đó.

3. Tải update.json lên Drive, cũng bật Ai có đường liên kết. Link tệp update.json là link cần nhập trong app.
4. Mỗi lần phát hành sửa nội dung **cùng file update.json** để link nguồn ổn định. Đổi link ZIP/version/hash/size đồng bộ. Đừng lấy link thư mục.
5. Nếu đóng gói nguồn sẵn cho mọi người: ghi link update.json thật vào `stock/ota-source.url` rồi `python3 tools/package-stock.py`. Không đóng gói placeholder.

Tệp update-template.json chỉ mô tả cấu trúc; hash/size/URL placeholder phải thay. Nếu bộ ngoài có update-v1.5.json, hash/size đã tính từ ZIP giao kèm, nhưng vẫn phải thay download_url bằng link ZIP thật trước khi tải manifest lên Drive. Mã nguồn ZIP không phải gói OTA; dùng **NXT_Retro_Music_v1.5_Stock.zip**.

v1.5 sẽ báo hiện tại khi nguồn cũng là v1.5; cài OTA chỉ mở khi nguồn có số phiên bản cao hơn. Không giả version để ép cài. Kiểm tra live trên v1.5 bằng bản phát hành mới hơn đúng config.

Chỉ dùng nguồn nhà phát hành tin cậy. HTTPS/SHA-256 không thay thế chữ ký phát hành. Giữ nguồn khi cài; bản cũ ở Apps/.NXT_Retro_Music.previous. Google Drive có thể giới hạn tải file công khai.
