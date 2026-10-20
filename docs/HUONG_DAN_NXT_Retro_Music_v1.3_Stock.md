# NXT Retro Music v1.3 — Stock

Dành cho **TrimUI Brick Pro / TG4040 dùng ROM Stock**. Build: **Stock · LED Driver Fix3**. Ngày phát hành: 08/10/2026.

## Cài đặt và nâng cấp

1. Dừng nhạc và thoát hẳn app. Nếu bản cũ đang phát nền, khởi động lại máy trước khi nâng cấp.
2. Giải nén **NXT_Retro_Music_v1.3_Stock.zip** trên máy tính.
3. Chép thư mục **Apps** vào gốc thẻ nhớ; cho phép thay thế tệp khi nâng cấp. Đường dẫn đúng là `Apps/NXT_Retro_Music/launch.sh`.
4. Chép toàn bộ app gồm `bin`, `res`, `compat`, `launch.sh`, `config.json`; sau đó lắp thẻ, bật máy và mở app từ Apps.
5. Vào **Thông tin & hướng dẫn**: phải thấy v1.3 và **Stock · LED Driver Fix3**. Màn hình khởi động chỉ ghi NXT Retro Music.

Dữ liệu người dùng nằm trong `.userdata/nxt-stock/`: giữ thư mục này để giữ cài đặt, playlist và đăng nhập. Không chép thư mục dữ liệu cá nhân của mình vào bộ cài public. Bộ này dành riêng cho Brick Pro Stock; bản NextUI dùng bộ cài khác.

## LED Driver Fix3

- Sửa lỗi chọn hiệu ứng LED theo log Brick Pro Stock: driver ghi `3 - sniff`, `4 - static`; app đọc bảng hiệu ứng để chọn màu cố định đúng firmware.
- Áp dụng 16 màu mẫu và RGB tự chọn; chế độ Theo nhạc giữ màu đã chọn và chỉ thay độ sáng theo tín hiệu âm thanh.
- Đọc lại RGB/chế độ sau khi ghi; nếu driver chưa nhận hoặc ROM ghi đè màu, app thử lại. Vẫn trả trạng thái LED gốc khi tắt điều khiển hoặc thoát hẳn.
- Mục **Thông tin → Y Kiểm tra** bổ sung bảng hiệu ứng và mã màu cố định được chọn vào `NXT_Device_Log.txt`.
- Bộ hỗ trợ Spotify/SoundCloud giữ nguyên từ Fix2. Khi nâng cấp, giữ `.userdata/nxt-stock/` để giữ đăng nhập đã hoạt động.

### Các sửa DAC và đăng nhập được giữ từ Fix2

- Bổ sung phục hồi khi DAC sẵn sàng muộn hơn 5 giây; nếu ghi cấu hình đầu ra thất bại, app thử lại và chỉ đổi tuyến sau khi ghi thành công. Giữ âm lượng riêng từng đầu ra.
- LED thử lại khi ghi RGB thất bại; áp lại màu/chế độ sau khi ROM thay đổi chúng, kể cả khi animation vẫn đang tắt. Khi chọn màu cố định, dùng màu Đỏ/Xanh dễ phân biệt để thử.
- Nhập file PC chọn nguồn PC cho phát nhạc, kể cả khi vẫn còn file QR cũ. QR được xác nhận chọn nguồn QR. **Kết nối lại** và mở app lần sau giữ lựa chọn đó; không tự đổi nguồn khi Spotify từ chối.
- **Thông tin & hướng dẫn → Y Kiểm tra** xuất `NXT_Device_Log.txt` cạnh `launch.sh`. File ghi đầu ra được yêu cầu/đang chọn, trạng thái mở PCM, retry và các giá trị LED thực tế.
- `NXT_Spotify_Log.txt` giữ các lần thử thay vì bị xóa khi kết nối lại. Khi file vượt khoảng 256 KiB, lần khởi động bộ hỗ trợ kế tiếp chuyển file cũ sang `NXT_Spotify_Log.txt.previous`.

