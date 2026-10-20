#!/usr/bin/env python3
"""Save source edits against the durable v1.8.1 source, without release binaries."""
from pathlib import Path
import hashlib,json,os,zlib,zipfile

R=Path(__file__).resolve().parents[1]
BASE=R.parent/'v182-input/NXT_Retro_Music_v1.8.1_Source.zip'
NAME='NXT_Retro_Music_v1.8.2_Source_Checkpoint_Bluetooth'
OUT=R/'dist/checkpoints';OUT.mkdir(parents=True,exist_ok=True)
SKIP={'.cache','.nxt-sdk','.host-tools','.git','__pycache__','dist','overlay','relink','.userdata','target','toolchain','host-sdk','screens-v1.8.2'}
PRIVATE={'credentials.json','spotify-credentials.json','soundcloud-credentials.json','playback.json','NXT_App_Log.txt','NXT_Device_Log.txt','NXT_Input_Log.txt','NXT_Spotify_Log.txt','NXT_SoundCloud_Log.txt'}
PREFIX='NXT_Retro_Music_v1.8.1_Source/'
changed={};proof={};paths=[]
with zipfile.ZipFile(BASE) as baseline:
    old={i.filename.removeprefix(PREFIX):i for i in baseline.infolist() if i.filename.startswith(PREFIX)}
    for root,dirs,names in os.walk(R,followlinks=False):
        dirs[:]=sorted(d for d in dirs if d not in SKIP and not (Path(root)/d).is_symlink())
        for name in sorted(names):
            p=Path(root)/name;rel=p.relative_to(R).as_posix()
            if p.is_symlink() or name in PRIVATE or name.startswith('FILES_SHA256') or name.startswith('BUILD_INPUTS_') or name=='overlay-manifest.json':continue
            if name.endswith(('.elf','.so','.o','.a','.pyc','.zip','.sha256','.part','.tmp','.building','.writing')):continue
            if rel.startswith('logs/'):
                if not name.startswith('v182-'):continue
                # Only current Bluetooth/regression evidence is part of this checkpoint.
                if not any(k in name for k in ['bluetooth','bt-','led-driver','stock-runtime','baseline']):continue
            data=p.read_bytes()
            if data.startswith(b'\x7fELF'):continue
            info=old.get(rel)
            if info and info.file_size==len(data) and info.CRC==zlib.crc32(data) and baseline.read(info)==data:continue
            changed[rel]=hashlib.sha256(data).hexdigest();paths.append(p)
    for rel in ['BLUETOOTH_CHECK_v1.8.2.md','upstream/src/nxt_player_core.c','upstream/src/nxt_hardware.c','upstream/src/nxt_nextui.c','upstream/NextUI/workspace/tg5040/libmsettings/msettings.c','upstream/src/nxt_performance.c','qa/test_v182_nextui_bluetooth.py']:assert rel in changed,rel

meta={'version':'1.8.2','kind':'source-only checkpoint, changes against v1.8.1','base_source':BASE.name,'base_sha256':hashlib.file_digest(BASE.open('rb'),'sha256').hexdigest(),'base_library_file_id':'libfile_50fe117961388191b0b222652072e3c6','release_build_paused_by_user':True,'contains_native_release_binary':False,'physical_Bluetooth_tested':False,'changed_files_sha256':changed}
readme='''# NXT Retro Music v1.8.2 — source checkpoint Bluetooth

Đây là bản lưu các thay đổi mã nguồn từ v1.8.1, bao gồm player NXT gốc,
ba chế độ hiệu năng, tên menu, mã đóng gói OTA và bản sửa Bluetooth NextUI.
Build/phát hành đang tạm dừng theo yêu cầu. Không có ELF hay ZIP cài/OTA mới.
Không giải nén gói này lên thẻ nhớ để cài app.

Để khôi phục môi trường mã nguồn: giải nén NXT_Retro_Music_v1.8.1_Source.zip,
chép nội dung project/ trong gói này đè lên thư mục source vừa giải nén.
Các thư viện/tài nguyên không đổi được lấy từ source gốc đó. Chạy các kiểm thử
Bluetooth theo BLUETOOTH_CHECK_v1.8.2.md; SDK/host tools cần dựng như README.

Khi người dùng cho tiếp tục build: bỏ cờ release_build_paused_by_user trong
SOURCE.json, build lại hai ROM rồi chạy toàn bộ kiểm tra gói/OTA và cập nhật
báo cáo. Chạy tools/sync-overlay.py sau build để tái tạo overlay. ELF trong
source nền v1.8.1 và ELF tạo trước bản sửa Bluetooth không chứa bản sửa mới.

Chưa xác nhận tai nghe Bluetooth hay nhiệt độ/độ trễ trên máy thật.
CHECKPOINT.json ghi hash source nền và từng tệp thay đổi để kiểm tra.
'''
archive=OUT/(NAME+'.zip')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    z.writestr(NAME+'/README.md',readme)
    z.writestr(NAME+'/CHECKPOINT.json',json.dumps(meta,ensure_ascii=False,indent=2)+'\n')
    for p in paths:
        rel=p.relative_to(R).as_posix();info=zipfile.ZipInfo.from_file(p,NAME+'/project/'+rel);info.compress_type=zipfile.ZIP_DEFLATED
        z.writestr(info,p.read_bytes())
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    for rel,digest in changed.items():assert hashlib.sha256(z.read(NAME+'/project/'+rel)).hexdigest()==digest
print(json.dumps({'checkpoint':str(archive),'files':len(changed),'bytes':archive.stat().st_size,'sha256':hashlib.file_digest(archive.open('rb'),'sha256').hexdigest()}))
