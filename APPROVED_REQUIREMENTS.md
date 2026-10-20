# Yêu cầu NXT Retro Music — bản chốt 1.1

**Phiên bản tài liệu:** 1.1, cập nhật ngày 05/10/2026.  
**Thiết bị ưu tiên:** TrimUI Brick Pro chạy NextUI.  
**Trạng thái:** người dùng đã chốt các lựa chọn ở mục 13; bổ sung nhiều font và ưu tiên hiển thị tên bài/lời đa ngôn ngữ. Xem demo giao diện trước khi triển khai và build app theo bản này.  
**Căn cứ:** file `Yeu_cau_NXT_Retro_Music update(1).txt` và yêu cầu xem bản tổng hợp trước khi build.

Các lựa chọn đã được người dùng chấp thuận trở thành yêu cầu triển khai. Bản 1.1 tăng lên 16 font chọn được, chọn nhận diện logo/icon và bổ sung yêu cầu hiển thị Unicode đa ngôn ngữ. Các tình huống kiểm tra mô tả kết quả cần đạt khi triển khai, không phải kết quả thử nghiệm app trên Brick Pro đã có.

## 1. Phạm vi và tên ứng dụng

- Tên thống nhất: **NXT Retro Music**, có khoảng trắng giữa các từ. Dùng cùng tên trong menu hệ điều hành, tiêu đề app và trang About.
- Phát triển dựa trên Music Player trên Pak Store; giữ chức năng nghe radio, podcast online và tìm/tải nhạc từ YouTube của ứng dụng nền.
- Bản đầu tiên triển khai cho **NextUI trên TrimUI Brick Pro**. Khả năng chạy trên hệ điều hành stock được giữ trong mục tiêu tiếp theo, cần bản build và kiểm tra riêng.
- Khung thiết kế mục tiêu theo yêu cầu: **1024 × 768, tỉ lệ 4:3**. Thiết kế theo khung này; bỏ cách ghi mơ hồ “resize lên 720”. Độ phân giải render và cách co giãn thực tế được đối chiếu khi triển khai.
- Build trực tiếp trong **Ubuntu WSL trên Windows 11**, không dùng Docker hoặc Podman. Bản PC phục vụ kiểm tra giao diện khi cần; sản phẩm cần giao cho người dùng là gói app cho NextUI.
- Bộ âm thanh ưu tiên khi thử: DAC CX31988 + HT97220 generic/Titan và Sony MDR-EX155AP. App vẫn hỗ trợ các đầu ra và DAC khác khi hệ điều hành đáp ứng.

## 2. Tổng hợp 11 nhóm chức năng

| STT | Nhóm | Nội dung đã yêu cầu |
| --- | --- | --- |
| 1 | Phát nhạc và chức năng online | Phát/tạm dừng, bài trước/sau, tua, lặp một bài/toàn bộ, ngẫu nhiên; FLAC, WAV, MP3, AAC, OGG, Opus và ALAC. Giữ radio, podcast và tải nhạc YouTube. |
| 2 | Thư viện | Duyệt thư mục, nghệ sĩ, album và tất cả bài; chọn thư mục cha có nhiều cấp thư mục con; tìm bài/nghệ sĩ, sắp xếp và cập nhật thư viện. |
| 3 | Playlist và hàng đợi | Tạo, đổi tên, sửa playlist; yêu thích; phát tiếp theo, thêm cuối và đổi thứ tự; nhập/xuất playlist; mở danh sách đang phát để chọn bài trực tiếp. |
| 4 | Màn hình đang phát | Ảnh album, thông tin bài, tiến trình, thời gian đã phát/còn lại, codec và thông số nguồn, đầu ra, phổ nhạc, hẹn giờ; pin, Wi-Fi, đồng hồ và icon đầu ra. Bố cục cân đối, ưu tiên icon cho trạng thái. |
| 5 | Lời bài hát | LRC đồng bộ, tô sáng dòng hiện tại, toàn màn hình, xuống dòng và cuộn, chỉnh cỡ chữ và độ trễ; hiển thị đúng tiếng Việt, Hàn, Nhật, Trung giản thể/phồn thể và Thái. |
| 6 | Đầu ra và DAC | Loa máy, DAC và Bluetooth khi OS hỗ trợ; nhận cắm/rút, nhớ âm lượng từng đầu ra, chọn xử lý khi rút DAC; thông tin DAC và điểm tham khảo /10 khi có dữ liệu. |
| 7 | Âm thanh nâng cao | Gapless, ReplayGain, crossfade và EQ/DSP theo hướng chuyên nghiệp; EQ chỉnh trực tiếp trên biểu đồ và preset theo thể loại. Phạm vi đã chốt ở mục 6. |
| 8 | Điều khiển bằng nút | Thiết kế lại ánh xạ nút; dùng hoàn toàn bằng nút máy; giữ để tua, mở lời/hàng đợi, tùy chỉnh và khóa thao tác; tránh thao tác trùng và phím hệ điều hành. |
| 9 | Phát nền, hẹn giờ và LED | Nghe khi màn hình tắt, thoát giao diện và chơi game; mở lại đúng phiên; dừng/lưu vị trí; hẹn giờ nhập phút tùy ý; LED theo nhạc, bật/tắt và đổi màu. |
| 10 | Giao diện và cài đặt | English/Tiếng Việt, sáng/tối, chỉnh font/cỡ chữ; nhiều font đi kèm, chọn thư mục nhạc; lưu tùy chỉnh và hiển thị không khuất chữ. |
| 11 | About, hướng dẫn và ủng hộ | Một mục chung chứa thông tin app, người lên ý tưởng, ghi công, bảng nút/hướng dẫn và MoMo; nội dung ngắn gọn, dễ tra. |