- LED: tắt animation mặc định của Stock khi app nhận điều khiển; màu mẫu và RGB tự chỉnh áp dụng cho màu cố định/theo nhạc. Khi tắt LED trong app hoặc thoát hẳn, trả trạng thái animation gốc.
- DAC: nạp lại cấu hình ALSA trong chính tiến trình trước khi mở đầu ra; cắm/rút DAC tự đổi loa ↔ DAC khi đang nghe. Nếu thiết bị USB chưa sẵn sàng, app thử nhanh trong 5 giây đầu rồi thử lại mỗi 2 giây cho đến khi mở được đầu ra; không cần kill app. Spotify/SoundCloud giữ trạng thái phát khi đổi đầu ra.
- Nâng cấp lần đầu bật **Tự chuyển sang DAC** và chọn **Khi rút DAC → Chuyển về loa**. Các tùy chọn khác được giữ. Sau khi lưu, thay đổi thủ công của bạn được tôn trọng ở những lần mở sau.
- Spotify: xác thực bộ phát qua AP/Login5 trước khi báo kết nối; tải thư viện API là bước riêng. Lỗi giới hạn thư viện không còn giữ app trong vòng chờ QR vô hạn. OAuth làm mới token không bị chặn bởi cooldown API thư viện.
- Nhập PC: giữ `spotify-credentials.json` gốc, lưu bản riêng, yêu cầu quyền 0600 trên hệ thống hỗ trợ quyền tệp Unix và không ghi đè bản đang có khi file nhập sai. Chấp nhận client ID hợp lệ của ứng dụng OAuth đã được bạn cấp quyền; không tự đổi client để vượt hạn mức. Kiểm tra PC/QR cùng tài khoản.
- Thêm **Phát link bài hát Spotify** khi bộ phát đã xác thực: nhập link `https://open.spotify.com/track/...` hoặc URI `spotify:track:...`. Tên bài, nghệ sĩ, album và ảnh lấy từ phiên phát. Link không thay quyền phát và không mở playlist khi API thư viện bị từ chối.

## Các thay đổi chính v1.3

- Wi-Fi đọc trạng thái giao diện mạng đang hoạt động của Stock; phần trăm nằm trong biểu tượng pin, áp dụng cả các màn hình dùng thanh tiêu đề chung.
- Hướng dẫn sử dụng, Spotify và SoundCloud thành trang văn bản cuộn. **Thông tin & hướng dẫn → X** mở các thay đổi v1.3; trong hướng dẫn sử dụng cũng có phím X.
- Nhạc online bỏ Wi-Fi và Công cụ hỗ trợ. Tự tắt màn hình chỉ chỉnh thời gian. Đổi tên **Phát nhạc dưới nền** và **Phát nền**.
- Gán mọi chức năng Đang phát cho 16 nút ứng dụng; đổi nút menu/bàn phím riêng; khôi phục mặc định.
- Spotify giữ đăng nhập QR; báo rõ quyền phát/thư viện và tôn trọng thời gian chờ máy chủ.
- LED: sửa giao thức Stock, 16 màu mẫu, chế độ màu cố định/theo nhạc và bảng R/G/B tự chỉnh.
- Google Drive: đệm tăng dần trên thẻ, dùng lại dữ liệu khi tua, tải trước bài tiếp theo và chuẩn bị giải mã khi đã đệm xong.
- Bàn phím: giữ đệm font/chữ/hướng dẫn và tăng tốc lặp hướng; vẫn dùng Telex/Latin và A/B/X/Y.
- Spotify/SoundCloud: nối thêm kết quả khi cuộn xuống gần cuối và mở rộng hàng đợi khi nghe; không dừng ở 20 bài đầu. Số bài cuối cùng phụ thuộc kết quả và giới hạn dịch vụ.
- Giữ DAC Volume Fix1, Home Stock, đĩa than quay/kim nghỉ, phát nền và thứ tự thư mục số tự nhiên rồi A–Z.

## Phím chung mặc định

D-pad và cả hai analog cùng điều hướng. SELECT trong tổ hợp là nút SELECT vật lý, dù đã đổi ánh xạ trong app.

