# NXT Retro Music

Hướng dẫn sử dụng v1.2 — NextUI & Stock

08/10/2026 · TrimUI Brick Pro / TG4040

## NXT Retro Music

Hướng dẫn sử dụng v1.2 — NextUI & Stock

Dành cho TrimUI Brick Pro / TG4040 · Cập nhật 08/10/2026

Màn hình đang phát với đĩa than. Ảnh minh họa từ giao diện ứng dụng, sử dụng dữ liệu bài hát mẫu.

### Bắt đầu nhanh

1. Cài đúng bộ dành cho ROM đang dùng; khởi động lại máy sau khi thay bộ cài.
2. Mở NXT Retro Music. Với nhạc trên thẻ: vào Thư viện, chọn bài và nhấn A.
3. Với nhạc online: kết nối Wi-Fi, mở Nhạc online rồi chọn Spotify, SoundCloud hoặc Google Drive.
4. Khi đang phát: A tạm dừng/phát tiếp, L1/R1 đổi bài, B về menu và giữ nhạc.
5. Giữ SELECT + lên/xuống để chỉnh âm lượng; SELECT + trái/phải để chỉnh độ sáng.

Bảng phím trong tài liệu dùng cấu hình mặc định. Nếu đã đổi phím, xem nhãn cuối màn hình hoặc khôi phục tại Cài đặt → Điều khiển.

## Tra cứu nội dung

| Trang | Nội dung |
| --- | --- |
| 3 | Cài đặt, cập nhật và nhận biết đúng bản |
| 4 | Phím chung, D-pad, hai analog và hai nút Home/Menu |
| 5 | Đang phát: đĩa than và tùy chọn nhanh |
| 6 | Thư viện: thư mục, tìm kiếm và sắp xếp |
| 7 | Playlist, hàng đợi và yêu thích |
| 8 | Spotify: đăng nhập QR, tìm bài, playlist |
| 9 | SoundCloud: nhạc công khai, link, bài đã lưu |
| 10 | Google Drive: link công khai và nghe cả thư mục |
| 11 | Bàn phím Telex tiếng Việt |
| 12 | Lời bài hát và căn độ trễ |
| 13 | Âm thanh, USB DAC và Bluetooth |
| 14 | EQ/DSP và preset |
| 15 | Cài đặt giao diện, thư mục nhạc và phát nhạc |
| 16 | Đổi phím, LED và tiện ích hệ thống |
| 17 | Radio và Podcast |
| 18 | YouTube và Wi-Fi |
| 19 | Phát nền, tắt màn hình và sao lưu |
| 20 | Xử lý lỗi thường gặp |

### Một vài tên gọi trong ứng dụng

| Tên | Ý nghĩa |
| --- | --- |
| Thư viện | Nhạc được quét từ thư mục trên thẻ nhớ. |
| Playlist | Danh sách do bạn lưu để mở lại; không tự chứa tệp nhạc. |
| Hàng đợi | Danh sách đang phát trong phiên nghe hiện tại. |
| Mini player | Dòng bài hát đang phát ở phần dưới các màn hình menu. |
| EQ / DSP | Các bộ xử lý âm thanh như cân bằng tần số, limiter, preamp. |
| DAC | Thiết bị âm thanh USB gắn ngoài. Không cần đăng nhập để nghe nhạc trên thẻ. |

Các nguồn online phụ thuộc kết nối mạng, quyền truy cập bài hát và dịch vụ. Màn hình Radio/Podcast dùng bộ điều khiển riêng; xem trang 17.

## Cài đặt và cập nhật

### Chọn đúng ROM

| ROM | Bộ cài / vị trí trên thẻ |
| --- | --- |
| Stock | Giải nén NXT_Retro_Music_v1.2_Stock_Vinyl_Fix1.zip. Chép thư mục Apps vào gốc thẻ; ứng dụng nằm tại Apps/NXT_Retro_Music/launch.sh. |
| NextUI | Dùng NXT_Retro_Music_v1.2_NextUI_Vinyl_Fix2.zip. Chép cả thư mục NXT Retro Music v1.2.pak vào Tools/tg5040/. |

1. Dừng ứng dụng hoàn toàn, tắt máy rồi lấy thẻ nhớ ra. Nếu đang cập nhật từ bản chạy nền, khởi động lại máy trước khi mở bản mới.
2. Giải nén bộ cài trên máy tính. Chép đúng thư mục theo bảng trên; chọn thay thế tệp khi cập nhật.
3. Chép toàn bộ bộ cài, gồm launch.sh, bin, res và compat của Stock. Không chỉ thay riêng musicplayer.elf.
4. Gắn thẻ, bật máy. Stock: mở từ Apps. NextUI: mở từ Tools.
5. Vào Thông tin & hướng dẫn để xem tên build. Stock mới phải hiện ‘Stock · Vinyl Player Fix1’; NextUI tương ứng là ‘NextUI · Vinyl Player Fix2’.

### Chuẩn bị nhạc trên thẻ

Bạn có thể chép nhạc vào Music ở gốc thẻ và chia thành thư mục con. Nếu dùng thư mục khác, chọn Cài đặt → Thư mục nhạc → Thư mục nhạc. Các định dạng đã hỗ trợ gồm MP3, FLAC, WAV, OGG/Vorbis, Opus, AAC và M4A với AAC/ALAC.

### Khi cập nhật từ bản cũ

Giữ thư mục dữ liệu người dùng để giữ playlist, cấu hình và đăng nhập. Stock dùng .userdata/nxt-stock/; NextUI dùng .userdata/shared/nxt-retro-music/. Hai ROM lưu dữ liệu riêng. Sau khi thay bộ cài, khởi động lại máy để đóng phiên chạy nền cũ.

Nếu tên thư mục trên thẻ bị lồng thành Apps/Apps hoặc Tools/tg5040/Tools/tg5040, ứng dụng sẽ không nằm đúng vị trí. Stock cần sẵn nền ROM và thẻ hệ thống của thiết bị; ZIP này là bộ cài ứng dụng.

## Phím chung và điều hướng

D-pad, analog trái và analog phải đều dùng được để điều hướng. Nhấn ngắn để đi một bước; giữ hướng để lặp. SELECT trong các tổ hợp dưới đây là nút SELECT vật lý trên máy.

