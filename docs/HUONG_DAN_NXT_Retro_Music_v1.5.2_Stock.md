# Hướng dẫn NXT Retro Music v1.5.2 — ROM Stock

Dành cho **TrimUI Brick Pro / TG4040 chạy ROM Stock**. Build: **Stock · Automatic OTA & UI Performance**. Bản này nối tiếp v1.5.1; v1.4 vẫn được bỏ qua.

## 1. Cài đặt và nâng cấp

1. Dừng nhạc, thoát hẳn NXT. Nếu bản cũ còn phát nền, khởi động lại máy trước khi chép bản mới.
2. Giải nén **NXT_Retro_Music_v1.5.2_Stock.zip** trên máy tính.
3. Chép thư mục **Apps/NXT_Retro_Music** vào **Apps** ở gốc thẻ; thay toàn bộ thư mục app cũ. Đường dẫn đúng: `Apps/NXT_Retro_Music/launch.sh`.
4. Giữ nguyên **.userdata/nxt-stock** để giữ tài khoản, cài đặt, playlist, lịch sử và hình nền. Không chép dữ liệu tài khoản của người khác vào bộ cài public.
5. Mở app. Chọn/tạo người nghe trước khi vào menu nhạc. Màn hình khởi động chỉ ghi **NXT Retro Music**.
6. Vào **Thông tin, hướng dẫn & cập nhật → Thông tin ứng dụng**, kiểm tra **v1.5.2** và tên build ở đầu tài liệu.

Gói này dùng cho Stock; NextUI cần bộ cài riêng. File có `_Source` là mã nguồn; file `NXT_OTA_Google_Drive` là công cụ phát hành. Hai file đó không thay cho ZIP cài đặt.

## 2. Chọn/tạo người nghe

Mỗi lần mở mới app, màn hình chọn người nghe xuất hiện trước menu. D-pad/hai analog chọn hồ sơ, **A** tiếp tục. Nếu hồ sơ mặc định còn tên **Người nghe**, chọn hồ sơ đó hoặc **Tạo người nghe**, nhập biệt danh rồi xác nhận bằng **SELECT nhấn và thả** trên bàn phím. Hồ sơ đầu tiên giữ lại playlist/yêu thích/vị trí nghe từ các bản cũ. **Khách** là hồ sơ cục bộ có sẵn.

**B** ở màn hình chọn khi khởi động đóng app. Mở lại một phiên đang phát nền sẽ trở về phiên nghe hiện tại, giữ nguyên người nghe; màn hình chọn chỉ xuất hiện khi khởi động một tiến trình mới.

Tối đa **16 hồ sơ**, gồm hồ sơ mặc định và Khách. Hồ sơ giữ riêng lịch sử/thống kê, yêu thích nhạc trong thẻ, playlist/queue/vị trí nghe, bookmark SoundCloud, EQ, ánh xạ nút và giao diện. Hồ sơ mới bắt đầu từ thiết lập hiện tại, với dữ liệu nghe trống.

Nhạc trên thẻ và chỉ mục thư viện dùng chung. **Phiên Spotify dùng chung trên thiết bị**; hồ sơ trong app không phải tài khoản Spotify mới. Đầu ra âm thanh, âm lượng phần cứng, thư mục nhạc, LED và nguồn OTA cũng dùng chung. Dữ liệu hồ sơ được lưu trên thẻ, không gửi lên máy chủ.

## 3. Trang chủ và phím chung

Trang chủ có **7 mục**: Thư viện, Playlist, Yêu thích, Nhạc online, Cá nhân, Cài đặt, **Thông tin, hướng dẫn & cập nhật**. OTA nằm trong mục thông tin; giao diện cá nhân nằm trong tab Hồ sơ.

| Vị trí | Phím mặc định | Chức năng |
| --- | --- | --- |
| Menu | D-pad / hai analog | Chọn mục |
| Menu | A / B | Mở / quay lại |
| Trang chủ | Y | Đang phát hoặc nghe tiếp |
| Trang chủ | START | Tùy chọn nhanh |
| Toàn app | Giữ SELECT + ↑/↓ | Tăng/giảm âm lượng |
| Toàn app | Giữ SELECT + ←/→ | Tăng/giảm độ sáng |
| Toàn app | Giữ SELECT khoảng 0,9 giây | Khóa/mở khóa nút |
| ROM Stock | Home cứng bên hông | Bảng nhanh / Task Manager Stock |
| Cá nhân | L1 / R1 | Chuyển giữa 4 tab |
| Hướng dẫn văn bản | ↑/↓, L1/R1, B | Cuộn, đổi trang, quay lại |

