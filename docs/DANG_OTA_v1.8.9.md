# Đăng OTA NXT Retro Music v1.8.9

Stock: https://drive.google.com/file/d/1OUrxqAs2fOnjKeqZscGcMEiX5Yv3jP5m/view?usp=sharing

NextUI: https://drive.google.com/file/d/16OuNQmcxwLE3VTTN2bUyZ0yL2EiGFeuD/view?usp=sharing

Ba link đã gắn trong đúng app. Nguồn cũ của các bản trước tự chuyển sang nguồn chính thức sau khi mở v1.8.9. Người dùng chỉ cần vào Cập nhật phần mềm khi bạn đã đăng JSON đúng.

1. Giải nén bộ OTA của đúng ROM. Đăng **ZIP cài đặt** đi kèm, nguyên vẹn, lên Drive và chia sẻ công khai Người xem. Không dùng ZIP mã nguồn hay ZIP bộ OTA làm gói cài.
2. Mở CHUAN_BI_OTA.html trong bộ đó bằng trình duyệt máy tính, dán link ZIP vừa đăng, bấm Tải update.json. HTML đã giữ đúng ROM, version=1.8.9, size và SHA-256 của ZIP.
3. Mở **chính tệp JSON có link ở trên** → Quản lý phiên bản → Tải phiên bản mới → chọn update.json đúng ROM vừa tạo. Giữ ID/link, không xóa rồi tạo tệp khác. Làm riêng cho Stock, NextUI và NX Redux.
4. Kiểm tra không cần đăng nhập: JSON và ZIP đều công khai, JSON Stock ghi rom=stock, JSON NextUI ghi rom=nextui, version=1.8.9. Giữ gói cài nguyên vẹn để hash khớp.

Đã kiểm tra lại ngày 2026-10-13: Stock và NextUI đều đang chứa JSON đúng ROM ở v1.8.8.2, size/SHA khớp gói trước đó. Khi phát hành v1.8.9, phải tạo JSON mới cho đúng ZIP mới; giữ cùng ba link nguồn đã gắn.

Những lần phát hành sau tiếp tục dùng cùng ba tệp JSON này. Đăng ZIP mới, tạo JSON mới và tải phiên bản JSON mới trên cùng ID. Nếu tạo tệp JSON mới với ID khác thì app cần nguồn mới; giữ cùng ID giúp người dùng không phải nhập link.

Bộ này chuẩn bị để bạn đăng OTA; chưa tự tải lên Drive. Để người dùng app cũ đang trỏ nguồn Stock v1.5 cũng chỉ cần bấm cập nhật, nếu bạn còn quản lý tệp nguồn cũ, tải **cùng JSON Stock v1.8.9** lên tệp đó nữa bằng Quản lý phiên bản: https://drive.google.com/file/d/1heNcSJ7Um70Z_82cS5atY093ObUZgNR-/view?usp=sharing . Sau khi lên v1.8.9, app sẽ tự chuyển sang nguồn Stock chính thức mới. Nếu không thể cập nhật tệp nguồn cũ, người dùng phải chọn nguồn đúng một lần hoặc cài ZIP v1.8.9 thủ công. App đã chạy v1.8.9 thì không cần thao tác nhập nguồn này nữa.

## NX Redux

Nguồn chính thức: https://drive.google.com/file/d/1rYgL1ao0wm3V8IEA_WxUTfJ-mstKG59r/view?usp=sharing

Dùng NXT_OTA_Google_Drive_v1.8.9_NXRedux.zip. JSON phải ghi rom=nxredux, version=1.8.9. Chỉ đăng ZIP NXT_Retro_Music_v1.8.9_NXRedux.zip làm gói cài. Thay nội dung update.json trên cùng ID; không đổi link đã gắn.
