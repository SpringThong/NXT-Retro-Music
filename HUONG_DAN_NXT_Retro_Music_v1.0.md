# NXT Retro Music v1.0 — NextUI / TrimUI Brick Pro

Bản cập nhật từ 0.3.0 theo năm yêu cầu đã chốt ngày 05/10/2026. Giao diện than đen / xanh mint, thư viện, playlist, DSP và bộ 16 font được giữ nguyên. Phiên bản hiển thị: **v1.0**.

## Cập nhật từ bản đang dùng

1. Trên máy, mở NXT Retro Music. Nhấn **START → Dừng và lưu vị trí**, rồi **START → Thoát giao diện**. Nếu vẫn còn phiên nghe nền, khởi động lại máy trước khi chép bản mới.
2. Sao lưu thư mục app cũ và thư mục dữ liệu `.userdata/shared/nxt-retro-music/` trên thẻ nhớ.
3. Giải nén **NXT_Retro_Music_v1.0_NextUI.zip**. Chép thư mục **NXT Retro Music.pak** vào **Tools/tg5040/** trên thẻ, thay thư mục app cũ. Đường dẫn sau khi chép là **Tools/tg5040/NXT Retro Music.pak/launch.sh**; tránh lồng thêm một thư mục `.pak` bên trong.
4. Giữ nguyên thư mục dữ liệu: playlist, yêu thích, EQ, font và vị trí nghe đang lưu sẽ tiếp tục được sử dụng.
5. Gắn lại thẻ, mở **Tools → NXT Retro Music**. Vào **Thông tin** để kiểm tra phiên bản v1.0. Bài nghe tiếp sau khi khởi động vẫn ở trạng thái tạm dừng, đợi bạn nhấn **A**.

Gói này dành cho **NextUI**, target ARM64 `tg5040`, cùng nền build với bản 0.3.0 bạn đã chạy trên Brick Pro. Có sẵn binary để cài; bạn không cần tự biên dịch.

## Điều khiển

| Nút mặc định | Chức năng |
| --- | --- |
| **Nguồn, nhấn ngắn** | Tắt / bật màn hình bằng đèn nền. Khi giao diện app đang mở, nhạc và xử lý âm thanh vẫn chạy. Bài đang tạm dừng vẫn giữ nguyên trạng thái. |
| **Nguồn, giữ lâu** | Tắt máy theo cơ chế NextUI; vị trí nghe hiện tại được lưu. |
| **L2**, trong Đang phát | Mỗi lượt nhấn đổi sang một màu: **mint → xanh dương → vàng → hồng → tím → tắt → mint**. Từ màu mint đang hiển thị mặc định, lần nhấn đầu chuyển sang xanh dương. Giữ nguyên kiểu phổ đang lưu. |
| **R2**, trong Đang phát | Thêm / bỏ yêu thích bài hiện tại, kể cả khi tạm dừng. Biểu tượng tim và danh sách yêu thích cập nhật ngay. |
| **A** | Phát / tạm dừng; chọn mục trong menu. |
| **B** | Về menu chính; giữ trạng thái đang nghe. |
| **L1 / R1** | Bài trước / bài tiếp. |
| **Trái / Phải** | Tua; giữ để tua liên tục. |
| **X** | Mở hàng đợi. |
| **Y** | Mở lời toàn màn hình. Ở menu chính: mở Đang phát / nghe tiếp. |
| **SELECT**, nhấn rồi thả | Mở EQ / DSP. |
| **SELECT**, giữ khoảng 0,9 giây | Khóa / mở khóa phím. Nút nguồn vẫn dùng được khi khóa. |
| **START** | Mở tùy chọn nhanh. |

Khi màn hình tắt, một nút bất kỳ có thể bật màn hình; lượt nhấn này chỉ dùng để đánh thức, không đồng thời phát / tạm dừng hoặc đổi màn hình. Hẹn giờ và chuyển bài vẫn chạy khi màn hình tắt. Tắt màn hình giữ CPU và âm thanh hoạt động nên không phải chế độ ngủ sâu.

**Cài đặt → Điều khiển** cho phép đổi cả L2 và R2. **Thông tin → Hướng dẫn sử dụng** hiển thị ánh xạ nút đang lưu bằng tiếng Việt hoặc English. Nút nguồn và thao tác giữ SELECT vật lý vẫn cố định.

## Thông tin và biểu tượng thiết bị

**Thông tin** chỉ có một lựa chọn: **Hướng dẫn sử dụng**. Các thông tin còn lại hiển thị cố định: NXT Retro Music, v1.0, Vibe Code, ý tưởng Nguyễn Xuân Thông, phát triển ChatGPT 6.1 Sol, **MoMo 0339838267** và **Vietcombank 0581000786219**.

Biểu tượng trạng thái tai nghe chỉ hiện khi jack **3,5 mm** được cắm. DAC dùng biểu tượng USB riêng khi hệ thống nhận thiết bị âm thanh USB; nếu cắm cả hai thì hiển thị cả hai. Rút thiết bị sẽ cập nhật trạng thái và xóa nhận diện DAC cũ. Dòng đầu ra trong Đang phát cho biết thiết bị đang được sử dụng. Logo tai nghe của NXT ở góc trái vẫn là logo ứng dụng.

## Font và lời bài hát

Giữ đủ **16 font lựa chọn**, font dự phòng CJK và các bộ chữ khác dùng offline. Font dự phòng áp dụng cho cả tên bài, nghệ sĩ, album, danh sách và lời bài hát. Hàn, Nhật, Trung giản thể / phồn thể, chữ Hán và tiếng Thái đã được kiểm tra lại trên mã ARM64.

Đặt file **.lrc UTF-8**, cùng tên với bài nhạc và cạnh file nhạc. Ví dụ `Bai_hat.flac` và `Bai_hat.lrc`. Khi thêm nhạc, vào Thư viện → START → Quét lại thư viện.

## Kết quả kiểm tra bản v1.0

- Binary **ELF64/AArch64**, bản release; không kèm kênh chèn nút, thiết bị giả hay chế độ kiểm thử.
- Font trên ARM64 qua QEMU: **576 lượt render**, **48 so sánh NFC**, kiểm tra glyph, cụm Thái và tìm kiếm tiếng Việt đạt.
- Decoder / DSP / lưu dữ liệu trên ARM64: 10 mẫu codec, tua, ReplayGain, EQ 10/15/31 band, limiter, preset, hồ sơ đầu ra và playlist Unicode đạt.
- Giao diện native SDL trong môi trường kiểm thử offline: nhạc tiếp tục tiến thời gian khi tắt màn hình; giữ tạm dừng, khóa phím và thao tác đánh thức; đủ chu kỳ L2; R2 và yêu thích lưu qua khởi động; giữ kiểu / màu phổ; chuyển bài và hẹn giờ thực **60 giây** khi màn hình tắt; giữ nguồn đi tới luồng tắt máy; About và hướng dẫn hiển thị đúng.
- Trạng thái jack / DAC được kiểm tra bằng thiết bị giả trong bản debug. Ảnh chụp trong mã nguồn là ảnh app native trong môi trường kiểm thử, không phải ảnh chụp trên Brick Pro.

Bạn đã xác nhận bản 0.3.0 chạy trên Brick Pro. Bản **v1.0 chưa được thử trực tiếp trên phần cứng** tại đây; sau khi cài, ưu tiên thử nhấn ngắn nút nguồn khi đang phát, rồi cắm / rút tai nghe và DAC để xác nhận tương tác thật của NextUI trên máy.

## Mã nguồn và log

**NXT_Retro_Music_v1.0_Source.zip** chứa mã nguồn đúng bản build, overlay, nguồn dependency, thư viện / object ARM64 để relink, giấy phép, script build và kiểm thử, log đạt và checksum. Xem README trong gói để build bằng WSL / Linux.

Log trên thẻ thường nằm ở **.userdata/shared/nxt-retro-music/nxt-retro-music.txt**, hoặc thư mục log NextUI chỉ định. Khi cần kiểm tra lỗi, giữ file log cùng mô tả nút đã nhấn và thiết bị âm thanh đang cắm.
