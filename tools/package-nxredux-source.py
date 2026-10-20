#!/usr/bin/env python3
"""Compatibility alias: v1.9.3 ships one source archive for all ROMs."""
import runpy
from pathlib import Path
runpy.run_path(str(Path(__file__).with_name('package-source.py')),run_name='__main__')
