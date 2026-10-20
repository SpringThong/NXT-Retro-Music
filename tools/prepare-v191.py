#!/usr/bin/env python3
"""Release v1.9.1 for three ROMs; share DSD/TXT, retain ROM integration guards."""
from pathlib import Path
import json,shutil
R=Path(__file__).resolve().parents[1]
def edit(n,a,b):
 p=R/n;s=p.read_text();assert a in s,(n,a[:90]);p.write_text(s.replace(a,b))
for n in ['upstream/src/nxt_decoder.c','upstream/src/nxt_bitperfect.c','upstream/src/nxt_audio_mode.c']:
 p=R/n;s=p.read_text();s=s.replace('#ifdef NXT_ROM_NXREDUX\n','')
 if n.endswith('nxt_audio_mode.c'):s=s.replace('#else\n (void)p;return false;\n#endif\n','')
 else:s=s.replace('#endif\n','')
 p.write_text(s)
edit('upstream/src/nxt_player_core.c','#ifdef NXT_ROM_NXREDUX\n if(NxtDSD_isPath(path))return AUDIO_FORMAT_DSD;\n#endif','if(NxtDSD_isPath(path))return AUDIO_FORMAT_DSD;')
edit('upstream/src/nxt_screens.c','#ifdef NXT_ROM_NXREDUX\nROW("DSD USB output","Đầu ra DSD USB","%s",nxt.dsd_output?"DoP":"Native DSD");\n#endif','ROW("DSD USB output","Đầu ra DSD USB","%s",nxt.dsd_output?"DoP":"Native DSD");')
edit('upstream/src/nxt_screens.c','#ifdef NXT_ROM_NXREDUX\n rows[n++]=Nxt_text("DSD:','rows[n++]=Nxt_text("DSD:')
edit('upstream/src/nxt_screens.c','Âm lượng phần cứng DAC khi được hỗ trợ.");\n#endif','Âm lượng phần cứng DAC khi được hỗ trợ.");')
edit('upstream/src/nxt_screens.c','ModuleExitReason Nxt_changes(DisplayContext *d){const char *rows[]={','ModuleExitReason Nxt_changes(DisplayContext *d){const char *rows[]={\n Nxt_text("v1.9.1 · all three ROMs: 256 MiB translation cache on SD, no 512-song cap, bounded background cleanup. Original USB DSD and Groq TXT import now also on Stock/NextUI. Existing LRCLIB original-lyrics lookup feeds automatic translation when enabled.","v1.9.1 · cả ba ROM: cache lời dịch 256 MB trên thẻ, bỏ giới hạn 512 bài, dọn nền với bộ nhớ giới hạn. Stock/NextUI thêm DSD USB nguyên bản và nhập Groq TXT. Giữ tìm lời gốc LRCLIB rồi tự dịch khi bật dịch."),')
edit('upstream/src/nxt_translate.c','Nxt_text("Translations are cached on the card. Offline playback can reuse saved lines. Retry resumes missing lines; service errors keep original lyrics.","Bản dịch lưu trên thẻ. Offline dùng lại các câu đã lưu. Thử lại tiếp tục phần còn thiếu; lỗi dịch vụ vẫn giữ lời gốc.")','Nxt_text("Translations: up to 256 MiB on SD, no 512-song limit. Offline reuses saved lines; the oldest saved files are removed only at the budget. No 256 MiB RAM allocation.","Lời dịch: tối đa 256 MB trên thẻ, không giới hạn 512 bài. Offline dùng lại câu đã lưu; chỉ dọn bản cũ khi đạt dung lượng. Không nạp 256 MB vào RAM.")')
p=R/'upstream/src/nxt.h';s=p.read_text().replace('"1.9"','"1.9.1"').replace('v1.9 · NativeDSD','v1.9.1 · DSDCache256').replace('1.9-NXRedux-NativeDSD','1.9.1-NXRedux-DSDCache256').replace('v1.8.9 · GroqOTA','v1.9.1 · DSDCache256').replace('1.8.9-Stock-GroqOTA','1.9.1-Stock-DSDCache256').replace('1.8.9-NextUI-GroqOTA','1.9.1-NextUI-DSDCache256');p.write_text(s)
description='v1.9.1: 256 MiB SD translation cache, no 512-song cap, original DSF/DFF USB DSD, Groq TXT import, original-lyrics lookup then translation, retained ROM-specific behavior and official OTA.'
for rom,label,d in [('stock','Stock','stock'),('nextui','NextUI','upstream'),('nxredux','NXRedux','nxredux')]:
 for filename in ['config.json','pak.json']:
  p=R/d/filename
  if not p.exists():continue
  v=json.loads(p.read_text());v['version']='1.9.1';v['release_filename']=f'NXT_Retro_Music_v1.9.1_{label}.zip';v['description']=description
  if 'changelog' in v:v['changelog']['v1.9.1']=description
  p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n')
 baseline='1.9' if rom=='nxredux' else '1.8.9'
 for suffix in ['.md']:
  for prefix in ['README','CHANGELOG']:
   src=R/f'{prefix}_v{baseline}_{label}{suffix}'
   if src.exists():(R/f'{prefix}_v1.9.1_{label}{suffix}').write_text(src.read_text().replace(f'v{baseline}', 'v1.9.1'))
 src=R/f'docs/HUONG_DAN_NXT_Retro_Music_v{baseline}_{label}.md';dst=R/f'docs/HUONG_DAN_NXT_Retro_Music_v1.9.1_{label}.md';dst.write_text(src.read_text().replace(f'v{baseline}','v1.9.1')+'\n\nXem THEM_MOI_v1.9.1.md: cache 256 MB, DSD nguyên bản và Groq TXT có trên cả ba ROM.\n')
 (R/f'CHANGELOG_v1.9.1_{label}.md').write_text('# NXT Retro Music v1.9.1 — '+label+'\n\n'+description+'\n\nCache compatibility retained; DAC/ROM must support Native DSD or explicitly selected DoP. Device heat/power and live services require hardware testing. NX-only CPU/LED/Home/icon integrations stay guarded to NX Redux.\n')
 p=R/d/'launch.sh';s=p.read_text().replace('v1.9 NX','v1.9.1 NX').replace('v1.8.9 ','v1.9.1 ');p.write_text(s)