| Phím / thao tác | Chức năng |
| --- | --- |
| ↑ / ↓ | Di chuyển lựa chọn trong danh sách. |
| ← / → | Chức năng theo màn hình: đổi giá trị, đổi chỗ hoặc tua nhạc. |
| A | Mở mục, xác nhận hoặc phát bài đang chọn. |
| B | Quay lại. Trong Đang phát, về menu và giữ trạng thái nghe. |
| SELECT + ↑ / ↓ | Tăng / giảm âm lượng đầu ra đang dùng; thang 0–20. |
| SELECT + ← / → | Giảm / tăng độ sáng màn hình; thang 0–10. |
| Giữ SELECT 0,9 giây | Khóa / mở khóa điều khiển. Giữ rồi thả khi thấy thông báo. |
| Nhấn ngắn nút nguồn | Tắt / bật màn hình trong khi nhạc tiếp tục. |
| Logo TrimUI ở mặt trước | Dừng nhạc, lưu trạng thái và thoát hoàn toàn ứng dụng. |
| Home vật lý bên hông — Stock | Mở bảng nhanh của ROM Stock (thường được gọi là Task Manager). Nhạc được giữ khi mở bảng. |
| Home bên hông — NextUI | Giữ cho hành vi của ROM. Để trở về ROM bằng phiên nền, dùng START → Thoát giao diện. |

### Tránh nhầm Home bên hông với logo mặt trước

Hai nút có chức năng khác nhau. Trên Stock, bảng nhanh là bảng của ROM; tùy ROM, nội dung và cách đóng bảng có thể khác. Logo mặt trước là thao tác thoát hoàn toàn NXT.

### Khi màn hình đang tắt hoặc nút đang khóa

Bấm một phím để đánh thức màn hình trước khi điều khiển. Lần đánh thức đầu tiên được dùng cho màn hình để tránh đổi bài ngoài ý muốn. Khi khóa nút, giữ SELECT để mở khóa rồi tiếp tục thao tác.

Trên menu chính, Y mở Đang phát/nghe tiếp và START mở Tùy chọn nhanh. Các nhãn ở cuối màn hình cho biết chức năng phím tại đúng nơi bạn đang đứng.

## Đang phát và đĩa than

Màn hình Đang phát dùng ảnh bìa tròn quay như đĩa than. Khi tạm dừng, đĩa đứng ở góc hiện tại và kim chuyển về vị trí nghỉ; phát tiếp thì đĩa quay tiếp và kim trở lại. Khi thiếu ảnh bìa, ứng dụng dùng nhãn NXT.

| Phím mặc định | Chức năng trong Đang phát |
| --- | --- |
| A / B | Phát–tạm dừng / về menu chính, giữ trạng thái nghe. |
| L1 / R1 | Bài trước / bài tiếp trong hàng đợi. |
| ← / → | Tua lùi / tới khi nguồn hỗ trợ tua. |
| X / Y | Mở hàng đợi / lời toàn màn hình. |
| L2 | Đổi màu phổ âm; đi hết màu thì tắt, nhấn tiếp để bật lại. |
| R2 | Thêm / bỏ yêu thích bài đang phát, theo nguồn nhạc. |
| Nhấn ngắn SELECT | Mở EQ/DSP sau khi thả nút. |
| START | Mở Tùy chọn nhanh; SoundCloud mở menu chọn Tùy chọn phát nhạc / QR bài gốc. |

### Các mục trong Tùy chọn nhanh

| Mục | Cách dùng |
| --- | --- |
| Ngẫu nhiên | Bật/tắt phát ngẫu nhiên trong hàng đợi. |
| Lặp | Tắt / Một bài / Tất cả. |
| Hẹn giờ dừng | 15, 30, 45, 60, 90 phút hoặc nhập số phút; 0 để hủy. |
| Đầu ra âm thanh | Mở nhóm Âm thanh & DAC. |
| Yêu thích bài đang phát | Đổi trạng thái yêu thích của bài hiện tại. |
| Lời thu gọn | Bật/tắt vùng lời trong màn hình đang phát. |
| Tắt màn hình | Tắt đèn màn hình, giữ nhạc. |
| Dừng và lưu vị trí | Dừng phát và ghi lại vị trí để nghe tiếp khi nguồn hỗ trợ. |
| Thoát giao diện | Trở về ROM; nhạc giữ trong phiên nền nếu đã bật Giữ phiên khi thoát. |

Logo Spotify/SoundCloud theo nguồn bài đang phát. Thông tin định dạng bài và PCM đầu ra có thể khác nhau: PCM là tín hiệu ứng dụng gửi đến thiết bị. Radio và Podcast có màn hình riêng, xem trang 17. Với SoundCloud, START → Tùy chọn phát nhạc mở các mục nhanh trong bảng trên.

## Thư viện nhạc trên thẻ

### Quét và duyệt nhạc

1. Mở Cài đặt → Thư mục nhạc, chọn thư mục chứa nhạc. Bật Gồm thư mục con nếu muốn quét cả các thư mục bên trong.
2. Vào Thư viện. Dùng L1/R1 đổi giữa Tất cả bài, Thư mục, Nghệ sĩ và Album.
3. Ở mục Thư mục, nhấn A để mở thư mục; chọn bài rồi nhấn A để phát. Hàng đợi được tạo từ danh sách hiện tại.
4. Khi thêm hoặc di chuyển nhạc trên thẻ, chọn START → Quét lại thư viện. Nếu vẫn thiếu bài, kiểm tra lại thư mục gốc và tùy chọn thư mục con.

| Phím | Trong Thư viện |
| --- | --- |
| A / B | Mở hoặc phát / về thư mục cha hay menu chính. |
| L1 / R1 | Chuyển bốn mục thư viện. |
| X | Nhập từ khóa tìm trong tên bài, nghệ sĩ hoặc album. |
| Y | Đổi thứ tự ở danh sách hỗ trợ sắp xếp. |
| ← / → | Lùi / tiến trang danh sách (10 dòng). |
| START | Quét lại, xóa tìm kiếm, chọn thư mục nhạc hoặc thao tác bài đang chọn. |

### Thao tác bài đang chọn

