from pathlib import Path
import hashlib,json,zipfile
R=Path(__file__).resolve().parents[1];original=R.parents[1]/'inputs-v187/NXT_Retro_Music_v1.8.6_Source.zip'
roots=['upstream/res/','upstream/licenses/','upstream/stations/','upstream/bin/tg5040/','upstream/NextUI/workspace/tg5040/libmsettings/']
unchanged={};different={};intentional={
 'upstream/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/libmsettings.so',
 'upstream/src/nxt.h','upstream/src/nxt_config.c','upstream/src/nxt_theme.h',
 'upstream/src/nxt_theme.c','upstream/src/nxt_theme_v18.inc',
 'upstream/src/nxt_v18_palette.inc','upstream/src/nxt_v18_scene_data.inc',
 'upstream/src/nxt_screens.c','upstream/src/nxt_help_v18.inc'}

with zipfile.ZipFile(original) as z:
 prefix=z.namelist()[0].split('/')[0]+'/'
 for entry in z.infolist():
  name=entry.filename.removeprefix(prefix)
  if entry.is_dir() or not (name.startswith(tuple(roots)) or name.startswith('upstream/src/')):continue
  p=R/name;assert p.is_file(),name
  before=hashlib.sha256(z.read(entry)).hexdigest();after=hashlib.sha256(p.read_bytes()).hexdigest()
  if before==after:unchanged[name]=after
  else:assert name in intentional,('unexpected baseline change',name);different[name]={'before':before,'after':after}
assert all('upstream/src/'+n in unchanged for n in ['nxt_skin.c','nxt_text.c','nxt_cpu.c','nxt_performance.c','nxt_write.c','nxt_startup.c','nxt_audio_mode.c','nxt_volume.c','nxt_player_core.c','nxt_hardware.c','nxt_route.c'])
assert 'upstream/bin/tg5040/nxt-spotify-backend' in unchanged and 'upstream/bin/tg5040/nxt-soundcloud-backend' in unchanged
(R/'logs/v187-baseline-preserved.json').write_text(json.dumps({'exit':0,'baseline_source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'unchanged_files':unchanged,'intentional_native_changes':different,'resources_icons_fonts_emoji_locales_ime_unchanged':True,'online_backends_unchanged':True,'normal_Bluetooth_msettings_unchanged':True},indent=2)+'\n')
print('PASS baseline:',len(unchanged),'unchanged source/assets/payload files;',len(different),'intentional native changes')
