# Đăng OTA v1.7.0 — Stock

1. Tải NXT_Retro_Music_v1.7.0_Stock.zip lên Google Drive, chia sẻ Bất kỳ ai có đường liên kết — Người xem. Dùng ZIP bộ cài, không dùng ZIP mã nguồn/bộ OTA.
2. Bộ OTA riêng Stock có update.json đã tính đúng version/rom/device/size/sha256 cho ZIP bộ cài. Chỉ thay download_url bằng link chia sẻ **tệp ZIP**. Nếu thay ZIP, chạy make-ota-manifest.py để tính lại dung lượng và SHA.
3. Đăng update.json công khai. Nếu đã có tệp cập nhật, dùng Quản lý phiên bản → Tải phiên bản mới trên tệp cũ để giữ ID/link. Giữ hai update.json và hai link riêng cho Stock/NextUI.
4. Trong app: Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm / OTA → Nguồn cập nhật. Nhập link **tệp update.json**, hoặc đọc từ .txt/.url. Không nhập link thư mục.
5. Mở trang cập nhật → Tải/kiểm tra → Cài, rồi mở lại app. Dữ liệu nghe nằm ngoài thư mục app được giữ; app cũ được sao lưu cạnh app hiện tại.

Manifest trong bộ OTA có URL giữ chỗ REPLACE_WITH_PUBLIC_ZIP_FILE_ID. **Chưa đăng OTA trực tuyến**; chỉ dùng sau khi hoàn thành các bước trên. Nguồn cập nhật đã lưu của người dùng vẫn được ưu tiên.

Ví dụ tạo lại manifest:

```sh
python3 make-ota-manifest.py --zip NXT_Retro_Music_v1.7.0_Stock.zip --url https://drive.google.com/file/d/YOUR_ZIP_ID/view --version 1.7.0 --notes CHANGELOG_v1.7.0_Stock.md --out update.json
```