| Phím | Chức năng |
| --- | --- |
| ↑ / ↓ | Di chuyển lựa chọn; giữ để lặp. |
| A | Mở/chọn/xác nhận theo màn hình. |
| B | Quay lại. |
| SELECT + ↑ / ↓ | Tăng/giảm âm lượng, thang 0–20. |
| SELECT + ← / → | Giảm/tăng độ sáng, thang 0–10. |
| Giữ SELECT khoảng 0,9 giây | Khóa/mở khóa nút. Thả sau thông báo. |
| Nhấn ngắn nguồn | Tắt/bật màn hình; nhạc tiếp tục. |
| Home bên hông | Bảng nhanh/Task Manager của Stock. |
| Logo TrimUI mặt trước | Dừng nhạc, lưu trạng thái và thoát hẳn app. |

Khi màn hình đang tắt, lần nhấn đầu chỉ bật màn hình. Home/Power thuộc ROM và không nằm trong bảng đổi nút của ứng dụng.

## Đang phát

Ảnh bìa tròn quay khi phát; khi tạm dừng đĩa đứng lại và kim về vị trí nghỉ. Phát tiếp giữ góc đĩa và đưa kim trở lại. Logo nguồn giúp phân biệt Spotify, SoundCloud và Google Drive.

| Nút mặc định | Chức năng |
| --- | --- |
| A | Phát/tạm dừng. |
| B | Về menu, giữ phiên nghe. |
| X | Hàng đợi. |
| Y | Lời bài hát. |
| L1 / R1 | Bài trước/bài tiếp. |
| L2 | Đổi màu phổ âm, qua cuối sẽ tắt, rồi bật lại. |
| R2 | Thêm/bỏ yêu thích bài hiện tại. |
| ← / → | Tua lùi/tới; nhạc trên thẻ hỗ trợ giữ để tua. |
| SELECT nhấn rồi thả | EQ / DSP. |
| START | Tùy chọn nhanh. |
| ↑, ↓, L3, R3 | Chưa gán chức năng Đang phát theo mặc định; có thể tự gán. |

**START → Phát nền** trở về ROM và giữ nhạc. Mở lại app để điều khiển phiên nền. Tùy chọn này bật phát nền nếu trước đó đang tắt. **Dừng và lưu vị trí** dừng nhạc; nhấn B về menu không dừng nhạc.

## Thư viện, playlist và hàng đợi

Chép nhạc vào `Music` ở gốc thẻ hoặc chọn thư mục tại **Cài đặt → Thư mục nhạc**. Có thể bật quét thư mục con và quét lại thư viện. Hỗ trợ MP3, FLAC, WAV, OGG/Vorbis, Opus, AAC và M4A AAC/ALAC.

Thư mục xếp số tự nhiên: **1, 2, 9, 10… rồi A–Z**. Trong thư viện, **L1/R1** đổi mục, **X** tìm, **Y** đổi cách sắp xếp. Chọn bài và nhấn A để phát.

Hàng đợi là danh sách của phiên nghe hiện tại. Trong màn hình hàng đợi, A phát bài, X bỏ bài, trái/phải đổi chỗ, Y lưu M3U; xem nhãn cuối màn hình nếu đã đổi nút. Playlist lưu đường dẫn bài, không sao chép tệp nhạc. Khi chuyển/tên đổi tệp gốc, cần cập nhật playlist.

## Spotify

1. Kết nối Wi-Fi trong **cài đặt ROM Stock**.
2. Mở **Nhạc online → Spotify → Đăng nhập bằng QR**.
3. Quét QR hoặc mở link hiển thị và nhập mã; xác nhận bằng tài khoản Spotify Premium trên điện thoại/máy tính.
4. Giữ app mở để hoàn tất kết nối. Đăng nhập được lưu cho lần sau.
5. Tìm bài/album/nghệ sĩ/playlist, chọn bài và nhấn A.

Cuộn xuống gần cuối để lấy trang kế; có thể dùng R1 để yêu cầu tải tiếp. Các trang đã lấy giữ trong danh sách. Khi nghe gần cuối hàng đợi, app yêu cầu thêm bài từ cùng kết quả/playlist. Chế độ Lặp tất cả chỉ quay về đầu khi nguồn báo hết trang; nếu còn trang, app chờ nối thêm.