Mở START → Thao tác bài đang chọn để phát tiếp bài này, thêm vào cuối hàng đợi, thêm/bỏ yêu thích hoặc thêm vào một playlist. Đó là cách bổ sung bài mà không cần tạo lại toàn bộ danh sách đang nghe.

### Thứ tự thư mục

Thư mục đứng trước tệp; tên dùng thứ tự số tự nhiên: 1, 2, 9, 10, 100… rồi A–Z. So sánh không phân biệt chữ hoa/thường và có xử lý chữ tiếng Việt. Vì vậy ‘2 Remix’ đứng trước ‘10 Remix’. Playlist giữ thứ tự bạn đã lưu.

### Ảnh bìa và thông tin bài

Ứng dụng đọc tên bài, nghệ sĩ, album và ảnh bìa từ tệp khi có. Nếu bài không có thông tin, tên tệp được dùng làm tên bài. Lời .lrc nên đặt cùng tên với tệp nhạc. Tệp nhạc nằm ngoài thư mục đã chọn sẽ không tự xuất hiện trong thư viện.

## Playlist, hàng đợi và yêu thích

### Tạo, nhập và sửa playlist

Menu chính → Playlist → Tạo danh sách phát: nhập tên bằng Telex, chọn bài trong thư viện bằng A, rồi B để kết thúc. Nhập M3U/M3U8 cho phép chọn tệp trên thẻ và đặt tên danh sách mới. Tệp nhạc trong danh sách vẫn phải tồn tại ở đường dẫn tương ứng.

| Nơi / phím | Chức năng |
| --- | --- |
| Danh sách playlist: A / X | Mở playlist / xóa playlist sau xác nhận. |
| Bên trong playlist: A / X | Phát từ bài đang chọn / bỏ bài khỏi playlist. |
| Bên trong playlist: ← / → | Đưa bài đang chọn lên / xuống một vị trí. |
| L1 / R1 rồi Y | Chọn thanh công cụ và thực hiện: Phát tất cả, Thêm bài, Đổi tên hoặc Xuất M3U8. |
| Xuất M3U8 | Lưu bản UTF-8 trong thư mục Playlists bên trong thư mục nhạc đã chọn. |

### Hàng đợi đang phát — X từ Đang phát

| Phím | Nhạc trên thẻ / Google Drive |
| --- | --- |
| A | Phát ngay bài đang chọn. |
| X | Bỏ bài khỏi hàng đợi; bài đang phát được giữ. |
| L1 / R1 | Đưa bài đang chọn lên / xuống một vị trí. |
| Y | Đặt tên, lưu playlist và xuất M3U8 vào Playlists. |
| B | Đóng hàng đợi. |

Hàng đợi Spotify/SoundCloud cũng hỗ trợ chọn bài, bỏ bài và đổi chỗ. Việc lưu M3U8 bằng Y ở trên dành cho hàng đợi nhạc trên thẻ/Drive. Playlist Drive lưu các liên kết phát online, không tạo bản nhạc offline.

### Yêu thích theo từng nguồn

R2 trong Đang phát hoặc mục Yêu thích bài đang phát sẽ đổi trạng thái yêu thích. Nhạc trên thẻ được lưu trong Yêu thích của NXT. Spotify lưu/bỏ thích vào tài khoản Spotify khi kết nối và quyền cho phép. SoundCloud lưu dấu trang trên máy, không đồng bộ tài khoản SoundCloud; mở lại qua SoundCloud → Bài đã lưu trên máy.

## Spotify

### Đăng nhập lần đầu bằng QR / mã ghép nối

1. Kết nối Wi-Fi cho máy. Mở Nhạc online → Spotify → Đăng nhập bằng QR.
2. Dùng điện thoại quét QR; hoặc mở liên kết hiển thị, thường là spotify.com/pair, và nhập mã của phiên ghép nối.
3. Đăng nhập đúng tài khoản Spotify Premium trên điện thoại và xác nhận ghép nối/quyền được yêu cầu.
4. Chờ màn hình trên máy báo kết nối và hiện tên tài khoản. Sau đó tìm bài hoặc mở playlist để phát.

Luồng đăng nhập tích hợp của bản này không yêu cầu người tải ứng dụng tự tạo Client ID/Client Secret. Phát Spotify trong NXT cần tài khoản Premium. Mã ghép nối có hạn: nếu quá hạn, yêu cầu mã mới và làm lại.

| Mục | Dùng để |
| --- | --- |
| Tìm kiếm | Chọn Bài hát / Album / Nghệ sĩ / Playlist rồi nhập từ khóa. |
| Khám phá | Tìm nhạc theo các chủ đề gợi ý. |
| Playlist của tôi | Mở danh sách phát thuộc tài khoản. |
| Bài hát đã thích | Duyệt bài đã lưu thích trên Spotify. |
| Album đã lưu | Duyệt album trong tài khoản. |
| Đang phát | Trở về màn hình bài hiện tại. |
| Tài khoản | Xem trạng thái, kết nối lại/ghép nối, đăng xuất và công cụ hỗ trợ. |
| Hướng dẫn đăng nhập | Xem QR, cách ghép nối và xử lý kết nối trong ứng dụng. |

### Điều khiển và đổi tài khoản

A mở/phát, B quay lại; L1/R1 lật trang tại danh sách kết quả. Trong Đang phát, dùng bảng phím trang 5. R2 thêm/bỏ bài thích trên tài khoản; X mở hàng đợi. Muốn dùng tài khoản khác, vào Tài khoản → Đăng xuất, rồi ghép nối lại bằng tài khoản mới.

### Khi không lấy được dữ liệu

Kiểm tra Wi-Fi, gói tài khoản và thông báo lỗi. Một số playlist/nội dung có thể không được dịch vụ cho đọc hoặc phát. Nếu báo thiếu quyền, kết nối lại và chấp thuận quyền; công cụ đăng nhập PC đi kèm là phương án hỗ trợ khi luồng QR gặp lỗi. Không chia sẻ tệp thông tin đăng nhập khi gửi mã nguồn hoặc public bộ cài.

## SoundCloud

Bản này dùng chế độ nhạc công khai. Người nghe không cần đăng ký ứng dụng SoundCloud, xin API key hoặc có tài khoản Pro để dùng các bài công khai mà công cụ tích hợp đọc được.

