#!/usr/bin/env python3
"""Test production USB role code with fixtures, including read-triggered transitions."""
from pathlib import Path
import ctypes,tempfile,os,threading,subprocess
r=Path(__file__).resolve().parents[1];q=r/'.cache/bottom-usb';q.mkdir(parents=True,exist_ok=True);so=q/'usb.so'
subprocess.run(['gcc','-shared','-fPIC','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',str(r/'upstream/src/nxt_usb.c'),'-o',str(so)],check=True)
l=ctypes.CDLL(str(so));l.NxtUSB_role.argtypes=[ctypes.c_char_p];l.NxtUSB_switch.argtypes=[ctypes.c_char_p,ctypes.c_bool]
with tempfile.TemporaryDirectory() as td:
 b=Path(td);arg=td.encode();role=b/'otg_role'
 assert l.NxtUSB_role(arg)==0 and l.NxtUSB_switch(arg,True)==0
 role.write_text('usb_device\n');assert l.NxtUSB_role(arg)==2
 assert l.NxtUSB_switch(arg,True)==0 # missing action node
 # FIFO behaves like a sysfs action that changes role when opened for reading.
 for host in [True,False]:
  action=b/('usb_host' if host else 'usb_device');os.mkfifo(action)
  def driver():
   with action.open('w') as f:
    role.write_text('usb_host\n' if host else 'usb_device\n');f.write('host_chose finished!\n' if host else 'device_chose finished!\n')
  worker=threading.Thread(target=driver,daemon=True);worker.start()
  assert l.NxtUSB_switch(arg,host)==2;worker.join(2);assert not worker.is_alive()
  # No writer remains: repeat switch/status MUST NOT open the action FIFO.
  assert l.NxtUSB_role(arg)==(1 if host else 2);assert l.NxtUSB_switch(arg,host)==1
 role.write_text(' \tusb_device\r\n');assert l.NxtUSB_role(arg)==2
 (b/'usb_host').unlink();(b/'usb_host').write_text('host_chose finished!\n')
 role.write_text('unknown\n');assert l.NxtUSB_switch(arg,True)==3
 assert l.NxtUSB_role(arg)==0
 (b/'usb_host').unlink();assert l.NxtUSB_switch(arg,True)==0
s=(r/'upstream/src/nxt_hardware.c').read_text()
assert 'NxtUSB_switch' not in s.split('void NxtHardware_tick(void)',1)[1].split('void NxtHardware_release',1)[0]
ui=(r/'upstream/src/nxt_screens.c').read_text();assert 'case 9:{if(p.action!=1)break;' in ui
print('PASS USB: status reads only otg_role; explicit read triggers host/device; idempotent requests never touch action FIFO; unknown role permits explicit request; unconfirmed transition is pending; missing action fails; no startup/tick auto-switch; menu applies only on A.')
