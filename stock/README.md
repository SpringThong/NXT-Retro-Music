# Stock v1.5.2

TrimUI Brick Pro TG4040; SDK tg5040; NXT_ROM_STOCK. Build từ upstream/src theo frontend-sources.json, lưu object ở stock/relink/obj. tools/relink-stock.sh tái liên kết để đối chiếu payload.

Cold-start profile chooser, bốn tab cá nhân/chart/history/profile; transparent wallpaper/theme, 24 icon/20 backgrounds; LED writer ownership và merged About/help/OTA. Giữ Bluetooth/DAC volume/hotplug, online helpers, Telex, vinyl và Drive.

STOCK_SOURCE.json và BUILD_REPORT_V1_5_2_STOCK.json ghi provenance/hash/current QA. Backend hashes giữ từ baseline. Không đóng gói userdata/tài khoản. Userdata ứng dụng ở .userdata/nxt-stock/shared/nxt-retro-music.

OTA config bắt buộc package/rom/device/version/launch. Có thể đặt link thật trong stock/ota-source-stock.url trước đóng gói. Sau thay ZIP cần tạo lại hash/size. Công cụ dưới docs/OTA_Google_Drive_v1.5.2.

OTA v1.5.2: tự kiểm tra, bốn mục, nguồn Stock đã nhúng. Nguồn NextUI riêng; không có bản NextUI trong release này. Vẽ/text/statistics được đệm, không vẽ nền ẩn/tắt màn hình. `logs/v152-render-benchmark.json` chỉ đo CPU vẽ native trên máy build, không đo nhiệt máy thật.
