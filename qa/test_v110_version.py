from pathlib import Path
import subprocess,tempfile
R=Path(__file__).resolve().parents[1]
s=(R/'upstream/src/nxt_ota_package.c').read_text();start=s.index('static bool version(');end=s.index('bool NxtOTA_https',start)
code='#include <stdbool.h>\n#include <ctype.h>\n#include <assert.h>\n#include <stdio.h>\n'+s[start:end]+r'''
int main(void){assert(NxtOTA_compareVersion("1.10","1.9.7.7")>0);assert(NxtOTA_compareVersion("1.10","1.9.9.9")>0);assert(NxtOTA_compareVersion("1.10","1.10.0")==0);assert(NxtOTA_compareVersion("1.10","1.11")<0);assert(NxtOTA_compareVersion("1.10x","1.9")==-2);puts("PASS production OTA numeric comparison: 1.10 upgrades all 1.9.x releases");}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'version.c';p.write_text(code);exe=Path(d)/'version';subprocess.run(['gcc','-std=gnu99','-O2',str(p),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