## 4. Cá nhân: bốn tab

**L1/R1 chuyển tab**, B quay về menu. Đây là các trang giao diện có thẻ nội dung và biểu đồ, không phải một danh sách tên chức năng.

### Dành cho bạn

D-pad chọn thẻ, A mở:

- **Nghe tiếp:** các lượt nghe gần nhất. Nếu bài trùng vị trí nghe đã lưu, app khôi phục qua chức năng nghe tiếp; các bài lịch sử khác được mở lại để nghe từ đầu.
- **Những bài quen thuộc:** chọn theo tổng thời gian nghe của hồ sơ.
- **Khám phá nhạc chưa nghe:** tối đa 500 bài trong thẻ chưa có lịch sử ở hồ sơ này. Mở Thư viện trước để quét nhạc.
- **Lâu chưa nghe:** các bài từng nghe nhưng lần gần nhất cách hơn 30 ngày.

Gợi ý dựa trên lịch sử trong máy. Hồ sơ chưa nghe nhạc sẽ hiển thị lời hướng dẫn. Khi chọn bài Spotify/SoundCloud từ lịch sử, app thử kết nối nguồn; bài bị xóa/riêng tư hoặc mất quyền truy cập có thể không mở được. Hàng đợi Local/Drive không trộn URI Spotify/SoundCloud vào bộ giải mã local.

### Thống kê

**←/→** đổi **Hôm nay / 7 ngày / Tháng này**. Các thẻ trên cùng cho biết thời gian nghe, lượt nghe và số nghệ sĩ trong khoảng đã chọn. **Y** mở chi tiết theo ngày, top nghệ sĩ và top bài.

Biểu đồ cột biểu diễn thời gian nghe từng ngày: ở Hôm nay và 7 ngày, biểu đồ ghi rõ **7 ngày gần nhất**; ở Tháng này, biểu đồ hiển thị các ngày từ đầu tháng đến hôm nay. Bên cạnh là thanh phân bổ **nhạc thẻ / Spotify / SoundCloud / Drive** trong khoảng đã chọn. Chưa có dữ liệu thì số liệu là 0; app không chèn số minh họa vào dữ liệu của bạn.

Thời gian được tính từ âm thanh trình phát đã tạo ra; tua qua 3 phút không cộng thêm 3 phút nghe. Tạm dừng, chờ dữ liệu hoặc đầu ra chưa mở được không cộng thời gian. Một lượt được tính sau **30 giây**, hoặc **nửa bài** nếu bài dưới 60 giây. Lặp lại bài tạo lượt mới khi đủ điều kiện. Radio/podcast chưa tính trong thống kê này.

Dữ liệu ghi khoảng 10 giây/lần và khi tạm dừng, đổi hồ sơ, thoát bình thường. Ngày phụ thuộc đồng hồ/múi giờ của máy. Top bài/nghệ sĩ dùng ngày cập nhật gần nhất của lượt nghe; lượt qua nửa đêm có thể được nhóm khác bảng tổng theo ngày.

### Lịch sử

Mỗi thẻ có tên bài, nghệ sĩ/nguồn, ngày giờ và thời gian đã nghe. **↑/↓** chọn; **A** phát; **X** thêm/bỏ yêu thích nhạc trong thẻ. **L2/R2** chuyển trang mới hơn/cũ hơn; mỗi trang tối đa 100 mục, có thẻ **Xem lịch sử cũ hơn**. L1/R1 vẫn chuyển tab.

Khi đổi người nghe, app dừng phiên cũ và xóa hàng đợi/bộ nhớ duyệt cũ trước khi nạp dữ liệu người mới. Trang lịch sử cũng quay về trang đầu của người mới.

### Hồ sơ

- **Sửa hồ sơ:** đổi biệt danh, chọn **24 biểu tượng**, bật/tắt ghi lịch sử. Trong bảng icon, dùng D-pad chọn, A áp dụng, B hủy.
- **Đổi người nghe:** chọn hồ sơ khác.
- **Thêm người nghe:** tạo hồ sơ cục bộ mới.
- **Giao diện cá nhân:** nơi duy nhất chứa màu, RGB, hình nền, độ trong suốt, font và ngôn ngữ.

