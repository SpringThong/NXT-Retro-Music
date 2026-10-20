#!/usr/bin/env python3
"""Exercise actual native SDL UI using debug-only controls, dummy audio and GL."""
from pathlib import Path
import os,json,shutil,subprocess,wave,re
ROOT=Path(__file__).resolve().parents[1]
DATA=Path('/var/tmp/nextui/sdcard/.userdata/shared/nxt-retro-music')
MUSIC=Path('/var/tmp/nextui/sdcard/MusicV1QA');MUSIC.mkdir(parents=True,exist_ok=True)
if DATA.exists():shutil.rmtree(DATA)
DATA.mkdir(parents=True,exist_ok=True)
with wave.open(str(ROOT/'qa/media/viet.wav'),'rb') as src:
 params=src.getparams();pcm=src.readframes(src.getnframes())
with wave.open(str(MUSIC/'01 Kiểm tra 한국어 日本語 中文 ไทย.wav'),'wb') as dst:
 dst.setparams(params);dst.writeframes(pcm*8)
shutil.copyfile(ROOT/'qa/media/korea.mp3',MUSIC/'02 사랑의 노래.mp3')
shutil.copyfile(ROOT/'qa/media/viet.lrc',MUSIC/'01 Kiểm tra 한국어 日本語 中文 ไทย.lrc')
(DATA/'nxt.json').write_text(json.dumps({'language':1,'music_root':str(MUSIC),'background':0,'transition':0},ensure_ascii=False))
SCREENS=ROOT/'qa/screens-v1';SCREENS.mkdir(exist_ok=True)
commands=[];labels=[]
def command(v):commands.append(v)
def wait(ms=220):command(f'wait({ms})')
def press(key,n=1):command(f'press({key},{n})')
def state(label):labels.append(label);command('status()')
def shot(name):command(f'screenshot({SCREENS/name}.png)')
wait(800);state('home-empty');shot('home-no-device')
press('A');wait(1600);press('A');wait(800);state('playing');shot('playing-multilingual')
for i in range(6):
 press('L2');wait(250);state(f'palette-{i+1}')
press('R2');wait();state('favorite-added');shot('favorite-heart')
press('R2');wait();state('favorite-removed')
press('POWER');wait(800);state('off-playing')
wait(1500);state('off-progress')
press('POWER');wait();state('wake-playing')
press('A');wait();state('paused')
press('POWER');wait(800);state('off-paused')
wait(700);state('off-paused-progress')
press('A');wait();state('wake-consumed-A')
press('A');wait();state('playing-after-wake')
command('hold(SELECT,1100)');wait();state('locked')
press('POWER');wait(900);state('off-locked')
press('POWER');wait();state('wake-locked')
command('hold(SELECT,1100)');wait();state('unlocked')
command('jack(1)');wait();state('jack-only');shot('headphones-connected')
command('dac(1)');wait(650);state('jack-and-dac');shot('headphones-and-dac')
command('jack(0)');wait();state('dac-only');shot('dac-connected')
command('dac(0)');wait(650);state('no-devices');shot('no-devices-playing')
press('B');wait();state('home-playing')
press('POWER');wait(800);state('off-home')
press('POWER');wait();state('wake-home')
press('DOWN',2);press('RIGHT');press('A');wait();shot('about')
press('DOWN',4);press('UP',4);press('A');wait();shot('usage-guide')
press('DOWN',6);wait();shot('usage-L2-R2')
press('B');press('B');press('Y');wait();state('return-to-playing')
press('R2');press('L2',5);wait();state('persist-off-favorite')
command('quit()')
control=ROOT/'qa/v1-integration.control';control.write_text('\n'.join(commands)+'\n')
replies=ROOT/'logs/v1-gui-replies.log'
env=os.environ.copy();env.update(NXT_QA_OFFLINE='1',SDL_VIDEODRIVER='offscreen',SDL_AUDIODRIVER='dummy',LIBGL_ALWAYS_SOFTWARE='1',USERDATA_PATH='/var/tmp/nextui/sdcard/.userdata/desktop',LD_LIBRARY_PATH=f'{ROOT}/.host-tools/usr/lib/x86_64-linux-gnu:/var/tmp/nextui/lib')
with open(ROOT/'logs/v1-gui.log','w') as log:
 result=subprocess.run([str(ROOT/'upstream/bin/desktop/musicplayer.elf'),'--test-control',f'{control},{replies}'],cwd=ROOT/'upstream',env=env,stdout=log,stderr=subprocess.STDOUT,timeout=65)
assert result.returncode==0, result.returncode
text=replies.read_text();assert 'err ' not in text,text
states=[]
for line in text.splitlines():
 if 'state playback=' in line:
  states.append({k:int(v) for k,v in re.findall(r'(playback|position_ms|lock|font|bands|off|spectrum|color|favorite|jack|dac|output)=([0-9]+)',line)})
assert len(states)==len(labels),(len(states),len(labels),text)
results=dict(zip(labels,states))
assert results['playing']['playback']==1
for i,(color,enabled) in enumerate([(1,1),(2,1),(3,1),(4,1),(4,0),(0,1)]):
 assert results[f'palette-{i+1}']['color']==color and results[f'palette-{i+1}']['spectrum']==enabled,results
assert results['favorite-added']['favorite']==1 and results['favorite-removed']['favorite']==0
for label in ('off-playing','off-progress','off-home','off-locked'):
 assert results[label]['off']==1 and results[label]['playback']==1,(label,results[label])
assert results['off-progress']['position_ms']-results['off-playing']['position_ms']>1000
for label in ('wake-playing','wake-home','wake-locked'):
 assert results[label]['off']==0 and results[label]['playback']==1,(label,results[label])
for label in ('paused','off-paused','off-paused-progress','wake-consumed-A'):
 assert results[label]['playback']==2,(label,results[label])
assert abs(results['off-paused-progress']['position_ms']-results['off-paused']['position_ms'])<20
assert results['wake-consumed-A']['off']==0
assert results['locked']['lock']==results['off-locked']['lock']==results['wake-locked']['lock']==1
assert results['unlocked']['lock']==0
for label,jack,dac in [('jack-only',1,0),('jack-and-dac',1,1),('dac-only',0,1),('no-devices',0,0)]:
 assert results[label]['jack']==jack and results[label]['dac']==dac,(label,results[label])
assert results['persist-off-favorite']['spectrum']==0 and results['persist-off-favorite']['favorite']==1
(ROOT/'logs/v1-gui-results.json').write_text(json.dumps(results,ensure_ascii=False,indent=2)+'\n')
print('PASS: native UI Power on/off, continuing PCM clock, paused/wake consumption, lock, six L2 steps, R2, connection indicators, About/guide captures; persisted off/favorite.')
