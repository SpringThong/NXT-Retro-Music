from pathlib import Path
import tempfile,subprocess,os,json,hashlib
r=Path(__file__).resolve().parents[1];s=r/'upstream/src'
with tempfile.TemporaryDirectory(prefix='nxt-dac-label-') as td:
 w=Path(td);c=w/'case.c';c.write_text(r'''
#include "nxt_dac_name.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int n,char**v){char out[128];NxtDACIdentity a={"1234","5678","TE-C","USB Audio"},b={"1234","5679","TE-C","USB Audio"},other={"1234","5678","Other","USB Audio"};
 NxtDACName_read(v[1],&a,out,sizeof(out));assert(!*out);
 assert(NxtDACName_save(v[1],&a,"TRN Black Pearl"));NxtDACName_read(v[1],&a,out,sizeof(out));assert(!strcmp(out,"TRN Black Pearl"));
 NxtDACName_read(v[1],&b,out,sizeof(out));assert(!*out);NxtDACName_read(v[1],&other,out,sizeof(out));assert(!*out);
 assert(NxtDACName_save(v[1],&b,"DAC thứ hai"));NxtDACName_read(v[1],&b,out,sizeof(out));assert(!strcmp(out,"DAC thứ hai"));
 assert(!NxtDACName_save(v[1],&a,"bad\nlabel"));char longname[120];memset(longname,'x',119);longname[119]=0;assert(!NxtDACName_save(v[1],&a,longname));
 NxtDACName_read(v[1],&a,out,sizeof(out));assert(!strcmp(out,"TRN Black Pearl"));
 assert(NxtDACName_save(v[1],&a,""));NxtDACName_read(v[1],&a,out,sizeof(out));assert(!*out);NxtDACName_read(v[1],&b,out,sizeof(out));assert(!strcmp(out,"DAC thứ hai"));
 for(int k=0;k<40;k++){char pid[20],label[20];snprintf(pid,sizeof(pid),"%04x",k);snprintf(label,sizeof(label),"DAC%d",k);NxtDACIdentity x={"1111",pid,"OEM","Device"};assert(NxtDACName_save(v[1],&x,label));}
 NxtDACName_read(v[1],&b,out,sizeof(out));assert(!*out);NxtDACIdentity newest={"1111","0027","OEM","Device"};NxtDACName_read(v[1],&newest,out,sizeof(out));assert(!strcmp(out,"DAC39"));
 puts("PASS USB-identity-specific user labels, no TE-C model inference, persistence, Unicode, reset, invalid labels, bounded 32-device storage");return 0;}
''')
 b=w/'test';cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-fsanitize=address,undefined','-I'+str(s),'-I'+str(s/'include'),str(c),str(s/'nxt_dac_name.c'),str(s/'include/parson/parson.c'),'-o',str(b)]
 subprocess.run(cmd,check=True)
 p=subprocess.run([str(b),str(w/'labels.json')],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1'},timeout=10);print(p.stdout+p.stderr);p.check_returncode();assert len(json.loads((w/'labels.json').read_text()))==32
 report={'exit':0,'ASan_UBSan':True,'user_labels_scoped_to_USB_tuple':True,'no_brand_or_DSD_capability_inference':True,'bounded_32_entries':True,'reset_Unicode_validation':True,'sources':{n:hashlib.sha256((s/n).read_bytes()).hexdigest() for n in ['nxt_dac_name.c','nxt_dac_name.h']}}
 (r/'logs/v192-dac-name.json').write_text(json.dumps(report,indent=2)+'\n')