Hai nhóm “Hướng dẫn sử dụng” và “Tên/thông tin ứng dụng” trong bản trước đã được gộp thành nhóm 11. Các phần dưới làm rõ hành vi của từng nhóm.

## 3. Phát nhạc, thư viện và playlist

### 3.1. Phát nhạc

- Phát/tạm dừng, bài trước/sau và tua tới/lùi bằng nút máy; giữ nút để tua liên tục.
- Lặp: tắt, lặp một bài và lặp toàn bộ. Ngẫu nhiên có trạng thái bật/tắt rõ ràng.
- Hỗ trợ FLAC, WAV, MP3, AAC, OGG/Vorbis, Opus và **ALAC bắt buộc**.
- Nhận diện codec thực tế: file `.m4a` có thể là AAC hoặc ALAC; phần thông tin phải ghi đúng codec, không đoán chỉ theo đuôi file.
- Giữ các chức năng online của app nền. Khi không có mạng, thông báo dễ hiểu; thư viện nhạc local vẫn dùng được.

### 3.2. Thư viện

- Cho chọn một thư mục nhạc cha và nhận nhạc trong cả thư mục đó lẫn thư mục con nhiều cấp.
- Vẫn duyệt được cấu trúc thư mục; đồng thời có các cách xem tất cả bài, nghệ sĩ và album.
- Tìm theo tên bài/nghệ sĩ; sắp xếp theo tên hoặc thứ tự bài trong album.
- Có thao tác cập nhật thư viện khi thêm nhạc.
- Thể hiện rõ phạm vi phát đã chốt: **thư mục đang mở** hoặc **thư mục đang mở cùng thư mục con**.

### 3.3. Playlist và hàng đợi đang phát

- Tạo, đổi tên, thêm/bỏ bài và sửa playlist; đánh dấu/bỏ yêu thích.
- Cho chọn “Phát tiếp theo”, “Thêm vào cuối hàng đợi” và đổi thứ tự bài.
- Có nhập/xuất playlist. **Đã chốt định dạng M3U**; xuất UTF-8 với đuôi `.m3u8` để giữ đường dẫn đa ngôn ngữ, nhận cả `.m3u` và `.m3u8` khi nhập.
- Từ màn hình đang phát, mở được hàng đợi hiện tại: danh sách từ thư mục, album hoặc playlist đang nghe.
- Mở/duyệt danh sách vẫn giữ bài hiện tại đang phát. Di chuyển con trỏ không tự đổi bài; xác nhận bài nào mới phát bài đó.
- Phân biệt bài đang phát với dòng đang chọn; danh sách dài có cuộn. Quay lại mà chưa chọn bài khác thì giữ nguyên bài và vị trí.
- Ví dụ kiểm tra: đang phát bài 3 trong 50 bài, mở danh sách và chọn thẳng bài 30.

## 4. Màn hình đang phát — Now Playing

### 4.1. Nội dung bắt buộc

| Khu vực thông tin | Nội dung |
| --- | --- |
| Trạng thái máy | Icon pin và mức pin, Wi-Fi, đồng hồ; icon tai nghe/DAC/Bluetooth theo trạng thái thực tế. |
| Bài đang phát | Ảnh album, tên bài, nghệ sĩ và album. |
| Thông số nguồn | Codec/định dạng, bitrate, sample rate và bit-depth khi thông tin có sẵn. |
| Đầu ra | Đầu ra đang sử dụng; nếu là DAC, hiện tên/loại nhận diện được với nhãn gọn `DAC: [tên thiết bị]`. |
| Tiến trình | Thanh tiến trình, thời gian đã phát và thời gian còn lại. |
| Phổ nhạc | Phản ánh tín hiệu đang phát, có thể bật/tắt hiển thị. |
| Chế độ phát | Icon phát/tạm dừng, ngẫu nhiên và lặp; phân biệt tắt, lặp một và lặp toàn bộ. |
| Hẹn giờ | Thời gian còn lại khi có hẹn giờ; trạng thái chưa đặt giờ dễ nhận biết. |

