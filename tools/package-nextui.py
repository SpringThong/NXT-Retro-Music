from release_zip_entry import add_file
#!/usr/bin/env python3
"""Package the current NextUI Pak and verify the installed target."""
from ota_source_guard import publisher_source
from pathlib import Path
import hashlib,json,os,shutil,struct,zipfile
R=Path(__file__).resolve().parents[1];SRC=R/'upstream';STAGE=R/'dist/nextui-stage';APP=STAGE/'Tools/tg5040/NXT Retro Music.pak';OUT=R/'dist/nextui-release'
def copy(s,t):
 t.parent.mkdir(parents=True,exist_ok=True)
 if s.is_dir():shutil.copytree(s,t,dirs_exist_ok=True,ignore=shutil.ignore_patterns('__pycache__','.*'))
 else:shutil.copy2(s,t)
def main():
 if STAGE.exists():shutil.rmtree(STAGE)
 APP.mkdir(parents=True);OUT.mkdir(parents=True,exist_ok=True)
 ota_url=publisher_source(R/'upstream/ota-source-nextui.url')
 copy(R/'docs/THEM_MOI_v1.9.1.md',STAGE/'THEM_MOI_v1.9.1.md')
 config=json.loads((SRC/'config.json').read_text());assert config['rom']=='nextui';version=config['version']
 for n in ['pak.json','config.json','launch.sh','ota-source-nextui.url','res','licenses','stations','state/app_version.txt','bin/tg5040']:
  copy(SRC/n,APP/n)
 copy(SRC/'res/nxt-logo.png',APP/'icon.png');copy(SRC/'LICENSE',APP/'LICENSE')
 (APP/'ota-source-nextui.url').write_text(ota_url+'\n')
 for n in ['musicplayer.elf','nxt-spotify-backend','nxt-soundcloud-backend']:
  p=APP/'bin/tg5040'/n;h=p.read_bytes()[:64];assert h[:6]==b'\x7fELF\2\1' and struct.unpack_from('<H',h,18)[0]==183;nxt_mode=0o755;p.chmod(nxt_mode)
 for n in ['launch.sh','bin/tg5040/yt-dlp','bin/tg5040/yt-dlp-runtime/yt-dlp']:(APP/n).chmod(0o755)
 assert '--attach-current' in (APP/'launch.sh').read_text()
 assert not (APP/'ota-source-stock.url').exists()
 for note in [f'LIBRARY_v{version}.md',f'DSD_v{version}_NextUI.md']:copy(R/'docs'/note,STAGE/note)
 copy(R/f'docs/HUONG_DAN_NXT_Retro_Music_v{version}_NextUI.md',STAGE/f'HUONG_DAN_NXT_Retro_Music_v{version}_NextUI.md')
 copy(R/f'CHANGELOG_v{version}_NextUI.md',STAGE/f'CHANGELOG_v{version}_NextUI.md')
 copy(R/f'docs/OTA_Google_Drive_v{version}',STAGE/f'OTA_Google_Drive_v{version}');copy(R/'Spotify_Login_PC',STAGE/'Spotify_Login_PC');copy(R/'tools/make-ota-manifest.py',STAGE/'OTA_Google_Drive_NextUI/make-ota-manifest.py')
 copy(R/f'CHANGELOG_v{version}_NextUI.md',STAGE/f'OTA_Google_Drive_NextUI/CHANGELOG_v{version}_NextUI.md')
 archive=OUT/config['release_filename'];temp=archive.with_suffix('.zip.building');forbidden={'groq-api-key.txt','groq-key.txt','translation-service.json', 'credentials.json','spotify-credentials.json','soundcloud-credentials.json','playback.json','NXT_App_Log.txt','NXT_Input_Log.txt','NXT_Spotify_Log.txt','NXT_SoundCloud_Log.txt'}
 with zipfile.ZipFile(temp,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for p in sorted(STAGE.rglob('*')):
   if p.is_file() and not any(part in (".rsync-tmp",".rsync-partial") for part in p.parts) and p.name not in forbidden:add_file(z,p,STAGE)
 with zipfile.ZipFile(temp) as z:
  assert z.testzip() is None
  for n in ["launch.sh","bin/tg5040/musicplayer.elf","bin/tg5040/nxt-spotify-backend","bin/tg5040/nxt-soundcloud-backend","bin/tg5040/yt-dlp","bin/tg5040/yt-dlp-runtime/yt-dlp"]:
   assert z.getinfo("Tools/tg5040/NXT Retro Music.pak/"+n).external_attr>>16&0o111,n
  assert all(not n.startswith(('.userdata/','.system/','Apps/')) for n in z.namelist())
 with temp.open('rb') as f:os.fsync(f.fileno())
 os.replace(temp,archive)
 with archive.open('rb') as f:sha=hashlib.file_digest(f,'sha256').hexdigest()
 archive.with_suffix('.zip.sha256').write_text(sha+'  '+archive.name+'\n')
 print(json.dumps({'installer':str(archive),'size':archive.stat().st_size,'sha256':sha},indent=2),flush=True)
if __name__=='__main__':main()
