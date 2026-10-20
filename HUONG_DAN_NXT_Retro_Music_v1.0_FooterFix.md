# NXT Retro Music v1.0 — Bản sửa thanh nút / NextUI

Gói **NXT_Retro_Music_v1.0_FooterFix_NextUI.zip** dành cho TrimUI Brick Pro chạy NextUI. About tiếp tục hiển thị **v1.0** như đã chốt.

Thanh cuối của Đang phát có 7 ô cùng chiều cao và khoảng cách. Tên nút và icon được căn giữa; tên chức năng nằm ở hàng dưới, cùng đường chân chữ. SELECT và START hiển thị đầy đủ. Nút A cập nhật icon và nhãn Phát / Tạm dừng theo trạng thái thật. Nếu đã đổi nút trong Cài đặt, tên nút vật lý vẫn được hiển thị đúng.

## Cập nhật

1. Trong app cũ, chọn **START → Dừng và lưu vị trí**, sau đó **START → Thoát giao diện**. Nếu vẫn còn phiên nghe nền, khởi động lại máy trước khi chép bản mới.
2. Sao lưu thư mục app cũ và dữ liệu `.userdata/shared/nxt-retro-music/` trên thẻ.
3. Giải nén gói mới. Chép thư mục **NXT Retro Music.pak** vào **Tools/tg5040/**, thay thư mục app cũ. Đường dẫn đúng: **Tools/tg5040/NXT Retro Music.pak/launch.sh**.
4. Giữ lại `.userdata/shared/nxt-retro-music/` để tiếp tục dùng playlist, yêu thích, EQ, font và vị trí nghe.
5. Mở app, nhấn **Y** ở menu để vào Đang phát và xem thanh nút mới. Bài được phục hồi ở trạng thái tạm dừng; nhấn **A** để phát.

## Các ô trên thanh cuối

| Nút mặc định | Chức năng |
| --- | --- |
| L1 | Bài trước |
| A | Phát / tạm dừng |
| R1 | Bài tiếp |
| X | Danh sách đang phát / hàng đợi |
| Y | Lời bài hát toàn màn hình |
| SELECT, nhấn rồi thả | EQ / DSP |
| START | Tùy chọn nhanh |

**L2** tiếp tục đổi màu phổ theo chu kỳ mint → xanh dương → vàng → hồng → tím → tắt → mint. **R2** thêm / bỏ yêu thích. Nhấn ngắn **Nguồn** tắt / bật màn hình và giữ phiên nghe hoạt động; giữ **SELECT** khoảng 0,9 giây khóa / mở khóa phím.

Bản sửa dùng cùng bộ **16 font** và font dự phòng offline cho tên bài và lời tiếng Hàn, Nhật, Trung, Thái và các nhóm chữ đã hỗ trợ. Hướng dẫn đầy đủ nằm trong **Thông tin → Hướng dẫn sử dụng** của app và README trong gói mã nguồn.

## Đã kiểm tra

- Giao diện SDL native **1024 × 768**: 19 lượt chụp, đủ 16 font; phát và tạm dừng; tiếng Việt / English; cỡ chữ 85% / 120%; giao diện tối / sáng; ánh xạ nút với tên dài. Đã xem ảnh để kiểm tra khoảng cách và chữ/icon.
- Build release **ELF64/AArch64, tg5040** dành cho NextUI; kiểm tra binary thật, quyền chạy, nội dung ZIP và checksum. Gói release không chứa kênh kiểm thử debug.
- Bộ font/font dự phòng được giữ nguyên từng byte so với bản đã chốt. Các kiểm tra âm thanh, lưu dữ liệu và vòng đời v1.0 được lưu trong gói mã nguồn làm kết quả nền; không chạy lại toàn bộ cho thay đổi bố cục này.

Bạn đã xác nhận v1.0 chạy trên Brick Pro. Ảnh xem trước của bản sửa được chụp từ app native trong môi trường kiểm thử, dùng nhạc mẫu tự tạo; bản sửa thanh nút chưa được thử trực tiếp trên Brick Pro tại đây.

**NXT_Retro_Music_v1.0_FooterFix_Source.zip** chứa mã nguồn tương ứng, dependency, giấy phép, object/thư viện để relink, log và ảnh kiểm tra. Xem README để build lại trên Linux / WSL.
