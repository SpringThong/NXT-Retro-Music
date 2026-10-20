from pathlib import Path
import subprocess,json,shutil
root=Path(__file__).resolve().parent/'media';root.mkdir(exist_ok=True)
items=[('viet.wav','pcm_s16le','Đường về nhà','Nguyễn Xuân Thông','Tiếng Việt'),('korea.mp3','libmp3lame','사랑의 노래','한국어 테스트','별빛 아래에서'),('japan.flac','flac','夜に駆ける','漢字とかな','日本語'),('chinese.m4a','aac','月亮代表我的心','简体中文','音乐'),('traditional-alac.m4a','alac','風中的聲音','繁體中文','愛與夢想'),('thai.ogg','libvorbis','น้ำตาและความฝัน','ภาษาไทย','เพลงรัก'),('thai.opus','libopus','เพลงรัก','ภาษาไทย','ความฝัน'),('test.aac','aac','AAC test','NXT','Codecs')]
for i,(filename,codec,title,artist,album) in enumerate(items):
 file=root/filename
 command=['ffmpeg','-v','error','-y','-f','lavfi','-i',f'sine=frequency={220+i*55}:sample_rate=44100:duration=15','-ac','2','-c:a',codec,'-metadata','title='+title,'-metadata','artist='+artist,'-metadata','album='+album,'-metadata','replaygain_track_gain=-3.0 dB',str(file)]
 subprocess.run(command,check=True)
 (file.with_suffix('.lrc')).write_text(f'[ar:{artist}]\n[ti:{title}]\n[00:00.00]{title}\n[00:03.00]한국어 · 사랑의 노래\n[00:06.00]日本語 · 漢字とひらがな\n[00:09.00]简体中文 / 繁體中文\n[00:12.00]ภาษาไทย · น้ำตาและความฝัน\n',encoding='utf8')
subprocess.run(['ffmpeg','-v','error','-y','-f','lavfi','-i','sine=duration=2:sample_rate=96000','-c:a','pcm_s24le',str(root/'24bit.wav')],check=True)
subprocess.run(['ffmpeg','-v','error','-y','-f','lavfi','-i','sine=duration=2:sample_rate=48000','-c:a','pcm_s32le',str(root/'32bit.wav')],check=True)
for file in root.iterdir():shutil.copy2(file,'/var/tmp/nextui/sdcard/Music/'+file.name)
(root/'fixtures.json').write_text(json.dumps(items,ensure_ascii=False,indent=2))
print('Generated original sine signals and multilingual metadata / UTF-8 lyrics.')
