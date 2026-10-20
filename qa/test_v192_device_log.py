from pathlib import Path
import tempfile,subprocess,os,json,hashlib
r=Path(__file__).resolve().parents[1];s=r/'upstream/src';sdk=r/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';h=r/'.cache/nxredux-build/headers'
ROM=os.environ.get('QA_ROM','nxredux').lower()
with tempfile.TemporaryDirectory(prefix='nxt-usb-log-') as td:
 w=Path(td);proc=w/'proc';sys=w/'sound';dev=w/'usb-device';dev.mkdir();(proc/'card1/pcm0p/sub0').mkdir(parents=True);(sys/'card1').mkdir(parents=True)
 for k,v in {'idVendor':'1234','idProduct':'5678','manufacturer':'TE-C','product':'USB Audio Device'}.items():(dev/k).write_text(v)
 (sys/'card1/device').symlink_to(dev,target_is_directory=True)
 for k,v in {'usbbus':'001/002','usbid':'1234:5678','id':'Device','stream0':'TE-C USB Audio Device\nPlayback:\n Format: S32_LE\n Rates: 44100, 176400, 352800\n'}.items():(proc/'card1'/k).write_text(v)
 (proc/'card1/pcm0p/sub0/hw_params').write_text('closed\n')
 c=w/'case.c';c.write_text(r'''
#include "nxt_hardware.c"
#include <assert.h>
NxtConfig nxt;
Uint32 SDL_GetTicks(void){return 100;}
int GetAudioSink(void){return 2;}int GetVolume(void){return 13;}
bool Player_outputReady(void){return false;}bool Player_outputRetryPending(void){return false;}bool Player_isBluetoothActive(void){return false;}
const char *Player_lastOutputError(void){return "Native format unavailable";}
int Player_getOutputRate(void){return 0;}int Player_getOutputBits(void){return 0;}
unsigned NxtLED_rgb(void){return 0;}
bool NxtBit_isDSD(void){return true;}const char*NxtBit_status(void){return "DSD output not ready";}const char*NxtBit_error(void){return "format failed";}
unsigned NxtBit_snapshot(TrackInfo*i,char*p,size_t n){memset(i,0,sizeof(*i));strcpy(i->codec,"DFF DSD64");i->sample_rate=2822400;i->channels=2;i->duration_ms=120000;if(n)*p=0;return 1;}
int userdata_snpath(const char*rel,char*out,size_t n){return snprintf(out,n,"%s",rel);}
const char*Nxt_text(const char*a,const char*b){return a;}
int main(void){identify();assert(usb_card==1&&!strcmp(manufacturer,"TE-C")&&!strcmp(vid,"1234")&&!strcmp(product,"USB Audio Device"));assert(!*NxtHardware_dacName());assert(NxtHardware_setDACName("TRN Black Pearl"));assert(!strcmp(NxtHardware_dacName(),"TRN Black Pearl"));identify();assert(!strcmp(NxtHardware_dacName(),"TRN Black Pearl"));assert(NxtHardware_writeDiagnostic());return 0;}
''')
 hdr=w/'include';hdr.mkdir();(hdr/'SDL2').symlink_to(sdk/'usr/include/SDL2')
 incs=[hdr,h,h/'utf8proc',s,s/'include',r/'upstream/NextUI/workspace/all/common',r/'upstream/NextUI/workspace/tg5040/platform',r/'upstream/NextUI/workspace/tg5040/libmsettings']
 b=w/'test';cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+ROM.upper(),'-DPLATFORM_DESKTOP','-DNXT_ASOUND_ROOT="'+str(proc)+'"','-DNXT_SOUND_SYS_ROOT="'+str(sys)+'"','-DNXT_LED_ROOT="'+str(w/'led')+'"','-fsanitize=address,undefined','-ffunction-sections','-fdata-sections',*['-I'+str(i) for i in incs],str(c),str(s/'nxt_dac_name.c'),str(s/'include/parson/parson.c'),'-Wl,--gc-sections','-lm','-o',str(b)]
 p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr
 p=subprocess.run([str(b)],cwd=w,capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1'},timeout=15);assert p.returncode==0,p.stdout+p.stderr
 text=(w/'NXT_Device_Log.txt').read_text()
 for v in ['DAC user label: TRN Black Pearl','USB descriptor manufacturer: TE-C','USB VID:PID: 1234:5678','Kernel: Linux','DFF DSD64','2822400','Format: S32_LE','352800','hw_params:\nclosed']:assert v in text,v
 assert not list(w.glob('NXT_Device_Log.txt.tmp.*'))
 report={'exit':0,'ASan_UBSan':True,'actual_sysfs_identity_walk':True,'USB_descriptor_not_assumed_retail_brand':True,'persistent_label':True,'DSD_source_USB_streams_hw_params_kernel_in_atomic_export':True,'hardware_fixture_not_physical':True,'hardware_source_sha256':hashlib.sha256((s/'nxt_hardware.c').read_bytes()).hexdigest()}
 (r/f'logs/v193-device-log-{ROM}.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS actual hardware identification and atomic diagnostic exporter with fixture USB tree')
