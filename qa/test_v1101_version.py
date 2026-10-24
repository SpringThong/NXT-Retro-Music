from pathlib import Path
import subprocess,tempfile
R=Path(__file__).resolve().parents[1]
s=(R/'upstream/src/nxt_ota_package.c').read_text();start=s.index('static bool version(');end=s.index('bool NxtOTA_https',start)
code='#include <stdbool.h>\n#include <ctype.h>\n#include <assert.h>\n#include <stdio.h>\n'+s[start:end]+r'''
int main(void){assert(NxtOTA_compareVersion("1.10.1","1.10")>0);assert(NxtOTA_compareVersion("1.10.1","1.10.0")>0);assert(NxtOTA_compareVersion("1.10.1","1.9.7.7")>0);assert(NxtOTA_compareVersion("1.10.1","1.10.1.0")==0);assert(NxtOTA_compareVersion("1.10.1","1.10.2")<0);assert(NxtOTA_compareVersion("1.10.1x","1.10")==-2);puts("PASS production OTA version comparison: 1.10.1 upgrades 1.10 and all 1.9.x releases");}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'version.c';p.write_text(code);exe=Path(d)/'version';subprocess.run(['gcc','-std=gnu99','-O2',str(p),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