1. Kết nối Wi-Fi, vào Nhạc online → SoundCloud.
2. Chọn Tìm kiếm để tìm bài/playlist; nhập tiếng Việt bằng Telex hoặc dùng Latin.
3. Chọn bài và nhấn A. Trong Đang phát, dùng A tạm dừng, L1/R1 đổi bài, X hàng đợi, Y lời và R2 lưu bài trên máy.
4. Nếu có link bài hoặc playlist, chọn Mở liên kết SoundCloud; nhập link HTTPS soundcloud.com hoặc link chia sẻ on.soundcloud.com.

| Mục | Cách dùng |
| --- | --- |
| Tìm bài / playlist | Tìm từ khóa; L1/R1 lật trang kết quả. |
| Khám phá | Tìm bài theo chủ đề gợi ý. |
| Bài đã lưu trên máy | Mở dấu trang do R2 lưu; thuộc dữ liệu NXT của ROM đang dùng. |
| Mở liên kết SoundCloud | Mở bài hoặc playlist công khai bằng URL. |
| Đang phát | Trở lại bài hiện tại. |
| Kết nối lại SoundCloud | Khởi động lại phần kết nối khi nguồn bị lỗi. |
| Hướng dẫn thiết lập | Xem cách dùng chế độ công khai và các hạn chế. |

### Hàng đợi và QR của bài

Danh sách được lấy theo trang, hiện dùng tối đa 20 bài mỗi trang. Hàng đợi tạo từ trang đang chọn; playlist dài không được tự nối tất cả các trang. Trong Đang phát SoundCloud, START mở menu với Tùy chọn phát nhạc và Mở bài gốc trên SoundCloud · QR. QR mở bài gốc trên điện thoại; đây không phải mã đăng nhập của NXT.

### Giới hạn cần biết

Bài riêng tư, yêu cầu đăng nhập, trả phí/Go+, giới hạn vùng hoặc chỉ cho nghe thử có thể không phát đủ. Khi mất mạng hoặc link hết hiệu lực, mở lại/kết nối lại. Âm thanh phụ thuộc bản mà dịch vụ cung cấp; ứng dụng không nâng một nguồn nén thành lossless. Lời chỉ hiện khi tìm được dữ liệu phù hợp.

## Google Drive — nghe online

### Chuẩn bị link công khai

1. Trong Google Drive, chia sẻ thư mục hoặc tệp nhạc để người có liên kết được xem. Kiểm tra cả quyền của các tệp nằm bên trong.
2. Sao chép link chia sẻ đầy đủ, ví dụ https://drive.google.com/drive/folders/ID_THU_MUC hoặc https://drive.google.com/file/d/ID_TEP/view.
3. Thử link bằng trình duyệt chưa đăng nhập. Nếu vẫn hỏi quyền truy cập hoặc bắt đăng nhập, NXT chưa đọc được link đó.
4. Trên máy: Nhạc online → Google Drive → Thêm link Google Drive. Nhập link ở chế độ Latin và xác nhận bằng SELECT.

### Nghe một bài hoặc cả thư mục

Mở link thư mục rồi chọn Nghe cả thư mục để tạo hàng đợi tất cả tệp nhạc trực tiếp trong thư mục đó. Nếu chọn một bài ở giữa danh sách, hàng đợi vẫn chứa các bài trong cùng thư mục và bắt đầu tại bài đã chọn. Hết bài sẽ sang bài kế tiếp; bật Lặp tất cả nếu muốn quay lại đầu.

| Thao tác | Kết quả |
| --- | --- |
| A trên thư mục / bài | Mở thư mục con / nghe online. |
| Y trong thư mục | Đọc lại danh sách khi vừa thêm hoặc đổi tệp trên Drive. |
| B trong thư mục | Về thư mục cha hoặc danh sách link. |
| X trên link đã lưu | Xóa link khỏi NXT; không xóa tệp trên Google Drive. |
| Nhập link từ .txt / .url | Chọn tệp trên thẻ. Với .txt, mỗi dòng một link đầy đủ. |
| X từ Đang phát | Kiểm tra hàng đợi, chọn bài hoặc đổi thứ tự. |

### Có cần tải nhạc về máy không?

Stock Vinyl Fix1 và NextUI Vinyl Fix2 nghe online qua bộ đệm RAM. Ứng dụng lưu link, tên và danh sách để mở lại; không lưu nguyên tệp nhạc lên thẻ để phát offline. Mạng yếu có thể phải chờ đệm. Khả năng tua phụ thuộc tệp và phản hồi tải theo vùng của Drive.

### Phạm vi của ‘cả thư mục’

Chỉ lấy tệp nhạc trong thư mục đang mở, không gộp tự động các thư mục con. Mở thư mục con rồi chọn Nghe cả thư mục để nghe nội dung bên trong. Giới hạn duyệt là 4096 mục/thư mục, tối đa 64 link đã lưu. Link riêng tư, quota tải hoặc tệp không còn tồn tại có thể gây lỗi. Không cần tạo Google API client cho cách dùng link công khai này.

## Bàn phím Telex tiếng Việt

Khi ngôn ngữ ứng dụng là Tiếng Việt, các ô tìm kiếm/đặt tên mở bằng Telex. Ô nhập URL và đường dẫn mở bằng Latin để giữ nguyên liên kết. Nhấn Y chỉ đổi giữa TELEX và LAT; bản này đã bỏ kiểu chữ Nga/Cyrillic.

| Phím | Khi bàn phím đang mở |
| --- | --- |
| D-pad / analog | Di chuyển đến ký tự hoặc nút trên bàn phím. |
| A | Nhập ký tự đang chọn; có thể giữ để chọn biến thể nếu phím có. |
| B | Xóa ký tự trước; khi ô đã trống, B hủy nhập. |
| X | Thêm dấu cách. |
| Y | Đổi TELEX ↔ LAT. Tiêu đề hiển thị chế độ đang dùng. |
| L1 | Viết hoa cho lượt nhập kế tiếp. |
| R1 | Bật/tắt khóa chữ hoa. |
| SELECT | Xác nhận nội dung và đóng bàn phím. |

### Ghép chữ và đặt dấu

