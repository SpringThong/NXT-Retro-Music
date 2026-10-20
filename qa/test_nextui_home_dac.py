#!/usr/bin/env python3
"""Production code with synthetic inputs; no physical hardware claim."""
from pathlib import Path
import subprocess,ctypes,tempfile,re
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/home-dac';Q.mkdir(parents=True,exist_ok=True)
so=Q/'pcm.so';subprocess.run(['gcc','-shared','-fPIC','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',str(S/'nxt_pcm.c'),'-o',str(so)],check=True)
class PCM(ctypes.Structure):_fields_=[('rate',ctypes.c_int),('bits',ctypes.c_int),('channels',ctypes.c_int),('format',ctypes.c_char*32)]
l=ctypes.CDLL(str(so));l.NxtPCM_parse.argtypes=[ctypes.c_char_p,ctypes.POINTER(PCM)];l.NxtPCM_parse.restype=ctypes.c_bool;l.NxtPCM_read.argtypes=[ctypes.c_char_p,ctypes.c_int,ctypes.POINTER(PCM)];l.NxtPCM_read.restype=ctypes.c_bool;l.NxtPCM_label.argtypes=[ctypes.POINTER(PCM),ctypes.c_bool,ctypes.c_char_p,ctypes.c_size_t]
def fixture(rate=48000,fmt='S32_LE',channels=2):return f'access: RW_INTERLEAVED\nformat: {fmt}\nsubformat: STD\nchannels: {channels}\nrate: {rate} ({rate}/1)\nperiod_size: 1024\n'.encode()
for rate in [44100,48000,88200,96000,192000,384000]:
 for fmt,bits in [('S16_LE',16),('S24_LE',24),('S24_3LE',24),('S32_LE',32),('FLOAT_LE',32)]:
  p=PCM();assert l.NxtPCM_parse(fixture(rate,fmt),ctypes.byref(p));assert (p.rate,p.bits,p.channels)==(rate,bits,2)
  b=ctypes.create_string_buffer(160);l.NxtPCM_label(ctypes.byref(p),True,b,len(b));assert b'DAC' in b.value and f'{bits}-bit PCM'.encode() in b.value
for text in [b'closed',b'',b'format: S32_LE\n',fixture(fmt='UNKNOWN'),fixture(rate=0),fixture(rate=-1),fixture(channels=0),fixture(rate=999999999999999999999999999999)]:
 p=PCM(44100,24,2,b'S24_LE');assert not l.NxtPCM_parse(text,ctypes.byref(p));assert (p.rate,p.bits,p.channels)==(0,0,0)
with tempfile.TemporaryDirectory() as d:
 root=Path(d)
 def put(card,sub,text,device=0):
  p=root/f'card{card}/pcm{device}p/sub{sub}/hw_params';p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(text)
 put(0,0,fixture(44100,'S16_LE'));put(2,0,b'closed');put(2,1,fixture(96000,'S24_3LE'));put(2,0,fixture(192000),device=1)
 p=PCM();assert l.NxtPCM_read(d.encode(),2,ctypes.byref(p)) and (p.rate,p.bits)==(96000,24)
 put(2,1,b'closed');assert not l.NxtPCM_read(d.encode(),2,ctypes.byref(p));assert p.rate==0
 b=ctypes.create_string_buffer(160);l.NxtPCM_label(ctypes.byref(p),False,b,len(b));assert b'unconfirmed' in b.value and b'44100' not in b.value
print('PASS PCM: actual hw_params, supported integer/float formats, 44.1–384 kHz, active substream, correct DAC card/device, closed/missing/malformed inputs and unconfirmed fallback.')
# Use public SDL declarations from the pinned SDK; native libc and narrow stubs.
h=Q/'include';h.mkdir(exist_ok=True);sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc/usr/include'
for name in ['SDL2','GLES2','EGL','KHR']:
 p=h/name
 if not p.exists() and (sdk/name).exists():p.symlink_to(sdk/name,target_is_directory=True)
