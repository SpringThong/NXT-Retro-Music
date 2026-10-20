#!/usr/bin/env python3
"""Production shell launcher with explicit frontend/firmware-command fixtures."""
from pathlib import Path
import json,os,subprocess,tempfile,time,shutil
R=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='nxt-nx-launch-') as td:
 w=Path(td);app=w/'Tools/NXT Retro Music.pak';b=app/'bin/tg5040';b.mkdir(parents=True);system=w/'.system';(system/'lib').mkdir(parents=True);(system/'bin').mkdir()
 (system/'version.txt').write_text('NXRedux-v2.0.0\n');(system/'lib/libmsettings.so').write_bytes(b'FIXTURE_SETTINGS_LIBRARY');shutil.copy2(R/'nxredux/launch.sh',app/'launch.sh')
 trace=w/'trace';ctl=system/'bin/musicplayerctl.elf';ctl.write_text('#!/bin/sh\nprintf "%s\\n" "$*" >> "$NXT_QA_TRACE"\n');ctl.chmod(0o755)
 fake=b/'musicplayer.elf';fake.write_text(r'''#!/usr/bin/env python3
import os,sys,time
from pathlib import Path
state=Path(os.environ['NXT_SESSION_SOCKET']+'.state')
trace=Path(os.environ['NXT_QA_TRACE'])
if '--attach-current' in sys.argv:
 if not state.exists():sys.exit(1)
 pid=state.read_text().split()[1];state.write_text('VISIBLE '+pid+'\n');sys.exit(0)
if '--session' not in sys.argv:sys.exit(2)
with trace.open('a') as f:f.write('start\n')
print('NX frontend fixture started',flush=True)
begin=time.monotonic();hide=None;state.write_text('VISIBLE '+str(os.getpid())+'\n')
while time.monotonic()-begin<1.2:
 if state.exists() and state.read_text().startswith('VISIBLE'):
  if hide is None:hide=time.monotonic()+.12
  if time.monotonic()>=hide:state.write_text('HIDDEN '+str(os.getpid())+'\n');hide=None
 time.sleep(.01)
state.unlink(missing_ok=True)
''');fake.chmod(0o755)
 data=w/'data';data.mkdir();(data/'nxt.json').write_text('PREFERENCES_KEEP');base={**os.environ,'SYSTEM_PATH':str(system),'SDCARD_PATH':td,'SHARED_USERDATA_PATH':str(data),'DEVICE':'brickpro','PLATFORM':'tg5040','NXT_SESSION_SOCKET':td+'/nxt.sock','NXT_QA_TRACE':str(trace)}
 for i in range(2):
  p=subprocess.run(['sh',str(app/'launch.sh')],env=base,capture_output=True,text=True,timeout=5);assert p.returncode==0,p.stderr
 assert trace.read_text().splitlines()==['stop','start','stop'],trace.read_text()
 assert (data/'nxt.json').read_text()=='PREFERENCES_KEEP' and 'fixture started' in (app/'NXT_Input_Log.txt').read_text()
 deadline=time.monotonic()+3
 while Path(td+'/nxt.sock.state').exists() and time.monotonic()<deadline:time.sleep(.02)
 assert not Path(td+'/nxt.sock.state').exists()
 for bad in [{'DEVICE':'brick'},{'PLATFORM':'tg5050'}]:
  p=subprocess.run(['sh',str(app/'launch.sh')],env={**base,**bad},capture_output=True,text=True,timeout=5);assert p.returncode==1
 (system/'version.txt').write_text('NextUI\n');p=subprocess.run(['sh',str(app/'launch.sh')],env=base,capture_output=True,text=True,timeout=5);assert p.returncode==1
 assert trace.read_text().splitlines()==['stop','start','stop']
print('PASS actual NX launcher: cold start, warm reattach without duplicate process, system PCM stop, stable logs, data preservation, wrong device/ROM rejection.')
(R/'logs/nxredux-launcher-test.json').write_text(json.dumps({'exit':0,'production_launch_script':True,'cold_start_and_reattach_without_duplicate':True,'firmware_player_stopped_without_killing_daemon':True,'stable_logs_and_userdata':True,'wrong_device_ROM_rejected':True,'frontend_process_fixture':True},indent=2)+'\n')