- Không coi DAC đang cắm là DAC đang phát nếu đầu ra đã chọn là loa hoặc Bluetooth.
- Trạng thái không được hệ điều hành cung cấp phải được biểu thị phù hợp; không hiển thị như đang kết nối khi chưa xác nhận.
- Thông số nguồn và đầu ra thực tế được ghi riêng. Nếu không đọc được cấu hình đầu ra, không lấy sample rate/bit-depth của file để thay thế.
- Ưu tiên icon cho chế độ và trạng thái; giữ chữ ở tên bài, thông số và các thông tin cần đọc chính xác.

### 4.2. Bố cục đã chốt

| Vùng | Cách sắp xếp đã chốt |
| --- | --- |
| Trên cùng | Thanh trạng thái gọn: tên app, đồng hồ, kết nối và pin. |
| Nội dung chính | Ảnh album bên trái; tên bài, nghệ sĩ, album và thông số bên phải. |
| Bên dưới nội dung chính | Vùng lời thu gọn khi bật và đủ chỗ; lời toàn màn hình có màn riêng. |
| Phía dưới | Phổ nhạc, thanh tiến trình và thời gian nghe. |
| Hàng trạng thái cuối | Icon chế độ phát, hẹn giờ và gợi ý nút cần thiết. |

- Bố cục cần cân đối, có khoảng cách giữa các vùng; tránh gom tất cả nội dung vào một khối chữ.
- Tên dài được xuống dòng hoặc cuộn trong vùng riêng. Khi đổi font, cỡ chữ hoặc ngôn ngữ, các vùng phải điều chỉnh để không chồng lên nhau.
- Hướng thiết kế đã được người dùng chọn; vị trí chính xác, font mặc định và kích thước từng vùng chưa có bản mẫu cụ thể trong file. Bố cục trên đã được chấp thuận; demo thể hiện thiết kế cụ thể để người dùng xem trước khi triển khai.

## 5. Lời bài hát

- Đọc file `.lrc`, đồng bộ thời gian và tô sáng dòng đang hát.
- Có màn hình lời toàn màn hình, chỉnh cỡ chữ, cuộn thủ công và chỉnh độ trễ lời.
- Dòng dài xuống dòng theo chiều rộng; hiển thị đúng các ngôn ngữ ở mục 11.1, không cắt giữa cụm ký tự hoặc làm sai vị trí dấu.
- Khi cuộn thủ công, có thao tác trở lại bám dòng đang hát.
- Khi đổi bài hoặc tua, lời và dòng tô sáng cập nhật theo đúng bài/vị trí hiện tại.
- Khi chưa có lời, hiện thông báo rõ và hướng dẫn đặt file LRC; không giữ lời của bài trước.
- Mở/đóng màn lời, đổi font hoặc độ trễ không làm khởi động lại bài.

## 6. EQ/DSP và chuyển bài

### 6.1. Nội dung đã yêu cầu

- Gapless: nối các bài trong album mà không thêm khoảng nghỉ ngoài ý muốn.
- Crossfade: chồng phần cuối bài trước và phần đầu bài sau; cho chỉnh thời lượng.
- ReplayGain: cân bằng độ lớn theo thông tin ReplayGain khi có.
- EQ/DSP chuyên nghiệp theo hướng tham khảo ViPER4Android, với **biểu đồ có thể chỉnh**, không chỉ một biểu đồ để xem.
- Có preset theo thể loại và cho lưu cấu hình riêng.
- DSP áp dụng cho âm thanh của app, kể cả khi phát nền; không tự thay đổi âm thanh game hay toàn bộ hệ điều hành.

### 6.2. Cách điều khiển EQ bằng nút máy — đã chốt

- Trái/phải chọn dải tần trên biểu đồ.
- Lên/xuống tăng/giảm gain của dải đang chọn; giữ để chỉnh liên tục.
- Hiện rõ dải đang chọn, tần số và mức dB. Biểu đồ cập nhật cùng mức gain.
- “Kéo lên/xuống” được thực hiện bằng D-pad, phù hợp yêu cầu thao tác hoàn toàn bằng nút máy; không đặt điều kiện phải có cảm ứng hoặc chuột.
- Có nút/thao tác riêng để mở preset, bật/tắt DSP và đặt lại Flat; ánh xạ cuối cùng hiển thị trong hướng dẫn.

### 6.3. Phạm vi DSP đã chốt

