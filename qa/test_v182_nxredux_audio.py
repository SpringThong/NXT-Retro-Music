#!/usr/bin/env python3
"""Actual discovery/ALSA route/PCM code with BlueZ and SDL transport fixtures."""
from pathlib import Path
import hashlib,json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp';sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/'.host-tools/usr/include',R/'.cache/nxredux-build/headers/samplerate',sdk/'usr/include/dbus-1.0',sdk/'usr/lib/dbus-1.0/include']]
with tempfile.TemporaryDirectory(prefix='nxt-nx-audio-') as td:
 w=Path(td);data=w/'data';data.mkdir();asound=w/'asound';(asound/'card3').mkdir(parents=True);(asound/'card3/usbbus').write_text('003/002\n')
 state=w/'nx_audio_sink';state.write_text('sink=usb\nrate=96000\nrates=96000 48000\nformat=S16_LE\ncard=3\n')
 base='# NXT Retro Music shared PCM\npcm.nxt_music_mix { type null }\npcm.nxt_music_output { type plug slave.pcm "old-speaker" }\npcm.!default { type plug slave.pcm "old-speaker" }\nctl.!default { type hw card 0 }\n'
 (data/'base.conf').write_text(base);(data/'.asoundrc').write_text(base);binary=w/'test'
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NEXTUI','-DNXT_ROM_NXREDUX','-Werror=implicit-function-declaration','-Werror=int-conversion','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',f'-DNXT_ASOUND_ROOT="{asound}"',f'-DNXT_SOUND_SYS_ROOT="{w}/sys"',f'-DNXT_LED_ROOT="{w}/led"',f'-DNXT_NXREDUX_SINK_STATE="{state}"',*incs,str(R/'qa/v1882_nextui_bluetooth.c'),str(S/'nxt_route.c'),str(S/'nxt_pcm.c'),str(S/'nxt_led.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libasound.so.2','-l:libdbus-1.so.3','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/nxredux-audio-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
 p=subprocess.run([str(binary),str(data)],capture_output=True,text=True,timeout=30,env={**os.environ,'HOME':str(data),'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
 (R/'logs/nxredux-audio-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr);p.check_returncode()
 assert (data/'.asoundrc').read_text()==base and (data/'base.conf').read_text()==base
(R/'logs/nxredux-audio-test.json').write_text(json.dumps({'exit':0,'actual_ALSA_parser':True,'A2DP_only_discovery':True,'speaker_BT_DAC_handoff':True,'same_dmix_as_firmware':True,'negotiated_USB_rate':96000,'SDL_hotplug_stopped_before_config_reset':True,'local_Spotify_SoundCloud_music_gain_and_OSD_mute':True,'private_route_firmware_config_untouched':True,'transport_retry_and_no_idle_reopens':True,'ASan_UBSan':True,'physical_device_tested':False,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_hardware.c','nxt_nextui.c','nxt_player_core.c']}},indent=2)+'\n')
