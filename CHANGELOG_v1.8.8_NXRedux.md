# NXT Retro Music v1.8.8 — NXRedux

## Thay đổi chính
- Thêm **dịch lời theo ngôn ngữ app**: Tắt / Bản dịch / Song ngữ.
- Now Playing hiện một câu tĩnh. Song ngữ ghép lời gốc và bản dịch ở màn hình lời toàn màn hình; giữ mốc LRC và độ trễ gốc.
- Một worker ưu tiên thấp, một yêu cầu mạng tại một thời điểm, dùng lại câu lặp và bản dịch đã lưu. Không thêm công việc vào callback âm thanh.
- Ẩn app/tắt màn hình sẽ tạm dừng dịch online. Đổi bài/ngôn ngữ/hồ sơ hủy tác vụ cũ; getter không đưa bản dịch cũ lên lời mới ngay cả trước tick tiếp theo.
- Lỗi mạng/hạn mức vẫn hiện lời gốc. Có cooldown và thử lại thủ công, không retry theo mỗi khung.
- MyMemory mặc định dùng công khai không cần khóa, có hạn mức. Google Cloud Basic v2 là tùy chọn với khóa riêng.
- Giữ 50 giao diện, độ nét, hồ sơ → Now Playing, EQ/SELECT, CPUFreq NextUI, loa/3,5mm/Bluetooth/DAC, Bit-Perfect/Tự động, online, LED đổi ngay, Wi-Fi transfer và timer thoát trọn app của v1.8.7.
- Hai trang thay đổi, changelog, hướng dẫn và OTA riêng của ba ROM đều ghi v1.8.8.

## Bật dịch lời
1. Kết nối Wi-Fi trong cài đặt ROM. Chọn ngôn ngữ app ở Cài đặt → Ngôn ngữ.
2. Mở **Cài đặt → Phát nhạc & âm thanh → Cách phát → Dịch lời bài hát**. Có thể mở ở cuối menu START trong Now Playing, hoặc bấm START trong màn hình lời.
3. Đổi Hiển thị sang Bản dịch hoặc Song ngữ. Mặc định Tắt để giữ hành vi cũ và không gửi lời đi trước khi bạn bật.
4. Ngôn ngữ đích tự theo app, có 14 lựa chọn. Ngôn ngữ lời có thể tự nhận diện hoặc chọn thủ công. MyMemory nhận diện dựa theo chữ; lời Latin, Kanji hoặc trộn ngôn ngữ có thể cần chọn lại nguồn. Google Cloud tự nhận diện bằng dịch vụ khi chọn Tự nhận diện.
5. Chờ dịch ở nền; nhạc tiếp tục và câu chưa dịch vẫn hiện lời gốc. Trạng thái & cách dùng hiện tiến độ; Thử dịch lại tiếp tục các câu còn thiếu.
6. Nếu chọn Google Cloud: bật Cloud Translation API cho dự án của bạn và nhập khóa API riêng. Bàn phím nhập khóa luôn Latin, khóa không hiện lại trong menu và chỉ lưu trên máy. Dịch vụ dùng hạn mức/chi phí của tài khoản Google Cloud của bạn; app không nhúng khóa dùng chung.


Giữ chức năng v1.8.7. Bộ OTA để đăng nguồn riêng; chưa đo nhiệt/pin trên máy thật. Dịch phụ thuộc mạng/hạn mức dịch vụ và luôn có lời gốc dự phòng.
