from pathlib import Path
import os,subprocess,sys
r=Path(__file__).resolve().parents[1]
for rom in ['stock','nextui','nxredux']:
 env={**os.environ,'NXT_BUILD_ROM':'NXT_ROM_'+rom.upper(),'QA_ROM':rom}
 for test in ['qa/test_v195_library_flow.py','qa/test_v195_dsd.py','qa/test_v195_ota.py']:
  subprocess.run([sys.executable,test],env=env,check=True)
