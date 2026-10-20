#!/usr/bin/env python3
import runpy
from pathlib import Path
runpy.run_path(str(Path(__file__).with_name("package-source.py")),run_name="__main__")
