# NXT Retro Music v1.10.1 — Stock

Sửa DSD USB và tên hiển thị DAC. Bản dành cho Stock; không dùng ZIP của ROM khác.

- Giữ nguyên dữ liệu/hồ sơ/cache 256 MiB, Groq TXT, CPU/LED/phím nền riêng của ROM, nguồn nhạc và OTA.
- DSD không tự chuyển PCM. Native cần driver ROM; DoP cần DAC thật sự hỗ trợ đúng tần số truyền.
- Mặc định và lựa chọn DSD cũ được giữ. Muốn thử Native rồi DoP: Cài đặt → Phát nhạc & âm thanh → Âm thanh & đầu ra → Đầu ra DSD USB → Tự động, xác nhận DoP.
- DSD64 qua DoP cần 176.4 kHz, DSD128 cần 352.8 kHz, DSD256 cần 705.6 kHz. DAC hỗ trợ DSD256 Native không đồng nghĩa DoP DSD256. Không hạ DSD tier hay chuyển PCM ngầm.
- Tên USB có thể là tên bộ điều khiển/OEM. Chọn Tên hiển thị DAC → TRN Black Pearl hoặc tự nhập. App lưu tên cho bộ VID/PID/manufacturer/product bạn xác nhận, không suy đoán model/tính năng từ chip hay tên TE-C. Hai thiết bị có descriptor giống hệt sẽ dùng chung nhãn.
- Log DSD giữ lỗi ở bước đã mở được phần cứng, không bị các cổng không tồn tại ghi đè. Native/DoP ghi rõ format, tần số truyền, bước thất bại. Trạng thái không còn kết luận mọi lỗi đều do thiếu DAC tương thích.
- Thông tin, hướng dẫn & cập nhật: START xuất NXT_App_Log.txt, Y xuất NXT_Device_Log.txt gồm VID/PID, descriptor, kernel, stream formats, trạng thái DSD. Giữ DAC cắm khi xuất. File ở Apps/NXT_Retro_Music cạnh launch.sh.

Cài đặt: thoát hẳn phiên nghe, giải nén ZIP ở gốc thẻ; giữ nguyên dữ liệu người dùng. OTA giữ nguyên link Stock đã tích hợp. Người dùng không cần nhập link; người phát hành vẫn cần cập nhật nội dung chính file update.json cũ trên Drive.

Bản sửa cần thử lại với Black Pearl và DFF thật. Các kiểm thử phần mềm dùng mô phỏng ALSA, không thay thế xác nhận phần cứng hay đo nhiệt/pin. Nếu Native vẫn lỗi, log mới sẽ xác định driver thiếu format hay thiết lập bị từ chối; app không thể tạo khả năng Native DSD mà kernel ROM không hỗ trợ.