Sau khi điện thoại/PC xác nhận QR, app kiểm tra tài khoản và quyền phát. Nếu xác thực phát thành công nhưng API thư viện bị giới hạn, app vẫn nhận kết nối và hiện cảnh báo; có thể dùng **Phát link bài hát Spotify**. Tìm kiếm, playlist và bài đã thích vẫn cần API thư viện được Spotify cho phép. API 429 được tôn trọng theo Retry-After; app không ghép lại hoặc chờ đăng nhập vô hạn.

Nếu app báo lỗi xác thực bộ phát, vào **Tài khoản → Kết nối lại**. Chỉ lấy QR mới khi đăng nhập hết hạn/cần cấp quyền lại. `NXT_Spotify_Log.txt` cạnh `launch.sh` ghi bước gặp lỗi; không ghi token. Không cần tạo client ID cho từng người sử dụng cách QR có sẵn; nếu ứng dụng/tài khoản bị dịch vụ từ chối thì mã nguồn app không tự cấp được quyền đó.

Đăng nhập PC: chạy công cụ trong `Spotify_Login_PC`, chép `spotify-credentials.json` vào `Apps/NXT_Retro_Music/` cạnh `launch.sh`, chọn **Tài khoản → Nâng cao · nhập đăng nhập PC cũ**. App giữ file gốc và lưu một bản riêng. Sau khi nhập, app dùng đăng nhập PC này cho cả xác thực phát và thư viện, không để QR cũ ưu tiên trước. Nếu bạn ghép QR mới, app chọn đăng nhập QR; một file PC thuộc tài khoản khác không được dùng ghép thư viện vào phiên QR đó. Có thể dùng `--client-id` với ứng dụng OAuth của bạn đã được Spotify cho phép. File chứa token riêng: giữ làm bản sao cá nhân, không đưa vào bản public và không gửi cho người khác. Quyền tệp trên thẻ FAT32/exFAT phụ thuộc cách ROM gắn thẻ; giữ thẻ và bản sao file đăng nhập riêng tư. Đăng xuất chỉ xóa phiên của app, giữ file PC gốc.

## SoundCloud

Chế độ nhạc công khai dùng bộ hỗ trợ trong app, không yêu cầu người nghe tạo API key hoặc mua Pro. Vào **Nhạc online → SoundCloud**, tìm bài/playlist hoặc thêm link công khai theo các mục nguồn. Hướng dẫn trong nguồn là trang văn bản cuộn.

Cuộn xuống gần cuối hoặc dùng R1 để tải thêm; app nối kết quả và tiếp tục mở rộng hàng đợi khi nghe. R2 mặc định lưu yêu thích **trên máy này**. Bài riêng tư, bị chặn vùng, chỉ có đoạn nghe thử hoặc yêu cầu Go+ có thể không phát được. QR liên kết bài gốc nằm trong menu nguồn.

## Google Drive: nghe online và cả thư mục

1. Trong Google Drive, đặt quyền **Bất kỳ ai có đường liên kết → Người xem** cho thư mục/tệp và các bài bên trong cần nghe.
2. Sao chép link chia sẻ, vào **Nhạc online → Google Drive**, thêm link thư mục hoặc tệp nhạc.
3. Với link dài, bàn phím chọn **LAT** để nhập URL nguyên dạng.
4. Mở thư mục. Chọn **Nghe cả thư mục** để phát từ đầu, hoặc chọn một bài để bắt đầu ở vị trí đó; các bài trong thư mục vẫn vào hàng đợi.
5. L1/R1 đổi bài; nghe hết sẽ chuyển bài theo hàng đợi. Thư mục con cần mở riêng.

v1.3 bắt đầu phát khi đã có dữ liệu đầu bài, rồi đệm tiếp. Vùng đã đệm được đọc lại khi tua; vùng chưa có vẫn cần tải qua HTTP Range. Khi bài hiện tại có đủ dữ liệu đi trước, app tải trước bài kế tiếp. Nếu tải xong, app có thể chuẩn bị sẵn bộ giải mã để giảm thời gian chuyển bài.

