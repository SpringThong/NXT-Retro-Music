#!/usr/bin/env python3
"""Validate final Stock/NextUI payloads, target isolation and offline OTA metadata."""
from pathlib import Path
import hashlib,json,zipfile,stat,re,os
R=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
meta=json.loads((R/'SOURCE.json').read_text())
for rom,label,folder,app in [('stock','Stock','stock','Apps/NXT_Retro_Music'),('nextui','NextUI','upstream','Tools/tg5040/NXT Retro Music.pak'),('nxredux','NXRedux','nxredux','Tools/NXT Retro Music.pak')]:
 if rom not in os.environ.get('QA_ROMS','stock,nextui,nxredux').split(','):continue
 path=R/f'dist/{rom}-release/NXT_Retro_Music_v1.10_{label}.zip';binary=R/f'{folder}/bin/tg5040/musicplayer.elf'
 with zipfile.ZipFile(path) as z:
  assert not any(any(part.startswith('.') for part in Path(n).parts) for n in z.namelist()), 'Unexpected temporary/private archive entry'
  assert z.testzip() is None
  assert hashlib.sha256(z.read(app+'/res/fonts/misans-semibold.ttf')).hexdigest()=='77c23f31ae124867778344970155a0c8d34a89897dedaab81aeee82ff00a4ce6'
  assert 'Xiaomi' in z.read(app+'/licenses/MiSans.txt').decode()
  assert z.read(app+'/launch.sh')==(R/folder/'launch.sh').read_bytes()
  assert not any('.rsync-tmp' in n or '.rsync-partial' in n for n in z.namelist())
  cfg=json.loads(z.read(app+'/config.json'));assert cfg['rom']==rom and cfg['version']=='1.10' and cfg['release_filename']==path.name
  assert z.read(app+'/state/app_version.txt').decode().strip()=='1.10'
  feed=z.read(app+f'/ota-source-{rom}.url').decode().strip();assert feed==meta['official_OTA_sources'][rom]
  assert z.read(app+'/bin/tg5040/musicplayer.elf')==binary.read_bytes()
  for n in ['launch.sh','bin/tg5040/musicplayer.elf','bin/tg5040/nxt-spotify-backend','bin/tg5040/nxt-soundcloud-backend','bin/tg5040/yt-dlp','bin/tg5040/yt-dlp-runtime/yt-dlp']:assert z.getinfo(app+'/'+n).external_attr>>16&0o111,n
  private={'groq-api-key.txt','groq-key.txt','translation-service.json','credentials.json','library.sqlite3','dac-names.json','last-playback.json','playback.json','NXT_App_Log.txt','NXT_Device_Log.txt'}
  for n in z.namelist():
   assert Path(n).name not in private,n
   if rom!='nxredux':assert 'libnxt-nxredux-settings.so' not in n and 'ota-source-nxredux.url' not in n,n
   assert not n.startswith(('.system/','.userdata/','Tools/')) if rom=='stock' else not n.startswith(('.system/','.userdata/','Apps/'))
  assert (app+'/integration/nextui-v2.1.2-nxt-icon.elf' in z.namelist())==(rom=='nxredux')
  if rom=='nxredux':assert app+'/bin/tg5040/libmsettings.so' not in z.namelist() and not any(n.startswith('Tools/tg5040/') for n in z.namelist())
  assert app+'/icon.png' in z.namelist() and z.read(app+'/icon.png').startswith(b'\x89PNG')
  for n in ['nxt-spotify-backend','nxt-soundcloud-backend']:assert z.read(app+'/bin/tg5040/'+n)==(R/'upstream/bin/tg5040'/n).read_bytes()
  for doc in [(app+'/docs/' if rom=='nxredux' else '')+f'LIBRARY_v1.10.md',(app+'/docs/' if rom=='nxredux' else '')+f'DSD_v1.10_{label}.md',f'HUONG_DAN_NXT_Retro_Music_v1.10_{label}.md',f'CHANGELOG_v1.10_{label}.md']:assert doc in z.namelist(),doc
  for other in ['stock','nextui','nxredux']:
   if other!=rom:assert not any(n.endswith('/ota-source-'+other+'.url') for n in z.namelist())
  original_assets=list((R/'upstream/res').rglob('*'));count=0
  for p in original_assets:
   if p.is_file() and not p.is_symlink() and not any(part.startswith('.') or part=='__pycache__' for part in p.relative_to(R/'upstream/res').parts):
    n=app+'/res/'+p.relative_to(R/'upstream/res').as_posix();assert z.read(n)==p.read_bytes(),n;count+=1
  if rom!='nxredux':
   instructions=z.read('OTA_Google_Drive_v1.10/HUONG_DAN.template.md').decode();assert 'v1.9.1' not in instructions and 'v1.10' in instructions
 kit=R/f'dist/{rom}-release/NXT_OTA_Google_Drive_v1.10_{label}.zip';prefix=f'OTA_Google_Drive_v1.10_{label}/'
 with zipfile.ZipFile(kit) as z:
  assert not any(any(part.startswith('.') for part in Path(n).parts) for n in z.namelist()), 'Unexpected temporary/private archive entry'
  assert z.testzip() is None;manifest=json.loads(z.read(prefix+'templates/update.json'))
  assert manifest['version']=='1.10' and manifest['rom']==rom and manifest['size']==path.stat().st_size and manifest['sha256']==sha(path)
  assert hashlib.sha256(z.read(prefix+path.name)).hexdigest()==sha(path)
  html=z.read(prefix+'CHUAN_BI_OTA.html').decode();assert '__OTA_RELEASE_DATA__' not in html and 'v1.9.1' not in html
  data=json.loads(re.search(r'const RELEASE = (.+);',html).group(1));assert data['publisherSource']==feed and data['label']==label and data['manifest']==manifest
  guide=z.read(prefix+'HUONG_DAN_DANG_OTA.md').decode();assert feed in guide and 'v1.9.1' not in guide and 'không cần nhập link' in guide
  assert prefix+'update.json' not in z.namelist()
 inputs=json.loads((R/f'{folder}/BUILD_INPUTS_v1.10.json').read_text());assert inputs['rom']=='NXT_ROM_'+rom.upper() and inputs['binary_sha256']==sha(binary)
 for name,digest in inputs['sources'].items():assert sha(R/name)==digest,name
 report={'exit':0,'version':'1.10','rom':rom,'binary_sha256':sha(binary),'installer_size':path.stat().st_size,'installer_sha256':sha(path),'kit_size':kit.stat().st_size,'kit_sha256':sha(kit),'official_OTA_URL':feed,'cached_version_and_config_current':True,'correct_ROM_paths_and_libraries':True,'original_resource_assets_verified':count,'online_backends_match_built_payloads':True,'no_userdata_credentials_or_other_ROM_feed':True,'executable_modes_CRC_checked':True,'current_build_fingerprints_match':True,'exact_installer_in_OTA_kit':True,'offline_OTA_generator_metadata_and_guide_current':True,'live_feed_uploaded':False,'hardware_tested':False}
 (R/f'logs/v110-package-qa-{rom}.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',rom,'installer, official feed, assets, target isolation, OTA kit, final fingerprints')
