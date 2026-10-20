#!/usr/bin/env python3
from pathlib import Path
import json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';ROM=json.loads((R/'SOURCE.json').read_text())['build_define']
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-v170-core-') as td:
 w=Path(td)
 for p in ['data/profiles','proc','sys/devices/system/cpu/cpu0/cpufreq','sys/class/thermal/thermal_zone0','sys/class/thermal/thermal_zone1','sys/class/power_supply/battery']:(w/p).mkdir(parents=True,exist_ok=True)
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-D'+ROM,'-DNXT_MONITOR_PROC="'+str(w/'proc')+'"','-DNXT_MONITOR_SYS="'+str(w/'sys')+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined,float-cast-overflow',*incs,str(R/'qa/v170_core.c'),*[str(S/f) for f in ['nxt_config.c','nxt_eq.c','nxt_presets.c','nxt_pad.c','nxt_dsp.c','include/parson/parson.c']],'-Wl,--gc-sections','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(w/'test')]
 with (R/'logs/v170-core-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 p=subprocess.run([str(w/'test')],capture_output=True,text=True,timeout=20,env={**os.environ,'QA_ROOT':str(w),'TZ':'UTC','ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
 (R/'logs/v170-core-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v170-core-test.json').write_text(json.dumps({'exit':0,'production_config_migration_persistence':True,'kernel_sensor_files_and_SQLite_history':True,'five_band_DSP_response_and_PCM':True,'ASan_UBSan':True,'physical_hardware_tested':False},indent=2)+'\n')
