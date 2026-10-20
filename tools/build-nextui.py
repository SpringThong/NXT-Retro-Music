import os,runpy
from pathlib import Path
os.environ["NXT_BUILD_ROM"]="NXT_ROM_NEXTUI"
runpy.run_path(str(Path(__file__).with_name("build-stock.py")),run_name="__main__")
