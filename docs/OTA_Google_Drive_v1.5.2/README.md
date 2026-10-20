# Đăng OTA — NXT Retro Music v1.5.2 Stock

1. Tải **NXT_Retro_Music_v1.5.2_Stock.zip** lên Drive, chia sẻ **Ai có đường liên kết → Người xem**, lấy link tệp ZIP.
2. Mở **update.json** trong bộ công cụ này, chỉ sửa **download_url** thành link ZIP bước 1. Version/size/sha256 đã tính đúng bộ cài gửi kèm.
3. Mở [tệp update.json Stock hiện tại](https://drive.google.com/file/d/1heNcSJ7Um70Z_82cS5atY093ObUZgNR-/view?usp=sharing) → **Quản lý phiên bản → Tải phiên bản mới** → chọn JSON vừa sửa. Không xóa/tạo lại tệp này để giữ ID/link app đã nhúng. Chia sẻ JSON công khai quyền Người xem.
4. v1.5.1: nhập link JSON Stock trong OTA một lần → Kiểm tra → Tải → Cài. Sau khi lên v1.5.2, mở OTA sẽ tự kiểm tra; chỉ 4 mục, chọn Tải rồi Cài khi có bản mới. B quay về vẫn tải.

**Lần sau:** thay manifest trên cùng tệp JSON; version/download_url/size/sha256 phải khớp ZIP mới. NextUI dùng tệp JSON/link và gói riêng, không ghi NextUI lên nguồn Stock này.

Trong kit có cả update-v1.5.2.json giống update.json để đối chiếu. Hash khớp **ZIP cài đặt**, không phải ZIP mã nguồn/ZIP kit. Không cần API key. App dùng link tệp, không dùng link thư mục.

Nếu tự thay ZIP, tạo lại JSON từ ZIP đó:

```sh
python3 make-ota-manifest.py --zip NXT_Retro_Music_v1.5.2_Stock.zip --url 'LINK_HTTPS_TEP_ZIP' --version 1.5.2 --notes CHANGELOG_v1.5.2_Stock.md --out update.json
```

Nguồn đã nhúng đúng ID; môi trường build chưa tải trực tiếp được JSON Drive này. Trước khi public, kiểm tra JSON/ZIP tải được trong cửa sổ không đăng nhập rồi thử trên máy. Không đổi hash sau khi đăng nếu ZIP không đổi.
