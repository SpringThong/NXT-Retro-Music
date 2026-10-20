from pathlib import Path
import hashlib,json,zipfile
R=Path(__file__).resolve().parents[1];original=Path('/workspace/scratch/nxt-v184-performance-check/inputs/NXT_Retro_Music_v1.8.4_Source.zip')
roots=['upstream/res/','upstream/licenses/','upstream/stations/','upstream/bin/tg5040/','upstream/NextUI/workspace/tg5040/libmsettings/']
unchanged={};different={};intentional={
 'upstream/bin/tg5040/musicplayer.elf','upstream/bin/tg5040/libmsettings.so',
 'upstream/src/nxt.h','upstream/src/nxt_config.c','upstream/src/nxt_controls.c','upstream/src/nxt_decoder.c','upstream/src/nxt_decoder.h','upstream/src/nxt_eq.c','upstream/src/nxt_eq.h','upstream/src/nxt_hardware.c','upstream/src/nxt_personal.h','upstream/src/nxt_startup.c','upstream/src/nxt_skin.c','upstream/src/nxt_skin.h','upstream/src/nxt_theme.c','upstream/src/nxt_performance.c','upstream/src/nxt_presets.c','upstream/src/nxt_audio_v184.inc','upstream/src/nxt_personal.c','upstream/src/nxt_playback.h','upstream/src/nxt_player_core.c','upstream/src/nxt_screens.c','upstream/src/nxt_settings_v18.inc','upstream/src/nxt_soundcloud.c','upstream/src/nxt_spotify.c','upstream/src/nxt_ui.c','upstream/src/module_common.c','upstream/src/module_player.c','upstream/src/module_player.h','upstream/src/musicplayer.c','upstream/src/radio.c','upstream/src/podcast.c'}
with zipfile.ZipFile(original) as z:
 prefix=z.namelist()[0].split('/')[0]+'/'
 for entry in z.infolist():
  name=entry.filename.removeprefix(prefix)
  if entry.is_dir() or not (name.startswith(tuple(roots)) or name.startswith('upstream/src/')):continue
  p=R/name;assert p.is_file(),name
  before=hashlib.sha256(z.read(entry)).hexdigest();after=hashlib.sha256(p.read_bytes()).hexdigest()
  if before==after:unchanged[name]=after
  else:assert name in intentional,('unexpected baseline change',name);different[name]={'before':before,'after':after}
assert 'upstream/src/nxt_theme_v18.inc' in unchanged
assert 'upstream/bin/tg5040/nxt-spotify-backend' in unchanged and 'upstream/bin/tg5040/nxt-soundcloud-backend' in unchanged
(R/'logs/v185-baseline-preserved.json').write_text(json.dumps({'exit':0,'baseline_source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'unchanged_files':unchanged,'intentional_native_changes':different,'resources_icons_fonts_emoji_locales_ime_unchanged':True,'online_backends_unchanged':True,'normal_Bluetooth_msettings_unchanged':True},indent=2)+'\n')
print('PASS baseline:',len(unchanged),'unchanged source/assets/payload files;',len(different),'intentional native changes')