| Thành phần | Cấu hình đã chốt |
| --- | --- |
| EQ đồ họa | Cho chọn 10/15/31 dải; mặc định 10 dải để dễ chỉnh. Chuyển số dải phải có hành vi rõ, không âm thầm mất cấu hình đã lưu. |
| Preset có sẵn | Flat, Pop, Rock, Jazz, Classical, Vocal và EDM. Cho lưu, đổi tên, nhập/xuất preset riêng. |
| Preamp | Tăng/giảm mức trước EQ để tạo khoảng dự phòng khi tăng các dải. |
| Limiter và báo clipping | Bật/tắt giới hạn đỉnh và báo mức quá cao. |
| EQ tham số | Chỉnh tần số, gain, Q và bật/tắt từng bộ lọc. Số bộ lọc chọn khi triển khai, không đổi các tham số đã yêu cầu. |
| Đường đáp ứng | Hiển thị đáp ứng của cấu hình EQ đang dùng. |
| Hồ sơ đầu ra | Lưu cấu hình riêng cho loa, DAC và Bluetooth nếu hỗ trợ. |
| Bypass | Bật/tắt toàn bộ DSP hoặc thành phần để so sánh. |
| Chỉnh nhanh | Bass, treble và cân bằng trái/phải. |

Các hiệu ứng compressor, tăng cường bass, stereo width/crossfeed, reverb, convolver/IR và hiệu chỉnh tai nghe thuộc phần mở rộng sau bản đầu. Phạm vi DSP của bản đầu là bảng trên; việc tham khảo ViPER không mặc nhiên bao gồm thuật toán, preset DDC hoặc IRS riêng của ViPER.

Trình bày chế độ chuyển bài thành **Thông thường / Gapless / Crossfade**, chọn một chế độ tại một thời điểm. Bit-perfect chưa phải yêu cầu bắt buộc; chỉ ghi khả năng bit-perfect hoặc hi-res ở đầu ra sau khi kiểm chứng đường âm thanh thực tế.

## 7. Đầu ra âm thanh và thông tin DAC

### 7.1. Đầu ra và cắm/rút

- Chọn loa máy, DAC và Bluetooth nếu OS hỗ trợ.
- Tự nhận biết cắm/rút DAC, cập nhật thông tin thiết bị và trạng thái đầu ra.
- Nhớ âm lượng riêng cho từng loại đầu ra.
- Có tùy chọn khi rút DAC: **tạm dừng** hoặc **chuyển sang đầu ra khác**. Mặc định đã chốt là tạm dừng; người dùng có thể đổi trong Cài đặt.
- Chính sách tự chuyển sang DAC khi cắm cần được thể hiện rõ trong cài đặt; việc nhận diện thiết bị không tự ghi đè lựa chọn đầu ra.

### 7.2. Nhận diện và điểm tham khảo

- Hiện tên/model khi nhận diện được; có trang chi tiết để xem thông tin hệ thống cung cấp, khả năng đầu ra và cấu hình đang dùng.
- Chỉ hiện model/chip DAC/chip khuếch đại cụ thể khi có căn cứ cho đúng thiết bị. Nếu hệ thống chỉ báo tên chung, hiển thị tên đó và phần chưa xác định.
- Đổi/rút DAC phải bỏ thông tin và điểm của DAC cũ.
- Điểm /10 là **đánh giá tham khảo tra theo đúng model/biến thể**, không phải phép đo tự thực hiện khi cắm USB.
- Chỉ cho điểm khi có dữ liệu đáng tin cậy; thiếu dữ liệu thì hiện **Chưa có đánh giá**, không gán 0/10 hoặc đoán theo tên chip/sample rate.
- Có thể kèm căn cứ đánh giá trong trang chi tiết DAC; phần này tách khỏi trang About, không tạo một mục nguồn tham khảo chung trong app.
- Chốt chính sách điểm: lấy từ danh mục đánh giá có căn cứ cho đúng model/biến thể, ghi rõ nguồn và cách quy đổi nếu dùng thang điểm khác. Công thức tự chấm 4 + 2 + 2 + 2 trong bản cũ không dùng để suy ra điểm từ thông tin USB.

## 8. Điều khiển bằng nút — thiết kế lại

### 8.1. Nguyên tắc đã yêu cầu

- Thiết kế ánh xạ mới; bảng thao tác cũ của Music Player không còn là yêu cầu phải giữ.
- Thao tác toàn bộ app bằng nút máy, có tùy chỉnh ánh xạ và khóa/mở khóa.
- Tránh xung đột với nút/tổ hợp hệ điều hành. Khi triển khai cần đối chiếu bản NextUI và nút vật lý thực tế trước khi khóa bảng mặc định.
- Khi dùng tổ hợp hoặc giữ nút, không đồng thời phát sinh thao tác nhấn ngắn ngoài ý muốn.
- App đang ở nền không lấy thao tác của game. Điều khiển nhạc bằng phím toàn hệ thống chưa được bổ sung thành yêu cầu.
- Hướng dẫn phải phản ánh ánh xạ thực tế sau khi người dùng tùy chỉnh; luôn có cách mở khóa và giữ thao tác nguồn của hệ điều hành.