Đệm riêng tối đa **2 tệp × 256 MiB**, không tạo bài nhạc cố định trong thư viện. Các tệp đệm được bỏ tên ngay sau khi tạo và giải phóng khi dừng/thoát phiên. Cần thẻ còn đủ trống; tệp quá lớn, nguồn không hỗ trợ Range/kích thước hoặc thẻ ít trống dùng luồng HTTP thông thường. Không thay giới hạn băng thông Wi-Fi/Google Drive: tệp lớn, mạng chậm hoặc hạn mức nguồn vẫn có thể gây chờ.

Hiện hàng đợi gồm bài trong thư mục đang mở, không gộp đệ quy tất cả thư mục con. Giới hạn duyệt: 4096 mục/thư mục, 32 cấp, 64 link lưu.

## Bàn phím Telex / Latin

| Phím | Chức năng mặc định |
| --- | --- |
| D-pad / analog | Chọn phím; giữ để lặp. |
| A | Nhập ký tự; giữ để mở biến thể khi phím có. |
| B | Xóa một bước; khi ô rỗng thì hủy. |
| X | Dấu cách. |
| Y | Đổi **TELEX ↔ LAT**. |
| L1 | Shift cho ký tự kế. |
| R1 | Khóa/mở Caps. |
| SELECT nhấn rồi thả | Xong/xác nhận. |

Telex: `aa → â`, `aw → ă`, `ee → ê`, `oo → ô`, `ow → ơ`, `uw → ư`, `dd → đ`; dấu `s/f/r/x/j` tương ứng sắc/huyền/hỏi/ngã/nặng; `z` bỏ dấu. Ví dụ `tieengs Vieetj → tiếng Việt`, `ddawng → đăng`. LAT dùng cho URL, mã đăng nhập và chữ cần giữ nguyên.

Nếu đã đổi nút menu/bàn phím, các thao tác này dùng ánh xạ đã lưu. v1.3 giữ lại bộ đệm chữ/font để giảm công việc mỗi khung hình, lặp hướng sau 210 ms rồi mỗi 45 ms; độ mượt thực tế còn phụ thuộc máy/font.

## Cài đặt và đổi nút

**Cài đặt → Phát nhạc** gồm chuyển bài, thời gian chồng âm, hẹn giờ dừng, **Phát nhạc dưới nền**, **Tự tắt màn hình**. Tự tắt màn hình chỉ đổi thời gian, không mở menu Wi-Fi/bộ đệm/công cụ YouTube. Wi-Fi và Bluetooth được kết nối trong ROM Stock.

**Cài đặt → Điều khiển**:

1. Chọn một trong 16 nút, nhấn A rồi chọn chức năng Đang phát.
2. Các chức năng gồm phát/tạm dừng, về menu, hàng đợi, lời, bài trước/tiếp, phổ âm, yêu thích, EQ, tùy chọn nhanh, tua, âm lượng, sáng, trộn/lặp, tắt màn hình, phát nền, dừng hoặc không gán.
3. **Đổi nút menu/bàn phím** đổi ánh xạ điều hướng riêng; chọn lại nút sẽ hoán đổi để các chức năng không mất nút truy cập.
4. **Khôi phục tất cả nút mặc định** trả cả hai bảng về ban đầu. Bảng menu cũng có nút khôi phục riêng.

SELECT + hướng và giữ SELECT khóa nút vẫn dùng SELECT vật lý. Home/nguồn giữ chức năng hệ thống. Nhãn trong Đang phát và bảng nút trong hướng dẫn phản ánh cấu hình Đang phát hiện tại.

## LED

Vào **Cài đặt → LED**: bật LED, chọn **Màu cố định** hoặc **Theo nhạc**, màu mẫu và độ sáng. Màu cố định hiện cả khi tạm dừng; theo nhạc thay độ sáng theo tín hiệu âm thanh.

**Bảng màu RGB tự chỉnh** có 16 ô màu mẫu và ba thanh R/G/B:

- ↑/↓ chọn thành phần; ←/→ chỉnh từng đơn vị 0–255; L1/R1 chỉnh nhanh 16 đơn vị.
- X/Y chuyển màu mẫu; màu được xem trước trên LED trong lúc chỉnh.
- A lưu; B hủy và trả cấu hình trước khi xem trước. Chế độ/bật/độ sáng cũ được khôi phục sau khi đóng bảng.