24 biểu tượng gồm tai nghe, đĩa nhạc, trái tim, nốt nhạc, tay cầm, micro, radio, podcast, mặt trăng, nhật nguyệt, sách, tia chớp, mèo, cún, tên lửa, ngôi sao, bông hoa, lá xanh, núi, sóng, cà phê, vương miện, lấp lánh và nụ cười.

## 5. Giao diện cá nhân và hình nền

Mở **Cá nhân → Hồ sơ → Giao diện cá nhân**. D-pad chọn dòng, A sửa, ←/→ thay đổi nhanh. Thiết lập lưu riêng theo người nghe.

- **Giao diện:** Theo hệ thống / Tối / Sáng. Màu chữ, chữ phụ, vùng chọn và bàn phím đổi đồng bộ.
- **Bảng màu:** 16 màu mint, biển xanh, tím, hồng, hổ phách, san hô, xanh ngọc, lá xanh, chàm, vàng chanh, trắng, bạc, cát, ngọc bích, oải hương, cam đào.
- **RGB/HEX:** chọn R/G/B, ←/→ ±1, L1/R1 ±16; A lưu, B hủy. Dòng HEX nhận 6 ký tự RRGGBB hoặc #RRGGBB. Màu nhấn hiển thị có thể được điều chỉnh độ sáng để chữ/vùng chọn rõ hơn ở chế độ sáng/tối.
- **Chọn hình nền:** A mở bộ sưu tập có ảnh xem trước. D-pad chọn, A áp dụng, B hủy. Có 20 nền có sẵn, thêm Không và Ảnh đã nhập.
- 20 nền: Biển, Hoàng hôn, Rừng, Cực quang, Bình minh, Mây hồng, Đêm chàm, Đồi ngọc, Sa mạc, Hoàng hôn retro, Sóng biển, Trời sao, Đồi oải hương, Chân trời đào, Mưa xanh, Đồi cà phê, Mùa đông, Đồng chanh, Thiên hà, Đơn sắc.
- **Nhập ảnh JPG/PNG:** chép ảnh vào thẻ rồi chọn file. Nhận JPG/JPEG/PNG tối đa 10 MiB, 16 megapixel, 8192 px mỗi chiều. App lưu bản thu nhỏ tối đa 1024×768 theo hồ sơ, giữ ảnh gốc.
- **Lớp phủ hình nền:** 0–95%, mặc định **20%**. Mức thấp làm hình nền rõ hơn.
- **Độ đậm ô menu:** 0–100%, mặc định **35%**. Mức thấp làm các ô trong suốt hơn. Ô đang chọn dùng màu nhấn rõ để dễ nhận biết.
- **Font:** chọn 16 bộ font, chữ giao diện 85–120%, chữ lời 24–48 px, độ trễ lời ±30 giây; ngôn ngữ Việt/Anh.

Khi nâng từ v1.5, lớp phủ cũ 65–95% được giảm về 20% một lần, ô menu về 35%; các lần chỉnh sau được giữ. Chữ có viền tương phản khi dùng hình nền. Hình nền và mặt nạ ô menu được đệm lại, tránh tải ảnh/thu nhỏ ảnh mỗi khung hình.

Gợi ý để thấy ảnh rõ: chọn nền hoặc nhập ảnh, đặt lớp phủ 10–25%, ô menu 20–40%. Dùng Tối cho chữ sáng; dùng Sáng cho chữ tối. Nếu chọn **Không**, menu trở về nền đặc.

## 6. Thư viện, playlist và đang phát

**Thư viện** duyệt bài hát/thư mục/nghệ sĩ/album. Thư mục xếp tự nhiên **1, 2, 9, 10… rồi A–Z**. Trong **Cài đặt → Thư mục nhạc**, chọn thư mục cha, bật quét thư mục con, quét lại khi chép thêm nhạc.

**Playlist** tạo/sửa danh sách M3U8; **Yêu thích** lưu riêng theo hồ sơ. Đang phát dùng ảnh bìa tròn quay như đĩa than: phát thì đĩa quay và kim ở vị trí nghe; tạm dừng thì đĩa dừng và kim nghỉ; phát tiếp thì quay tiếp.

| Đang phát | Chức năng mặc định |
| --- | --- |
| A | Phát/tạm dừng |
| B | Về menu, giữ phiên nghe |
| L1 / R1 | Bài trước / bài tiếp |
| ←/→ giữ | Tua |
| X | Hàng đợi |
| Y | Lời toàn màn hình |
| SELECT nhấn và thả | EQ / DSP |
| START | Tùy chọn nhanh |
| L2 | Đổi màu phổ âm; sau màu cuối là tắt |
| R2 | Yêu thích bài hiện tại |