**Cách hiểu đã chốt về “không trùng thao tác”:** trong cùng một màn hình, không gán hai nút/tổ hợp khác nhau cho cùng một thao tác trực tiếp, và một cử chỉ chỉ gây một hành động. Cùng một nút có thể có vai trò phù hợp ở các màn hình khác nhau, chẳng hạn A xác nhận trong menu và phát/tạm dừng ở Now Playing. Không giữ tình trạng cả D-pad và L/R cùng chuyển bài ở cùng màn hình.

### 8.2. Bảng Now Playing đã chốt

| Nút/cử chỉ | Thao tác |
| --- | --- |
| A | Phát / tạm dừng. |
| B | Quay về menu, giữ phiên đang nghe. |
| X | Mở danh sách đang phát. |
| Y | Mở lời toàn màn hình. |
| L1 / R1 | Bài trước / bài tiếp theo. |
| Trái / phải | Tua lùi / tới; giữ để tua liên tục. |
| SELECT nhấn ngắn | Mở EQ/DSP. |
| SELECT giữ | Khóa / mở khóa; không kích hoạt thao tác mở EQ của nhấn ngắn. |
| START | Mở tùy chọn nhanh. |
| Lên / xuống | Chưa gán thao tác ở Now Playing; không dùng làm bản sao của bài trước/sau. |

Tùy chọn nhanh chứa ngẫu nhiên, chế độ lặp, yêu thích, hẹn giờ, tắt màn hình, Dừng và Thoát giao diện. Các chức năng này chọn bằng D-pad và A. Mục About chứa hướng dẫn nút, không giữ START mở trợ giúp như bảng cũ.

Ở các màn hình danh sách/menu: D-pad để duyệt, A xác nhận và B quay lại. Ở EQ: trái/phải chọn dải, lên/xuống chỉnh gain. Các thao tác sửa hàng đợi và màn lời cần có bảng riêng, tuân thủ cùng nguyên tắc không trùng.

Bảng này đã chốt về chức năng; khả năng không xung đột với phiên bản NextUI đang dùng cần được kiểm tra khi triển khai. Không mặc định dùng thêm L2/L3/R2/R3 nếu chưa đối chiếu nút vật lý.

## 9. Phiên phát nền, Dừng và hẹn giờ

### 9.1. Hành vi cần đạt

| Tình huống | Hành vi |
| --- | --- |
| Tắt màn hình khi đang phát | Nhạc tiếp tục. Tắt màn hình phải được xử lý khác với ngủ sâu làm dừng tiến trình. |
| Thoát giao diện khi chưa Dừng | Giữ phiên phát nền. |
| Mở/chơi game | Nhạc tiếp tục, không tự Pause vì khởi chạy game; không lấy nút của game. |
| Pause | Ngừng âm thanh tạm thời, giữ bài và vị trí. |
| Dừng rồi thoát | Ngừng phát và lưu đúng bài/vị trí tại thời điểm Dừng. |
| Mở lại khi phiên nền còn hoạt động | Nối vào đúng phiên, hiện bài/vị trí hiện tại; không reset bài hoặc tạo phiên trùng. |
| Mở lại sau khi đã Dừng | Khôi phục bài và vị trí. Chờ người dùng bấm Play, không tự phát. |

Khi phiên nền đổi từ bài A sang B, mở lại phải hiện B. Ảnh album, lời, phổ, trạng thái phát, EQ/DSP, hẹn giờ và đầu ra đều theo phiên hiện tại; bản lưu lúc thoát không ghi đè phiên đang hoạt động.

Mục tiêu là giữ nhạc khi chơi game. Việc chia sẻ đầu ra với âm thanh game cần kiểm tra trên NextUI; chưa coi việc tắt tiếng game là yêu cầu và chưa tuyên bố đã thử thành công trên phần cứng.

### 9.2. Hẹn giờ dừng

- Nhập số phút bằng bàn phím số điều khiển bằng nút; không chỉ có mốc cố định.
- Nhận số phút nguyên dương, có thể sửa và hủy hẹn giờ.
- Hiện thời gian còn lại trên Now Playing; mở lại giao diện không đặt lại bộ đếm.
- Hẹn giờ tiếp tục khi màn hình tắt hoặc khi thoát giao diện mà phiên nền còn hoạt động.
- Hết giờ: Dừng và lưu bài/vị trí theo mục 9.1.
- **Đã chốt:** đếm theo thời gian thực, kể cả trong lúc Pause.

## 10. LED theo nhạc

