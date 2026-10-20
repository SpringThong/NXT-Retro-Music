#!/usr/bin/env python3
from pathlib import Path
import os,json,subprocess,sqlite3,re,time
ROOT=Path(__file__).resolve().parents[1];DATA=Path('/var/tmp/nextui/sdcard/.userdata/shared/nxt-retro-music')
config=json.loads((DATA/'nxt.json').read_text());config['repeat_mode']=2;(DATA/'nxt.json').write_text(json.dumps(config,ensure_ascii=False))
c=sqlite3.connect(DATA/'music-player.db');c.execute("INSERT OR REPLACE INTO settings(name,type,value) VALUES('spectrum_style','int',1)");c.commit();c.close()
env=os.environ.copy();env.update(NXT_QA_OFFLINE='1',SDL_VIDEODRIVER='offscreen',SDL_AUDIODRIVER='dummy',LIBGL_ALWAYS_SOFTWARE='1',USERDATA_PATH='/var/tmp/nextui/sdcard/.userdata/desktop',LD_LIBRARY_PATH=f'{ROOT}/.host-tools/usr/lib/x86_64-linux-gnu:/var/tmp/nextui/lib')
def run(name,commands,timeout=30):
 control=ROOT/f'qa/v1-{name}.control';control.write_text('\n'.join(commands)+'\n');reply=ROOT/f'logs/v1-{name}-replies.log'
 with open(ROOT/f'logs/v1-{name}.log','w') as log:
  p=subprocess.run([str(ROOT/'upstream/bin/desktop/musicplayer.elf'),'--test-control',f'{control},{reply}'],cwd=ROOT/'upstream',env=env,stdout=log,stderr=subprocess.STDOUT,timeout=timeout)
 assert p.returncode==0,(name,p.returncode)
 text=reply.read_text();assert 'err ' not in text,text
 return [{k:int(v) for k,v in re.findall(r'(playback|position_ms|off|spectrum|color|favorite|timer)=([0-9]+)',line)} for line in text.splitlines() if 'state playback=' in line]
states=run('resume-colors',['wait(600)','press(Y)','wait(500)','status()','press(R2)','wait(200)','status()','press(L2)','wait(200)','status()','press(L2,2)','wait(200)','status()','quit()'])
assert states[0]['playback']==2 and states[0]['spectrum']==0 and states[0]['color']==4 and states[0]['favorite']==1,states
assert states[1]['playback']==2 and states[1]['favorite']==0,states
assert states[2]['spectrum']==1 and states[2]['color']==0,states
assert states[3]['color']==2 and states[3]['playback']==2,states
c=sqlite3.connect(DATA/'music-player.db');assert c.execute("SELECT value FROM settings WHERE name='spectrum_style'").fetchone()[0]==1;c.close()
print('PASS: cold resume stays paused; OFF and favorite persist; R2 while paused; color cycle preserves saved style.',flush=True)
states=run('queue-off',['wait(600)','press(Y)','wait(400)','status()','press(A)','press(R1)','wait(400)','status()','press(POWER)','wait(16500)','status()','press(POWER)','wait(200)','status()','quit()'],timeout=27)
assert states[0]['color']==2 and states[0]['spectrum']==1,states
assert states[2]['off']==1 and states[2]['playback']==1 and states[2]['position_ms']<8000,states
assert states[3]['off']==0 and states[3]['playback']==1,states
print('PASS: saved color survives restart; normal queue wraps to next track with the backlight off.',flush=True)
states=run('timer-off',['wait(600)','press(Y)','wait(400)','press(A)','timer(1)','wait(200)','press(POWER)','wait(500)','status()','wait(62000)','status()','press(POWER)','wait(200)','status()','quit()'],timeout=72)
assert states[0]['off']==1 and states[0]['playback']==1 and states[0]['timer']>50,states
assert states[1]['off']==1 and states[1]['playback']==0 and states[1]['timer']==0,states
assert states[2]['off']==0 and states[2]['playback']==0,states
print('PASS: actual 60-second sleep timer expires while screen is off; wake does not restart stopped music.',flush=True)
states=run('long-power',['wait(600)','status()','hold(POWER,1300)','wait(2000)','status()'],timeout=8)
assert len(states)==1,states
print('PASS: long Power reaches the platform shutdown path and exits.',flush=True)
