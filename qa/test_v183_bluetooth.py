#!/usr/bin/env python3
"""Real NextUI C helpers and ALSA parser; explicit SDL/audio/BlueZ fixtures."""
from pathlib import Path
import hashlib,json,os,subprocess,tempfile,zipfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';Q=R/'.cache/render-sp'
release_paused=json.loads((R/'SOURCE.json').read_text()).get('release_build_paused_by_user',False)
sdk=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc'
incs=(Q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings',R/'.host-tools/usr/include',R/'.cache/nextui18-build/headers/samplerate',sdk/'usr/include/dbus-1.0',sdk/'usr/lib/dbus-1.0/include']]
with tempfile.TemporaryDirectory(prefix='nxt-v183-nextui-bt-') as td:
 w=Path(td);data=w/'data';data.mkdir();asound=w/'asound';(asound/'card3').mkdir(parents=True);(asound/'card3/usbbus').write_text('003/002\n')
 base='# NXT Retro Music shared PCM\npcm.nxt_music_mix { type null }\npcm.nxt_music_output { type plug slave.pcm "old-speaker" }\npcm.!default { type plug slave.pcm "old-speaker" }\nctl.!default { type hw card 0 }\n'
 (data/'base.conf').write_text(base);(data/'.asoundrc').write_text(base);binary=w/'test'
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_NEXTUI','-Werror=implicit-function-declaration','-Werror=int-conversion','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',f'-DNXT_ASOUND_ROOT="{asound}"',f'-DNXT_SOUND_SYS_ROOT="{w}/sys"',f'-DNXT_LED_ROOT="{w}/led"',*incs,str(R/'qa/v182_nextui_bluetooth.c'),str(S/'nxt_route.c'),str(S/'nxt_pcm.c'),str(S/'nxt_led.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-L'+str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'-l:libasound.so.2','-l:libdbus-1.so.3','-lpthread','-lm','-o',str(binary)]
 with (R/'logs/v183-nextui-bluetooth-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 env={**os.environ,'HOME':str(data),'LD_LIBRARY_PATH':str(R/'.host-tools/usr/lib/x86_64-linux-gnu'),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
 p=subprocess.run([str(binary),str(data)],env=env,capture_output=True,text=True,timeout=30)
 (R/'logs/v183-nextui-bluetooth-test.log').write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode();assert (data/'base.conf').read_text()==base
 assert (data/'.asoundrc').read_text()==base
 for rom in ['STOCK','NEXTUI']:
  volume=w/('volume-'+rom.lower())
  vc=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-Werror=implicit-function-declaration','-Werror=int-conversion','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-I'+str(sdk/'usr/include'),*incs,str(R/'qa/v182_bluetooth_volume.c'),'-Wl,--gc-sections','-o',str(volume)]
  with (R/('logs/v183-bt-volume-build-'+rom.lower()+'.log')).open('w') as log:subprocess.run(vc,stdout=log,stderr=log,check=True)
  p=subprocess.run([str(volume)],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=10)
  (R/('logs/v183-bt-volume-test-'+rom.lower()+'.log')).write_text(p.stdout+p.stderr);print(p.stdout+p.stderr,end='');p.check_returncode()
(R/'logs/v183-nextui-bluetooth-test.json').write_text(json.dumps({'exit':0,'production_NextUI_discovery_route_and_audio_reopen':True,'actual_ALSA_config_parser':True,'connected_A2DP_not_gamepad':True,'private_route_not_firmware_global':True,'fixed_source_recovers_requested_BT_with_stale_speaker_config':True,'firmware_global_asoundrc_unchanged':True,'speaker_BT_DAC_handoff':True,'new_headset_and_stale_firmware_setting':True,'PCM_transport_retry':True,'nonzero_local_Spotify_SoundCloud_PCM':True,'idle_no_audio_reopen':True,'Bluetooth_volume_explicit_unmute_both_ROMs':True,'ASan_UBSan':True,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['nxt_nextui.c','nxt_hardware.c','nxt_player_core.c']},'msettings_sha256':hashlib.sha256((R/'upstream/NextUI/workspace/tg5040/libmsettings/msettings.c').read_bytes()).hexdigest(),'hardware_tested':False,'actual_Bluetooth_transport_tested':False,'release_build_paused_by_user':release_paused},indent=2)+'\n')