- LED theo nhạc, bật/tắt và đổi màu là **yêu cầu đã chọn**, không còn ghi như một ý tưởng chưa quyết định có triển khai hay không.
- Người dùng đã xác nhận máy có khả năng điều khiển LED; điều này là căn cứ chọn chức năng, không thay cho kiểm tra hoạt động của app mới.
- Có chọn màu và lưu lựa chọn. Khi bật, phản ứng theo âm thanh đang phát, kể cả phát nền; khi tắt chế độ theo nhạc, ngừng hiệu ứng do app điều khiển.
- Các vùng LED thực tế được đối chiếu với máy; không bỏ yêu cầu đổi màu hoặc phát nền khi triển khai.
- **Mặc định đã chốt:** nháy theo mức âm thanh, một màu do người dùng chọn, áp dụng trên các vùng được hỗ trợ. Điều khiển màu riêng từng vùng và các kiểu hiệu ứng khác thuộc phần mở rộng sau bản đầu.

## 11. Ngôn ngữ, font và lưu cài đặt

- Chọn English hoặc Tiếng Việt; áp dụng cho menu, cài đặt, thông báo, trợ giúp, About và mô tả các chức năng âm thanh.
- Chuyển ngôn ngữ không làm khởi động lại bài; không khuất, cắt hoặc chồng chữ khi bản dịch dài hơn.
- Theme sáng/tối, chỉnh font và cỡ chữ. Lời bài hát có cỡ chữ riêng.
- **Nhiều font được đóng gói sẵn**, chọn ngay trong app, hỗ trợ đầy đủ tiếng Việt; người dùng không phải tải hoặc chép font thủ công.
- Đóng gói **16 font để lựa chọn**: Noto Sans, Inter, Roboto, Source Sans 3, Nunito Sans, Manrope, Rubik, Ubuntu, Lato, Montserrat, Noto Serif, Source Serif 4, Lora, Merriweather, JetBrains Mono và Fira Code. Mặc định Noto Sans. Giữ giấy phép đi kèm từng font.
- Lưu lựa chọn ngôn ngữ, theme, font/cỡ chữ, thư mục nhạc, nút điều khiển, EQ/DSP/preset, đầu ra/âm lượng, chính sách DAC và LED.
- Hiển thị tên dài bằng xuống dòng/cuộn phù hợp. Yêu cầu không mất chữ áp dụng cả với mức cỡ chữ lớn được app cho phép chọn.

### 11.1. Hiển thị đa ngôn ngữ — ưu tiên cao nhất

- Yêu cầu bắt buộc cho **tên bài, nghệ sĩ, album, tên thư mục/file, hàng đợi và lời LRC**: Việt/Latin, Hàn, Nhật (Kanji/Hiragana/Katakana), Trung giản thể/phồn thể và Thái. Giao diện vẫn chỉ có English/Tiếng Việt; ngôn ngữ nội dung nhạc độc lập với ngôn ngữ giao diện.
- Bộ font bổ sung đóng gói offline: Noto Sans CJK theo vùng JP/KR/SC/TC, Noto Sans Thai; thêm Noto Sans Lao, Noto Sans Khmer, Noto Sans Devanagari, Noto Naskh Arabic và bộ ký hiệu phù hợp để mở rộng Unicode.
- Người dùng chọn bất kỳ font trang trí nào vẫn được hỗ trợ tự động khi font đó thiếu chữ. Không phải tự chuyển font, tải thêm hoặc có Internet mới xem được nội dung.
- Không chỉ gói những ký tự xuất hiện trong demo; nội dung nhạc mới phải sử dụng phạm vi glyph của bộ font đã đóng gói. Ưu tiên font khu vực phù hợp với tiếng Nhật/Hàn/Trung khi xác định được ngôn ngữ; không khẳng định suy ra được ngôn ngữ chỉ từ một chữ Hán dùng chung.
- Đọc/giữ đúng Unicode trong tag, đường dẫn và LRC; không làm hỏng mã hóa hoặc đổi dấu sau khi lưu/mở lại.
- Renderer phải hỗ trợ font bổ sung theo cụm chữ và text shaping: dấu/số phụ trong tiếng Thái, dấu ghép và chữ nhiều thành phần được đặt đúng, không tách rời dấu để chọn font khác. Xuống dòng theo cụm ký tự và quy tắc của hệ chữ; không cắt ở giữa byte UTF-8 hoặc cụm chữ.
- Đo kích thước bằng kết quả render thực tế, chừa vùng dấu trên/dưới, đặc biệt khi tăng cỡ lời hoặc chọn font serif/mono.
- Ký tự hiếm ngoài bộ font đã đóng gói phải được phát hiện trong kiểm tra và bổ sung hỗ trợ tương ứng; không tự bỏ hoặc thay nội dung thành dấu hỏi để che lỗi.
- Icon ứng dụng dùng đồ họa riêng để tránh phụ thuộc glyph emoji/ký hiệu của font.
- Điều kiện nghiệm thu: kiểm tra các ngôn ngữ bắt buộc với mọi font chọn được, cỡ chữ nhỏ/lớn, tên/lời dài và chuỗi trộn nhiều hệ chữ; không ô vuông, ký tự thiếu, dấu sai vị trí hay chữ bị cắt. Demo trên trình duyệt là bước duyệt thiết kế; kiểm tra renderer trên NextUI được làm trong bước build sau.

