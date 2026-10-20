# Đăng OTA NXT Retro Music v1.9.7.2 — __LABEL__

Đây là bộ dành cho **__LABEL__**, TrimUI Brick Pro. Giải nén bộ này trên máy tính trước khi sử dụng. Gói cài bên trong là đúng bản native v1.9.7.2; giữ nguyên ZIP này khi đăng OTA.

## 1. Đăng ZIP cài đặt

Đăng đúng tệp **NXT_Retro_Music_v1.9.7.2___LABEL__.zip** bên trong bộ này lên Google Drive.

Chia sẻ → Quyền truy cập chung → Bất kỳ ai có đường liên kết → Người xem. Cho phép tải xuống, rồi sao chép link **tệp ZIP**, không dùng link thư mục.

Không đăng ZIP bộ OTA làm gói cài. Không dùng ZIP mã nguồn và không giải nén/nén lại ZIP cài đặt.

## 2. Tạo file cập nhật

Mở **CHUAN_BI_OTA.html** bằng Edge hoặc Chrome. Trang chạy trên máy tính và không cần Python.

Dán link ZIP ở bước 2 của trang, bấm **Tải update.json**. Trang tự giữ phiên bản, ROM, máy, dung lượng và SHA-256 của gói cài. File mới được lưu vào nơi trình duyệt tải xuống; nếu đã có update.json cũ, chọn đúng file vừa tạo.

Có thể mở phần Kiểm tra ZIP để đối chiếu tệp cài trên máy tính trước khi đăng. Trang không kiểm tra quyền chia sẻ hoặc tải gói từ Drive.

Hai JSON trong thư mục **templates** chỉ là mẫu có chỗ điền. Dùng file **update.json vừa tạo bằng trang**, không đăng mẫu chưa thay link.

## 3. Đăng update.json

Với nguồn chính thức đã có: mở đúng link update.json của __LABEL__ bên dưới, chọn Quản lý phiên bản → Tải phiên bản mới và chọn update.json vừa tạo. Giữ nguyên ID/link của tệp chính thức; không tạo tệp mới để thay nguồn cho người dùng. Bật quyền xem/tải công khai.

__PUBLISHER_NOTE__

Dán link update.json vào bước 3 của CHUAN_BI_OTA.html, bấm **Tải tệp nguồn OTA**. Kết quả là **NXT_OTA___ROM__.txt**, chứa một dòng HTTPS của update.json.

Stock, NextUI và NX Redux phải dùng nguồn update.json riêng cho từng ROM. Stock và NextUI v1.9.7.2 có ZIP và mã kiểm tra khác nhau; NX Redux dùng bộ riêng theo phiên bản được phát hành.

## 4. Người dùng chỉ cần kiểm tra cập nhật

App __LABEL__ đã gắn đúng nguồn chính thức. Vào **Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm → Kiểm tra cập nhật**; không cần nhập link hay chép tệp nguồn OTA.

App phải đúng ROM và nằm ở __APP_PATH__. Nguồn cũ chưa được chọn thủ công sẽ chuyển sang nguồn chính thức. Nếu trước đây đã chọn nguồn tùy chỉnh, chọn **Khôi phục nguồn nhà phát hành** để quay về link chính thức.

Chỉ khi chủ động dùng nguồn tùy chỉnh khác: có thể nhập link update.json hoặc đọc NXT_OTA___ROM__.txt trong mục Nguồn cập nhật. Giữ tính năng này cho người quản lý nguồn riêng.

## 5. Kiểm tra bản phát hành

Mở link ZIP và link update.json trong cửa sổ riêng tư, kiểm tra có thể tải không cần đăng nhập.

Trên app **cũ hơn v1.9.7.2** có hỗ trợ OTA: vào cập nhật, chờ hiện v1.9.7.2, chọn **Tải**, chờ xác minh, rồi chọn **Cài**. Cài sẽ dừng nhạc và đóng app. Mở lại app, kiểm tra Thông tin ứng dụng hiện v1.9.7.2 và hồ sơ/đăng nhập cũ còn nguyên.

Máy **đã cài v1.9.7.2** sẽ báo chưa có phiên bản mới hơn khi đọc nguồn v1.9.7.2. Đây là kết quả đúng, không phải OTA bị thiếu. Không tăng giả version trong JSON để ép cài: version phải khớp config và binary của ZIP.

App kiểm tra đúng app, ROM, máy, dung lượng, SHA-256 và nội dung ZIP trước khi cài. Bản cũ được sao lưu cạnh app; dữ liệu cá nhân nằm ngoài thư mục được thay. Cần khoảng 400 MiB trống cho gói hiện tại và giữ nguồn ổn định lúc cài.

Bước phát hành này được kiểm tra bằng gói cài và mã OTA thật trong môi trường test; chưa có link công khai của bạn để kiểm tra tải qua Internet hoặc máy vật lý.

## Các lần phát hành sau

Đăng ZIP cài mới trước. Tạo update.json từ đúng ZIP/version/ROM mới, rồi dùng **Quản lý phiên bản → Tải phiên bản mới** trên chính tệp update.json hiện tại trong Drive. Giữ tệp nguồn ổn định để máy cũ tiếp tục theo dõi cùng link.

CHUAN_BI_OTA.html trong bộ này chỉ dành cho **v1.9.7.2 __LABEL__**. Với bản mới, dùng bộ OTA của bản mới hoặc chạy công cụ Python kèm theo:

```text
python make-ota-manifest.py --rom __ROM__ --zip NXT_Retro_Music_vVERSION___LABEL__.zip --url LINK_HTTPS_CUA_ZIP --version VERSION --notes CHANGELOG.md --out update.json
```

Công cụ Python kiểm tra config, ROM, máy, version, CRC và tự tính dung lượng/SHA-256. Không lấy hash của bộ OTA hoặc mã nguồn.

## Khi gặp lỗi

- **Chưa có phiên bản mới:** kiểm tra app đang cài số nào và update.json đã trỏ tới v1.9.7.2 chưa.
- **Sai ROM:** dùng đúng bộ __LABEL__ và đúng nguồn __ROM__.
- **Lỗi dung lượng/SHA:** đăng nguyên ZIP bên trong bộ; tạo lại JSON từ đúng bản ZIP.
- **Trang HTML / yêu cầu đăng nhập:** kiểm tra chia sẻ công khai cho cả ZIP và JSON, dùng link tệp.
- **Hạn mức tải của Drive:** thử lại khi nguồn tải được hoặc chuyển ZIP sang máy chủ HTTPS có thể tải công khai và tạo lại JSON.
- **Nguồn không đổi:** nhập/đọc lại link trong mục Nguồn cập nhật.
- **Cần log:** START trong Thông tin, hướng dẫn & cập nhật xuất NXT_App_Log.txt.

Hướng dẫn Google Drive chính thức: [Chia sẻ tệp](https://support.google.com/drive/answer/2494822) · [Quản lý phiên bản](https://support.google.com/drive/answer/2409045).

Ý tưởng và thực hiện: **Nguyễn Xuân Thông · Spring Thông · Vibe Code**.