| Gõ | Ra chữ | Gõ dấu | Kết quả với a |
| --- | --- | --- | --- |
| aa / aw | â / ă | s — sắc | as → á |
| ee | ê | f — huyền | af → à |
| oo / ow | ô / ơ | r — hỏi | ar → ả |
| uw | ư | x — ngã | ax → ã |
| dd | đ | j — nặng | aj → ạ |
| z | Bỏ dấu | Ví dụ | asz → a |

### Ví dụ thực hành

| Chuỗi phím chữ | Kết quả |
| --- | --- |
| tieengs Vieetj | tiếng Việt |
| nguyeenx xuaan thoong | nguyễn xuân thông |
| Tooi yeeu Vieetj Nam | Tôi yêu Việt Nam |
| dduwowngf | đường |

Chọn lần lượt các chữ trên lưới rồi bấm A; X chèn cách giữa các từ. Khi cần gõ tên tiếng Anh hoặc một URL nguyên dạng, đổi sang LAT bằng Y. Nếu gõ URL trong TELEX, các chữ s/f/r/x/j có thể bị hiểu là dấu và làm hỏng link.

## Lời bài hát

### Lời thu gọn và toàn màn hình

Y từ Đang phát mở lời toàn màn hình. START → Lời thu gọn bật/tắt lời trong vùng nhỏ của màn hình đang phát. Lời có thời gian đồng bộ sẽ theo vị trí bài; lời không có thời gian chỉ hiển thị văn bản và bạn cuộn bằng tay.

| Phím | Trong lời toàn màn hình |
| --- | --- |
| A / B | Phát–tạm dừng / quay lại. |
| ↑ / ↓ | Cuộn lời bằng tay và tạm ngừng bám tự động. |
| Y | Bật/tắt bám theo câu đang hát. |
| X | Đổi cỡ chữ lời từ 24 đến 48 px. |
| L1 / R1 | Giảm / tăng độ trễ lời 100 ms mỗi lần. |
| ← / → | Tua lùi / tới 5 giây khi nguồn cho phép. |

### Thêm file .lrc cho nhạc trên thẻ

Đặt tệp LRC UTF-8 bên cạnh nhạc và dùng cùng tên gốc: ‘Bai hat.flac’ đi với ‘Bai hat.lrc’. LRC có mốc thời gian, ví dụ [00:12.50] cho câu bắt đầu ở 12,5 giây. Có thể đọc lời nhúng trong tệp khi có; nguồn online dùng lời từ nguồn hoặc dữ liệu tra cứu phù hợp.

Nếu chưa thấy lời, kiểm tra tên tệp, mã hóa UTF-8 và dữ liệu bài. Spotify/SoundCloud/Drive không bảo đảm mọi bài đều có lời. Sai tên nghệ sĩ hoặc bản remix khác có thể làm kết quả lời không khớp.

### Căn lời sớm / muộn

1. Mở Y và chọn đoạn có câu hát rõ.
2. Dùng L1 hoặc R1 chỉnh từng 100 ms; quan sát câu được tô màu để chọn chiều phù hợp.
3. Dùng Y bật lại chế độ bám lời sau khi đã cuộn bằng tay.
4. Có thể chỉnh giá trị trực tiếp tại Cài đặt → Giao diện & font → Độ trễ lời. Thiết lập này áp dụng chung cho phiên/config, không phải bản sửa riêng từng file LRC.

Tăng cỡ chữ khiến mỗi dòng dài có thể được ngắt thành nhiều dòng. Font đa ngôn ngữ đã đi kèm và hoạt động offline cho nội dung được hỗ trợ.

## Âm thanh, DAC và Bluetooth

### Chọn đầu ra

1. Vào Cài đặt → Âm thanh & DAC → Đầu ra âm thanh, hoặc START → Đầu ra âm thanh.
2. Chọn Loa máy, Bluetooth hoặc DAC. Bluetooth phải được ghép nối/kết nối trong phần cài đặt của ROM trước; USB DAC phải được nhận qua cổng hỗ trợ.
3. Chỉnh âm lượng của đầu ra đang dùng bằng SELECT + lên/xuống. Có thể đặt riêng Âm lượng loa, Bluetooth và DAC trong nhóm này.

| Mục | Ý nghĩa |
| --- | --- |
| EQ / DSP | Mở bộ chỉnh âm; cấu hình có thể lưu riêng theo đầu ra. |
| ReplayGain | Tắt / Theo bài / Theo album; dùng metadata mức âm lượng có sẵn trong tệp. |
| Khi rút DAC | Tạm dừng hoặc chuyển về loa theo lựa chọn. |
| Tự chuyển sang DAC | Tự chọn DAC khi thiết bị USB được nhận. |
| Âm lượng loa / Bluetooth / DAC | Mỗi đầu ra có mức lưu riêng trong thang 0–20. |
| Thông tin đầu ra | Xem tên đầu ra, thông tin DAC và PCM đang được xác nhận. |
| Cổng USB dưới | Chọn Nghe DAC hoặc Chế độ sạc, theo hỗ trợ của ROM/thiết bị. |

### Dùng cổng USB dưới

Trước khi chọn Nghe DAC, rút cáp sạc/PC rồi chọn chế độ và cắm DAC. Trước khi chọn Chế độ sạc, rút DAC. Nếu chỉ báo ‘Đã gửi yêu cầu’, kiểm tra lại trạng thái cổng; đây chưa phải xác nhận DAC đã kết nối.

### Bản Stock và nút âm lượng trên Black Pearl

Bản Stock giữ xử lý từ DAC Volume Fix1: áp dụng mức âm lượng bằng PCM của ứng dụng, giữ đầu ra DAC khi ROM đổi đầu ra mặc định, chờ xác nhận mất DAC để tránh chuyển loa do ngắt USB thoáng qua. Nút HID âm lượng của DAC vẫn được đọc khi phát nền.

Đã kiểm tra logic bằng mô phỏng nút/USB và thư viện âm thanh Stock; cần kiểm tra lại combo Black Pearl + Delci trên máy thật. Nếu DAC có chỉnh gain riêng hoàn toàn bên trong phần cứng, hành vi còn phụ thuộc DAC. NXT không tự suy ra chất lượng âm thanh từ tên chip hay thông số USB.

## EQ/DSP và preset

