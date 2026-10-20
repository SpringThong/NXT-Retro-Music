from release_zip_entry import add_file
#!/usr/bin/env python3
"""Package NX Redux separately, preserving native backends and all UI assets."""
from pathlib import Path
import hashlib,json,os,shutil,struct,zipfile
from ota_source_guard import publisher_source
R=Path(__file__).resolve().parents[1]
STAGE=R/'dist/nxredux-stage';APP=STAGE/'Tools/NXT Retro Music.pak';OUT=R/'dist/nxredux-release'
def copy(s,t):
 t.parent.mkdir(parents=True,exist_ok=True)
 if s.is_dir():shutil.copytree(s,t,dirs_exist_ok=True,ignore=shutil.ignore_patterns('__pycache__','.*'))
 else:shutil.copy2(s,t)
def main():
 if STAGE.exists():shutil.rmtree(STAGE)
 APP.mkdir(parents=True);OUT.mkdir(parents=True,exist_ok=True)
 c=json.loads((R/'nxredux/config.json').read_text());assert c['rom']=='nxredux';version=c['version']
 publisher=publisher_source(R/'nxredux/ota-source-nxredux.url')
 for n in ['config.json','pak.json','launch.sh']:copy(R/'nxredux'/n,APP/n)
 (APP/'ota-source-nxredux.url').write_text(publisher+'\n')
 # Common backends/runtime are kept once in source. Only the ROM frontend and
 # settings adapter differ; never include NextUI's old settings library.
 for p in (R/'upstream/bin/tg5040').iterdir():
  if p.name not in {'musicplayer.elf','libmsettings.so'}:copy(p,APP/'bin/tg5040'/p.name)
 for name in ['musicplayer.elf','libnxt-nxredux-settings.so']:copy(R/'nxredux/bin/tg5040'/name,APP/'bin/tg5040'/name)
 for n in ['res','licenses','stations','state/app_version.txt','LICENSE']:copy(R/'upstream'/n,APP/n)
 (APP/'state/app_version.txt').write_text(version+'\n')
 copy(R/'upstream/res/nxt-logo.png',APP/'icon.png')
 for n in ['nextui-v2.1.2-nxt-icon.elf','icon-integration.json','install-tools-icon.sh','menu_icon_nxt.png']:copy(R/'nxredux/integration'/n,APP/'integration'/n)
 assert not (APP/'bin/tg5040/libmsettings.so').exists()
 for name in ['musicplayer.elf','nxt-spotify-backend','nxt-soundcloud-backend','libnxt-nxredux-settings.so']:
  p=APP/'bin/tg5040'/name;h=p.read_bytes()[:64];assert h[:6]==b'\x7fELF\2\1' and struct.unpack_from('<H',h,18)[0]==183;p.chmod(0o755)
 for name in ['launch.sh','bin/tg5040/yt-dlp','bin/tg5040/yt-dlp-runtime/yt-dlp']:(APP/name).chmod(0o755)
 copy(R/f'docs/HUONG_DAN_NXT_Retro_Music_v{version}_NXRedux.md',STAGE/f'HUONG_DAN_NXT_Retro_Music_v{version}_NXRedux.md')
 copy(R/f'CHANGELOG_v{version}_NXRedux.md',STAGE/f'CHANGELOG_v{version}_NXRedux.md')
 copy(R/'docs/THEM_MOI_v1.9.1.md',STAGE/'THEM_MOI_v1.9.1.md')
 copy(R/f'docs/DSD_v{version}_NXRedux.md',APP/f'docs/DSD_v{version}_NXRedux.md')
 copy(R/f'docs/LIBRARY_v{version}.md',APP/f'docs/LIBRARY_v{version}.md')
 copy(R/'Spotify_Login_PC',STAGE/'Spotify_Login_PC')
 archive=OUT/c['release_filename'];temp=archive.with_suffix('.zip.building')
 private={'groq-api-key.txt','groq-key.txt','groq-api-key.txt','groq-key.txt','translation-service.json', 'credentials.json','spotify-credentials.json','soundcloud-credentials.json','playback.json','NXT_App_Log.txt','NXT_Input_Log.txt','NXT_Spotify_Log.txt','NXT_SoundCloud_Log.txt'}
 with zipfile.ZipFile(temp,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for p in sorted(STAGE.rglob('*')):
   if p.is_file() and not any(part in (".rsync-tmp",".rsync-partial") for part in p.parts) and p.name not in private:add_file(z,p,STAGE)
 with zipfile.ZipFile(temp) as z:
  assert z.testzip() is None
  assert all(not n.startswith(('.system/','.userdata/','Apps/','Tools/tg5040/')) for n in z.namelist())
  for n in ['launch.sh','bin/tg5040/musicplayer.elf','bin/tg5040/nxt-spotify-backend','bin/tg5040/nxt-soundcloud-backend','bin/tg5040/yt-dlp-runtime/yt-dlp']:
   assert z.getinfo('Tools/NXT Retro Music.pak/'+n).external_attr>>16&0o111,n
 with temp.open('rb') as f:os.fsync(f.fileno())
 os.replace(temp,archive)
 with archive.open('rb') as f:sha=hashlib.file_digest(f,'sha256').hexdigest()
 archive.with_suffix('.zip.sha256').write_text(sha+'  '+archive.name+'\n')
 print(json.dumps({'installer':str(archive),'size':archive.stat().st_size,'sha256':sha},indent=2),flush=True)
if __name__=='__main__':main()
