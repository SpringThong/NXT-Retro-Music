#!/usr/bin/env python3
"""Exercise the production cache/HTTP reader with deterministic 100 MiB pipes."""
from pathlib import Path
import os,subprocess,tempfile,json
R=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='nxt-drive-buffer-') as td:
 w=Path(td);(w/'data').mkdir();curl=w/'curl'
 curl.write_text('''#!/usr/bin/env python3
import sys,os,time
args=sys.argv[1:];start=int(args[args.index('--range')+1].split('-')[0]);size=100*1024*1024
with open(os.environ['QA_CURL_CALLS'],'a') as f:f.write(str(start)+'\\n')
os.write(2,('HTTP/1.1 206 Partial Content\\r\\nContent-Range: bytes %d-%d/%d\\r\\nContent-Length: %d\\r\\nAccept-Ranges: bytes\\r\\nContent-Type: audio/flac\\r\\n\\r\\n'%(start,size-1,size,size-start)).encode())
block=bytes(range(256))*256
try:
 for offset in range(start,size,len(block)):
  b=block[:min(len(block),size-offset)];n=0
  while n<len(b):n+=os.write(1,b[n:])
  time.sleep(.001)
except BrokenPipeError:pass
''');curl.chmod(0o755)
 c=w/'test.c';c.write_text(r'''
#include "nxt_stream.c"
#include <assert.h>
int userdata_snpath(const char *name,char *out,size_t n){return snprintf(out,n,"%s/%s",getenv("QA_DATA"),name);}
bool NxtDrive_url(const NxtDriveEntry *e,bool download,char *out,size_t n){snprintf(out,n,"https://drive.google.com/%s",e->id);return true;}
bool NxtDrive_confirmation(const char *html,const NxtDriveEntry *e,char *out,size_t n){return false;}
static int calls(void){FILE *f=fopen(getenv("QA_CURL_CALLS"),"r");if(!f)return 0;int n=0;char line[80];while(fgets(line,sizeof(line),f))n++;fclose(f);return n;}
static void read_bytes(NxtStream *s,size_t total){unsigned char b[65536];while(total){int64_t offset=NxtStream_position(s);size_t want=total<sizeof(b)?total:sizeof(b);int n=NxtStream_read(s,b,want);assert(n>0);for(int i=0;i<n;i++)assert(b[i]==(unsigned char)(offset+i));total-=n;}}
int main(void){
 NxtDriveEntry a={.id="first-file",.name="first.flac"},b={.id="next-file",.name="next.flac"},c={.id="third-file",.name="third.flac"};
 unsigned epoch=NxtStream_epoch();int64_t begin=millis();NxtStream *s=NxtStream_open(&a,epoch);assert(s&&NxtStream_size(s)==100LL*1024*1024&&NxtStream_seekable(s));
 read_bytes(s,3*1024*1024);assert(millis()-begin<6000);assert(s->cache->complete<s->cache->page_count);
 int before=calls();assert(NxtStream_seek(s,65536)==65536);read_bytes(s,65536);assert(calls()==before);
 assert(NxtStream_seek(s,90LL*1024*1024)>0);read_bytes(s,65536);assert(calls()>before);
 assert(NxtStream_seek(s,0)==0);read_bytes(s,65536);
 begin=millis();while(!NxtStream_prefetched(&b)&&millis()-begin<20000){NxtStream_prefetch(&b);usleep(20000);}assert(NxtStream_prefetched(&b));
 NxtStream_cancelAll();unsigned char byte;assert(NxtStream_read(s,&byte,1)==-ECANCELED);NxtStream_close(s);
 before=calls();s=NxtStream_open(&b,NxtStream_epoch());assert(s);assert(NxtStream_seek(s,98LL*1024*1024)>0);read_bytes(s,65536);assert(calls()==before);NxtStream_close(s);
 s=NxtStream_open(&c,NxtStream_epoch());assert(s);read_bytes(s,65536);NxtStream_close(s);NxtStream_cleanup();for(int i=0;i<CACHE_SLOTS;i++)assert(!pool[i]);
 puts("PASS production 100 MiB progressive Drive cache: starts before full download, cached seeks reuse bytes, missing seeks request priority Range, next-file prefetch survives reader cancellation and is reusable without a new request, two-slot eviction and cleanup. Network rate is a fixture.");return 0;
}
''')
 exe=w/'test';cmd=['gcc','-O1','-g','-D_GNU_SOURCE','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(R/'upstream/src'),str(c),'-Wl,--gc-sections','-pthread','-o',str(exe)]
 subprocess.run(cmd,check=True)
 result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=45,env={**os.environ,'PATH':str(w)+':'+os.environ['PATH'],'QA_DATA':str(w/'data'),'QA_CURL_CALLS':str(w/'calls'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
 (R/'logs/v13-drive-buffer-test.log').write_text(result.stdout+result.stderr);print(result.stdout+result.stderr,end='');result.check_returncode()
 assert not list((w/'data/drive-buffer').iterdir()),'Temporary buffer names leaked'
(R/'logs/v13-drive-buffer-test.json').write_text(json.dumps({'progressive_buffer':{'exit':0,'file_bytes':100*1024*1024,'cached_seek':True,'priority_range_seek':True,'warm_next':True,'cancellation_eviction_cleanup':True,'asan_ubsan':True,'live_google_drive_tested':False}},indent=2)+'\n')