Nhấn ngắn SELECT trong Đang phát hoặc mở Cài đặt → Âm thanh & DAC → EQ/DSP. DSP dùng chung cho nhạc mà NXT đưa qua bộ xử lý; hỗ trợ EQ 10/15/31 dải, preamp, limiter và các tùy chỉnh nâng cao.

| Phím | Trong đồ thị EQ |
| --- | --- |
| ← / → | Chọn dải tần. |
| ↑ / ↓ | Tăng / giảm gain từng 0,5 dB (±12 dB). |
| Y | Đổi số dải: 10 → 15 → 31. |
| X | Bật/tắt bỏ qua DSP (bypass). |
| L1 / R1 | Giảm / tăng preamp từng 0,5 dB. |
| A / B | Mở tùy chọn EQ / quay lại. |

### Các mục tùy chọn EQ

A mở các mục: bật DSP/bypass, preset có sẵn, preamp, limiter, Nâng cao/EQ tham số, khôi phục Flat và quản lý preset riêng. Preset sẵn gồm Flat, Pop, Rock, Jazz, Classical, Vocal và EDM. Chọn preset để áp dụng; có thể chỉnh tiếp rồi lưu thành preset riêng.

### Nâng cao

| Mục | Tác dụng / phạm vi |
| --- | --- |
| Bass / Treble | Chỉnh phần trầm / cao (±12 dB). |
| Balance / Stereo width | Cân bằng trái–phải / thay đổi độ rộng stereo. |
| Compressor / Crossfeed | Điều chỉnh độ nén / mức pha trộn chéo hai kênh. |
| Limiter | Giới hạn đỉnh tín hiệu để hạn chế clipping. |
| PEQ | 6 bộ lọc với tần số, gain và Q; Q điều khiển độ rộng vùng tác động. |
| Đáp tuyến / các dải | Xem đường đáp ứng và chỉnh từng dải. |

### Lưu, nhập và xuất preset

Mở Preset riêng để Lưu hiện tại hoặc Nhập từ thẻ. A áp dụng preset, X mở quản lý để đổi tên, xuất hoặc xóa sau xác nhận. Preset .nxt-eq xuất vào Presets trong thư mục nhạc đã chọn. Khi báo CLIP, giảm preamp hoặc mức boost; thử Flat/bypass để đối chiếu khi âm bị méo.

## Cài đặt giao diện và phát nhạc

Menu chính → Cài đặt. A mở nhóm hoặc đổi mục; trái/phải đổi giá trị ở các dòng cho phép. Thay đổi được lưu trong dữ liệu ứng dụng của ROM đang dùng.

### Giao diện & font

| Mục | Giá trị / cách dùng |
| --- | --- |
| Giao diện | Theo hệ thống / Tối / Sáng. |
| Bộ font | Chọn font đi kèm; dùng font đa ngôn ngữ để hiện chữ ngoài Latin. |
| Chữ giao diện | 85–120%, mỗi bước 5%. |
| Chữ lời bài hát | 24–48 px; màn hình lời có phím X để đổi nhanh. |
| Độ trễ lời | Chỉnh sớm/muộn theo ms, phạm vi ±30.000 ms ở mục cài đặt. |
| Ngôn ngữ | Tiếng Việt / English; ảnh hưởng chế độ bàn phím mặc định. |
| Font đa ngôn ngữ | Luôn bật; font dự phòng dùng được offline. |

### Thư mục nhạc

Thư mục nhạc chọn nơi quét thư viện; Gồm thư mục con quyết định có quét sâu hay không; Quét lại thư viện đọc lại sau khi thêm/xóa/di chuyển bài. Một số chức năng xuất playlist/preset tạo thư mục Playlists/Presets bên trong thư mục nhạc này.

### Phát nhạc

| Mục | Giá trị / cách dùng |
| --- | --- |
| Chuyển bài | Thông thường / Liền mạch / Chồng âm. Liền mạch/chồng âm phù hợp nhạc trên thẻ; nguồn online có thể vẫn chờ mở luồng kế tiếp. |
| Thời gian chồng âm | 0,5–12 giây; có hiệu lực khi chọn Chồng âm. |
| Hẹn giờ dừng | Chọn số phút hoặc nhập riêng; 0 hủy. Thời gian vẫn tính khi tạm dừng. |
| Giữ phiên khi thoát | Bật để dùng Thoát giao diện và mở lại phiên đang chạy từ ROM. |
| Tự tắt màn hình | Mở tiện ích hệ thống để chọn 60/90/120 giây hoặc tắt tự động. |

Âm thanh & DAC được giải thích ở trang 13; Điều khiển và LED ở trang 16. Hẹn giờ dừng dừng nhạc, không phải lịch tắt nguồn máy.

## Đổi phím, LED và tiện ích

### Cài đặt → Điều khiển

Có thể gán lại 10 thao tác: Phát/tạm dừng, Menu, Hàng đợi, Lời toàn màn hình, Bài trước, Bài tiếp, Màu phổ/tắt phổ, Yêu thích, EQ/DSP và Tùy chọn nhanh.

1. Chọn thao tác muốn đổi.
2. Dùng trái/phải hoặc A để chuyển phím. Khi phím mới đã được thao tác khác dùng, ứng dụng đổi chỗ hai gán phím để tránh trùng.
3. Kiểm tra nhãn phím ở chân màn hình và thử lại các thao tác.
4. Chọn Khôi phục nút mặc định nếu muốn dùng đúng bảng phím trong tài liệu.

Tổ hợp chỉnh âm lượng/độ sáng và giữ 0,9 giây để khóa luôn dùng SELECT vật lý. Hai analog và D-pad vẫn dùng chung hướng. Việc đổi phím có thể ảnh hưởng các nơi dùng cùng gán phím, nên hãy đọc nhãn tại từng màn hình.

### Cài đặt → LED

| Mục | Cách dùng |
| --- | --- |
| LED theo nhạc | Bật/tắt hiệu ứng phản ứng theo nhạc. |
| Màu LED | Xanh mint / xanh dương / vàng / hồng / trắng. |
| Độ sáng | 0–100%, bước 5%; 0 là không sáng. |

Hiệu ứng phụ thuộc đèn và quyền điều khiển mà thiết bị/ROM cung cấp. Nếu không đổi được đèn, kiểm tra thiết lập LED của ROM rồi thử lại.

