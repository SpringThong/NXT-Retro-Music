# NXT Retro Music v1.8.9.1 — NX Redux

Bản tích hợp cho NX Redux v2.1.2 trên TrimUI Brick Pro, khôi phục từ source v1.8.9 đã xác minh SHA. Chỉ NX Redux có binary mới; Stock/NextUI giữ binary lịch sử v1.8.9.

Build: python3 tools/build-nxredux.py, sau đó python3 tools/build-nxredux-icon.py. SDK được tải và xác minh theo SOURCE.json. Nếu rebuild launcher, cập nhật SHA và kích thước trong icon-integration.json và install-tools-icon.sh trước khi đóng gói.

Launcher source tương ứng nằm trong nxredux/launcher-source; chỉ thêm một mapping trong tiles.c. Launcher dùng SDL_ttf 2.0.13 của ROM; NXT giữ thư viện chữ riêng. Hai file *-reference.so chỉ để liên kết khi build, không được cài đè thư viện firmware.

Đóng gói: python3 tools/package-nxredux.py; NXT_PACKAGE_ROM=nxredux python3 tools/package-ota-kit.py. Xem CHANGELOG, hướng dẫn trong docs và BUILD_REPORT_v1.8.9.1_NXRedux.json. Chưa kiểm tra trực tiếp trên thiết bị thật.
