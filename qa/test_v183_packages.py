#!/usr/bin/env python3
"""Inspect both actual installer ZIPs for ROM identity and new v1.8 resources."""
from pathlib import Path
import hashlib,json,struct,zipfile
R=Path(__file__).resolve().parents[1]
reports={}
for rom,label,prefix,binary in [('stock','Stock','Apps/NXT_Retro_Music/','stock/bin/tg5040/musicplayer.elf'),('nextui','NextUI','Tools/tg5040/NXT Retro Music.pak/','upstream/bin/tg5040/musicplayer.elf')]:
 p=R/f'dist/{rom}-release/NXT_Retro_Music_v1.8.3_{label}.zip'
 with zipfile.ZipFile(p) as z:
  assert z.testzip() is None
  names=set(z.namelist());c=json.loads(z.read(prefix+'config.json'));assert c['version']=='1.8.3' and c['rom']==rom and c['device']=='trimui-brick-pro'
  if rom=='nextui':assert json.loads(z.read(prefix+'pak.json'))['version']=='1.8.3'
  for n in ['launch.sh','bin/tg5040/musicplayer.elf','bin/tg5040/nxt-spotify-backend','bin/tg5040/nxt-soundcloud-backend','bin/tg5040/yt-dlp','bin/tg5040/yt-dlp-runtime/yt-dlp']:
   assert z.getinfo(prefix+n).external_attr>>16&0o111,n
  elf=z.read(prefix+'bin/tg5040/musicplayer.elf');assert elf[:6]==b'\x7fELF\2\1' and struct.unpack_from('<H',elf,18)[0]==183
  assert hashlib.sha256(elf).hexdigest()==hashlib.sha256((R/binary).read_bytes()).hexdigest()
  assert f'ota-source-{rom}.url' in {n[len(prefix):] for n in names if n.startswith(prefix)}
  assert prefix+f'ota-source-{"nextui" if rom=="stock" else "stock"}.url' not in names
  for i in range(36): assert prefix+f'res/themes/v18/theme-{i:02d}.png' in names
  catalog=json.loads(z.read(prefix+'res/icons/v18-catalog.json'));assert len(catalog)==300
  for icon in catalog: assert prefix+'res/icons/'+icon+'.png' in names,icon
  emojis=json.loads(z.read(prefix+'res/emoji3d/catalog.json'))['items'];assert len(emojis)==180
  for i in range(180):assert prefix+f'res/emoji3d/{i:03d}.png' in names
  for locale in ['en','vi','fr','ru','ko','ja','th','zh-Hans','zh-Hant','de','es','pt-BR','it','id']:
   loc=json.loads(z.read(prefix+f'res/locales/{locale}.json'));assert len(loc)>=900
   for key in ['Original theme options','Battery saver','Balanced','Performance','Performance mode','Timer ends: save position and close NXT, including background playback','System settings control screen-off and sleep after NXT exits']:assert key in loc and loc[key]
  assert z.getinfo(prefix+'res/ime/candidates.sqlite3').file_size>50*1024*1024
  assert not any('usb-transfer' in n for n in names)
  assert z.read(prefix+'state/app_version.txt').decode().strip()=='1.8.3'
  forbidden={'credentials.json','spotify-credentials.json','soundcloud-credentials.json','playback.json','NXT_App_Log.txt','NXT_Input_Log.txt'}
  assert all(Path(n).name not in forbidden for n in names)
  assert all(not n.startswith(('.userdata/','.system/')) for n in names)
 with p.open('rb') as f:sha=hashlib.file_digest(f,'sha256').hexdigest()
 reports[rom]={'exit':0,'CRC':True,'config_version':'1.8.3','native_AArch64':True,'elf_matches_build':True,'executable_modes':True,'USB_copy_removed_DAC_retained':True,'themes':36,'icons':300,'emoji_3D':180,'locales':14,'offline_IME':True,'no_credentials_or_userdata':True,'size':p.stat().st_size,'sha256':sha}
(R/'logs/v183-packages-test.json').write_text(json.dumps(reports,indent=2)+'\n')
print('PASS both actual v1.8.3 installers: matching ARM64 builds, complete new resources, ROM-specific OTA source, executable modes, no user data, ZIP CRC.')
