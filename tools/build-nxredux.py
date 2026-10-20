#!/usr/bin/env python3
"""Build the NX Redux frontend and its firmware-API adapter."""
import os,runpy,json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
if json.loads((root/'nxredux/config.json').read_text())['version'] != json.loads((root/'SOURCE.json').read_text())['version']:
    raise SystemExit('NX Redux config.json version must match SOURCE.json before building.')
os.environ["NXT_BUILD_ROM"]="NXT_ROM_NXREDUX"
runpy.run_path(str(Path(__file__).with_name("build-stock.py")),run_name="__main__")
