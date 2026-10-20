# Phát hành OTA v1.8

Tạo manifest riêng cho Stock và NextUI bằng `make-ota-manifest.py --rom stock` hoặc `--rom nextui`. ZIP, version, ROM, máy, dung lượng và SHA-256 phải khớp. Kit chứa manifest dùng đúng hash/size của bản cài, nhưng link ZIP là placeholder: thay bằng link file HTTPS công khai của bạn trước khi đưa lên Drive. Không dùng link folder. Bản cài giữ nguồn OTA đã có; lần làm này chưa đưa bản mới lên nguồn cập nhật của nhà phát hành.
