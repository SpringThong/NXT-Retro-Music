from pathlib import Path
import hashlib,json,zipfile
R=Path(__file__).resolve().parents[1];original=R.parents[1]/'releases-v1.8.8.1/NXT_Retro_Music_v1.8.8.1_Source.zip'
unchanged={};different={};intentional={'upstream/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/libmsettings.so','upstream/src/nxt.h','upstream/src/nxt_personal.c','upstream/src/nxt_screens.c','upstream/src/nxt_help_v18.inc'}
with zipfile.ZipFile(original) as z:
 prefix=z.namelist()[0].split('/')[0]+'/'
 for e in z.infolist():
  n=e.filename.removeprefix(prefix)
  if e.is_dir() or not n.startswith(('upstream/src/','upstream/res/','upstream/licenses/','upstream/stations/','upstream/bin/tg5040/','upstream/NextUI/workspace/tg5040/libmsettings/')):continue
  p=R/n;assert p.is_file(),n;a=hashlib.sha256(z.read(e)).hexdigest();b=hashlib.sha256(p.read_bytes()).hexdigest()
  if a==b:unchanged[n]=b
  else:assert n in intentional,n;different[n]={'before':a,'after':b}
assert all('upstream/src/'+n in unchanged for n in ['nxt_text.c','nxt_cpu.c','nxt_performance.c','nxt_write.c','nxt_startup.c','nxt_audio_mode.c','nxt_volume.c','nxt_player_core.c','nxt_hardware.c','nxt_route.c','nxt_theme.c','nxt_theme.h','nxt_config.c'])
(R/'logs/v1882-baseline-preserved.json').write_text(json.dumps({'exit':0,'baseline_source_sha256':hashlib.file_digest(original.open('rb'),'sha256').hexdigest(),'unchanged_files':unchanged,'intentional_native_changes':different,'resources_icons_fonts_emoji_locales_ime_unchanged':True,'online_backends_unchanged':True},indent=2)+'\n');print('PASS baseline1881',len(unchanged),len(different))
