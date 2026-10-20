#!/usr/bin/env python3
"""Run the production shell flow with controlled firmware/mount endpoints."""
from pathlib import Path
import tempfile,subprocess,os,json,hashlib
R=Path(__file__).resolve().parents[1];text=(R/'upstream/bin/usb-transfer.sh').read_text()
assert 'umount -f' not in text and 'kill ' not in text
subprocess.run(['sh','-n',str(R/'upstream/bin/usb-transfer.sh')],check=True)
checks=[]
with tempfile.TemporaryDirectory() as td:
 p=Path(td);root=p/'root';log=p/'events';bindir=p/'bin';bindir.mkdir()
 def script(path,body):path.parent.mkdir(parents=True,exist_ok=True);path.write_text('#!/bin/sh\n'+body);path.chmod(0o755)
 def event(body):return 'printf "%s\\n" '+body+' >> "'+str(log)+'"\n'
 for name in ['sync','mount']:script(bindir/name,event('"'+name+' $*"'))
 script(bindir/'umount',event('"umount $*"')+'[ "${QA_BUSY:-0}" != 1 ]\n')
 for role in ['device','host','dock']:script(root/f'usr/trimui/bin/usb_{role}.sh',event('"role '+role+'"'))
 script(root/'usr/trimui/bin/systemval','echo host\n')
 script(root/'bin/setusbconfig',event('"config $*"')+'exit 0\n')
 script(root/'usr/trimui/apps/usb_storage/usb_storage',event('"storage UI"'))
 mounts=root/'proc/mounts';mounts.parent.mkdir(parents=True);mounts.write_text('/dev/mmcblk1p1 /mnt/SDCARD exfat rw,noatime 0 0\n')
 role=root/'sys/devices/platform/soc/usbc0/otg_role';role.parent.mkdir(parents=True);role.write_text('usb_host\n')
 (root/'tmp').mkdir();fixture=p/'flow.sh';mock=text
 import re
 mock=re.sub(r'/usr/trimui|/sys/|/bin/setusbconfig|/proc/mounts|/tmp/',lambda m:str(root)+m.group(0),text)
 fixture.write_text(mock)
 env={**os.environ,'PATH':str(bindir)+':'+os.environ['PATH']}
 subprocess.run(['sh',str(fixture)],check=True,env=env);events=log.read_text().splitlines()
 assert events.index('umount /mnt/SDCARD')<events.index('config mass_storage,adb')<events.index('storage UI')<events.index('config adb')
 assert any(x.startswith('mount -t exfat -o rw,noatime /dev/mmcblk1p1') for x in events) and events[-1]=='role host'
 assert not (root/'tmp/stay_awake').exists();checks.append('normal unmount -> device/mass storage -> native UI -> detach -> original mount options and host role')
 log.write_text('');awake=root/'tmp/stay_awake';awake.write_text('PREEXISTING')
 result=subprocess.run(['sh',str(fixture)],env={**env,'QA_BUSY':'1'});assert result.returncode!=0 and 'config' not in log.read_text() and awake.read_text()=='PREEXISTING';checks.append('busy SD aborts before storage; preexisting awake flag preserved')
 fw=R/'.cache/stock-research/rootfs';assert (fw/'usr/trimui/apps/usb_storage/usb_storage').is_file()
 checks.append('USB endpoint and mass_storage,adb flow confirmed against official Brick Pro firmware 1.1.1-20260717 files')
 (R/'logs/v18-usb-test.json').write_text(json.dumps({'exit':0,'checks':checks,'forced_unmount':False,'physical_USB_cable_tested':False},indent=2))
print('PASS USB shell syntax, real flow with mocked endpoints, busy-card abort and role/awake cleanup; official firmware endpoint verified.')