Ứng dụng đọc bảng hiệu ứng của driver, chọn chế độ màu cố định (mã 4 trên Brick Pro Stock đã ghi log), lưu màu riêng và tạm tắt animation mặc định của Stock. App kiểm tra giá trị sau khi ghi và áp lại màu nếu ROM ghi đè. Chế độ Theo nhạc sử dụng cùng màu cố định này, thay độ sáng theo âm thanh. Khi tắt điều khiển LED hoặc thoát hẳn, app trả các thuộc tính LED và animation về giá trị lúc nhận điều khiển.

## DAC, Bluetooth, EQ và lời bài hát

Trong **Cài đặt → Âm thanh & DAC**, chọn Loa/Bluetooth/DAC. Bluetooth phải ghép đôi ở ROM trước. Với cổng USB dưới, chọn chế độ nghe DAC hoặc sạc theo nhắc trên màn hình.

DAC Volume Fix1 vẫn được giữ: chỉnh âm lượng DAC dùng gain PCM và giữ tuyến phát đã chọn. Các sửa DAC từ Fix2 nạp lại cấu hình ALSA trước khi mở thiết bị, cho phép đổi loa ↔ DAC trong cùng phiên. Mặc định **Tự chuyển sang DAC** bật, **Khi rút DAC** chọn chuyển về loa; vì cần xác nhận mất USB nên có khoảng chờ 1,5 giây khi rút. Cắm/rút ngắn vẫn ưu tiên giữ DAC; thiết bị đang khởi tạo được thử nhanh trong 5 giây đầu rồi thử lại mỗi 2 giây; app không bỏ cuộc chỉ vì DAC khởi tạo lâu. Chọn tạm dừng khi rút DAC nếu muốn, app sẽ giữ lựa chọn của bạn.

SELECT mở EQ/DSP theo nút mặc định. ReplayGain Track/Album cân mức âm nếu bài có metadata; chỉnh EQ/preamp vừa phải và dùng limiter khi cần tránh vỡ tiếng.

Y mở lời theo nút mặc định. Nhạc trên thẻ đặt `.lrc` UTF-8 cùng tên cạnh tệp nhạc. Lời Spotify/SoundCloud chỉ xuất hiện khi dịch vụ hoặc nguồn tra cứu có kết quả; chỉnh **Độ trễ lời** trong Giao diện & font nếu cần.

## Radio, Podcast và YouTube

Các nguồn này nằm trong Nhạc online và có màn hình điều khiển riêng; thao tác theo nhãn cuối màn hình. YouTube vẫn là nguồn riêng, không còn mục công cụ trong cài đặt Phát nhạc. URL/nội dung không công khai hoặc nguồn bị hạn chế có thể không sử dụng được.

## Xử lý nhanh

| Hiện tượng | Cách kiểm tra |
| --- | --- |
| Không thấy app trong Apps | Kiểm tra không chép lồng `Apps/Apps`; phải có `Apps/NXT_Retro_Music/config.json`. |
| Vẫn hiện bản cũ | Dừng phiên nền/khởi động lại, chép toàn bộ bộ cài; xem build trong Thông tin. |
| Wi-Fi chưa đúng ngay | Chờ khoảng một giây để làm mới; kiểm tra mạng có địa chỉ IP và vẫn đang nối ở ROM. |
| QR đã xác nhận nhưng Spotify chưa kết nối | Xem bước lỗi trong NXT_Spotify_Log.txt; chọn Tài khoản → Kết nối lại. Nếu chỉ thư viện bị giới hạn mà bộ phát đã xác thực, dùng Phát link bài hát Spotify. |
| Drive không mở | Kiểm tra quyền công khai của tệp bên trong, link đầy đủ và hạn mức Drive. |
| Drive tua/chuyển bài còn chờ | Vùng/bài chưa đệm, thẻ ít trống, tệp quá lớn hoặc mạng chậm; bài kế chỉ có thể tải nhanh bằng tốc độ nguồn cung cấp. |
| Danh sách online chưa nhiều | Cuộn gần cuối/R1; kết nối còn hoạt động, nguồn còn trang; khi nghe gần cuối app cũng yêu cầu thêm. |
| LED còn màu mặc định | Kiểm tra Thông tin có LED Driver Fix3 và khởi động lại để đóng phiên nền cũ. Bật LED → Màu cố định, tăng độ sáng và đổi Đỏ/Xanh. Vào Thông tin → Y Kiểm tra để lưu giá trị driver trong NXT_Device_Log.txt. |
| Cắm/rút DAC chưa có tiếng | Chờ xác nhận rút khoảng 1,5 giây hoặc khởi tạo DAC; app tự thử lại. Giữ Tự chuyển sang DAC bật. Vào Thông tin → Y Kiểm tra ngay khi lỗi xảy ra. |
| Không thấy LED khi dừng | Chọn Màu cố định, bật LED và tăng độ sáng. |
| Đổi nút xong khó điều khiển | Vào Điều khiển → Khôi phục mặc định; nhớ SELECT + hướng vẫn dùng nút vật lý. |

