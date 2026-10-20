# NXT Retro Music

**Trình nghe nhạc cho dòng TrimUI Brick, Brick Hammer và Brick Pro trên NX Redux và NextUI, kèm bộ cài Stock riêng cho Brick Pro.**

**Một dự án vibe coding, tập trung vào nghe nhạc và cá nhân hóa.**

[English](README.md) · [Tiếng Việt](README.vi.md)

NXT Retro Music kết hợp nhạc trong thẻ nhớ, các nguồn nghe online, USB DAC, lời song ngữ và nhiều tùy chọn cá nhân hóa trên một chiếc máy chơi game cầm tay. Chọn hồ sơ, chọn giao diện và tạo không gian nghe nhạc riêng của bạn.

**Ý tưởng và thực hiện:** Nguyễn Xuân Thông — **Spring Thông**. Đây là dự án cá nhân được phát triển theo cách **vibe coding**, dùng công cụ AI hỗ trợ triển khai, sau đó chỉnh sửa qua nhiều vòng thử máy và phản hồi. Dự án kế thừa những thành phần mã nguồn mở đã có; phần ghi công và giấy phép được giữ lại ở cuối trang.

README này giới thiệu **v1.9.7.7**, với bộ cài riêng cho **Stock, NextUI và NX Redux**.

## Thông tin

- **Tác giả:** Nguyễn Xuân Thông — **Spring Thông**
- **Dòng thiết bị:** TrimUI Brick · Brick Hammer · Brick Pro
- **Các bộ cài:** Stock · NextUI · NX Redux
- **Phiên bản:** **1.9.7.7**
- **Kiểm chứng:** Tác giả thử sử dụng trên Brick Pro. Brick và Brick Hammer cần kiểm tra trên máy/ROM cụ thể; nút bấm, LED và DAC có thể khác nhau.

