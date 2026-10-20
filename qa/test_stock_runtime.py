#!/usr/bin/env python3
from pathlib import Path
import os
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="nxt-stock-runtime-") as directory:
    tmp=Path(directory)
    inc=tmp/"include"; inc.mkdir()
    (tmp/"proc").mkdir()
    (inc/"defines.h").write_text('#define RES_PATH "/unused"\n#define USERDATA_PATH "/unused"\n')
    (inc/"msettings.h").write_text('int InitializedSettings(void); int GetJack(void); void SetJack(int);\n')
    sdk=ROOT/".nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include"
    (inc/"SDL2").symlink_to(sdk/"SDL2",target_is_directory=True)
    executable=tmp/"runtime"
    args=["gcc","-O2","-D_GNU_SOURCE","-DNXT_ROM_STOCK",
          '-DNXT_STOCK_TMP_PATH="'+str(tmp)+'"',
          '-DNXT_STOCK_PROC_PATH="'+str(tmp/"proc")+'"',
          "-ffunction-sections","-fdata-sections","-I"+str(inc),
          "-I"+str(ROOT/"upstream/src"),
          "-Werror=implicit-function-declaration",
          str(ROOT/"qa/stock_runtime.c"),str(ROOT/"upstream/src/nxt_stock.c"),
          "-Wl,--gc-sections","-o",str(executable)]
    subprocess.run(args,check=True)
    env=os.environ.copy()
    env.update(QA_STOCK_TMP=str(tmp),NXT_STOCK_RUNTIME_DIR=str(tmp/"runtime-state"))
    result=subprocess.run([str(executable)],env=env,timeout=10,text=True)
    if result.returncode:
        raise RuntimeError(str(result.returncode))
