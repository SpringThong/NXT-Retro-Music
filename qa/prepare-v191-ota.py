#!/usr/bin/env python3
from pathlib import Path
R=Path(__file__).resolve().parents[1]
src=(R/'qa/test_v19_ota.py').read_text()
for rom,label,baseline in [('stock','Stock','1.8.9'),('nextui','NextUI','1.8.9'),('nxredux','NXRedux','1.9')]:
 s=src.replace('1.8.9.1','__BASELINE__').replace('1.9','1.9.1').replace('__BASELINE__',baseline).replace("ROM='nxredux'",f"ROM='{rom}'").replace('v19-','v191-').replace('v19_','v191_')
 (R/f'qa/test_v191_ota_{rom}.py').write_text(s)
