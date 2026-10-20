# NXT Retro Music v1.8.2 — NX Redux / TrimUI Brick Pro

Đây là bộ source đầy đủ để build NXT v1.8.2 cho NX Redux. Giữ các tính năng của
bản v1.8.2: 36 giao diện, hồ sơ cá nhân, nhạc thẻ/online, Bluetooth/USB DAC, thông
tin chất lượng bài và đầu ra, EQ, LED thay đổi trực tiếp, Wi-Fi copy, 14 ngôn ngữ,
chế độ hiệu năng và OTA.

Thay đổi riêng NX Redux: dùng API cài đặt của firmware, chia sẻ đường âm thanh
đúng với firmware, nhường PCM từ trình phát hệ thống khi mở NXT, thư mục Tools
phẳng và nguồn OTA riêng. Khi hẹn giờ hết, NXT lưu dữ liệu rồi thoát hoàn toàn cả
phiên chạy nền. NX Redux quyết định thời gian tắt màn hình/suspend/deep sleep.

## Build NX Redux

```bash
NXT_JOBS=3 make nxredux
```

Kết quả nằm trong `dist/nxredux-release/`. Build lần đầu cần tải SDK đã khóa SHA-256;
không cần Docker. Source mới và bộ adapter nằm tại `nxredux/src/`; thư viện và
backend dùng chung nằm tại `upstream/bin/tg5040/`. Không đóng gói hay thay thế
`libmsettings.so` của firmware NX Redux bằng thư viện NextUI cũ.

Gói có mã nguồn dùng chung cho cả ba ROM. Binary và báo cáo Stock/NextUI đi kèm
là bản v1.8.2 đã phát hành trước đó; chúng được giữ nguyên, chưa build lại và chưa
thêm hành vi thoát app khi hết hẹn giờ. Báo cáo mới là `BUILD_REPORT_v1.8.2_NXRedux.json`.

## Kiểm tra và hướng dẫn

- `docs/HUONG_DAN_NXT_Retro_Music_v1.8.2_NXRedux.md`: cài đặt, sử dụng và test trên máy.
- `CHANGELOG_v1.8.2_NXRedux.md`: các thay đổi dành riêng cho NX Redux.
- `NXREDUX_SOURCE.json`: firmware tham chiếu, compiler và bản source nền.
- `logs/nxredux-*-test.json`: kết quả kiểm tra tương ứng; OTA nằm trong
  `logs/v182-ota-qa-nxredux.json`.
- `tools/fetch-nxredux-reference.py`: tải runtime tham chiếu NX Redux v2.0.0 Brick Pro
  đã khóa checksum để kiểm tra API/loader ARM64; không cài firmware lên máy.

Đã kiểm tra âm thanh, hẹn giờ, giao diện, launcher, OTA và thư viện firmware trong
môi trường phát triển. Bluetooth/DAC vật lý, pin, nhiệt và deep sleep cần test trên
Brick Pro thật. Tài khoản online mới chưa được đăng nhập trong lần build này.

Source NXT giữ GPL-3.0-only; giữ giấy phép mã nền, thư viện và font đi kèm.