Nhấn START để trộn/lặp, hẹn giờ dừng, đổi đầu ra, tắt màn hình, dừng/lưu vị trí hoặc **Phát nền**. Trong **Cài đặt → Phát nhạc**, có chuyển bài, thời gian chồng âm, hẹn giờ, Phát nhạc dưới nền và Tự tắt màn hình.

Trong **Cài đặt → Điều khiển**, gán chức năng Đang phát cho toàn bộ 16 nút, đổi ánh xạ menu/bàn phím và khôi phục mặc định. Nhãn trong Đang phát đổi theo cách gán. Home/nguồn và SELECT vật lý dùng cho thao tác hệ thống/phím tắt.

## 7. Nhạc online

### Spotify

Vào **Nhạc online → Spotify** để kết nối, tìm bài, mở playlist hoặc phát link. Spotify playback cần tài khoản Premium và quyền truy cập của phiên đăng nhập. Có hướng dẫn dạng văn bản trong mục Spotify. Giữ file đăng nhập PC gốc; không công khai credentials cùng bộ cài.

Thông báo **thư viện bị giới hạn** là trạng thái API thư viện/quyền/hạn mức riêng với xác thực phát nhạc. Khi playback đã kết nối, app vẫn có thể phát bằng link bài được tài khoản cho phép. Không quét QR liên tục khi đang chờ giới hạn phía dịch vụ.

### SoundCloud

Vào **Nhạc online → SoundCloud** để tìm bài, phát và lưu bookmark. Bookmark trong app thuộc hồ sơ đang chọn. Nhạc riêng tư/không còn tồn tại/giới hạn địa lý có thể không phát được. Hướng dẫn SoundCloud là trang văn bản cuộn.

Spotify/SoundCloud nối thêm trang khi cuộn đến gần cuối và mở rộng hàng đợi khi nghe. Không có giới hạn 20 bài cố định trong app; số bài khả dụng phụ thuộc kết quả và quyền của dịch vụ.

### Google Drive

Thêm **link tệp/thư mục công khai** trong Nhạc online → Google Drive. Nhận link Drive thật; đặt quyền **Ai có đường liên kết → Người xem**. Với thư mục, những tệp nhạc bên trong cũng phải có quyền truy cập phù hợp. Chọn bài để tạo hàng đợi cả danh sách; hát hết bài sẽ tự sang bài kế.

App phát online bằng đệm tạm trên thẻ, không yêu cầu tải đầy đủ từng bài trước khi bắt đầu. Dữ liệu đã đệm được dùng lại khi tua. Vùng chưa đệm vẫn phải tải qua mạng. Khi có đủ dữ liệu đi trước, app tải trước bài kế; tải xong có thể chuẩn bị bộ giải mã để giảm khoảng chờ.

Đệm giữ tối đa hai tệp, mỗi tệp 256 MiB; thiếu dung lượng hoặc tệp lớn hơn giới hạn có thể dùng HTTPS Range. Tốc độ còn phụ thuộc Wi-Fi, thẻ, định dạng và hạn mức Drive. Vào hướng dẫn trong Google Drive để xem cách tạo link và xử lý link chưa công khai.

## 8. DAC, Bluetooth và loa

**Cài đặt → Âm thanh & DAC** chọn đầu ra, EQ/DSP, ReplayGain, âm lượng riêng, tự chuyển DAC/Bluetooth, hành vi rút DAC và cổng USB dưới.

Mặc định tự chuyển sang DAC khi cắm; rút DAC chuyển về Bluetooth sẵn sàng nếu tự Bluetooth đang bật, hoặc loa. DAC có ưu tiên khi đang dùng. Bộ điều khiển giữ đường PCM đã chọn khi dùng nút âm lượng DAC và thử lại nếu đầu ra chưa mở được; đứt USB ngắn có thời gian chờ để tránh chuyển nhầm.

Bluetooth tự chuyển cần thiết bị đã ghép/kết nối trong ROM Stock và đầu ra âm thanh A2DP sẵn sàng. Kết nối chỉ hiện biểu tượng Bluetooth chưa đủ để chắc chắn đầu ra nghe nhạc đã sẵn sàng. Khi tự chuyển bị tắt, lựa chọn đầu ra thủ công được giữ. v1.3 chưa có phần tự chuyển Bluetooth đầy đủ; chức năng này được thêm từ v1.5.

