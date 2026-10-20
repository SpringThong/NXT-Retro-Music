#!/usr/bin/env python3
from pathlib import Path
import json,os,subprocess,tempfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'qa',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-v15-profiles-') as td:
    w=Path(td);data=w/'data';binary=w/'profiles'
    cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-DFILE_UTILS_USERDATA_DIR="'+str(data)+'"','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined','-fno-omit-frame-pointer',*incs,'-include',str(S/'nxt_i18n.h'),str(R/'qa/v15_personal_test.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),'-Wl,--gc-sections','-l:libsqlite3.so.0','-lpthread','-lm','-o',str(binary)]
    with (R/'logs/v15-core-build.log').open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
    output=''
    for args in [['--mixed-queue'],[]]:
        result=subprocess.run([str(binary),*args],capture_output=True,text=True,timeout=30,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
        output+=result.stdout+result.stderr
        (R/'logs/v15-core-test.log').write_text(output);print(result.stdout+result.stderr,end='');result.check_returncode()
(R/'logs/v15-core-test.json').write_text(json.dumps({'profiles_and_statistics':{'exit':0,'sqlite_actual':True,'asan_ubsan':True,'paths_and_favorites_isolated':True,'seek_pause_buffer_not_counted':True,'short_track_repeat_qualification':True,'profile_persistence':True,'recording_off':True,'mixed_source_history_queue':True,'profile_switch_releases_player_module':True,'audio_and_hardware_boundaries':'fixtures'}},indent=2)+'\n')
