# Build NXT Retro Music v1.9.7 — một source cho ba ROM

Đây là source chung của **TrimUI Brick Pro**: Stock, NextUI và NX Redux. Không cần ghép source phiên bản trước. Mã app/âm thanh/giao diện dùng chung trong `upstream/src`; mỗi ROM giữ launcher, settings adapter và link OTA riêng. Không cài cùng một ZIP app cho cả ba ROM.

## Chuẩn bị

- Linux x86_64 hoặc WSL2 Ubuntu x86_64 trên Windows. Không chạy trực tiếp bằng Python Windows; mở terminal Ubuntu/WSL.
- Python **3.11 trở lên** (dùng hashlib.file_digest), bash và make nếu muốn dùng lệnh make. Các bước build native chính dùng thư viện chuẩn Python và compiler đã ghim, không cần Docker, không cần cài SDL/FFmpeg dev của máy host hay Rust cho frontend.
- Giải nén toàn bộ source vào thư mục làm việc trên ổ Linux/WSL, mở terminal tại thư mục có SOURCE.json. Tránh chép đè vào source cũ.
- Dự trù khoảng 8 GB trống cho source giải nén, SDK và object của ba ROM. Lần đầu cần Internet để tải compiler và sysroot đã ghim SHA-256; các lần sau dùng lại SDK đã xác minh trong `.nxt-sdk`.
- Archive giữ mã vendor, thư viện ARM64 tĩnh, runtime/backend Spotify và SoundCloud đã hoạt động, font, 51 theme và icon. SDK/compiler tải riêng, không kèm trong ZIP. Không cần bản app cũ hay kho Git bên ngoài để build frontend.

## Kiểm tra trước khi build

```sh
python3 tools/build-release.py --rom all --check
```

Lệnh kiểm tra version, nguồn frontend, thư viện tĩnh, runtime và link OTA; không tải SDK và không biên dịch. Thiếu đầu vào sẽ báo lỗi, không âm thầm lấy binary frontend của ROM khác.

## Build riêng từng ROM

```sh
python3 tools/build-release.py --rom stock
python3 tools/build-release.py --rom nextui
python3 tools/build-release.py --rom nxredux
```

Hoặc `make stock`, `make nextui`, `make nxredux`. Chạy một lệnh tương ứng ROM cần dùng. SDK được tải tự động nếu chưa có compiler; có thể chuẩn bị trước bằng `python3 tools/sdk-assets.py`.

## Build cả ba

```sh
python3 tools/build-release.py --rom all --jobs 4
```

Hoặc `make all-roms`. Ba ROM build tuần tự, mỗi ROM dùng thư mục object và binary riêng. `--jobs` là số compiler chạy song song bên trong từng ROM; máy ít RAM có thể dùng `--jobs 2`. Không cần đổi build_define trong SOURCE.json. Script tự truyền đúng NXT_BUILD_ROM và NXT_PACKAGE_ROM cho từng bước.

Chỉ biên dịch, không tạo ZIP: thêm `--build-only`. Các script thấp hơn vẫn có thể dùng:

```sh
NXT_BUILD_ROM=NXT_ROM_STOCK python3 tools/build-stock.py
python3 tools/build-nextui.py
python3 tools/build-nxredux.py
```

## Đầu ra và đường dẫn cài

| ROM | ZIP cài / bộ OTA | Thư mục app trên thẻ |
| --- | --- | --- |
| Stock | `dist/stock-release/` | `Apps/NXT_Retro_Music` |
| NextUI | `dist/nextui-release/` | `Tools/tg5040/NXT Retro Music.pak` |
| NX Redux | `dist/nxredux-release/` | `Tools/NXT Retro Music.pak` |

ZIP cài có tên `NXT_Retro_Music_v1.9.7_<ROM>.zip`; bộ phát hành OTA có tên `NXT_OTA_Google_Drive_v1.9.7_<ROM>.zip`. Thoát app trước khi cài đè. Không xóa dữ liệu người dùng. Dùng đúng bản ROM.

Các lệnh chỉ tạo file, **không tự đăng OTA**. Link update.json chính thức của cả ba ROM đã lưu riêng trong source và gắn vào app. Dùng CHUAN_BI_OTA.html trong bộ OTA đúng ROM, đăng ZIP cài nguyên bản và thay nội dung **cùng tệp update.json cũ** trên Drive bằng Quản lý phiên bản, giữ nguyên ID/link. Không đăng mẫu chưa sửa link tải ZIP. Người dùng vào kiểm tra cập nhật, không phải nhập nguồn.

## Khi tiếp tục phát triển

- Mã dùng chung: `upstream/src/`; asset: `upstream/res/`; bản overlay được đồng bộ bằng `python3 tools/sync-overlay.py`.
- Stock: `stock/`. NextUI: config/pak/launcher ở `upstream/`. NX Redux: `nxredux/`, có settings adapter và tích hợp Tools/Home riêng. Không sao chép adapter của NX sang hai ROM kia.
- Chỉ build lại v1.9.7 khi cần khôi phục bản này. Nếu sửa chức năng để phát hành tiếp, tăng version đồng bộ trong SOURCE.json, nxt.h, config/pak/state/launcher và chuẩn bị changelog/hướng dẫn/OTA template của version mới. Không tăng giả version trong JSON để ép OTA cùng bản.
- `BUILD_INPUTS_v1.9.7.json` từng ROM ghi hash nguồn và frontend thực tế. `BUILD_REPORT_v1.9.7_<ROM>.json` cùng `RELEASE_QA_v1.9.7.md` là bằng chứng kiểm tra của source đi kèm. Sau khi đổi code, chạy lại kiểm tra phù hợp và cập nhật báo cáo dựa trên kết quả thật trước khi đóng source; không sửa hash cho khớp nếu chưa build.
- Đóng source đã xác minh: `make source`. Packager từ chối nếu fingerprint/binary/report không khớp một trong ba ROM. Một build mới ở đường dẫn khác có thể thay đổi hash do đường dẫn compiler; nếu báo cáo không khớp phải tạo lại báo cáo từ build/kiểm thử đó, không bỏ kiểm tra. Việc tạo ZIP app/OTA qua build-release.py không phụ thuộc báo cáo cũ.
- Backend Spotify giữ source trong `spotify/backend`; muốn sửa rồi build lại cần toolchain Rust và làm theo `tools/build-spotify.sh`. SoundCloud có `tools/build-soundcloud.sh` và runtime đã ghim. Thư viện và giấy phép/relink giữ trong source theo bản nền.

## Phạm vi bảo đảm

Giữ sửa DSD/DoP và DAC v1.9.2, thư viện tăng dần v1.9.7, cache lời dịch 256 MiB, Groq TXT, nguồn nghe, hồ sơ, giao diện và thao tác theo từng ROM. Không chuyển DSD sang PCM ngầm; Native/DoP vẫn cần driver ROM và DAC hỗ trợ. Build/kiểm thử phần mềm không thay thế nghe thử và đo nhiệt/pin trên thiết bị.

Lệnh desktop cũ là công cụ phát triển riêng, không phải cách tạo ZIP cài ARM64 ở trên. Các báo cáo/tài liệu version trước được giữ để tra lịch sử; đọc README.md, BUILD_v1.9.7.md và SOURCE.json cho bản hiện tại.