## Lấy thông tin khi cần kiểm tra lỗi

1. Xác nhận Thông tin hiển thị **Stock · LED Driver Fix3**. Thử trong phiên mới sau khi đã thoát phiên nền cũ/khởi động lại máy.
2. LED: chọn màu cố định, bật LED, tăng độ sáng rồi đổi Đỏ/Xanh. Nếu màu không đổi, ngay lúc đó mở **Thông tin & hướng dẫn → Y Kiểm tra**.
3. DAC: phát một bài qua loa, cắm DAC, thử âm lượng; rút DAC và chờ chuyển về loa. Nếu mất tiếng, xuất thông tin bằng Y trước khi thoát app.
4. Spotify PC: vào **Tài khoản → Nâng cao · nhập đăng nhập PC cũ** một lần để chọn rõ nguồn PC. Spotify QR: dùng **Tài khoản → Đăng nhập / đổi tài khoản bằng QR** để chọn nguồn QR. Xem lỗi cụ thể trong `NXT_Spotify_Log.txt`.
5. Khi cần gửi thông tin lỗi, dùng `NXT_Device_Log.txt` và `NXT_Spotify_Log.txt` trong `Apps/NXT_Retro_Music/`; nếu có thì thêm file `.previous`. Không gửi `spotify-credentials.json` hoặc các file đăng nhập trong `.userdata`.

Thông tin thiết bị không chứa token, nội dung file đăng nhập, địa chỉ Bluetooth hay đường dẫn bài nhạc. Nhật ký Spotify ghi lựa chọn nguồn đăng nhập và bước xác thực bị từ chối; lỗi hạn mức/quyền do Spotify cấp vẫn phải được dịch vụ cho phép.

## Kiểm tra của bản build

Mã nguồn có kiểm tra khung hình giao diện và Telex, 21 chức năng × 16 nút, danh sách 80/81 bài, đệm Drive 100 MiB giả lập, LED sysfs, DAC và bộ giải mã ARM64 trên rootfs Stock chính thức. Các kiểm tra Fix1 được giữ: tái hiện cache ALSA thật trên SDL/ALSA ARM64 Stock trong một tiến trình, chuyển loa → DAC → loa bằng PCM giả lập, retry mở USB, migration tùy chọn, quyền animation LED, nhập PC giữ file và cooldown API riêng với OAuth. Fix2 chạy lại và bổ sung phục hồi mở DAC sau 5 giây, ghi tuyến/RGB thất bại rồi hồi phục, ROM ghi đè màu, xuất trạng thái driver, chọn PC/QR và giữ nhật ký. Backend Spotify có 18 kiểm tra Rust trên ARM64 được giữ từ Fix2. Fix3 chạy lại kiểm tra DAC/LED, phiên nền, màn hình hướng dẫn/Thông tin và rootfs ARM64; kiểm tra mới dùng bảng hiệu ứng trích từ log máy thật, đúng mã 4, bảng mã firmware khác, fallback khi không có bảng, tất cả màu mẫu, độ sáng theo nhạc, lỗi đọc lại driver và ROM ghi đè. Đây là kiểm tra phần mềm/giả lập, chưa thử trực tiếp Black Pearl + Delci, LED, GPU hay tài khoản online trên Brick Pro thật. Các kết quả và giới hạn được ghi trong mã nguồn/báo cáo build.
