# Đăng OTA v1.6 — Stock

1. Tải **NXT_Retro_Music_v1.6_Stock.zip** lên Google Drive, bật “Bất kỳ ai có đường liên kết — Người xem”. Đây cũng là ZIP dùng cho OTA, không dùng ZIP mã nguồn hoặc ZIP bộ công cụ.
2. Mở manifest Stock/update.json trong bộ OTA. Chỉ thay **download_url** bằng link chia sẻ của TỆP ZIP vừa tải. version, rom, device, size và sha256 đã khớp ZIP gửi kèm. Đừng sửa SHA hoặc size nếu chưa thay ZIP.
3. Lưu dưới tên **update.json** rồi tải lên Drive, chia sẻ công khai. Nếu đã có tệp update.json cho ROM này, chọn **Quản lý phiên bản → Tải phiên bản mới** trên tệp cũ để giữ ID/link. Đừng xóa và tạo tệp mới.
4. Trong app Stock: **Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm / OTA → Nguồn cập nhật**. Nhập link TỆP update.json; không nhập link thư mục. Có thể chọn đọc link từ .txt/.url trong mục nguồn.
5. Mở trang cập nhật: app tự kiểm tra → thấy v1.6 → Tải/kiểm tra → xác nhận Cài. B quay lại vẫn tải nền; sau khi cài xong mở app lại.

Stock và NextUI dùng **hai tệp update.json và hai link riêng**, giữ trường rom tương ứng. Stock giữ nguồn mặc định cũ của v1.5.2; nguồn đã tự đổi trong app vẫn được ưu tiên. NextUI chưa có link mới của nhà phát hành nên cần nhập một lần nguồn NextUI. Không tự dùng link Stock cho NextUI.

Dữ liệu hồ sơ, thống kê, playlist, yêu thích, hình nền đã tải và đăng nhập nằm ngoài thư mục app, OTA không chép đè chúng. App tạo bản sao app cũ cạnh app hiện tại. Dùng mạng ổn định và giữ nguồn khi cài.

**Sau này lên bản khác:** thay ZIP trên Drive rồi thay nội dung trên cùng tệp update.json, cập nhật version/download_url/size/sha256. Có thể chạy make-ota-manifest.py trong mã nguồn hoặc bộ OTA để tính SHA và size tự động. Thay ZIP mà giữ JSON cũ sẽ bị từ chối do SHA/size khác. Manifest gửi kèm hiện có chỗ giữ link `THAY_FILE_ID_...`; chỉ dùng được sau bước 2.
