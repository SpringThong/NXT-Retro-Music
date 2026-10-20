#!/usr/bin/env python3
"""Independent binary goldens and actual production worker/policy, ASan/UBSan."""
from pathlib import Path
import subprocess,tempfile,struct,os,json,hashlib,random
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/nxredux-build/headers';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
ROM=os.environ.get('QA_ROM','NXREDUX').lower()
env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
def dsf(data,rate=2822400,lsb=True,block=4096):
 ch=len(data);n=len(data[0]);payload=b''
 for base in range(0,n,block):
  for stream in data:
   b=stream[base:base+block]
   if lsb:b=bytes(int(f'{x:08b}'[::-1],2) for x in b)
   payload+=b.ljust(block,b'\0')
 fmt=b'fmt '+struct.pack('<QIIIIIIQII',52,1,0,ch,ch,rate,1 if lsb else 8,n*8,block,0)
 return b'DSD '+struct.pack('<QQQ',28,28+len(fmt)+12+len(payload),0)+fmt+b'data'+struct.pack('<Q',12+len(payload))+payload
def chunk(id,data):return id+struct.pack('>Q',len(data))+data+(b'\0' if len(data)%2 else b'')
def dff(data,rate=2822400,dst=False):
 ch=len(data);prop=b'SND '+chunk(b'FS  ',struct.pack('>I',rate))+chunk(b'CHNL',struct.pack('>H',ch)+(b'SLFTSRGT' if ch==2 else b'C   '))+chunk(b'CMPR',(b'DST ' if dst else b'DSD ')+b'\0');payload=bytes(x for pair in zip(*data) for x in pair)
 body=b'DSD '+chunk(b'FVER',b'\x01\x05\0\0')+chunk(b'PROP',prop)+chunk(b'JUNK',b'x')+chunk(b'DST ' if dst else b'DSD ',payload)
 return b'FRM8'+struct.pack('>Q',len(body))+body
def golden(data,f):
 width=[1,2,4,2,4,3,4,4][f];stride=2 if f>=5 else width;out=bytearray()
 for i in range(0,len(data[0]),stride):
  marker=5 if i//stride%2==0 else 250
  for stream in data:
   b=stream[i:i+stride]
   if f<5:out+=b[::-1] if f in [1,2] else b
   elif f==5:out+=bytes([b[1],b[0],marker])
   elif f==6:out+=bytes([b[1],b[0],marker,0])
   else:out+=bytes([0,b[1],b[0],marker])
 return bytes(out)
def build(out,files,incs):
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+os.environ.get('QA_ROM','NXREDUX'),'-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*incs,*map(str,files),'-Wl,--gc-sections','-lpthread','-lm','-o',str(out)]
 p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr
def run(args):
 p=subprocess.run(args,env=env,capture_output=True,text=True,timeout=45);assert p.returncode==0,p.stdout+p.stderr;return p.stdout
