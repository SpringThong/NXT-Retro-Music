#!/usr/bin/env python3
"""Finalize current three-ROM artifacts with exact source and QA fingerprints."""
from pathlib import Path
import hashlib,json,datetime,struct
R=Path(__file__).resolve().parents[1];V='1.8.4'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def passed(x):
 if isinstance(x,dict):
  for k,v in x.items():
   if k=='exit':assert v==0,(k,v)
   passed(v)
 elif isinstance(x,list):
  for v in x:passed(v)
checks=['v184-integer-decoder-test.json','v184-normal-audio-test.json','v184-bit-worker-test.json','v184-ui-test.json','v184-config-test.json','v184-timer-test.json','v184-nextui-bluetooth-test.json','v184-ARM64-loader-test.json','v184-baseline-preserved.json']
for n in checks:
 p=R/'logs'/n;assert p.is_file(),n;passed(json.loads(p.read_text()))
baseline=json.loads((R/'logs/v184-baseline-preserved.json').read_text())
for n,h in baseline['unchanged_files'].items():assert sha(R/n)==h,n
for n,h in baseline['intentional_native_changes'].items():assert sha(R/n)==h['after'],n
assert json.loads((R/'logs/v184-bit-worker-test.json').read_text())['source_sha256']==sha(R/'upstream/src/nxt_bitperfect.c')
for rom,label,directory in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 binary=R/directory/'bin/tg5040/musicplayer.elf';inputs=json.loads((R/directory/f'BUILD_INPUTS_v{V}.json').read_text());assert inputs['binary_sha256']==sha(binary)
 for n,h in inputs['sources'].items():assert sha(R/n)==h,n
 config=json.loads((R/directory/'config.json').read_text());assert config['version']==V and config['rom']==rom
 installer=R/f'dist/{rom}-release'/config['release_filename'];kit=installer.parent/f'NXT_OTA_Google_Drive_v{V}_{label}.zip'
 localchecks=checks+[f'v184-ota-qa-{rom}.json',f'v184-ota-publisher-kit-{rom}.json',f'v184-arm64-{rom}-test.json']
 for n in localchecks:passed(json.loads((R/'logs'/n).read_text()))
 ota=json.loads((R/f'logs/v184-ota-qa-{rom}.json').read_text());pub=json.loads((R/f'logs/v184-ota-publisher-kit-{rom}.json').read_text());assert sha(installer)==ota['installer_sha256']==pub['installer_sha256'];assert sha(kit)==pub['kit_sha256']
 for n,h in ota['OTA_source_sha256'].items():assert sha(R/'upstream/src'/n)==h,n
 settings='libnxt-nxredux-settings.so' if rom=='nxredux' else 'libmsettings.so'
 report={'project':'NXT Retro Music','version':V,'rom':rom,'device':'trimui-brick-pro','build':{'exit':0,'compiler':'pinned GCC 8.3.0 AArch64','all_source_inputs_verified':True,'source_list':'stock/frontend-sources.json','log':f'logs/build-v184-{rom}.log'},'binary':binary.relative_to(R).as_posix(),'binary_sha256':sha(binary),'settings_library':settings,'settings_library_sha256':sha(binary.parent/settings),'installer':installer.name,'installer_sha256':sha(installer),'installer_bytes':installer.stat().st_size,'publisher_kit':kit.name,'publisher_kit_sha256':sha(kit),'checks':{n:{'sha256':sha(R/'logs'/n)} for n in localchecks},'features':{'SELECT_EQ_toggle':True,'saved_Now_Playing_startup':True,'default_paused':True,'questionnaire_removed_from_cold_start':True,'Bit_Perfect_exclusive_integer_USB_PCM':True,'normal_mode_presets_retained':True,'timer_full_exit':True,'native_layouts':36,'system_languages':14,'ROM_specific_OTA':True},'hardware_tested':False,'physical_DAC_Bluetooth_tested':False,'physical_battery_temperature_measured':False,'physical_sleep_wakeup_tested':False,'authenticated_online_playback_in_this_build':False,'OTA_public_source_published':False,'verification_notes':f'RELEASE_QA_v{V}.md','baseline_source_sha256':baseline['baseline_source_sha256'],'created_UTC':datetime.datetime.now(datetime.timezone.utc).isoformat()}
 (R/f'BUILD_REPORT_v{V}_{label}.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
meta=json.loads((R/'SOURCE.json').read_text());meta['stage']='v1.8.4-native-packaged';meta['stock_status']='Fresh Stock, NextUI and NX Redux v1.8.4 installers and OTA publisher kits; current native source inputs and exact package fingerprints checked';meta['hardware_test_status']='Software, source/PCM comparisons, controller/UI regression and ARM64 loader checks passed; physical DAC/Bluetooth, battery/temperature, sleep/wakeup and live public OTA remain device tests';(R/'SOURCE.json').write_text(json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
(R/f'RELEASE_QA_v{V}.md').write_text('''# NXT Retro Music v1.8.4 — Kiểm tra phát hành

## Các phần đã kiểm tra

- Ba frontend C native compile đầy đủ cho Stock, NextUI, NX Redux bằng GCC 8.3.0/SDK đã khóa SHA-256. ELF AArch64 nạp được bằng QEMU/pinned libc; --build-info và --help nhận đúng v1.8.4/từng ROM. Backend Spotify/SoundCloud khởi động và nhận quit qua IPC thật; chưa đăng nhập tài khoản.
- Decoder ARM64 thật: 13 mẫu WAV/FLAC/ALAC ở 44.1/48/96/192 kHz, mono/stereo, nguồn 16/24/32 bit. Dữ liệu PCM nguyên bản và vị trí tua đối chiếu từng byte với fixture gốc; nguồn MP3 bị từ chối trong native integer mode.
- Chế độ Thường: 10 định dạng WAV/MP3/FLAC/AAC/ALAC/Vorbis/Opus, giải mã/tua bằng FFmpeg ARM64; DSP bypass/ReplayGain, 10/15/31-band EQ và limiter vẫn qua kiểm tra.
- Worker Bit-Perfect C thật dùng simulator ALSA/decoder rõ ràng: S16_LE/S24_3LE/S24_LE/S32_LE, sample bytes, partial write/EAGAIN, tạm dừng không gửi PCM, EOF/nghỉ, tua/tiếp tục, mất DAC, bận DAC, Hz không phù hợp, nguồn không phù hợp, không fallback plug/dmix/loa, thoát worker. ASan/UBSan qua; simulator không chứng nhận phần cứng.
- UI production code render tại 640×480 và 1024×768: 36 giao diện/lời nguyên câu, giữ NXT gốc/ảnh nền/album và nút, 300 icon/180 emoji, 105 phối phổ và 6 hàm màu LED. SELECT release đóng preset mà không đổi EQ, đóng toàn bộ mục con, đóng màn hình bypass. Lưu bài/vị trí theo hồ sơ, giữ file thiếu, mở Now Playing trống, khôi phục tạm dừng, tự phát tùy chọn và không ghi đè vị trí khi seek đang chờ. Audio/ROM input trong kiểm tra UI là fixture.
- Config production của cả ba ROM: migration từ dữ liệu cũ, mặc định English/Cân bằng/Thường/tạm dừng, lưu/load Bit-Perfect và tự phát, giữ chế độ thiết bị khi đổi hồ sơ.
- Timer/config/session C của cả ba ROM: deadline/hủy/thay timer/wrap, tạm dừng/tắt màn hình, radio/podcast, lưu vị trí/cài đặt rồi đóng cả phiên nền. Không thêm SIGKILL, lệnh ép suspend hoặc daemon.
- Bluetooth production helpers trong chế độ Thường: route A2DP, private ALSA config, reopen/retry, mixer/unmute và PCM local/Spotify/SoundCloud vẫn hoạt động với transport fixture. Firmware settings source không thay đổi.
- Đối chiếu v1.8.3: 2.074 file source/assets/runtime không đổi, gồm renderer/themes/fonts/icon/emoji/locales/IME và backend online; 23 file native cũ thay đổi có chủ đích, cộng các module mới.
- Ba ZIP cài: CRC, ELF AArch64, executable bits, đúng cây Apps/Tools và ROM metadata. Bộ OTA chứa chính xác ZIP cài và SHA-256 của ROM đó.
- OTA worker/installer production kiểm tra với ZIP đầy đủ: nâng từ fixture v1.8.3, Drive confirmation, size/SHA sai bị từ chối, tải/stage/cài, backup app cũ, giữ userdata/appearance/last-playback, từ chối khác ROM/HTML, hủy và thu hồi child process. Nguồn tải trong bài thử là fixture, không phải link công khai.

## Giới hạn

Không có TrimUI/DAC/tai nghe thật trong môi trường build. Chưa thử âm thanh USB/Bluetooth trên thiết bị, chưa đo pin/nhiệt hoặc ngủ/thức. Chưa thử streaming đăng nhập và chưa tự đăng nguồn OTA công khai. Bit-Perfect trên máy chỉ báo sẵn sàng sau khi hw PCM mở đúng Hz/kênh/format/precision; DAC bận hoặc không phù hợp sẽ dừng, không hạ chất lượng âm thầm. Việc vào deep sleep do chính sách ROM sau khi NXT đóng.

Bằng chứng JSON/log và hash native source/binary/gói cài/OTA nằm trong BUILD_REPORT_v1.8.4_<ROM>.json cùng logs/v184-*. Các báo cáo phiên bản cũ được giữ như tài liệu lịch sử và không được dùng thay cho bằng chứng bản này.
''')
print('PASS final current source inputs, three native binaries, installers, OTA kits and current-release QA fingerprints')
