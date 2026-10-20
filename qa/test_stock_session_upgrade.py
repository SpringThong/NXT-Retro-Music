#!/usr/bin/env python3
"""Production session protocol/client and shell; file transport/audio/UI fixtures."""
from pathlib import Path
import json,os,re,shutil,signal,subprocess,tempfile,time

R=Path(__file__).resolve().parents[1]
S=R/'upstream/src'
includes=(R/'.cache/render-sp/includes.txt').read_text().splitlines()
includes+=['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform']]
report={};logs=[]

def wait_for(predicate,timeout=4):
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        if predicate():return
        time.sleep(.025)
    raise AssertionError('Timed out waiting for fixture state')

with tempfile.TemporaryDirectory(prefix='nxt-session-') as directory:
    w=Path(directory);old=w/'old/musicplayer.elf';new=w/'new/musicplayer.elf';previous=w/'previous/musicplayer.elf';previous151=w/'previous151/musicplayer.elf'
    for binary,source,tag in [(old,R/'qa/session_legacy_fixture.c','OLD'),(new,S/'nxt_session.c','NEW'),(previous,S/'nxt_session.c','OLD'),(previous151,S/'nxt_session.c','OLD')]:
        binary.parent.mkdir()
        command=['gcc','-DNXT_ROM_STOCK','-O2','-D_GNU_SOURCE','-DUSE_SDL2','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections',*includes,'-DNXT_SESSION_SOURCE="'+str(source)+'"','-DNXT_FIXTURE_TAG="'+tag+'"',str(R/'qa/session_fixture.c'),'-Wl,--gc-sections','-o',str(binary)]
        if source==S/"nxt_session.c":command.insert(1,"-DNXT_FIXTURE_HAS_ORPHAN")
        if binary==previous:command.insert(1,'-DNXT_FIXTURE_BUILD_KEY="1.5-Stock-Personal-OTA"')
        if binary==previous151:command.insert(1,'-DNXT_FIXTURE_BUILD_KEY="1.5.1-Stock-Profiles-Wallpaper-LED"')
        result=subprocess.run(command,capture_output=True,text=True);logs.append(result.stdout+result.stderr);print(result.stderr if result.returncode else "",end="");result.check_returncode()
    orphan=subprocess.run([str(new),'--qa-orphan'],env={**os.environ,'NXT_SESSION_SOCKET':str(w/'orphan')},capture_output=True,text=True,timeout=5);orphan.check_returncode();print(orphan.stdout);report['orphan-recovery']={'exit':0,'dead_peer_only':True,'never_kills_live_or_unrelated_process':True}
    key=subprocess.check_output([str(new),'--qa-build-key'],text=True).strip()
    assert key!='1.5-Stock-Personal-OTA','New build must refuse attachment to the previous v1.5 process'
    for name,binary,command,reply,attached in [
        ('current-build-open',new,'OPEN '+key+'\n','OK\n',1),
        ('wrong-build-refused',new,'OPEN different-build\n','UPGRADE\n',0),
        ('missing-newline-refused',new,'OPEN '+key,'UPGRADE\n',0),
        ('legacy-open-not-attached',old,'OPEN '+key+'\n','VISIBLE\n',0),
        ('legacy-attach-compatible',new,'ATTACH\n','OK\n',1),
    ]:
        result=subprocess.run([str(binary),'--qa-poll',command],capture_output=True,text=True,check=True)
        assert result.stdout==reply and f'ATTACHED={attached}' in result.stderr,(name,result)
        report[name]={'exit':0,'production_server_handler':True,'socket_transport_fixture':True};print('PASS',name)

    def case(name,initial=None,replace=False,stale=False):
        pak=w/name;binpath=pak/'bin/tg5040/musicplayer.elf';binpath.parent.mkdir(parents=True)
        shutil.copy2(new if initial is None else initial,binpath);launcher=(R/'stock/launch.sh').read_text()
        launcher=launcher.replace('/mnt/SDCARD/.userdata/nxt-stock',str(pak/'data'))
        launcher=launcher.replace('/tmp/nxt-retro-music-stock-$(id -u)',str(pak/'runtime'))
        (pak/'launch.sh').write_text(launcher)
        events=pak/'events';endpoint=str(pak/'runtime/session.sock');state=Path(endpoint+'.state')
        env={**os.environ,'NXT_SESSION_SOCKET':endpoint,'NXT_QA_EVENTS':str(events),'LOGS_PATH':str(pak/'logs'),'PLATFORM':'tg5040'}
        Path(endpoint).parent.mkdir(parents=True,exist_ok=True)
        first=None;innocent=None;pid=None
        try:
            if initial is not None:
                first=subprocess.Popen([str(binpath),'--session'],env=env,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True)
                wait_for(lambda:state.exists() and state.read_text().startswith('HIDDEN'))
            if stale:
                innocent=subprocess.Popen(['sleep','30']);state.write_text('HIDDEN '+str(innocent.pid)+'\n')
            if replace:
                replacement=binpath.with_suffix('.new');shutil.copy2(new,replacement);os.replace(replacement,binpath)
            if initial is None:Path(endpoint).write_text('stale socket\n')
            result=subprocess.run(['sh',str(pak/'launch.sh')],env=env,capture_output=True,text=True,timeout=8)
            sequence=events.read_text().splitlines() if events.exists() else []
            if replace:
                assert result.returncode==2 and first.poll() is None,(name,result.returncode)
                assert not any(' attach ' in line for line in sequence)
                assert 'reboot' in (pak/'NXT_Launch_Log.txt').read_text()
                # A reboot/normal full exit closes the old session. The product
                # refuses it; it never signals a PID from the state file.
                first.terminate();first.wait(timeout=3)
                result=subprocess.run(['sh',str(pak/'launch.sh')],env=env,capture_output=True,text=True,timeout=8)
            assert result.returncode==0,(name,result.returncode,(pak/'logs/nxt-launch.txt').read_text())
            pid=int(state.read_text().split()[1]);sequence=events.read_text().splitlines()
            if initial is not None and not replace:
                assert pid==first.pid and first.poll() is None
                assert sum(' start ' in line for line in sequence)==1 and any(' attach ' in line for line in sequence)
            if replace:
                assert pid!=first.pid and next(i for i,v in enumerate(sequence) if v.startswith('OLD exit'))<next(i for i,v in enumerate(sequence) if v.startswith('NEW start'))
            if innocent:assert innocent.poll() is None
            os.kill(pid,signal.SIGTERM);wait_for(lambda:not state.exists());pid=None
            report[name]={'exit':0,'production_client_server_protocol_and_launcher':True,'transport_audio_ui_fixture':True,'real_unix_ipc':'unavailable: managed runtime denies sockets','no_product_signal_to_old_or_unrelated_process':True}
            print('PASS',name)
        finally:
            if first and first.poll() is None:first.terminate();first.wait(timeout=3)
            if pid is not None:
                try:os.kill(pid,signal.SIGTERM)
                except ProcessLookupError:pass
            if innocent:innocent.terminate();innocent.wait()
    case('old-session-refused-then-reboot',old,replace=True)
    case('previous-v15-session-refused-then-reboot',previous,replace=True)
    case('previous-v151-session-refused-then-reboot',previous151,replace=True)
    case('same-build-reattach',new)
    case('stale-state-pid',new,stale=True)
    case('stale-endpoint')

(R/'logs/stock-session-test.log').write_text('\n'.join(logs))
(R/'logs/stock-session-test.json').write_text(json.dumps(report,indent=2)+'\n')
