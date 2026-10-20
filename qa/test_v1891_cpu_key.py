from pathlib import Path
import subprocess,tempfile,json,os
R=Path(__file__).resolve().parents[1];S=R/'upstream/src'
os.environ['ASAN_OPTIONS']='detect_leaks=0:halt_on_error=1'
with tempfile.TemporaryDirectory(prefix='nxt-1891-cpu-key-') as td:
 w=Path(td);cpu=w/'cpu';cpu.mkdir()
 fixture=(R/'qa/v1882_cpu.c').read_text().replace('int main(void){baseline();','''int main(void){baseline();
 put("scaling_governor","schedutil");put("scaling_available_governors","schedutil userspace");put("scaling_min_freq","408000");put("scaling_max_freq","600000");
 assert(NxtCPU_apply(2));assert(get("scaling_max_freq")==2000000);NxtCPU_release();assert(get("scaling_max_freq")==600000);
 baseline();''')
 c=w/'cpu.c';c.write_text(fixture);b=w/'cpu-test'
 subprocess.run(['gcc','-no-pie','-O1','-g','-fsanitize=address,undefined','-DNXT_ROM_NXREDUX','-DNXT_CPU_TEST','-I'+str(S),'-DNXT_CPU_ROOT="'+str(cpu)+'"',str(c),str(S/'nxt_cpu.c'),'-o',str(b)],check=True)
 subprocess.run([str(b)],check=True)
 c=w/'key.c';c.write_text('''#include "nxt_keyfile.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
int main(int n,char **v){char key[256];memset(key,0x55,sizeof(key));int r=NxtKeyFile_readGroq(v[1],key,sizeof(key));assert(r==atoi(v[2]));if(!r)assert(!strcmp(key,"gsk_FAKE_fixture_only_1891"));else for(int i=0;i<256;i++)assert(!key[i]);return 0;}''')
 b=w/'key-test';subprocess.run(['gcc','-no-pie','-O1','-g','-fsanitize=address,undefined','-I'+str(S),str(c),str(S/'nxt_keyfile.c'),'-o',str(b)],check=True)
 key='gsk_FAKE_fixture_only_1891';cases=[(key.encode(),0),(('\ufeff'+key+'\r\n').encode(),0),(key.encode('utf-16'),0),(b'\xfe\xff'+key.encode('utf-16-be'),0),(('# key\nGROQ_API_KEY="'+key+'"\r\n').encode(),0),(b'',2),(b'not a key',2),((key+'\n'+key).encode(),2),(('gsk_'+'x'*260).encode(),2),(b'x'*4097,2),(b'gsk_FAKE\x00fixture',2),(b'\xff\xfeb\x00x',2)]
 for i,(data,expected) in enumerate(cases):
  p=w/('key-'+str(i)+'.txt');p.write_bytes(data);subprocess.run([str(b),str(p),str(expected)],check=True);assert p.read_bytes()==data
 subprocess.run([str(b),str(w/'missing.txt'),'1'],check=True)
 p=w/'symlink.txt';p.symlink_to(w/'key-0.txt');subprocess.run([str(b),str(p),'2'],check=True)
 p=w/'fifo.txt';os.mkfifo(p);subprocess.run([str(b),str(p),'2'],check=True,timeout=2)
report={'exit':0,'ASan_UBSan':True,'NX_CPU_three_modes':True,'launcher_cap_not_hardware_cap':True,'dynamic_governor':True,'restore_and_external_policy':True,'Groq_TXT_UTF8_BOM_CRLF_UTF16':True,'invalid_ambiguous_and_nonregular_input_rejected':True,'source_TXT_retained':True,'physical_hardware_tested':False}
(R/'logs/v1891-cpu-key-test.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS CPU modes and Groq TXT fixtures.')