p=R/'upstream/state/app_version.txt';p.write_text('1.9.1\n')
for n in ['SOURCE.json','NXREDUX_SOURCE.json']:
 p=R/n;v=json.loads(p.read_text());v['version']='1.9.1';v['stage']='v1.9.1-dsd-cache256-three-roms';v['release_roms']=['stock','nextui','nxredux'];v['release_ready']=True;v['release_build_paused_by_user']=False;v['baseline_versions']={'stock':'1.8.9','nextui':'1.8.9','nxredux':'1.9'};v['translation_cache_bytes']=256*1024*1024;v['translation_cache_song_limit']=None;v['current_build_report']='BUILD_REPORT_v1.9.1_NXRedux.json';v['current_build_reports']=[f'BUILD_REPORT_v1.9.1_{l}.json' for l in ['Stock','NextUI','NXRedux']];v['hardware_test_status']='ARM64 builds/software fixtures only; physical ROM/DAC Native DSD or DoP, heat, power and live-service behavior require device testing.';p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n')
shutil.copytree(R/'docs/OTA_Google_Drive_v1.9',R/'docs/OTA_Google_Drive_v1.9.1',dirs_exist_ok=True)
for p in (R/'docs/OTA_Google_Drive_v1.9.1').rglob('*'):
 if p.is_file():p.write_text(p.read_text().replace('v1.9','v1.9.1'))
for n in ['tools/package-nxredux.py','tools/package-stock.py','tools/package-nextui.py']:
 p=R/n;s=p.read_text()
 if n.endswith('nxredux.py'):s=s.replace("if version=='1.9':copy(R/'docs/DSD_v1.9_NXRedux.md',STAGE/'DSD_v1.9_NXRedux.md')","copy(R/'docs/THEM_MOI_v1.9.1.md',STAGE/'THEM_MOI_v1.9.1.md')")
 else:
  key='    version=json.loads' if n.endswith('stock.py') else " config=json.loads"
  pos=s.index(key);s=s[:pos]+("    copy(ROOT/'docs/THEM_MOI_v1.9.1.md',STAGE/'THEM_MOI_v1.9.1.md')\n" if n.endswith('stock.py') else " copy(R/'docs/THEM_MOI_v1.9.1.md',STAGE/'THEM_MOI_v1.9.1.md')\n")+s[pos:]
 s=s.replace("'translation-service.json'","'groq-api-key.txt','groq-key.txt','translation-service.json'").replace('"translation-service.json"','"groq-api-key.txt","groq-key.txt","translation-service.json"');p.write_text(s)
p=R/'tools/package-source.py';s=p.read_text().replace("PRIVATE = {'translation-service.json'","PRIVATE = {'groq-api-key.txt','groq-key.txt','translation-service.json'");p.write_text(s)
print('Prepared v1.9.1 metadata and common DSD/TXT for three ROMs; ROM integration guards retained.')