### Nhạc online → Wi-Fi & công cụ hỗ trợ

| Mục | Cách dùng |
| --- | --- |
| Tự tắt màn hình | Chọn 60 / 90 / 120 giây hoặc Tắt. |
| Kết nối Wi-Fi | Mở luồng kết nối Wi-Fi; chọn mạng và nhập mật khẩu. |
| Xóa bộ nhớ đệm ảnh album | Xóa ảnh đã đệm để lấy lại khi ảnh bị sai/cũ; không xóa tệp nhạc. |
| Cài/cập nhật công cụ YouTube | Mở công cụ hỗ trợ tải nhạc. Không dùng mục này để cập nhật toàn bộ NXT Stock. |

### Thông tin & hướng dẫn

Mục Thông tin & hướng dẫn mở màn hình Thông tin, hiển thị phiên bản/build và tác giả. Nhấn A tại đây để xem Hướng dẫn các phím; nhấn B để quay lại. Màn hình khởi động chỉ mang tên NXT Retro Music, còn số phiên bản được đặt tại thông tin ứng dụng.

## Radio và Podcast

### Radio — Nhạc online → Radio

Chọn một đài trong danh sách, A để nghe. Y mở quản lý: chọn quốc gia, duyệt đài và thêm vào danh sách. X trên đài đã lưu mở xác nhận xóa; thao tác chỉ bỏ đài khỏi danh sách của ứng dụng. Cần Wi-Fi và luồng đài còn hoạt động.

| Phím trong màn hình nghe Radio | Chức năng |
| --- | --- |
| A | Dừng / mở lại đài; không giữ nguyên thời điểm của radio trực tiếp. |
| ↑ hoặc R1 / ↓ hoặc L1 | Đài tiếp / đài trước trong danh sách. |
| B | Về danh sách và giữ radio nếu đang phát. |
| L2 (hoặc L3) | Đổi màu/tắt phổ âm. |
| Nhấn ngắn SELECT | Tắt màn hình, giữ radio. |

### Podcast — Nhạc online → Podcast

1. Ở danh sách chương trình, Y mở Quản lý. Chọn Tìm podcast hoặc Top shows để tìm chương trình.
2. Chọn kết quả và A để theo dõi, rồi mở chương trình để xem các tập.
3. Chọn tập chưa có trên thẻ, A đưa vào tải; khi đã tải xong, A phát và khôi phục tiến độ đã lưu.
4. Trong danh sách tập, Y làm mới, X đổi trạng thái đã nghe/chưa nghe. Ở danh sách chương trình, X hủy theo dõi sau xác nhận.

| Phím khi nghe Podcast | Chức năng |
| --- | --- |
| A / B | Tạm dừng–phát tiếp / về danh sách tập; khi còn phát, giữ tiếng. |
| ← / → | Lùi 10 giây / tới 30 giây; giữ để tua. |
| ↑ / ↓ | Tăng / giảm tốc độ từng 0,25x; phạm vi 0,5–2x. |
| L2 (hoặc L3) | Đổi màu/tắt phổ âm. |
| Nhấn ngắn SELECT | Tắt màn hình, giữ podcast. |

Podcast trong bản này tải tập về thẻ trước khi nghe; cần dung lượng trống. Các dòng Tiếp tục nghe lưu tiến độ. SELECT + hướng vẫn chỉnh âm lượng/độ sáng trong hai mục này; SELECT đơn có chức năng khác màn hình Đang phát nhạc.

## YouTube và Wi-Fi

### YouTube — tải nhạc về máy

Nhạc online → YouTube là công cụ tìm và tải âm thanh về thẻ. Mục này không phải trình phát YouTube online giống luồng Google Drive. Công cụ tải được đóng gói cùng bộ cài và có mục cài/cập nhật hỗ trợ.

1. Kết nối Wi-Fi. Mở Nhạc online → YouTube → Tìm kiếm nhạc.
2. Nhập từ khóa. Chọn một kết quả và A để đưa vào hàng đợi tải.
3. Mở Hàng đợi tải để xem tiến độ. Chọn mục và X để hủy/bỏ theo trạng thái. Sau khi tải xong, vào Thư viện để chọn và phát tệp.
4. Tệp tải nằm tại Music/Downloaded ở gốc thẻ. Muốn thấy trong Thư viện, quét lại và bảo đảm thư mục này thuộc vùng nhạc đang quét.

### Nếu tìm/tải không chạy

Kiểm tra Wi-Fi, thẻ còn trống và trạng thái công cụ YouTube. Có thể mở Nhạc online → Wi-Fi & công cụ hỗ trợ → Cài/cập nhật công cụ YouTube rồi thử lại. Video riêng tư, giới hạn vùng/tài khoản hoặc nguồn thay đổi có thể không dùng được. Chỉ tải nội dung bạn có quyền sử dụng.

### Kết nối Wi-Fi

1. Bật Wi-Fi trong ROM. Có thể kết nối mạng từ cài đặt của ROM trước khi mở ứng dụng.
2. Trong NXT, vào Nhạc online → Wi-Fi & công cụ hỗ trợ → Kết nối Wi-Fi.
3. Chọn mạng, nhập đúng mật khẩu và xác nhận. Dùng Latin nếu cần giữ nguyên chuỗi mật khẩu.
4. Mở lại nguồn online sau khi máy báo kết nối. Nếu mạng cần trang đăng nhập, NXT không có trình duyệt để hoàn tất trang đó; dùng mạng đã truy cập Internet được.

### Nghe offline và online

| Nguồn | Sau khi tắt Wi-Fi |
| --- | --- |
| Nhạc trên thẻ / tệp tải xong | Có thể nghe offline khi tệp vẫn tồn tại. |
| Spotify / SoundCloud / Drive / Radio | Cần mạng cho luồng tích hợp; không có chế độ tải offline của các ứng dụng chính thức. |
| Podcast | Tập đã tải về có thể nghe offline; tìm/làm mới/tải tập mới cần mạng. |

Mạng mất giữa bài online có thể làm bài chờ đệm hoặc dừng. Khôi phục mạng rồi chọn lại bài; việc tua/khôi phục vị trí phụ thuộc nguồn.

## Phát nền, cập nhật và sao lưu