Để dùng cổng USB dưới cho DAC, chọn chế độ nghe DAC sau khi rút cáp sạc/PC. Để sạc, rút DAC rồi chọn chế độ sạc. EQ/DSP và gain cao có thể gây méo; giảm preamp nếu clipping.

## 9. LED

Vào **Cài đặt → LED**. Bật LED, đặt độ sáng trên 0, thử **Màu cố định → Đỏ/Xanh** trước. Chế độ **Theo nhạc** giữ màu đã chọn, thay độ sáng theo tín hiệu; lúc tạm dừng có thể không sáng.

v1.5.2 giữ cách ghi RGB và chọn hiệu ứng driver từ **v1.3 LED Driver Fix3**, đồng thời ngăn bộ điều khiển LED nền của app ghi đè màu NXT khi NXT đang nhận điều khiển. Đọc lại giá trị sau ghi, thử lại khi lỗi và áp lại màu khi trạng thái bị đổi. Tắt điều khiển hoặc thoát hẳn sẽ trả trạng thái gốc.

Trong **Bảng màu RGB tự chỉnh**, ↑/↓ chọn R/G/B, ←/→ ±1, L1/R1 ±16, X/Y đổi 16 màu mẫu. **A** lưu màu và bật điều khiển LED, giữ chế độ/độ sáng cũ. **B** hủy, trả màu và toàn bộ cấu hình trước khi xem trước. Nếu vẫn không thấy màu, kiểm tra độ sáng và chế độ theo nhạc; xuất log máy bằng Y trong mục thông tin để xem RGB/effect thực tế.

Cơ chế LED đã kiểm tra bằng trạng thái driver mô phỏng và bộ ghi LED thật trong mã; cần thử lại trên máy để xác nhận kết quả vật lý của bản mới.

## 10. Bàn phím Telex / Latin

| Phím | Chức năng mặc định |
| --- | --- |
| D-pad / analog | Chọn phím, giữ để lặp |
| A | Nhập ký tự; giữ mở biến thể nếu có |
| B | Xóa một bước; ô rỗng thì hủy |
| X | Dấu cách |
| Y | Đổi TELEX ↔ LAT |
| L1 | Shift cho ký tự kế |
| R1 | Khóa/mở Caps |
| SELECT nhấn và thả | Xong/xác nhận |

Telex: `aa → â`, `aw → ă`, `ee → ê`, `oo → ô`, `ow → ơ`, `uw → ư`, `dd → đ`; `s/f/r/x/j` là sắc/huyền/hỏi/ngã/nặng; `z` bỏ dấu. Ví dụ `tieengs Vieetj → tiếng Việt`, `ddawng → đăng`. LAT dùng cho link, mã đăng nhập và chữ cần giữ nguyên. Nếu đã đổi nút menu/bàn phím, dùng phím đã gán.

## 11. Thông tin, hướng dẫn & cập nhật

Một mục chung chứa **Thông tin ứng dụng**, **Hướng dẫn sử dụng**, **Tính năng & thay đổi**, **Cập nhật phần mềm / OTA**, **Hướng dẫn cập nhật OTA**, **Xuất log ứng dụng** và **Thông tin chẩn đoán máy**. A mở dòng; START xuất log app; Y xuất log máy. Các hướng dẫn là trang văn bản cuộn.

### Cách bạn đăng bản OTA lên Google Drive

