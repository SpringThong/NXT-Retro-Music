# NXT Retro Music v1.1 — NextUI / TrimUI Brick Pro

NXT Retro Music v1.1 tích hợp nghe Spotify trực tiếp trên Brick Pro, đăng nhập QR, giao diện Đang phát chung, lời bài hát và phát nền. Người dùng đã xác nhận bản tích hợp chạy tốt trên máy; bản v1.1 chuẩn hóa tên ứng dụng, phiên bản và hướng dẫn.

## Thay đổi từ v1.0

- Nghe Spotify trực tiếp, tìm bài, playlist, album và bài hát đã thích ngay trên Brick.
- Đăng nhập QR và tự kết nối lại bằng phiên đã lưu.
- Spotify dùng giao diện Đang phát, ảnh album và thanh phát thu gọn của NXT.
- Lấy lời từ Spotify hoặc LRCLIB, đồng bộ khi nguồn có mốc thời gian; giữ font đa ngôn ngữ.
- L2 đổi màu / tắt bật phổ, R2 lưu yêu thích Spotify, X mở hàng đợi, Y mở lời toàn màn hình.
- Nhạc tiếp tục khi về menu, tắt màn hình hoặc thoát giao diện về NextUI theo cài đặt. Logo TrimUI mặt trước dừng nhạc và thoát hẳn.
- Giữ điều khiển DAC / cổng USB dưới và hiển thị thông số PCM đầu ra.
- Tên hiển thị và bộ cài thống nhất **NXT Retro Music v1.1**; phần Thông tin và Hướng dẫn trong app có các thay đổi này.

**Nghe Spotify cần tài khoản Premium và kết nối mạng. Nhạc trên thẻ nhớ không cần tài khoản Spotify.** Bộ phát librespot đang dùng chỉ hỗ trợ Premium; Spotify Free không phát được qua bộ phát này. Tham khảo: https://github.com/librespot-org/librespot#librespot

## Cài và nâng cấp