### Giữ nhạc khi rời giao diện

1. Bật Cài đặt → Phát nhạc → Giữ phiên khi thoát.
2. Trong START → Tùy chọn nhanh, chọn Thoát giao diện để về ROM. B từ Đang phát chỉ trở lại menu NXT.
3. Mở lại NXT từ Apps/Tools để nối lại phiên đang chạy. Nếu đã thay bộ cài, khởi động lại máy trước.
4. Muốn dừng hoàn toàn, dùng logo TrimUI mặt trước hoặc Dừng và lưu vị trí rồi thoát.

Trên Stock, Home bên hông mở bảng nhanh của ROM. Khi NXT đang ở nền, nút nguồn được ROM quản lý; âm thanh khi chạy cùng game/app khác còn tùy cách ROM và ứng dụng kia sử dụng đầu ra. Khi tạm dừng/rút DAC/chuyển đầu ra, hãy kiểm tra biểu tượng và tên đầu ra trước khi phát lại.

### Tắt màn hình và hẹn giờ

Nguồn nhấn ngắn hoặc mục Tắt màn hình chỉ tắt màn hình, giữ nhạc. Tự tắt màn hình áp dụng sau khoảng không thao tác. Giữ nút nguồn là thao tác nguồn của máy/ROM; lưu vị trí có thể thực hiện khi tắt bình thường, nhưng rút nguồn hoặc tháo thẻ bất ngờ có thể mất trạng thái mới nhất.

### Sao lưu dữ liệu của từng ROM

| Dữ liệu | Vị trí |
| --- | --- |
| Stock | .userdata/nxt-stock/ trên thẻ; gồm shared/nxt-retro-music cho cấu hình, playlist và trạng thái NXT. |
| NextUI | .userdata/shared/nxt-retro-music/ trên thẻ. |
| Nhạc | Thư mục nhạc bạn chọn; mặc định thường là Music. |
| Playlist / preset xuất | Playlists/ và Presets/ bên trong thư mục nhạc đã chọn. |

Bật hiển thị tệp ẩn trên máy tính để thấy .userdata. Dừng hẳn NXT rồi sao chép thư mục dữ liệu và nhạc. Để phục hồi, dùng dữ liệu đúng ROM sau khi đã cài lại ứng dụng. Xuất playlist không sao chép các tệp âm thanh; cần giữ các đường dẫn liên quan.

### Khi public dự án

Chia sẻ bộ cài và mã nguồn sạch, giữ giấy phép/thông tin tác giả. Không đóng gói .userdata, token Spotify, credentials, dữ liệu tài khoản hoặc bản sao nhạc cá nhân. ZIP Source dùng để sửa/build; người dùng thông thường cài ZIP Stock/NextUI tương ứng.

## Xử lý lỗi thường gặp

| Hiện tượng | Kiểm tra / cách xử lý |
| --- | --- |
| Vẫn là ảnh bìa vuông | Kiểm tra tên build trong Thông tin & hướng dẫn. Chép lại toàn bộ đúng bộ cài, kiểm tra đúng đường dẫn và khởi động lại để đóng phiên nền cũ. |
| App không hiện trong Apps/Tools | Kiểm tra thư mục bị lồng thêm một cấp, ROM dùng bộ cài khác hoặc chưa chép launch.sh/res/bin/compat đầy đủ. |
| Telex ra chữ lạ / link bị đổi | Tiêu đề phải là Tiếng Việt (Telex) hoặc Latin. Y đổi chế độ; link dùng Latin. Nếu còn CYR, đang mở bản cũ. |
| Không thấy bài trong thư viện | Chọn đúng thư mục, bật thư mục con nếu cần, xóa từ khóa tìm kiếm rồi quét lại. Kiểm tra định dạng và tệp thực tế. |
| Drive không nhận link | Dùng link HTTPS chia sẻ đầy đủ, không dùng link trang Drive chung; kiểm tra quyền bằng trình duyệt chưa đăng nhập và giữ resourcekey nếu link có. |
| Drive hết một bài rồi dừng | Mở thư mục, chọn Nghe cả thư mục; X kiểm tra số bài trong hàng đợi. Link một tệp chỉ có một bài. Lặp tất cả để nghe vòng lại. |
| Drive đệm lâu / không mở bài | Kiểm tra mạng, quota tải, quyền của từng tệp và định dạng. Y làm mới thư mục; thử lại bài sau khi mạng ổn. |
| Spotify chưa phát / không ghép nối | Kiểm tra Premium, mạng và mã còn hạn; tạo mã mới hoặc kết nối lại/quyền tài khoản. Đổi tài khoản qua Đăng xuất. |
| SoundCloud thiếu bài / chỉ nghe thử | Kiểm tra link công khai, giới hạn bài và vùng. Trang kết quả/hàng đợi hiện theo trang; thử Kết nối lại SoundCloud. |
| Chỉnh DAC làm nhạc ra loa | Dùng đúng Stock Vinyl Fix1 và kiểm tra đầu ra DAC. Thử lại nút âm lượng trên DAC, rút/cắm lại khi cần; ghi rõ thiết bị và thao tác để báo lỗi. |
| Không đổi được âm lượng/phím | Mở khóa bằng giữ SELECT 0,9 giây; thử SELECT vật lý + hướng; khôi phục gán phím mặc định. |
| Không có lời / lời lệch | Kiểm tra LRC cùng tên, UTF-8, bản bài hát khớp. Dùng L1/R1 trong màn hình lời để căn trễ và Y bật bám lời. |
| Âm méo / CLIP | Thử EQ Flat hoặc bypass, giảm preamp/boost và kiểm tra gain của đầu ra. |
| Bluetooth không có tiếng | Ghép nối và kết nối trong ROM trước, rồi chọn Bluetooth ở Âm thanh & DAC. |

### Gửi thông tin khi báo lỗi

Ghi tên ROM, tên build, nguồn nhạc, đầu ra âm thanh, các bước bấm và thời điểm lỗi. Có thể gửi ảnh hoặc video kèm NXT_Input_Log.txt/NXT_Launch_Log.txt trong thư mục ứng dụng; log Spotify/SoundCloud nếu được tạo ở đó. Kiểm tra và che thông tin tài khoản/token/link riêng tư trước khi chia sẻ.