## 12. Một mục About / hướng dẫn / ủng hộ

Một điểm truy cập chung trong menu, tên đã chọn là **Thông tin & hướng dẫn / About & Help**. Nội dung có thể chia thành các phần nhỏ hoặc tab bên trong để dễ đọc.

### 12.1. Thông tin hiển thị

| Trường | Nội dung thống nhất |
| --- | --- |
| Tên | NXT Retro Music |
| Nhãn | Vibe Code |
| Lên ý tưởng | Nguyễn Xuân Thông |
| Hợp tác phát triển | ChatGPT 6.1 Sol |
| Ủng hộ qua MoMo | **0339838267** |
| Phiên bản app/ngày phát hành | Điền theo bản thực sự được build và phát hành; phiên bản tài liệu 1.1 không tự động là số phiên bản app. |
| Ghi công | Music Player nền và các thành phần liên quan; ngắn gọn, giữ ghi công và giấy phép cần thiết. |

- Mục ủng hộ là tự nguyện, không chặn chức năng.
- Dùng đúng số MoMo có số 0 ở đầu; bỏ các ghi chú cũ “chưa cung cấp MoMo” và “86.1 Sol cần làm rõ”.
- Logo/icon app đã chọn: **tai nghe ôm ba cột phổ âm dạng pixel**, nền than đậm và nét xanh ngọc, góc bo nhẹ. Có bản sáng/tối và đơn sắc; không dùng ký tự font làm hình logo.
- Icon chức năng dùng bộ nét đơn giản, thống nhất theo Lucide; phát/tạm dừng, lặp, ngẫu nhiên, pin, Wi-Fi, Bluetooth, DAC, hàng đợi và lời được vẽ bằng hình vector/raster từ vector. Việc hiển thị icon không phụ thuộc font ký tự.
- Không đưa danh sách nguồn tham khảo kỹ thuật/bibliography vào trang About. Phần ghi công ngắn và giấy phép của thành phần sử dụng vẫn được giữ.

### 12.2. Hướng dẫn

- Bảng nút theo màn hình: menu/thư viện, Now Playing, hàng đợi, lời và EQ; ghi rõ nhấn ngắn, giữ hoặc tổ hợp nếu dùng.
- Bảng tự phản ánh nút đã tùy chỉnh, không chỉ ghi bản mặc định.
- Hướng dẫn ngắn cho chọn thư mục, tìm/chọn bài, playlist, lời LRC, EQ/preset, đầu ra/DAC, Pause/Dừng/thoát, phát nền, hẹn giờ, LED và ngôn ngữ.
- Xem được offline, bằng English/Tiếng Việt; chữ xuống dòng và cuộn khi cần.
- Mở/đóng About và hướng dẫn giữ nguyên phiên nhạc.

## 13. Các lựa chọn đã chốt

| Mục | Cấu hình đã chốt |
| --- | --- |
| Bố cục Now Playing | Thanh trạng thái trên; ảnh trái/thông tin phải; lời thu gọn tùy hiển thị; phổ và tiến trình dưới; icon chế độ/hẹn giờ ở cuối. |
| Không trùng nút | Hiểu theo từng màn hình và từng cử chỉ; dùng bảng đã chốt ở mục 8.2. Đối chiếu phím OS khi triển khai. |
| EQ/DSP | EQ 10/15/31 dải, mặc định 10; preset Flat/Pop/Rock/Jazz/Classical/Vocal/EDM; các thành phần ở bảng mục 6.3. |
| Hẹn giờ khi Pause | Tiếp tục đếm theo thời gian thực. |
| Mở lại sau Dừng | Khôi phục bài/vị trí và chờ Play. |
| Chính sách rút DAC | Mặc định tạm dừng; có tùy chọn chuyển đầu ra. Chính sách khi cắm DAC cũng cần thể hiện rõ. |
| Font và LED | 16 font đi kèm, mặc định Noto Sans; font hỗ trợ đa ngôn ngữ tự động. LED theo mức âm thanh, màu người dùng chọn. |
| Nhập/xuất playlist | M3U; xuất UTF-8 `.m3u8`. |
| Điểm DAC | Điểm tham khảo /10 theo danh mục có căn cứ cho đúng model; thiếu dữ liệu hiện “Chưa có đánh giá”. |
| Logo/icon app | Tai nghe + ba cột phổ dạng pixel, tông xanh ngọc/than đậm; icon chức năng kiểu nét thống nhất. |

Người dùng đã chốt các lựa chọn trong bảng, đồng thời yêu cầu thêm nhiều font, chọn logo/icon và xem demo trước khi build. Các hiệu ứng DSP mở rộng ở mục 6.3 nằm sau phạm vi bản đầu. Việc cho điểm DAC vẫn cần dữ liệu thật và căn cứ đủ; chốt chính sách không đồng nghĩa đã có điểm cho từng model.