1. Nhấn **nút logo TrimUI ở mặt trước** để thoát hẳn bản đang chạy.
2. Nếu đang dùng bản tích hợp trước, đổi tên folder Pak cũ thành **NXT Retro Music v1.1.pak**. Giải nén bộ cài và chép đè **đầy đủ** nội dung Pak v1.1 vào folder đó, thường là **Tools/tg5040/**. Nếu cài mới, chép folder **NXT Retro Music v1.1.pak** vào **Tools/tg5040/**.
3. Giữ nguyên dữ liệu người dùng. App vẫn dùng thư mục dữ liệu chung `.userdata/shared/nxt-retro-music` trên thẻ, nên đổi tên Pak không làm mất cài đặt, thư viện hay đăng nhập. Không xóa `credentials.json` hoặc `playback.json` khi nâng cấp.
4. Bật Wi-Fi, mở **NXT Retro Music v1.1 → Nhạc online → Spotify**. Đăng nhập đã lưu tiếp tục dùng được; chỉ quét QR lại khi app yêu cầu cấp thêm quyền hoặc đăng nhập hết hạn.

Chỉ dùng bộ cài này cho NextUI / TrimUI Brick Pro. Nút logo phải thoát hẳn phiên đang chạy trước khi nâng cấp; đổi tên hoặc chép file khi phiên còn chạy sẽ không cập nhật phiên đó. Đừng giữ hai Pak trùng nhau nếu chỉ dùng bản v1.1.

## Đăng nhập bằng một lần QR

1. Chọn **Đăng nhập bằng QR**, hoặc để app mở màn hình đăng nhập khi chưa có tài khoản.
2. Quét QR trên Brick bằng điện thoại. Có thể mở **https://spotify.com/pair** và nhập mã đang hiện.
3. Đăng nhập tài khoản **Spotify Premium** trên trang Spotify rồi xác nhận quyền.
4. Giữ màn hình QR trên Brick đến khi app báo **Đã đăng nhập; chọn bài để nghe**. App tự tải tài khoản và thư viện.
5. Vào Playlist của tôi, Bài hát đã thích, hoặc Tìm kiếm; chọn bài và nhấn A.

**Nếu đăng nhập cũ chưa có quyền thư viện và yêu thích, quét QR lại một lần** để cấp thêm quyền. Những lần mở tiếp theo app tự kết nối bằng đăng nhập đã lưu. Không phải đăng nhập PC trong quy trình thông thường. Điện thoại chỉ dùng xác nhận tài khoản; chọn bài và phát nhạc trực tiếp trên Brick.

Trang xác nhận có thể mang tên Spotify for Desktop do luồng đăng nhập desktop của bộ phát. Không nhập mật khẩu vào NXT. Nhấn B để hủy; lấy mã mới nếu bị từ chối hoặc quá thời gian chờ. Vào Spotify → Tài khoản để đăng nhập lại, đổi tài khoản, kết nối lại hoặc đăng xuất.

Đăng nhập PC cũ được giữ làm phương án tương thích cho Web API nếu cần, và chỉ được ghép với cùng tài khoản. Công cụ Spotify_Login_PC vẫn có trong ZIP, nằm dưới mục **Tài khoản → Nâng cao · nhập đăng nhập PC cũ**. Dùng khi thư viện của tài khoản bị Spotify từ chối; quyền ứng dụng Developer cũ vẫn có thể cần kiểm tra trong User Management.

## Màn hình và nút điều khiển

Spotify dùng giao diện Đang phát chung của NXT: tên bài, nghệ sĩ, album, ảnh album, thời gian, phổ nhạc, đầu ra và thông số PCM DAC. Thanh phát thu gọn trên trang chủ và danh sách hiển thị bài Spotify đang phát. Tên quá dài sẽ cuộn hoặc xuống dòng; font fallback của NXT được giữ cho tiếng Việt, Hàn, Nhật, Trung, Thái và các bộ chữ khác có trong gói font.

| Nút | Trong Đang phát Spotify |
| --- | --- |
| A | Phát / tạm dừng |
| B | Về trang chủ NXT, nhạc tiếp tục |
| L1 / R1 | Bài trước / tiếp theo |
| Trái / Phải | Tua 10 giây |
| X | Hàng đợi: A phát, X bỏ bài khác, L1/R1 đổi thứ tự |
| Y | Mở lời toàn màn hình |
| L2 | Đổi màu phổ; đi hết các màu thì tắt, nhấn tiếp để bật |
| R2 | Thêm / bỏ yêu thích trên tài khoản Spotify; chỉ cập nhật biểu tượng khi lưu thành công |
| SELECT | EQ / DSP |
| START | Tùy chọn nhanh: ngẫu nhiên, lặp, hẹn giờ, đầu ra, thoát giao diện |
| Logo TrimUI ở mặt trước | Dừng nhạc, đóng bộ phát và thoát hẳn app |

Trang chủ: **Y** mở lại bài đang phát. Nút HOME bên hông giữ hành vi cũ. Hàng đợi hiện gồm các bài của trang kết quả được chọn; chưa tự tải thêm trang. Ngẫu nhiên và lặp một bài / tất cả áp dụng cho hàng đợi này.

## Lời bài hát

App thử lấy lời từ Spotify, rồi LRCLIB khi Spotify không cung cấp lời, và lưu cache theo bài. Có mốc thời gian thì lời chạy theo vị trí phát; nguồn chỉ có lời thường thì hiển thị **Lời · không đồng bộ**, cuộn bằng tay. Bài chưa có lời hoặc mất mạng sẽ có thông báo, không tự tạo lời.

Ở màn hình lời: A phát/tạm dừng; ↑↓ cuộn; Y bật/tắt bám lời đồng bộ; X đổi cỡ chữ; L1/R1 chỉnh độ trễ 100 ms; Trái/Phải tua 5 giây; B về Đang phát. Logo TrimUI vẫn thoát hẳn, kể cả đang khóa phím.

## Phát nền

- Nhấn **B trong Đang phát** để về trang chủ, hoặc mở danh sách/cài đặt: Spotify tiếp tục phát.
- Nhấn nút nguồn để **tắt màn hình**: nhạc tiếp tục; nhấn lại để mở màn hình. Tắt nguồn hẳn máy sẽ kết thúc app.
- Muốn về giao diện NextUI và giữ nhạc: bật **Cài đặt → Phát nhạc → Giữ phiên khi thoát**, rồi **START → Thoát giao diện**. Cài đặt này mặc định bật nhưng giữ giá trị bạn đã lưu.
- Mở lại **cùng Pak v1.1** để điều khiển phiên đang phát, không tạo thêm bộ phát mới.
- **Logo TrimUI luôn thoát hẳn**, không chạy nền. Hẹn giờ dừng cũng dừng Spotify.
- Chọn nhạc nội bộ / Radio / Podcast sẽ dừng Spotify để chuyển nguồn.

## Chất lượng và DAC

Bộ phát yêu cầu chất lượng 320 kbps. Đây không phải số đo bitrate của mọi bài hoặc cam kết lossless. PCM Spotify đi qua EQ/DSP, âm lượng, LED và đường DAC của NXT. Dòng thông số DAC đọc từ trạng thái đầu ra kernel; có thể khác tần số nguồn do resample. Nếu không đọc được trạng thái DAC, app ghi chưa xác nhận.

Cắm DAC hoặc chọn đầu ra trước khi phát. Khi đổi đầu ra, app tạm dừng; nhấn A để tiếp tục qua đầu ra mới. Quy tắc rút DAC theo cài đặt được giữ. Chức năng DAC/sạc ở USB dưới của BottomDAC Fix1 được giữ.

Khám phá là tìm kiếm theo chủ đề của NXT, chưa phải đề xuất cá nhân. Quyền mở playlist/tìm kiếm/yêu thích phụ thuộc Spotify và tài khoản. Không chạy dịch vụ Connect để điều khiển từ điện thoại.

## Nếu cần gửi lỗi

Thoát bằng logo TrimUI và gửi **NXT_Spotify_Log.txt** trong folder Pak, cùng tên bài và bước bị lỗi. Giữ log trước khi mở lại Spotify vì phiên mới ghi log mới. Không gửi file token hoặc QR còn hiệu lực.

- Không lấy được QR: kiểm tra Wi-Fi, giờ máy và lấy mã mới.
- Đăng nhập hết hạn: Tài khoản → Đăng nhập bằng QR.
- Thư viện hoặc yêu thích bị từ chối: đăng nhập QR lại để cấp quyền; nếu vẫn lỗi, gửi log. Phương án PC cũ còn trong mục Nâng cao.
- Không có lời: thử bài khác; không phải mọi bài đều có lời đồng bộ.
- Không có tiếng: thử chọn bài, chờ tối đa khoảng 60 giây, kiểm tra đầu ra rồi gửi log. Log phân biệt khâu xác thực, tải/giải mã và việc PCM đến NXT.
- Phát nền về NextUI không hoạt động: kiểm tra Giữ phiên khi thoát đang bật và dùng Thoát giao diện, thay vì nút logo.

## Kiểm tra và build lại

Bản tích hợp làm nền cho v1.1 đã được người dùng xác nhận chạy tốt trên Brick Pro. Build v1.1 chuẩn hóa nhận diện và tài liệu; phần phát nhạc, QR, font, DAC và phiên nền giữ cùng luồng đã hoạt động. Kiểm tra ở môi trường build gồm build ARM64, 12 kiểm tra Rust, C IPC/QR, renderer NXT ở 1024×768 và 640×480 và tính toàn vẹn bộ cài/mã nguồn. Hình kiểm tra renderer dùng dữ liệu minh họa; không phải ảnh chụp nghe nhạc thật. Môi trường build không đăng nhập tài khoản Spotify thật hoặc kết nối trực tiếp Brick.

Project đầy đủ có nguồn C, Rust, Cargo.lock, vendor librespot được ghim, object/thư viện relink, tài nguyên, script và kết quả kiểm tra. Không kèm token cá nhân, SDK, Rust toolchain hoặc crate cache. Cài SDK theo SOURCE.json và Rust 1.99.0, thêm target aarch64-unknown-linux-gnu. `make spotify` build helper; `make nextui` build helper và app. Build nhanh: `make spotify`, `python3 tools/build-home-dac-incremental.py`, `python3 tools/package-nextui.py`. Đóng gói cả project bằng `python3 tools/package-spotify-source.py`; mặc định kết quả nằm trong `dist/release/`, hoặc chỉ định `--output-dir` để chọn nơi lưu.