with tempfile.TemporaryDirectory(prefix='nxt19-dsd-') as td:
 w=Path(td);driver=w/'reader.c';driver.write_text('''#include "nxt_dsd.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
int main(int n,char**v){char e[256];NxtDSD*d=NxtDSD_open(v[1],e);if(n==2){assert(!d);return 0;}assert(d);int f=atoi(v[2]);if(n>4)assert(NxtDSD_seek(d,atoi(v[4]),f)>=0);FILE*out=fopen(v[3],"wb");assert(out);unsigned char b[65536*8];assert(NxtDSD_read(d,b,0,f)==0);int count,k=0;while((count=NxtDSD_read(d,b,++k%3?7:3333,f))>0)assert(fwrite(b,NxtDSD_width(f)*NxtDSD_info(d)->channels,count,out)==(size_t)count);assert(count==0);fclose(out);NxtDSD_close(d);return 0;}''')
 reader=w/'reader';build(reader,[driver,S/'nxt_dsd.c'],['-I'+str(S)]);rng=random.Random(19);data=[bytes(rng.randrange(256) for _ in range(12288)) for _ in range(2)];cases=0
 for ch in [1,2]:
  for rate in [2822400,5644800,11289600,22579200,3072000,6144000,12288000,24576000]:
   for kind,blob in [('lsb.dsf',dsf(data[:ch],rate)),('msb.dsf',dsf(data[:ch],rate,False)),('raw.dff',dff(data[:ch],rate))]:
    file=w/kind;file.write_bytes(blob);out=w/'packed'
    for fmt in range(8):
     run([str(reader),str(file),str(fmt),str(out)]);assert out.read_bytes()==golden(data[:ch],fmt),(ch,rate,kind,fmt);cases+=1
    for fmt in [0,2,5,7]:
     stride=2 if fmt>=5 else [1,2,4,2,4][fmt];base=rate//8000;base-=base%stride;run([str(reader),str(file),str(fmt),str(out),'1']);assert out.read_bytes()==golden([d[base:] for d in data[:ch]],fmt);cases+=1
 for blob in [b'',dsf(data)[:-1],dff(data)[:-1],dsf(data,12345),dff(data,dst=True),dsf([data[0]]).replace(b'fmt ',b'NOPE',1),b'DSD '+b'\xff'*100]:
  p=w/'bad.dsf';p.write_bytes(blob);run([str(reader),str(p)])
 fifo=w/'fifo.dsf';os.mkfifo(fifo);run([str(reader),str(fifo)])
 for i in range(40):
  p=w/'bad.dsf';p.write_bytes((b'DSD ' if i%2 else b'FRM8')+rng.randbytes(rng.randrange(512)));run([str(reader),str(p)])
 hdr=w/'headers';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2');(hdr/'alsa').symlink_to(SDK/'usr/include/alsa');incs=['-I'+str(x) for x in [hdr,H,H/'utf8proc',S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
 worker=(R/'qa/v186_bit_worker.c').read_text().replace('int main(void){','int pcm_regression(void){').replace('static unsigned char bytes[65536]','static unsigned char bytes[262144]')
 worker=worker.replace('int width=p->format==SND_PCM_FORMAT_S16_LE?2:p->format==SND_PCM_FORMAT_S24_3LE?3:4;','int width=snd_pcm_format_physical_width(p->format)/8;')
 worker=worker.replace('static unsigned epoch=1;','static unsigned epoch=1;static int mock_observed_mismatch;').replace('*v=s->rate;','*v=s->rate+mock_observed_mismatch;')
 worker=worker.replace('if(busy)return -EBUSY;*out=', 'if(busy)return -EBUSY;if(strstr(name,",0")==NULL)return -ENOENT;*out=')
 worker+='''
#include "nxt_dsd.h"
int snd_pcm_format_physical_width(snd_pcm_format_t f){switch(f){case SND_PCM_FORMAT_DSD_U8:return 8;case SND_PCM_FORMAT_DSD_U16_LE:case SND_PCM_FORMAT_DSD_U16_BE:case SND_PCM_FORMAT_S16_LE:return 16;case SND_PCM_FORMAT_S24_3LE:return 24;default:return 32;}}
int main(int argc,char**argv){pcm_regression();assert(!NxtBit_init());NxtBit_route(3,true);const snd_pcm_format_t formats[]={SND_PCM_FORMAT_DSD_U8,SND_PCM_FORMAT_DSD_U16_LE,SND_PCM_FORMAT_DSD_U32_LE,SND_PCM_FORMAT_DSD_U16_BE,SND_PCM_FORMAT_DSD_U32_BE,SND_PCM_FORMAT_S24_3LE,SND_PCM_FORMAT_S24_LE,SND_PCM_FORMAT_S32_LE};
 for(int f=0;f<8;f++){reset_mock(24,2,formats[f]);int stride=f>=5?2:f==0?1:f==1||f==3?2:4;mock_rate=2822400/(stride*8);nxt.dsd_output=f>=5;assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PAUSED)&&!NxtBit_waitLoaded());assert(NxtBit_isDSD()&&NxtBit_ready());assert(strstr(NxtBit_status(),f>=5?"DoP":"Native DSD"));assert(byte_count==0);NxtBit_play();for(int t=0;t<1000&&NxtBit_state()==PLAYER_STATE_PLAYING;t++)usleep(2000);assert(NxtBit_state()==PLAYER_STATE_STOPPED);char path[2048];snprintf(path,sizeof(path),"%s/%d.bin",argv[2],f);FILE*p=fopen(path,"rb");assert(p);unsigned char wanted[262144];size_t n=fread(wanted,1,sizeof(wanted),p);fclose(p);assert(n==byte_count&&!memcmp(bytes,wanted,n));int16_t vis[2048];assert(NxtBit_vis(vis,2048)==0&&NxtBit_peak()==0);}

 reset_mock(24,2,SND_PCM_FORMAT_S24_3LE);mock_rate=176400;nxt.dsd_output=2;assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PAUSED)&&!NxtBit_waitLoaded());assert(NxtBit_ready()&&NxtBit_rate()==176400&&strstr(NxtBit_status(),"DoP"));assert(byte_count==0);NxtBit_play();for(int t=0;t<1000&&NxtBit_state()==PLAYER_STATE_PLAYING;t++)usleep(2000);assert(NxtBit_state()==PLAYER_STATE_STOPPED);char ap[2048];snprintf(ap,sizeof(ap),"%s/5.bin",argv[2]);FILE*af=fopen(ap,"rb");assert(af);unsigned char ag[262144];size_t an=fread(ag,1,sizeof(ag),af);fclose(af);assert(an==byte_count&&!memcmp(bytes,ag,an));
 reset_mock(24,2,SND_PCM_FORMAT_DSD_U8);mock_rate=352800;nxt.dsd_output=2;assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PAUSED)&&!NxtBit_waitLoaded());assert(NxtBit_ready()&&strstr(NxtBit_status(),"Native DSD")&&byte_count==0);
 reset_mock(24,2,SND_PCM_FORMAT_S24_3LE);nxt.dsd_output=0;assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PLAYING));assert(NxtBit_waitLoaded()==-1&&byte_count==0&&NxtBit_state()==PLAYER_STATE_PAUSED);assert(strstr(NxtBit_error(),"format failed")&&!strstr(NxtBit_error(),"unavailable at card"));
 reset_mock(24,2,SND_PCM_FORMAT_DSD_U8);mock_rate=352800;mock_bad_rate=1;assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PLAYING)&&NxtBit_waitLoaded()==-1&&byte_count==0);
 reset_mock(24,2,SND_PCM_FORMAT_DSD_U8);mock_rate=352800;mock_observed_mismatch=1;assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PLAYING)&&NxtBit_waitLoaded()==-1&&byte_count==0);mock_observed_mismatch=0;
 reset_mock(24,2,SND_PCM_FORMAT_DSD_U8);mock_rate=352800;NxtBit_route(-1,false);assert(!NxtBit_load(argv[1],0,PLAYER_STATE_PLAYING)&&NxtBit_waitLoaded()==-1&&mock_opens==0);NxtBit_route(3,true);assert(!NxtBit_waitLoaded());NxtBit_play();usleep(20000);NxtBit_route(-1,false);settled();int writes=mock_writes;settled();assert(mock_writes==writes&&!NxtBit_ready()&&NxtBit_state()==PLAYER_STATE_PAUSED);NxtBit_route(3,true);assert(!NxtBit_waitLoaded());NxtBit_pause();settled();NxtBit_seek(1);settled();assert(NxtBit_position()==1);NxtBit_setDSDMode(1);assert(NxtBit_waitLoaded()==-1);NxtBit_stop();NxtBit_quit();puts("PASS production DSD worker: eight byte-identical transports, partial writes/EAGAIN, paused silence, seek/EOF/removal, observed-rate mismatch and PCM-only Native rejection. PCM regression retained.");return 0;}
'''
 driver=w/'worker.c';driver.write_text(worker);bin_=w/'worker';build(bin_,[driver,S/'nxt_bitperfect.c',S/'nxt_dsd.c',S/'nxt_dsd_alsa.c'],incs);src=w/'worker.dsf';src.write_bytes(dsf(data));gold=w/'gold';gold.mkdir()
 for f in range(8):(gold/f'{f}.bin').write_bytes(golden(data,f))
 output=run([str(bin_),str(src),str(gold)]);(R/f'logs/v193-dsd-worker-{ROM}.log').write_text(output);print(output)
 policy=(R/'qa/v186_audio_policy.c').read_text().replace('int main(void){','int old_policy(void){')+'''
int main(void){old_policy();card=3;nxt.output=2;load_status=0;error[0]=0;ready=true;NxtAudio_setMode(0);strcpy(current,"local.dsf");strcpy(raw,current);NxtAudio_prepare(current);assert(nxt.bit_perfect);NxtAudio_tick();assert(nxt.bit_perfect);NxtAudio_volumeFallback();assert(nxt.bit_perfect);NxtAudio_setMode(2);ready=false;load_status=-1;strcpy(error,"Native DSD unavailable");int before=switches;for(int i=0;i<100;i++)NxtAudio_tick();assert(nxt.bit_perfect&&switches==before);card=-1;nxt.output=0;NxtAudio_routeChanged();NxtAudio_tick();assert(nxt.bit_perfect);NxtAudio_streaming();assert(!nxt.bit_perfect);NxtAudio_tick();assert(!nxt.bit_perfect);strcpy(current,"local.flac");strcpy(raw,current);NxtAudio_prepare(current);assert(!nxt.bit_perfect);card=3;nxt.output=2;NxtAudio_routeChanged();load_status=0;error[0]=0;ready=true;NxtAudio_tick();assert(nxt.bit_perfect);puts("PASS production DSD/PCM/online policy: no implicit DSD conversion, DAC removal keeps raw paused, online Normal and original PCM policies restored.");return 0;}
'''
 driver=w/'policy.c';driver.write_text(policy);bin_=w/'policy';build(bin_,[driver,S/'nxt_audio_mode.c',S/'nxt_dsd.c'],incs);output=run([str(bin_)]);(R/f'logs/v193-dsd-policy-{ROM}.log').write_text(output);print(output)
report={'exit':0,'ASan_UBSan':True,'parser_golden_cases':cases,'eight_transports_byte_equal':True,'DSF_LSB_MSB_DFF_mono_stereo_eight_rates':True,'DoP_markers_partial_reads_seek':True,'malformed_DST_truncated_nonregular_rejected':True,'production_worker_pause_seek_EOF_removal':True,'PCM_regression':True,'PCM_only_DAC_native_rejected':True,'exact_rate_and_observed_mismatch_rejected':True,'production_audio_policy':True,'explicit_auto_Native_then_DoP_byte_equal':True,'preserve_device0_format_error_when_later_devices_absent':True,'hardware_tested':False,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_dsd.c','nxt_dsd_alsa.c','nxt_bitperfect.c','nxt_audio_mode.c']}}
(R/f'logs/v193-dsd-test-{ROM}.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS',cases,'independent parser/packing cases; actual worker and policy')