## 14. Kiểm tra nghiệm thu khi triển khai

| Tình huống | Kết quả cần đạt |
| --- | --- |
| Mở app trên NextUI / Brick Pro | Gói đúng thiết bị, mở được, nút và bố cục theo bản được chốt. |
| Phát các định dạng đã yêu cầu | Phát và tua được; AAC/ALAC cùng đuôi M4A vẫn ghi đúng codec. |
| Chọn thư mục cha nhiều cấp | Nhận đủ nhạc theo phạm vi đã chọn; vẫn duyệt theo thư mục. |
| Mở hàng đợi và chọn bài 30 | Nhạc tiếp tục khi duyệt; chỉ đổi bài khi xác nhận. Con trỏ và bài đang phát phân biệt được. |
| Sửa playlist, yêu thích, nhập/xuất | Danh sách và thứ tự được lưu đúng, mở lại dùng được. |
| Tên bài/lời Hàn, Nhật, Trung giản thể/phồn thể, Thái và tiếng Việt | Thử cả 16 font và các mức chữ cho phép: không ô vuông, sai dấu, mất glyph, cắt cụm chữ hoặc chồng vùng; kiểm tra riêng chuỗi trộn nhiều hệ chữ. |
| Unicode trong tag, tên file/thư mục và LRC | Đọc và lưu đúng nội dung; chọn/phát được file có đường dẫn đa ngôn ngữ; đổi ngôn ngữ UI vẫn giữ nguyên chữ. |
| Đổi English/Tiếng Việt và theme/font | Toàn bộ nội dung liên quan cập nhật và lưu; phiên phát giữ nguyên. |
| Chỉnh EQ trên biểu đồ và chọn preset | Gain, đường biểu đồ và âm thanh thay đổi tương ứng; preset riêng lưu/khôi phục đúng. |
| Gapless, ReplayGain và crossfade | Hoạt động theo lựa chọn; hiển thị đúng chế độ chuyển bài. |
| Pause, Dừng, thoát và mở lại | Phân biệt đúng hành vi; dừng lưu vị trí, mở lại phiên nền không tạo phiên trùng. |
| Thoát khi bài A đang phát, ở nền sang B | Vào lại hiển thị B, đúng vị trí, ảnh, lời, phổ, đầu ra và cài đặt. |
| Tắt màn hình và mở/chơi game | Nhạc tiếp tục; app không lấy thao tác game. Thử loa và DAC với âm thanh game. |
| Hẹn giờ rồi Pause/tắt màn hình/thoát UI | Bộ đếm và thời điểm Dừng theo chính sách đã chốt; lưu bài/vị trí khi hết giờ. |
| Cắm/rút/đổi DAC | Đầu ra, tên/model, thông số, âm lượng và điểm nếu có cập nhật đúng; không giữ dữ liệu DAC cũ. |
| DAC chưa có dữ liệu đánh giá | Hiện “Chưa có đánh giá”; không suy đoán chip hoặc điểm. |
| Bật/tắt và đổi màu LED | Hiệu ứng phản ánh nhạc, màu và cài đặt được lưu; kiểm tra khi phát nền. |
| Nhấn, giữ, khóa và đổi nút | Không phát sinh thao tác kép; mở khóa được; hướng dẫn đúng nút đã đổi. |
| About / hướng dẫn / MoMo | Một điểm truy cập chung; đúng Nguyễn Xuân Thông, ChatGPT 6.1 Sol và 0339838267; nhạc vẫn tiếp tục. |
| Radio, podcast và YouTube | Các chức năng được giữ và kiểm tra với mạng/dịch vụ thực tế khi triển khai. |

## 15. Sản phẩm bàn giao sau khi đã duyệt và build

- Gói `.pak` cho NextUI / TrimUI Brick Pro, đóng ZIP để chép vào thẻ.
- Mã nguồn và bộ build trực tiếp trên WSL, có thông tin phiên bản nguồn/phụ thuộc để build lại.
- Hướng dẫn cài đặt, sử dụng nút và danh sách chức năng đã hoàn thành hoặc còn giới hạn.
- Kết quả kiểm tra build/giao diện tách rõ khỏi kết quả thử phần cứng; chỉ ghi “đã kiểm tra trên Brick Pro” khi có thử thực tế.
- Giữ giấy phép và ghi công cần thiết trong gói mã nguồn/app; không đưa nhật ký build cũ hoặc danh sách tham khảo kỹ thuật vào luồng giao diện chính.

**Bước tiếp theo:** xem demo giao diện và đưa ý kiến chỉnh nếu có, sau đó triển khai và build theo bản yêu cầu đã chốt. Trong lượt cập nhật này chỉ tạo demo thiết kế, chưa build gói app cho Brick Pro.