Nguồn Stock đã nhúng sẵn trong app: [update.json Stock](https://drive.google.com/file/d/1heNcSJ7Um70Z_82cS5atY093ObUZgNR-/view?usp=sharing). NextUI sẽ dùng tệp JSON/link khác. Giữ nguyên ID tệp Stock này.

1. Tải **NXT_Retro_Music_v1.5.2_Stock.zip** lên Drive, chia sẻ **Ai có đường liên kết → Người xem**; lấy link **tệp ZIP**.
2. Giải nén **NXT_OTA_Google_Drive_v1.5.2.zip**, mở **update.json**. Chỉ sửa **download_url** thành link tệp ZIP bước 1. Version, size và sha256 đã khớp ZIP cài đặt gửi kèm.
3. Tại **chính tệp update.json Stock đang có** trên Drive máy tính, mở **Quản lý phiên bản → Tải phiên bản mới**, chọn JSON vừa sửa. Giữ link/ID hiện tại; không xóa rồi tạo tệp khác.
4. Cả JSON và ZIP đều cần chia sẻ công khai quyền Người xem. Kiểm tra tải được hai tệp trong cửa sổ không đăng nhập. Không chuyển JSON thành Google Docs hay lưu đuôi `.json.txt`.

Người dùng v1.5.1 mở **Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm / OTA**, nhập link tệp JSON Stock trên một lần và chọn **Kiểm tra → Tải và kiểm tra → Cài**. Sau khi lên v1.5.2, lần mở mục cập nhật tiếp theo app tự kiểm tra nguồn.

Các bản sau như v1.6: ZIP mới cần version lớn hơn, và update.json phải có **version, download_url, size, sha256** đúng ZIP đó. Chỉ thay ZIP mà giữ hash/version JSON cũ sẽ không cập nhật được. Link ZIP có thể đổi; link update.json Stock giữ nguyên. Mình sẽ gửi manifest tính sẵn khi build bản tiếp theo.

Nếu bạn tự đóng gói ZIP khác, dùng công cụ để tạo JSON mới, không sửa hash bằng tay:

```sh
python3 make-ota-manifest.py --zip NXT_Retro_Music_v1.5.2_Stock.zip --url 'LINK_HTTPS_TEP_ZIP' --version 1.5.2 --notes CHANGELOG_v1.5.2_Stock.md --out update.json
```

Trong mã nguồn, link mặc định nằm ở `stock/ota-source-stock.url`, được chép thành `ota-source-stock.url` trong bộ cài. Nguồn Stock lưu riêng trong `ota/stock/source.json`; dữ liệu nguồn cũ của Stock được chuyển một lần nếu hợp lệ. Bản NextUI sau này dùng `ota-source-nextui.url` và `ota/nextui/source.json`; chưa có build NextUI trong gói này.

Tài liệu Google: [Chia sẻ tệp](https://support.google.com/drive/answer/2494822?hl=vi) · [Quản lý phiên bản tệp](https://support.google.com/drive/answer/2409045?hl=vi).

### Người dùng cập nhật trên máy v1.5.2 trở đi

1. Kết nối Wi-Fi bằng ROM Stock, mở **Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm / OTA**. App tự kiểm tra nguồn Stock đã nhúng, hoặc nguồn Stock bạn đã chọn trước đó.
2. Trang OTA có **4 mục**: nút chính **Kiểm tra/Tải/Cài/Hủy**, **Thay đổi bản mới**, **Nguồn cập nhật**, **Hướng dẫn OTA**. Tiêu đề cho biết bản hiện tại và bản mới; dòng đầu hiện trạng thái/dung lượng/tiến trình.
3. Khi có bản mới, mở **Thay đổi bản mới**, quay lại rồi chọn **Tải phiên bản…**. App kiểm tra đúng ROM/máy, kích thước, SHA-256, ZIP và ARM64.
4. **B** quay về menu, bản tải vẫn tiếp tục. Mở lại OTA để xem tiến trình; gói đã xác minh giữ nguyên, không tự tải lại. **A** ở dòng đầu khi đang kiểm tra/tải sẽ hủy bước đang chạy.
5. Khi sẵn sàng, chọn **Cài phiên bản… → Cài ngay**. App lưu dữ liệu, dừng nhạc, thay thư mục app và lưu bản cũ ở `Apps/.NXT_Retro_Music.previous`, rồi đóng. Mở lại app để dùng bản mới.
6. **X** kiểm tra lại nguồn khi chưa tải/cài. Gói đã xác minh được giữ để cài. **Nguồn cập nhật** cho nhập link TỆP JSON, đọc link từ `.txt/.url`, hoặc khôi phục nguồn Stock nhà phát hành. Link thư mục không dùng làm manifest.

v1.5.2 sẽ báo chưa có bản mới nếu nguồn đang là v1.5.2 hoặc cũ hơn. OTA cập nhật app, không flash ROM. Dữ liệu người nghe/tài khoản nằm ngoài thư mục app. SHA-256 kiểm tra nguyên vẹn, manifest chưa có chữ ký phát hành. Giữ nguồn khi cài. Lỗi chia sẻ/hạn mức tải cần xem log hoặc cài ZIP thủ công; nếu mất điện lúc đổi thư mục, khôi phục bản cũ từ `.previous`.

## 12. Log và dữ liệu

- Log app: **.userdata/nxt-stock/shared/nxt-retro-music/app-log.txt**; xoay sang **app-log.txt.previous** sau khoảng 1 MiB.
- START trong mục thông tin xuất **Apps/NXT_Retro_Music/NXT_App_Log.txt**: thời điểm, phiên bản, thành phần, bước, OK/ERROR.
- Y xuất **NXT_Device_Log.txt**: đầu ra, PCM/DAC/Bluetooth, retry và RGB/effect/animation/độ sáng LED thực tế.
- Cấu hình và thống kê: **nxt.json**, **listening.sqlite3** trong cùng thư mục dữ liệu; có WAL/SHM khi đang chạy.
- Hồ sơ/hình nền: **listeners/<id>/**. Hồ sơ mặc định giữ vị trí playlist/yêu thích/resume cũ để tương thích.

Log mới ghi bước lỗi/khởi động/lưu, hồ sơ, hình nền/RGB, LED, đổi đầu ra, Spotify/SoundCloud và OTA; không ghi token hay URL tải trong log app. Khi báo lỗi, kèm phiên bản, thao tác, đầu ra đang dùng và hai log xuất từ mục thông tin.

## 13. Những thay đổi chính v1.5.2

- OTA chỉ 4 mục, tự kiểm tra khi mở; một nút chính đổi theo trạng thái kiểm tra/tải/cài/hủy.
- Nhúng link update.json Stock đã cung cấp; mặc định/nguồn lưu/gói cập nhật tách riêng Stock và NextUI.
- B quay về vẫn tải; mở lại giữ tiến trình hoặc gói đã xác minh. Không tự cài khi chỉ mở mục cập nhật.
- Đệm chữ theo font, dùng lại màu tương phản và giới hạn bộ đệm chữ 6 MiB/96 mục; giải phóng khi đổi font.
- Trang đứng yên cập nhật khoảng mỗi giây; chữ dài cuộn giữ nhịp riêng, chọn nút vẽ ngay. Trang chọn hồ sơ/icon/nền không còn vẽ mọi vòng lặp.
- Spotify/SoundCloud không còn vẽ toàn bộ màn hình đang phát ở mọi vòng lặp; vẫn giữ đĩa/phổ hoạt ảnh riêng.
- Không vẽ trang đang ẩn hoặc đã tắt màn hình; âm thanh, theo dõi đầu ra và OTA tiếp tục chạy.
- Thống kê nạp lại khi dữ liệu SQLite/ngày/người nghe/khoảng thống kê thay đổi, giảm truy vấn lặp.
- Giữ bốn tab cá nhân, thống kê thật, 24 icon, 20 nền, độ trong suốt, Telex/ABXY, đĩa than, Drive, đăng nhập, LED v1.5.1 và DAC/Bluetooth.

## 14. Độ mượt và nhiệt độ

Cá nhân hóa thêm ảnh nền, ô trong suốt, viền chữ và biểu đồ sẽ tăng công việc vẽ. v1.5.2 giảm vẽ dư và tính lại chữ/số liệu khi không cần. Nút vẫn được kiểm tra mỗi vòng lặp; không bị giới hạn một lần mỗi giây.

Phép đo trong `logs/v152-render-benchmark.json` so sánh renderer native với nguồn v1.5.1 tại 1024×768, cùng nền/độ trong suốt/font và dữ liệu thử. Nó đo thời gian CPU vẽ trên máy build, **không đo nhiệt độ, CPU toàn máy, pin hoặc khả năng giải mã của Brick Pro**. Nhiệt thực tế còn phụ thuộc âm thanh/mạng/độ sáng; cần so sánh cùng bài và thiết lập trên máy.

Khi báo lag/nóng, gửi phiên bản, màn hình đang mở, nguồn nhạc, đầu ra, hình nền/độ trong suốt và log xuất từ mục thông tin. Không tự đổi xung CPU/governor trong bản này.

Các kiểm tra phần mềm xác nhận build ARM64, giao diện native, dữ liệu SQLite, LED/đầu ra qua fixture, OTA và gói cài. Chúng không thay cho thử màn hình, LED, DAC/Bluetooth, mạng và tài khoản thật trên thiết bị. Link Drive được nhúng đúng; môi trường build chưa tải trực tiếp được JSON công khai này, nên cần thử nguồn thật trên máy sau khi đăng manifest/ZIP.
