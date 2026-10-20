# Đăng OTA Google Drive — NXT Retro Music v1.5.1 Stock

1. Tải **NXT_Retro_Music_v1.5.1_Stock.zip** lên Drive. Chia sẻ **Ai có đường liên kết → Người xem**, chép link tệp ZIP.
2. Mở **update-v1.5.1.json** có trong bộ công cụ phát hành. Chỉ sửa **download_url** thành link ZIP ở bước 1. SHA-256 và dung lượng đã tính đúng từ ZIP gửi kèm.
3. Lưu UTF-8 thành **update.json**, kiểm tra không có đuôi `.txt`. Tải lên Drive dưới dạng tệp JSON bình thường, chia sẻ công khai quyền Người xem. Không chuyển thành Google Docs.
4. Nhập **link TỆP update.json** vào app: v1.5 cũ ở Cập nhật OTA trên trang chủ; v1.5.1 ở **Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm / OTA**.
5. Kiểm tra → tải/kiểm tra → xác nhận cài → mở lại app. v1.5.1 là bản mới hơn v1.5; app đang v1.5.1 sẽ không cài lại cùng số phiên bản.

Không nhập link thư mục hoặc link ZIP vào ô nguồn. Không dùng ZIP mã nguồn/ZIP công cụ OTA làm bộ cài. Không cần API key hay tài khoản lập trình.

Các lần sau giữ **cùng tệp update.json** bằng **Quản lý phiên bản → Tải phiên bản mới** trên Drive máy tính, để link không đổi. Tạo manifest mới có version lớn hơn, ZIP link, size và sha256 đồng bộ. Không xóa/tạo lại update.json.

Nếu đã sửa ZIP hoặc đóng gói sẵn nguồn OTA, phải tạo lại manifest từ ZIP mới:

```sh
python3 make-ota-manifest.py --zip NXT_Retro_Music_v1.5.1_Stock.zip --url 'LINK_HTTPS_TEP_ZIP' --version 1.5.1 --notes CHANGELOG_v1.5.1_Stock.md --out update.json
```

File JSON được tạo sau khi đóng gói release, nên thư mục hướng dẫn trong mã nguồn/bộ cài có thể chỉ chứa README và script. File **update-v1.5.1.json** với hash thật nằm trong **NXT_OTA_Google_Drive_v1.5.1.zip** gửi riêng.

Dùng nguồn của người phát hành tin cậy. Kiểm tra quyền công khai của cả JSON và ZIP. Giữ nguồn khi cài; dữ liệu người nghe ngoài thư mục app; bản cũ ở Apps/.NXT_Retro_Music.previous. Drive có thể giới hạn tải tệp công khai.

[Chia sẻ tệp](https://support.google.com/drive/answer/2494822?hl=vi) · [Quản lý phiên bản](https://support.google.com/drive/answer/2409045?hl=vi).
