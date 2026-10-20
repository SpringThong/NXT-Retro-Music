from pathlib import Path
import os,subprocess,sys,shutil
r=Path(__file__).resolve().parents[1]
for rom in ['stock','nextui','nxredux']:
 env={**os.environ,'NXT_BUILD_ROM':'NXT_ROM_'+rom.upper(),'QA_ROM':rom}
 subprocess.run([sys.executable,'-c','import runpy; a=runpy.run_path("tools/build-stock.py"); a["prepare_headers"]()'],env=env,check=True)
 for test in ['qa/test_v194_library_flow.py','qa/test_v194_dsd.py']:
  subprocess.run([sys.executable,test],env=env,check=True)