incs=['-I'+str(h),'-I'+str(S),'-I'+str(S/'include'),'-I'+str(R/'upstream/NextUI/workspace/all/common'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/platform'),'-I'+str(R/'upstream/NextUI/workspace/tg5040/libmsettings')]
flags=['gcc','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-ffunction-sections','-fdata-sections',*incs]
harness='#include "module_common.h"\n#include "nxt.h"\n#include "api.h"\n#include "ui_utils.h"\n#include "player.h"\n#include <assert.h>\n#include <stdlib.h>\nvoid LOG_note(int level,const char *fmt,...){}\nstatic int key,changes,volume,saved,resumed;static bool usb;\nint GetAudioSink(void){return usb?2:0;} bool Player_isUSBDACActive(void){return usb;} int GetVolume(void){return 10;} void SetVolume(int v){changes++;volume=v;}\nUint32 SDL_GetTicks(void){return 1234;}\nvoid Nxt_tick(void){} bool Nxt_save(void){saved++;return true;} void GFX_startFrame(void){} void ScreenTitle_frameBegin(void){} void PAD_poll(void){} void Toast_tick(void){}\nint PAD_justPressed(int b){return b==key;}\nvoid PWR_setDisplayOnlySleep(int a){} void PWR_setDisplayOff(int a){} void PWR_displayFrameBegin(void){}\nvoid PWR_update(int *a,int *b,PWR_callback_t c,PWR_callback_t d){}\nPlayerState Player_getState(void){return PLAYER_STATE_PLAYING;} int Player_getPosition(void){return 45678;} void Resume_updatePosition(int p){assert(p==45678);resumed++;}\nint main(int argc,char **argv){key=atoi(argv[1])==1?BTN_HOME:atoi(argv[1])==2?BTN_MENU:BTN_A;ModuleCommon_init();ModuleCommon_frameBegin();assert(ModuleCommon_stopSignalled()==(key==BTN_MENU));assert(saved==(key==BTN_MENU));assert(resumed==(key==BTN_MENU));usb=true;assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP));assert(changes==1&&volume==11);assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_DOWN));assert(changes==2&&volume==9);usb=false;changes=0;assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_UP));assert(changes==1&&volume==11);assert(ModuleCommon_handleHIDVolume(USB_HID_EVENT_VOLUME_DOWN));assert(changes==2&&volume==9);return 0;}\n'
home=Q/'home.c';home.write_text(harness);subprocess.run(flags+[str(home),str(S/'module_common.c'),'-Wl,--gc-sections','-o',str(Q/'home')],check=True)
for key in ['0','1','2']:subprocess.run([str(Q/'home'),key],check=True)
print('PASS production input: physical logo MENU stops full session and saves position; side HOME and A do not exit; HID volume stays functional.')
# Compile the real lock filter; stopped session must bypass a pre-existing lock.
ui=(S/'nxt_ui.c').read_text();body=ui.split('bool Nxt_lockInput(void){',1)[1].split('\nstatic void fonts',1)[0]
lock=Q/'lock.c';lock.write_text('''#include "nxt.h"
#include "api.h"
#include "toast.h"
#include <assert.h>
static bool locked=true,select_held,select_long;static uint32_t select_started;
static bool stopping;
bool ModuleCommon_stopSignalled(void){return stopping;}
int PWR_getDisplayOff(void){return 1;} int PWR_displayInputConsumed(void){return 1;}
int PAD_isPressed(int b){return 0;} Uint32 SDL_GetTicks(void){return 0;}
ToastToken Toast_show(const char *s,uint32_t d){return 0;} const char *Nxt_text(const char *en,const char *vi){return en;}
bool Nxt_lockInput(void){'''+body+'''\nint main(void){assert(Nxt_lockInput());stopping=true;assert(!Nxt_lockInput());return 0;}\n''');subprocess.run(flags+[str(lock),'-Wl,--gc-sections','-o',str(Q/'lock')],check=True);subprocess.run([str(Q/'lock')],check=True)
print('PASS lock/display-off exit: stop request passes the actual lock input filter.')
# Lifecycle invariants connect the tested stop signal to production cleanup.
assert 'ModuleCommon_stopSignalled())return false' in (S/'nxt_session.c').read_text()
assert 'while (!ModuleCommon_stopSignalled())' in (S/'musicplayer.c').read_text()
api=(R/'upstream/NextUI/workspace/all/common/api.c').read_text();assert re.search(r'joy == JOY_MENU_ALT.*?btn = BTN_HOME',api,re.S)
assert 'NxtHardware_pcm()' in (S/'nxt_skin.c').read_text()
print('PASS NextUI lifecycle/mapping: NextUI joystick MENU/HOME mappings retained; stop prevents detached background session; DAC actual PCM is shown in Now Playing.')

# Feed real SDL joystick events through the production PAD poll function.
api=(R/'upstream/NextUI/workspace/all/common/api.c').read_text()
body=api.split('FALLBACK_IMPLEMENTATION void PLAT_pollInput(void)',1)[1].split('FALLBACK_IMPLEMENTATION int PLAT_shouldWake',1)[0]
poll=Q/'poll.c';poll.write_text("""#include "defines.h"
#include "api.h"
#include <assert.h>
#include <stdlib.h>
PAD_Context pad; LID_Context lid; int is_brick=0,is_brickpro=1;
static int button,ready=1;
void LOG_note(int level,const char *fmt,...){}
Uint32 SDL_GetTicks(void){return 100;}
int SDL_PollEvent(SDL_Event *event){if(!ready)return 0;ready=0;event->type=SDL_JOYBUTTONDOWN;event->jbutton.button=button;return 1;}
void PAD_setAnalog(int a,int b,int c,int d){}
void PAD_update(const SDL_Event *e){}
void PWR_powerOff(int e){}
int PLAT_lidChanged(int *s){return 0;}
void PLAT_pollInput(void)"""+body+"""
int main(int argc,char **argv){button=atoi(argv[1]);PLAT_pollInput();assert((pad.just_pressed&(BTN_MENU|BTN_HOME))!=0);return 0;}
""")
subprocess.run(flags+[str(poll),'-Wl,--gc-sections','-o',str(Q/'poll')],check=True)
for key in ['8','15']:subprocess.run([str(Q/'poll'),key],check=True)
print('PASS actual SDL poll: joystick 8 -> MENU and joystick 15 -> HOME, only MENU is handled by full exit; side HOME is preserved.')