[Tải các bản phát hành](https://github.com/SpringThong/NXT-Retro-Music/releases) · [Báo lỗi](https://github.com/SpringThong/NXT-Retro-Music/issues/new)

## Tải và cài đặt

Chọn đúng ROM, giải nén vào gốc thẻ nhớ, rồi mở NXT trong Apps/Tools. **Chỉ cần tải một bản cài; dùng app không cần tải mã nguồn.**

| ROM đang dùng | Bản cài v1.9.7.7 |
| --- | --- |
| **Stock** | [Tải bản cài Stock](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Stock.zip) |
| **NextUI** | [Tải bản cài NextUI](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_NextUI.zip) |
| **NX Redux** | [Tải bản cài NX Redux](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_NXRedux.zip) |

[Tất cả file phát hành](https://github.com/SpringThong/NXT-Retro-Music/releases/tag/v1.9.7.7) · [Mã kiểm tra SHA-256](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/SHA256SUMS.txt) · [Mã nguồn tương ứng](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Source.zip)

![Auralis Now Playing với lời song ngữ](docs/screenshots/auralis-now-playing.png)

*Ảnh render từ giao diện native, sử dụng bìa và lời mẫu. Đây là ảnh dựng kiểm tra giao diện tiếng Việt, không phải ảnh chụp máy thật.*

## Mục lục

- [Nhìn nhanh các tính năng](#nhìn-nhanh-các-tính-năng)
- [Tương thích và cài đặt](#tương-thích-và-cài-đặt)
- [Phát nhạc, thư viện và playlist](#phát-nhạc-thư-viện-và-playlist)
- [Âm thanh, DAC và DSD](#âm-thanh-dac-và-dsd)
- [Lời bài hát, dịch lời và tô từng chữ](#lời-bài-hát-dịch-lời-và-tô-từng-chữ)
- [Cá nhân hóa theo ý bạn](#cá-nhân-hóa-theo-ý-bạn)
- [Hồ sơ, gợi ý và thống kê](#hồ-sơ-gợi-ý-và-thống-kê)
- [Các nút điều khiển mặc định](#các-nút-điều-khiển-mặc-định)
- [Thiết lập các nguồn online](#thiết-lập-các-nguồn-online)
- [Hiệu năng, pin và thao tác khi tắt màn hình](#hiệu-năng-pin-và-thao-tác-khi-tắt-màn-hình)
- [Chuyển file qua Wi-Fi](#chuyển-file-qua-wi-fi)
- [Cập nhật OTA](#cập-nhật-ota)
- [Build từ mã nguồn](#build-từ-mã-nguồn)
- [Xử lý lỗi và góp ý](#xử-lý-lỗi-và-góp-ý)
- [Ghi công và giấy phép](#ghi-công-và-giấy-phép)
- [Ủng hộ dự án](#ủng-hộ-dự-án)

## Nhìn nhanh các tính năng

| Nhóm | Bạn có thể làm gì? |
| --- | --- |
| Nghe nhạc | Phát nhạc trong thẻ; duyệt theo bài, thư mục, nghệ sĩ, album; quản lý hàng đợi, yêu thích và playlist. |
| Online | Spotify Premium, bài/playlist SoundCloud công khai, nhạc và thư mục Google Drive công khai, radio internet và podcast. |
| Âm thanh | Loa máy, tai nghe 3,5 mm, Bluetooth, USB DAC; chọn Thường, Bit-Perfect hoặc Tự động. |
| DSD | Phát DSF/DFF trên thẻ qua USB DAC tương thích bằng Native DSD, DoP hoặc tự động chọn Native/DoP. |
| EQ/DSP | 12 preset nhanh, EQ đồ họa tùy chỉnh, EQ nâng cao 10/15/31 dải, 6 bộ lọc PEQ và preset riêng. |
| Lời nhạc | Lời trên Now Playing và full màn hình, song ngữ, bản dịch nghiêng, tô từng chữ và tùy chọn phân tích mốc hát bằng Groq. |
| Cá nhân hóa | **51 theme, 60 font, 300 icon để chọn, 180 emoji 3D**, hình nền, màu RGB/HEX và chỉnh giao diện có xem trước. |
| Hiệu ứng | Phổ nhạc nhiều bảng màu/kiểu hiển thị; LED theo nhạc, màu và hiệu ứng áp dụng ngay. |
| Hồ sơ | Tối đa 16 người nghe cục bộ, giao diện/lịch sử riêng, thống kê và gợi ý theo sở thích. |
| Tiện ích | Nhớ bài và vị trí nghe, phát nền, hẹn giờ dừng và đóng app, thao tác khi tắt màn hình, chép nhạc Wi-Fi và OTA riêng từng ROM. |
| Ngôn ngữ | 14 ngôn ngữ app, bàn phím trong app theo ngôn ngữ và font dự phòng đa ngôn ngữ dùng offline. |

### Một vài giao diện khác

| Cassette C60 | Studio Meter |
| --- | --- |
| ![Trình phát Cassette](docs/screenshots/cassette-player.png) | ![Trình phát Studio Meter](docs/screenshots/studio-meter-player.png) |

| Menu Forest Dial | Full lời Auralis |
| --- | --- |
| ![Menu Forest Dial](docs/screenshots/forest-dial-menu.png) | ![Full lời song ngữ Auralis](docs/screenshots/auralis-full-lyrics.png) |

## Tương thích và cài đặt

Mục tiêu của bản hiện tại là **TrimUI Brick Pro**. Chọn đúng bộ cài cho ROM đang sử dụng. Ba bản dùng chung mã nguồn dự án nhưng có đường dẫn khởi chạy và phần tích hợp ROM riêng.

| ROM | Bộ cài | Thư mục app trên thẻ nhớ |
| --- | --- | --- |
| Stock | `NXT_Retro_Music_v1.9.7.7_Stock.zip` | `Apps/NXT_Retro_Music/` |
| NextUI | `NXT_Retro_Music_v1.9.7.7_NextUI.zip` | `Tools/tg5040/NXT Retro Music.pak/` |
| NX Redux | `NXT_Retro_Music_v1.9.7.7_NXRedux.zip` | `Tools/NXT Retro Music.pak/` |

1. Tải ZIP cài đặt đúng ROM từ nơi phát hành của dự án. File `_Source.zip` dành cho phát triển; file `NXT_OTA_Google_Drive_…` là bộ chuẩn bị OTA cho người phát hành.
2. Thoát hẳn NXT trước khi thay bản cũ. Sao lưu dữ liệu cá nhân trước khi cập nhật thủ công.
3. Giải nén bộ cài **vào gốc thẻ nhớ**, gộp thư mục `Apps` hoặc `Tools` có sẵn trong ZIP. Tránh tạo thêm một lớp thư mục app bị lồng bên ngoài.
4. Mở NXT trong mục Apps/Tools của ROM.
5. Chọn hoặc tạo hồ sơ người nghe. App mở Now Playing với bài và vị trí đã lưu của hồ sơ đó. Mặc định chưa tự phát, trừ khi bạn đã bật tùy chọn tự phát.
6. Vào **Cài đặt → Thư viện & tệp → Thư mục nhạc** để chọn nguồn nhạc, sau đó mở Thư viện.

Bật Wi-Fi, ghép và kết nối Bluetooth trong ROM trước. USB DAC còn phụ thuộc phần cứng, driver USB/âm thanh của ROM và DAC. Các bộ cài này chưa xác nhận khả năng chạy trên những mẫu TrimUI khác.

## Phát nhạc, thư viện và playlist

- Trình phát native hiện nhận các định dạng **FLAC, WAV, MP3, OGG, Opus, M4A, AAC, ALAC và âm thanh trong MP4**, cùng đường phát DSD USB riêng cho DSF/DFF. Khả năng giải mã còn phụ thuộc codec thực bên trong file.
- Duyệt theo **Tất cả bài / Thư mục / Nghệ sĩ / Album**, tìm trong thông tin bài và đổi cách sắp xếp. Sắp xếp tự nhiên giữ tên như `1`, `2`, `10` theo thứ tự hợp lý.
- Thư viện có chỉ mục lưu sẵn để mở nhanh. App kiểm tra thay đổi tăng dần ở nền, bỏ đọc lại metadata của file không đổi và gom cập nhật cơ sở dữ liệu. Trong tùy chọn Thư viện vẫn có kiểm tra nhạc mới và quét lại toàn bộ.
- Quản lý hàng đợi, phát tiếp theo, yêu thích, ngẫu nhiên và lặp Tắt/Một bài/Tất cả. Chuyển bài trước/sau tuân theo chế độ ngẫu nhiên đã chọn.
- Tạo, đổi tên, đổi thứ tự và xóa playlist; nhập M3U/M3U8, xuất M3U8.
- Nhớ vị trí nghe, tùy chọn tự phát khi mở app và tiếp tục phát khi quay về menu hoặc đưa app xuống nền.
- Luồng PCM Thường có các lựa chọn **Thông thường, Liền mạch và Chồng âm**, kèm thời gian crossfade tùy chỉnh. Việc áp dụng phụ thuộc nguồn nhạc và chế độ âm thanh.
- Now Playing và mục thông tin bài hát/đầu ra hiển thị số thứ tự/tổng bài, tên bài/nghệ sĩ/album, codec, sample rate nguồn, độ sâu bit, bitrate, số kênh và thông tin DAC/đầu ra thực tế khi có dữ liệu.

## Âm thanh, DAC và DSD

### Đầu ra và chế độ âm thanh

Mở **Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra**.

| Chế độ | Cách hoạt động |
| --- | --- |
| Thường | Luồng PCM thông thường; có thể áp dụng EQ/DSP và các xử lý phát nhạc tương ứng. |
| Bit-Perfect | Yêu cầu đường USB PCM số nguyên nguyên bản cho nhạc local phù hợp và DAC tương thích. Khi đường này hoạt động, bỏ qua EQ/DSP, limiter, ReplayGain và đổi tần số lấy mẫu. |
| Tự động | Ưu tiên Bit-Perfect USB cho nguồn local đủ điều kiện; dùng Thường nếu đầu ra nguyên bản không khả dụng. Nguồn online dùng luồng Thường. |

App xử lý thay đổi đầu ra, có tùy chọn tự chuyển, lưu âm lượng theo từng đường phát, chọn cách xử lý khi rút DAC và xem chẩn đoán đầu ra. Bạn có thể chỉnh âm lượng trên máy hoặc DAC tùy hỗ trợ. Nếu PCM cần điều chỉnh âm lượng bằng phần mềm, app có thể chuyển sang luồng Thường; chỉ chọn Bit-Perfect trong cài đặt chưa chứng minh mọi mẫu âm thanh đều giữ nguyên. Hãy xem trạng thái đầu ra đang hoạt động.

### EQ và DSP

Nhấn ngắn **SELECT** để mở bảng preset EQ. Nhấn SELECT thêm lần nữa để đóng mà không phải chọn preset; B vẫn quay lại được.

- **12 preset:** Flat, Pop, Rock, Jazz, Classical, Vocal, EDM, Acoustic, Bass Boost, Treble, Lo-Fi và Loudness.
- Tùy chỉnh nhanh 5 dải; EQ đồ họa nâng cao 10, 15 hoặc 31 dải.
- 6 bộ lọc EQ tham số, chỉnh tần số, gain, Q và bật/tắt từng bộ lọc.
- Preamp, bass, treble, cân bằng trái/phải, độ rộng stereo, compressor, crossfeed, limiter và xem đường đáp ứng.
- ReplayGain theo bài/album trong luồng Thường được hỗ trợ.
- Lưu, nạp, đổi tên, nhập, xuất và xóa preset `.nxt-eq`; có cấu hình âm thanh theo đầu ra.

Các thiết lập DSP đã lưu vẫn được giữ khi đường USB nguyên bản bỏ qua xử lý.

### DSF / DFF

Trong Âm thanh & đầu ra, chọn **Đầu ra DSD USB → Native DSD / DoP / Tự động Native / DoP**. Đường DSD nguyên bản dành cho USB DAC tương thích, không tự chuyển ngầm DSD thành PCM để phát qua loa máy hoặc Bluetooth.

Cả **DAC lẫn driver ROM/kernel** phải hỗ trợ cách truyền đang yêu cầu. DoP còn cần đúng tần số PCM làm lớp mang: DSD64 dùng 176,4 kHz; DSD128 dùng 352,8 kHz; DSD256 dùng 705,6 kHz. DAC ghi hỗ trợ Native DSD chưa đồng nghĩa mọi mức DoP đều dùng được.

DAC có thể khai báo tên USB của OEM/bộ điều khiển. Mục **Tên hiển thị DAC** cho phép lưu tên dễ nhận biết thay vì coi USB descriptor là tên thương mại chính xác. Xem thông tin nguồn, cách truyền và phần cứng trong trang đầu ra hoặc log thiết bị.

## Lời bài hát, dịch lời và tô từng chữ

### Đọc lời

NXT có thể dùng `.lrc` cùng tên đặt cạnh bài trên thẻ, lời nhúng trong file, lời đã lưu và các nguồn tìm lời online. Khi thiếu lời gốc, app có thể tìm lời trước rồi mới dịch; không bảo đảm mọi bản thu đều có kết quả khớp.

- Xem lời ngay trên Now Playing hoặc nhấn **Y** để mở full màn hình.
- Chọn hiển thị dịch **Tắt / Bản dịch / Song ngữ**.
- Song ngữ giữ lời gốc và bản dịch cạnh nhau ở cả hai màn hình. Bản dịch dùng chữ nghiêng để phân biệt với lời gốc.
- Bám câu đang hát, cuộn tay, đổi cỡ chữ và điều chỉnh độ trễ lời.
- Chỉnh riêng màu lời đang hát/chưa hát/bản dịch, chữ đậm, viền và bóng. Auralis còn có màu/cỡ thông tin bài và độ sáng bìa riêng cho Now Playing/full lời.
- Chọn 10 thiết lập chuyển lời, gồm Tắt. Phổ nhạc không vẽ chồng vào vùng full lời.

### Dịch vụ dịch và cache offline

| Dịch vụ | Thiết lập trong app |
| --- | --- |
| MyMemory | Không nhập API key trong NXT; vẫn có giới hạn dịch vụ. |
| Groq | Khóa API riêng và model chọn trong app; nhập trên máy hoặc qua TXT. |
| Google Cloud | Khóa/tài khoản Cloud Translation riêng, quyền truy cập và chi phí riêng. |

Ngôn ngữ đích đi theo **ngôn ngữ app**. Có thể chọn lại ngôn ngữ lời gốc khi tự nhận diện chưa đúng. Dịch chạy ở worker nền, ưu tiên các câu cần dùng và tái sử dụng kết quả đã lưu. Cache lời dịch trên thẻ giới hạn **256 MiB**, dùng lại các câu đã dịch khi offline; đây là hạn mức lưu trữ, không phải nạp 256 MiB vào RAM hay bảo đảm mọi bài đều đã được dịch. Các bản lưu cũ có thể được dọn khi đạt hạn mức cache.

Dịch chữ gửi nội dung lời khi tính năng được bật. **Phân tích mốc hát là tính năng riêng, chỉ gửi âm thanh sau khi bạn đồng ý.** Hạn mức, quyền dùng model, chất lượng và chi phí nếu có phụ thuộc dịch vụ/tài khoản. HTTP 429 là giới hạn tốc độ/hạn mức; app chờ thử lại, không vượt giới hạn. Khi ẩn app hoặc tắt màn hình, yêu cầu dịch còn chờ được tạm dừng để giảm việc chạy nền.

### Tô từng chữ theo nhạc

App phân biệt ba nguồn mốc thời gian:

1. **Mốc chữ thật:** Enhanced LRC có thời điểm từng chữ, dùng offline mà không cần khóa dịch vụ dịch.
2. **Phân tích giọng hát Groq tùy chọn:** Dùng khóa Groq và Whisper Large v3 để phân tích file local được hỗ trợ, tối đa **10 phút**, sau khi xác nhận gửi âm thanh. Tự phân tích mặc định tắt. Kết quả có cache riêng dùng offline, trong giới hạn **16 MiB / 256 file mốc chữ**.
3. **Ước lượng offline:** Tùy chọn ước lượng theo mốc câu và độ dài chữ, có nhãn ước lượng. Cách này không nhận diện giọng ca sĩ.

Tắt ước lượng nếu chỉ muốn tô chữ khi có mốc gốc hoặc mốc tạo ra đã ghép khớp. Groq có thể nhận sai tiếng hát, bè, phát âm hoặc bản thu khác; tăng/giảm độ trễ chung không sửa được từng mốc chữ sai. App không ghi lại luồng online được bảo vệ hoặc đường DSD raw để phân tích.

## Cá nhân hóa theo ý bạn

Mở **Cá nhân → Hồ sơ → Giao diện cá nhân**. Theme, LED và phổ nhạc nằm ở đây; các thiết lập thiết bị/âm thanh nằm trong Cài đặt.

### Theme và Auralis

- **Bộ sưu tập giao diện:** 51 bố cục, gồm Original NXT, cassette/radio/đĩa nhạc, các bảng màu hiện đại và Auralis.
- **Tùy chọn giao diện gốc:** theo hệ thống/sáng/tối, bảng màu, RGB/HEX riêng, font, cỡ chữ giao diện/lời, kho hình nền, lớp phủ, độ đậm ô menu, ảnh bìa/đĩa than, chuyển động, icon menu, độ trễ và kiểu lời.
- **Auralis:** bìa album phủ toàn màn hình phía sau lời, nút glass, các cặp lời song ngữ và màn hình full lời riêng. Mục tùy chỉnh riêng **chỉ hiện khi đang chọn Auralis**.
- Chỉnh độc lập độ sáng bìa Now Playing/full lời của Auralis; dùng font tham chiếu hoặc bộ font đã chọn.
- **Tinh chỉnh v1.9.7.7:** dựng lại 316 icon có sẵn từ vector, làm mịn góc bo/đường chéo, tăng nhẹ nút Auralis, nền nút bật rõ hơn, lọc ảnh khi đổi cỡ và sửa vùng thông tin của Ocean Blue/Boombox.

Ảnh bìa giữ chi tiết nguồn và tỉ lệ. Phóng lớn ảnh nhỏ không tạo thêm chi tiết HD thật.

<details>
<summary>Danh sách đủ 51 theme</summary>

|  |  |  |
| --- | --- | --- |
| 1. Original NXT | 2. Cassette C60 | 3. Vintage Radio |
| 4. Walkman | 5. Reel to Reel | 6. Hi-Fi Rack |
| 7. CD Deck | 8. Jukebox | 9. CRT Terminal |
| 10. Turntable | 11. Modern Focus | 12. Modern Ambient |
| 13. Boombox | 14. MiniDisc | 15. Pocket Wheel |
| 16. Classic 90s | 17. Tube Amplifier | 18. Gramophone |
| 19. Car Audio | 20. Synth Studio | 21. Oscilloscope |
| 22. Bento | 23. E-Ink | 24. Hologram |
| 25. Nordic Studio | 26. Night Drive | 27. Jazz Lounge |
| 28. Soft Paper | 29. Aurora | 30. Ocean Blue |
| 31. Lo-fi Desk | 32. Film Gallery | 33. Studio Meter |
| 34. Mono Slate | 35. Rose Quartz | 36. Gallery Noir |
| 37. Forest Dial | 38. Coral Tape | 39. Midnight Indigo |
| 40. Copper Sunset | 41. Sakura Glass | 42. Cobalt Grid |
| 43. Desert Radio | 44. Emerald Studio | 45. Lavender Paper |
| 46. Ice Blue | 47. Amber Console | 48. Ruby Velvet |
| 49. Graphite Split | 50. Olive Notebook | 51. Auralis Themes |

</details>

### Font, hình nền, avatar và xem trước

- **60 lựa chọn font**, gồm MiSans, HarmonyOS Sans, OPPO Sans, Inter, Roboto, Be Vietnam Pro, Geist, Public Sans và nhiều font serif/monospace.
- Font dự phòng offline hỗ trợ tiếng Việt và các hệ chữ CJK/Thái tương ứng. Giữ tên và giấy phép của font bên thứ ba; không đóng gói Apple SF Pro.
- Hình nền dựng sẵn, nhập JPG/PNG từ thẻ và kho hình nền online có bản lưu; chỉnh lớp phủ/độ đậm menu cho dễ đọc. Các tùy chọn áp dụng khác nhau giữa nền Original và bố cục theme cố định.
- **300 biểu tượng để chọn, 180 emoji 3D**, avatar từ ảnh/chữ và đổi icon từng mục menu.
- Trình chỉnh có xem trước trực tiếp màu/chữ/hiệu ứng trên bìa đang dùng; đổi qua lại Now Playing/full lời, sau đó **Áp dụng hoặc Hủy**. Không liên tục lưu cấu hình khi đang kéo/chỉnh bản nháp.

| Nút trong xem trước | Thao tác |
| --- | --- |
| D-pad | Chọn thuộc tính và đổi giá trị. |
| Y ở thuộc tính màu | Vào/ra chỉnh RGB. |
| L1 / R1 khi chỉnh RGB | Đổi thành phần màu bước lớn hơn. |
| X | Đổi xem trước Now Playing / full lời. |
| SELECT | Chạy lại hiệu ứng mẫu. |
| A / B | Áp dụng / hủy. |

Lời mẫu được ghi rõ là xem trước; không thay lời thật của bài đang nghe hoặc phát sinh yêu cầu dịch.

<details>
<summary>Danh sách đủ 60 font</summary>

|  |  |  |
| --- | --- | --- |
| 1. Noto Sans | 2. Inter | 3. Roboto |
| 4. Source Sans 3 | 5. Nunito Sans | 6. Manrope |
| 7. Rubik | 8. Ubuntu | 9. Lato |
| 10. Montserrat | 11. Noto Serif | 12. Source Serif 4 |
| 13. Lora | 14. Merriweather | 15. JetBrains Mono |
| 16. Fira Code | 17. MiSans · Xiaomi / NX Redux | 18. HarmonyOS Sans · Huawei |
| 19. HarmonyOS Sans Condensed · Huawei | 20. OPPO Sans 4.0 · OPPO | 21. Roboto Condensed · Google |
| 22. Roboto Slab · Google | 23. Be Vietnam Pro | 24. Plus Jakarta Sans |
| 25. Work Sans | 26. Mulish | 27. Public Sans |
| 28. IBM Plex Sans | 29. Lexend | 30. Onest |
| 31. Geist | 32. Wix Madefor Text | 33. Readex Pro |
| 34. Barlow | 35. Asap | 36. Archivo |
| 37. Exo 2 | 38. Chakra Petch | 39. Prompt |
| 40. Kanit | 41. Sarabun | 42. Quicksand |
| 43. Comfortaa | 44. Space Grotesk | 45. Cabin |
| 46. Open Sans | 47. Arimo | 48. Josefin Sans |
| 49. Bitter | 50. Playfair Display | 51. Literata |
| 52. Vollkorn | 53. Cormorant Garamond | 54. Alegreya |
| 55. Crimson Pro | 56. Fraunces | 57. Spectral |
| 58. Gelasio | 59. Epilogue | 60. Newsreader |

</details>

### Phổ nhạc và LED

**Phổ nhạc:** 21 bảng màu, hai màu RGB tùy chỉnh và 5 kiểu: Solid, Gradient, 7-color Rainbow, VU, Multicolor Mix. VU là kiểu hiển thị mức tín hiệu, đổi màu theo độ lớn. Bật/đổi bảng màu từ trình phát hoặc chỉnh trong Giao diện cá nhân.

**LED:** bật/tắt, độ sáng, màu cố định hoặc theo nhạc, màu sẵn/RGB tự chọn và 6 hiệu ứng: Một màu, Gradient hai màu, Sóng cầu vồng, Chia hai màu, Cực quang, Nhịp thở. Thay đổi áp dụng mà không cần tắt/bật LED thêm lần nữa khi giao tiếp ROM/phần cứng hỗ trợ.

### Ngôn ngữ và bàn phím

Ngôn ngữ mở lần đầu là **tiếng Anh**. Có tiếng Anh, Việt, Pháp, Nga, Hàn, Nhật, Thái, Trung giản thể, Trung phồn thể, Đức, Tây Ban Nha, Bồ Đào Nha Brazil, Ý và Indonesia.

Chọn tại **Cài đặt → Ngôn ngữ**. Bàn phím trong app đi theo ngôn ngữ/layout đã chọn, có Telex tiếng Việt, ghép chữ Hàn, nhập tiếng Nhật và gợi ý chữ Nhật/Trung offline ở các layout tương ứng. Thiết lập này dành cho bàn phím NXT, không đổi bàn phím toàn ROM.

## Hồ sơ, gợi ý và thống kê

- Tạo, đổi tên, chuyển và xóa tối đa **16 hồ sơ người nghe cục bộ**, có xác nhận trước khi xóa. Xóa hồ sơ sẽ xóa dữ liệu của hồ sơ đó, không xóa file nhạc trên thẻ.
- Chỉnh biệt danh, avatar, sinh nhật và giao diện; chọn thể loại, tâm trạng, quốc gia ở Cá nhân, không bắt trả lời check-in mỗi lần vào app.
- Giữ lịch sử, yêu thích, giao diện và tiến trình nghe theo hồ sơ. Có thể tắt ghi lịch sử.
- Khám phá **Nghe tiếp**, **Những bài quen thuộc**, **Chưa nghe**, **Lâu chưa nghe** và nhạc/playlist Spotify, SoundCloud gợi ý theo sở thích, tùy quyền truy cập nguồn.
- Xem thống kê **Hôm nay / 7 ngày / Tháng này**: thời gian nghe, lượt phát, nghệ sĩ, bài/nghệ sĩ nghe nhiều và biểu đồ theo ngày. Lịch sử có phân trang và mở lại bài đã nghe.
- Thống kê hiện áp dụng cho các phiên Local/Spotify/SoundCloud/Drive được hỗ trợ; radio và podcast không tính vào tổng lịch sử của hồ sơ này.

## Các nút điều khiển mặc định

Đây là gán phím mặc định trong **Now Playing**; có thể đổi 16 nút ở **Cài đặt → Cài đặt phím**. Menu và màn hình full lời có hướng dẫn phím theo ngữ cảnh riêng.

| Nút | Chức năng mặc định |
| --- | --- |
| A | Phát / tạm dừng. |
| B | Về menu. |
| X | Hàng đợi / danh sách bài. |
| Y | Full lời. |
| L1 / R1 | Bài trước / bài sau. |
| L2 | Đổi bảng màu/trạng thái hiển thị phổ nhạc. |
| R2 | Yêu thích bài hiện tại. |
| Nhấn ngắn SELECT | Mở/đóng EQ. |
| START | Tùy chọn nhanh: đầu ra, chế độ âm thanh, hẹn giờ, ngẫu nhiên, lặp, thông tin bài/đầu ra… |
| Trái / phải | Tua lùi / tới. |
| SELECT + lên/xuống | Âm lượng máy. |
| SELECT + trái/phải | Độ sáng màn hình. |
| Giữ SELECT | Khóa / mở khóa điều khiển. |
| L3 / R3 | Mặc định không gán; là thao tác nhấn xuống cần analog. |

Nút logo phía trước là phím thoát app. Home/task manager bên hông thuộc ROM; bản NX Redux có tích hợp dịch vụ nhạc của ROM trên firmware được hỗ trợ. Chọn **Phát nền** nếu muốn rời màn hình app mà nhạc tiếp tục.

Trong full lời: lên/xuống cuộn, Y đổi bám lời, X đổi cỡ, L1/R1 chỉnh lệch lời, START mở dịch/mốc chữ, A phát/dừng, B quay lại.

## Thiết lập các nguồn online

### Spotify Premium — file đăng nhập PC và xác thực QR

Mở **Nhạc online → Spotify**. Cần Spotify Premium và xác thực quyền phát hợp lệ. Tìm kiếm, playlist tài khoản, bài đã thích và album đã lưu còn phụ thuộc quyền Web API; app kiểm tra quyền thư viện và quyền phát riêng.

Quy trình đầy đủ dưới đây dùng **nhập file PC + QR**, gồm trường hợp chỉ quét QR thì tài khoản vẫn hiện chưa xác thực:

1. Trên PC, cài Python 3 và mở thư mục `Spotify_Login_PC` đi kèm. Windows chạy `Dang_nhap_Spotify.bat`, hoặc dùng lệnh ngay trong thư mục đó:

   ```bash
   python Spotify_Login.py
   ```

2. Đăng nhập và cấp quyền trong trình duyệt. Công cụ xuất **`spotify-credentials.json`** vào thư mục chạy/nơi xuất đã chọn. Nếu dùng ứng dụng Spotify OAuth riêng đã được cấp quyền, công cụ nhận `--client-id`; callback mặc định là `http://127.0.0.1:8080/callback`.
3. Chép JSON **cạnh `launch.sh` trong thư mục app đúng ROM**, theo bảng cài đặt ở trên.
4. Trên Brick, vào **Spotify → Tài khoản / Tài khoản & công cụ → Nâng cao · nhập đăng nhập PC cũ**.
5. Chọn **Đăng nhập / đổi tài khoản bằng QR**, quét mã hoặc nhập mã đang hiện tại **https://spotify.com/pair**; xác nhận bằng **đúng tài khoản Spotify Premium đã đăng nhập trên PC**.
6. Chờ app hoàn tất xác thực quyền phát rồi chọn nhạc. Phiên đăng nhập hợp lệ được lưu cho lần sử dụng sau.

Một số thiết lập được cấp quyền có thể hoàn tất trực tiếp bằng QR. Nếu chỉ thư viện bị giới hạn, **Phát link bài hát Spotify** có thể dùng phiên đã có quyền phát; cách này không vượt qua việc từ chối xác thực phát nhạc hoặc yêu cầu Premium. Khi lỗi, xem `NXT_Spotify_Log.txt`. Quyền ứng dụng OAuth, scope và chính sách Spotify vẫn được áp dụng.

### Groq — nhập khóa dịch mà không gõ tay trên máy

1. Tạo API key trong tài khoản của bạn tại **https://console.groq.com/keys**.
2. Trên PC, tạo **`groq-api-key.txt`**, ghi một khóa đầy đủ trên một dòng:

   ```text
   gsk_REPLACE_WITH_YOUR_COMPLETE_KEY
   ```

3. Đặt file ở **`Music/groq-api-key.txt` tại gốc thẻ**, hoặc cạnh `launch.sh` của app.
4. Mở **Cài đặt → Phát nhạc & âm thanh → Cách phát → Dịch lời bài hát**, hoặc mở tùy chọn dịch bằng START từ full lời.
5. Chọn dịch vụ **Groq**, vào **Khóa API Groq → Nhập từ TXT**, chọn model và chế độ Song ngữ/Bản dịch.

File dạng `GROQ_API_KEY=...` cũng được chấp nhận. Menu model đi kèm v1.9.7.7 có `openai/gpt-oss-20b`, `openai/gpt-oss-120b` và `qwen/qwen3.8-27b`; việc tài khoản dùng được ID nào do phía dịch vụ quyết định. Khi không có quyền/model không khả dụng, thử model khác trong app hoặc đổi dịch vụ.

Khóa này cũng dùng được cho tính năng phân tích mốc hát riêng, nhưng app phải hỏi đồng ý gửi âm thanh và dùng hạn mức âm thanh của dịch vụ. Giữ riêng các khóa và JSON đăng nhập, không đưa vào repository công khai.

### Google Drive — thư mục và tệp nhạc công khai

1. Chia sẻ tệp/thư mục muốn nghe để người có link đọc được công khai.
2. Vào **Nhạc online → Google Drive → Thêm link Google Drive**, nhập link chia sẻ.
3. Hoặc nhập file `.txt`/`.url`, mỗi dòng một link Drive.
4. Mở link đã lưu, duyệt thư mục con, chọn bài hoặc **Nghe cả thư mục** để tạo hàng đợi nhạc trong thư mục.

App stream file được hỗ trợ, không bắt tải xong toàn bộ để nghe; vẫn có bộ đệm mạng. Link riêng tư/bắt đăng nhập hoặc giới hạn tải của Drive có thể không truy cập được. Có thể làm mới hoặc xóa link đã lưu trong app.

### SoundCloud, radio, podcast và công cụ tải

- **SoundCloud:** tìm bài/playlist công khai và nhạc gợi ý có thể phát, không cần nhập API key SoundCloud. Các hạn chế với bài riêng tư, preview hoặc thuê bao vẫn còn hiệu lực. Giữ bản sửa nhạc công khai từ v1.9.7.6.
- **Radio:** duyệt và nghe đài internet qua module radio.
- **Podcast:** duyệt đăng ký và các tập qua module podcast.
- **Công cụ tải âm thanh YouTube:** source này còn có module tìm/tải/hàng đợi riêng dùng yt-dlp. Đây không phải tích hợp thư viện tài khoản YouTube Music; khả năng hoạt động phụ thuộc helper và nguồn. Sử dụng nội dung được phép tải.

## Hiệu năng, pin và thao tác khi tắt màn hình

Mở **Cài đặt → Hiệu năng thiết bị** để chọn **Tiết kiệm pin / Cân bằng / Hiệu năng** và xem thông số máy.

Các chế độ điều chỉnh khối lượng hiệu ứng. NextUI và NX Redux còn thử áp dụng chính sách CPUFreq có thể khôi phục khi ROM cho phép; Stock dùng chính sách CPU của ROM. Tên chế độ không bảo đảm ROM đã nhận mức giới hạn CPU. Dòng trạng thái cho biết phần đã áp dụng; khi rời phiên, app khôi phục những thiết lập CPU mà nó vẫn quản lý.

Màn hình theo dõi đọc tải/xung CPU, nhiệt độ, RAM và pin từ thông tin phần cứng sẵn có. Lịch sử pin là **mức giảm phần trăm của cả máy trong phiên NXT**, bỏ qua lúc sạc/khoảng ngắt; không phải phép đo riêng điện năng app.

- Đặt hẹn giờ trong tùy chọn nhanh hoặc Cách phát. Hết giờ, NXT lưu tiến trình/cài đặt, dừng nhạc và đóng app, kể cả phiên phát nền. ROM quyết định khi nào máy ngủ; app không bảo đảm deep sleep ngay lập tức.
- Có tự tắt màn hình và phát nhạc dưới nền.
- Bật **Cài đặt → Phát nhạc & âm thanh → Cách phát → Điều khiển khi tắt màn hình** để bấm các nút đã gán mà màn hình vẫn tối. Nút nguồn bật sáng lại; giữ SELECT vẫn khóa/mở khóa được. Mặc định tùy chọn này tắt.
- Cache ảnh/chữ/icon có giới hạn, chỉ mục nhạc tăng dần và giảm việc chạy khi ẩn app giúp hạn chế xử lý không cần thiết. Chuyển động đang hoạt động có thể nhắm nhịp khoảng 60 Hz, thay vì vẽ lại liên tục mọi trang đứng yên.

Độ mượt, nhiệt độ và pin thực tế phụ thuộc ROM, đầu ra, công việc mạng, hiệu ứng và nội dung. v1.9.7.7 có kiểm tra phần mềm/render/build/OTA; **không công bố cam kết FPS hoặc nhiệt độ đã đo trên máy thật**.

## Chuyển file qua Wi-Fi

Mở **Cài đặt → Thư viện & tệp → Chuyển file qua Wi-Fi**. Điện thoại/PC cùng mạng Wi-Fi tin cậy, mở **đúng địa chỉ app hiển thị**, nhập PIN 6 chữ số rồi chọn các file tải lên thư mục nhạc đã cấu hình.

Máy chủ chỉ chạy khi cần. Màn hình có tiến trình; X hủy file, B dừng chuyển. File chép xong được đưa vào lượt cập nhật thư viện. Bản này có chép file Wi-Fi; USB-C dùng cho phần cứng âm thanh được hỗ trợ, không có chế độ chép file USB riêng của app.

## Cập nhật OTA

Mỗi bộ cài có sẵn nguồn cập nhật chính thức đúng ROM. Vào:

**Thông tin, hướng dẫn & cập nhật → Cập nhật phần mềm → Kiểm tra cập nhật**

Người dùng thông thường không phải nhập URL nguồn. Nếu trước đây đã chọn nguồn tùy chỉnh, dùng **Khôi phục nguồn nhà phát hành** để về nguồn đi kèm.

App kiểm tra định danh, ROM/máy, phiên bản, dung lượng, SHA-256 và nội dung ZIP trước khi cài; chuẩn bị bản mới, giữ bản app cũ dự phòng và dữ liệu cá nhân ở ngoài thư mục bị thay. Cần đủ dung lượng thẻ cho ZIP, phần giải nén và bản dự phòng.

**Dành cho người phát hành:** mỗi `NXT_OTA_Google_Drive_v1.9.7.7_<ROM>.zip` chứa đúng bộ cài và trang `CHUAN_BI_OTA.html` tạo `update.json`. Đăng ZIP trước, tạo JSON từ đúng ZIP đó rồi cập nhật nội dung **chính file manifest Drive đã dùng cho ROM đó**, giữ nguyên ID/link. Bộ ZIP không tự đăng OTA. Stock, NextUI và NX Redux phải có nguồn riêng.

## Build từ mã nguồn

Kho công khai này chứa tài liệu phát hành và ảnh minh họa. Kho phát triển và lịch sử commit vẫn riêng tư. Tải [mã nguồn đầy đủ tương ứng v1.9.7.7](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Source.zip), giải nén, rồi chạy các lệnh bên dưới từ thư mục gốc của source. Hai file “Source code” GitHub tự tạo cho kho tài liệu này không chứa mã nguồn ứng dụng.

Ứng dụng chạy native bằng **C/SDL**, có helper Spotify/SoundCloud viết bằng Rust và tài nguyên/thư viện đi kèm. Một cây source tạo ra ba frontend/bộ cài riêng theo ROM.

Bắt đầu từ **source đầy đủ**, giữ thư viện, tài nguyên, vendor source và helper binary cần thiết. Chỉ tải `upstream/src` lên web chưa tạo thành môi trường build hoàn chỉnh. Cross-build frontend cần **Linux x86_64, Python 3.11+**, các tiện ích hệ thống thông thường và mạng để lấy SDK ghim phiên bản ở lần đầu. Công cụ tải/kiểm tra compiler và sysroot tương ứng; không tự thay bằng ARM compiler bất kỳ trong máy.

```bash
# Kiểm tra đầu vào và metadata ba ROM.
python3 tools/build-release.py --rom all --check

# Build bộ cài và bộ chuẩn bị OTA cho cả ba ROM.
python3 tools/build-release.py --rom all --jobs 4

# Hoặc chỉ chọn một ROM.
python3 tools/build-release.py --rom nxredux --jobs 4

# Chỉ biên dịch frontend, chưa đóng gói.
python3 tools/build-release.py --rom all --jobs 4 --build-only
```

Kết quả ở `dist/stock-release/`, `dist/nextui-release/`, `dist/nxredux-release/`. Lệnh build frontend dùng lại helper executable có sẵn; nếu sửa helper cần dùng quy trình/toolchain Rust riêng cho helper đó.

| Vị trí source | Nội dung |
| --- | --- |
| `upstream/src/` | App native dùng chung, trình phát, âm thanh, lời, hồ sơ và UI. |
| `upstream/res/`, `upstream/licenses/` | Ảnh/font/catalog và giấy phép bên thứ ba. |
| `stock/`, `nxredux/`, `upstream/` | Khởi chạy, cấu hình và payload theo ROM. |
| `spotify/`, `soundcloud/` | Source helper online và các thành phần vendor. |
| `relink/`, `sources/` | Thư viện build, source archive và đầu vào ghim phiên bản. |
| `tools/`, `qa/` | Công cụ build/đóng gói/OTA và kiểm tra phần mềm. |
| `Spotify_Login_PC/` | Công cụ OAuth trên PC nếu cần. |
| `SOURCE.json`, `RELEASE_QA_v1.9.7.7.md` | Nguồn gốc snapshot, kết quả kiểm tra và giới hạn xác nhận. |

Mỗi phiên bản phát hành phải giữ source ZIP đầy đủ tương ứng bên cạnh các bản cài. Không cần công khai lịch sử phát triển.

## Xử lý lỗi và góp ý

Khi báo lỗi, ghi phiên bản app, ROM/phiên bản ROM, nguồn nhạc, bước tái hiện, định dạng file và đầu ra/DAC đang cắm. Gửi log liên quan sau khi xem bỏ thông tin cá nhân; không đính kèm API key hoặc dữ liệu đăng nhập.

| Hiện tượng | Nên kiểm tra |
| --- | --- |
| Spotify vẫn chưa xác thực | Hoàn tất nhập PC và xác thực phát QR cùng tài khoản; xem log Spotify. |
| Dịch báo HTTP 429 | Chờ hạn mức dịch vụ phục hồi hoặc chọn nguồn khác đã cấu hình. |
| Bài mới không có bản dịch khi offline | Cần có lời gốc và dịch online trước; offline chỉ dùng lại kết quả đã lưu. |
| Tô chữ lệch giọng hát | Kiểm tra đúng bản thu, độ trễ và mốc chữ là thật, tạo bằng AI hay ước lượng. |
| Native DSD không chạy | Kiểm tra cách truyền/tần số DAC và driver ROM; xem log Native/DoP. |
| Bluetooth kết nối nhưng phát ở nơi khác | Kết nối trong ROM và kiểm tra đầu ra đang hoạt động trong NXT. |
| OTA chưa có bản mới | So phiên bản đã cài với manifest đã đăng; manifest phải mô tả đúng ZIP mới hơn của ROM đó. |

Trong **Thông tin ứng dụng**, START xuất `NXT_App_Log.txt`, Y xuất `NXT_Device_Log.txt`. Có log nguồn như `NXT_Spotify_Log.txt`, `NXT_SoundCloud_Log.txt` để chẩn đoán riêng. Log app/thiết bị xuất cạnh launcher; log launcher/phím có sẵn tùy ROM.

## Ghi công và giấy phép

- **Nguyễn Xuân Thông — Spring Thông:** ý tưởng, định hướng thực hiện, cá nhân hóa và phát triển dự án theo cách vibe coding.
- **Mohammad Syuhada** và **nborodikhin / NextUI Music Player:** phần trình phát gốc. Repository upstream được ghi trong snapshot: https://github.com/nborodikhin/nextui-music-player.
- **NextUI, NX Redux và các dự án cộng đồng TrimUI:** nền tảng firmware và tích hợp.
- **SDL, FFmpeg, librespot, yt-dlp, libsamplerate, SQLite, FreeType, HarfBuzz, FriBidi, utf8proc**, tác giả icon/font và các thành phần phụ thuộc khác.

Ứng dụng kết hợp được phân phối theo **GPL-3.0**; các file mở rộng NXT mang thông báo **GPL-3.0-only**. Xem [văn bản GPL](licenses/NXT-GPL-3.0.txt). Trình phát gốc giữ [giấy phép MIT](licenses/Music-Player-MIT.txt); các thành phần, font và icon khác giữ giấy phép riêng trong source đầy đủ và bản cài. [Source ZIP tương ứng](https://github.com/SpringThong/NXT-Retro-Music/releases/download/v1.9.7.7/NXT_Retro_Music_v1.9.7.7_Source.zip) được cung cấp riêng cho mọi bản cài của phiên bản này. Khi phân phối lại cần giữ các thông báo áp dụng; người nhận vẫn có các quyền theo giấy phép tương ứng.

Đây là dự án cộng đồng độc lập, không phải ứng dụng chính thức của TrimUI, Spotify, SoundCloud, Google hay Groq. Các tên trên chỉ các sản phẩm/dịch vụ tương ứng.

## Ủng hộ dự án

Nếu thích NXT Retro Music, bạn có thể chia sẻ video trải nghiệm, góp lỗi có bước tái hiện, hỗ trợ bản dịch hoặc bấm Star cho repository.

Ủng hộ tùy tâm qua **PayPal: `thongkhung2@gmail.com`**. Việc ủng hộ là tự nguyện, không mở hạn mức dịch vụ đám mây hay quyền tài khoản trả phí.

Làm cho một chiếc máy nhỏ, theo cách mình thích nghe nhạc. — **Spring Thông**
