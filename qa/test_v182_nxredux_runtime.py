#!/usr/bin/env python3
"""Actual ARM64 ELF and installed firmware API, under QEMU, not device emulation."""
from pathlib import Path
import hashlib,json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];B=R/'nxredux/bin/tg5040';sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';system=R/'.cache/nx-redux/root/.system'
if not (system/'lib/libmsettings.so').exists():subprocess.run(['python3',str(R/'tools/fetch-nxredux-reference.py')],check=True)
env={**os.environ,'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu')}
base=[str(R/'.host-tools/usr/bin/qemu-aarch64'),'-L',str(sdk),'-E','LD_LIBRARY_PATH='+':'.join(map(str,[B,sdk/'lib',sdk/'usr/lib',system/'lib']))]
results={}
for args in [['--build-info'],['--help']]:
 p=subprocess.run([*base,str(B/'musicplayer.elf'),*args],capture_output=True,text=True,timeout=15,env=env);p.check_returncode();assert ('NX Redux' in p.stdout) if args==['--build-info'] else '--build-info' in p.stdout;results[str(args)]={'exit':0,'stdout':p.stdout[:1200]}
with tempfile.TemporaryDirectory(prefix='nxt-nx-runtime-') as td:
 w=Path(td);bin_=w/'bin';bin_.mkdir();(bin_/'amixer').write_text('#!/bin/sh\nexit 0\n');(bin_/'amixer').chmod(0o755)
 driver=w/'driver.c';driver.write_text(r'''
#include "msettings.h"
#include <assert.h>
#include <stdio.h>
int main(void){InitSettings();assert(InitializedSettings());
 assert(GetAudioSink()==1&&GetVolume()==9&&GetMusicVolume()==6&&GetSpeakerMute()==0);
 assert(GetMutedBrightness()==-69&&GetDisplayCalEnabled()==0);
 SetVolume(11);assert(GetVolume()==11);SetAudioSink(2);assert(GetAudioSink()==2);
 assert(GetMusicVolume()==6);QuitSettings();assert(!InitializedSettings());
 puts("PASS actual NX Redux v2.0.0 libmsettings API through NXT adapter, live volume/sink/music gain and optional display calibration fallback");return 0;}
''')
 compiler=R/'.nxt-sdk/tg5040/compiler/bin/aarch64-nextui-linux-gnu-gcc';driver_bin=w/'driver'
 subprocess.run([str(compiler),'-O2','-DNXT_ROM_NXREDUX','-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings'),str(driver),'-L'+str(B),'-l:libnxt-nxredux-settings.so','-Wl,-rpath-link,'+str(sdk/'lib')+':'+str(sdk/'usr/lib'),'-lpthread','-ldl','-lrt','-o',str(driver_bin)],check=True)
 seed=w/'seed.c';seed.write_text(r'''
#include "nxredux_msettings_shm_v2.h"
#include <stdio.h>
int main(int argc,char**argv){SettingsShm s={0};s.version=12;s.brightness=5;s.colortemperature=20;
 s.speaker=13;s.headphones=9;s.fn_brightness=s.fn_colortemperature=s.fn_contrast=s.fn_saturation=s.fn_exposure=s.fn_volume=-69;
 s.game_volume=14;s.music_volume=7;s.rumble_strength=2;s.audiosink=1;
 FILE*f=fopen(argv[1],"wb");if(!f)return 1;fwrite(&s,sizeof(s),1,f);fclose(f);return 0;}
''');seed_bin=w/'seed';subprocess.run(['gcc','-I'+str(R/'qa'),str(seed),'-o',str(seed_bin)],check=True)
 shared=Path('/dev/shm/SharedSettings');assert not shared.exists(),'Host shared settings already exist; refuse to touch them'
 try:
  subprocess.run([str(seed_bin),str(shared)],check=True);original=shared.read_bytes()
  p=subprocess.run([*base,'-E','NXT_NXREDUX_SETTINGS_LIB='+str(system/'lib/libmsettings.so'),'-E','USERDATA_PATH='+td,'-E','DEVICE=brickpro','-E','PATH='+str(bin_)+':/usr/bin:/bin',str(driver_bin)],capture_output=True,text=True,timeout=20,env=env)
  (R/'logs/nxredux-firmware-api-test.log').write_text(p.stdout+p.stderr);p.check_returncode();results['firmware_API']={'exit':0,'stdout':p.stdout[-1000:]}
  current=shared.read_bytes();assert len(current)==len(original) and current!=original
  import struct
  before=struct.unpack('<'+'i'*(len(original)//4),original);after=struct.unpack('<'+'i'*(len(current)//4),current)
  # Only the intended headphone volume and live sink slots may change.
  changed=[i for i,(a,b) in enumerate(zip(before,after)) if a!=b];assert changed==[3,len(before)-1],changed
  assert (w/'msettings.bin').read_bytes()==current
 finally:shared.unlink(missing_ok=True)
report={'exit':0,'actual_NXRedux_v200_firmware_library':True,'actual_native_frontend_loader':True,'binary_sha256':hashlib.file_digest((B/'musicplayer.elf').open('rb'),'sha256').hexdigest(),'firmware_library_sha256':hashlib.file_digest((system/'lib/libmsettings.so').open('rb'),'sha256').hexdigest(),'checks':results,'hardware_tested':False,'authenticated_services_tested':False}
(R/'logs/nxredux-runtime-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');print(json.dumps(report,ensure_ascii=False,indent=2))
