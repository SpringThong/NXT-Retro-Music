from pathlib import Path
import subprocess,tempfile,json,os,zipfile,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp';baseline=R.parents[2]/'releases_v1.9.7.3/NXT_Retro_Music_v1.9.7.3_Source.zip'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory() as td:
 p=Path(td)
 with zipfile.ZipFile(baseline) as z:old=z.read('NXT_Retro_Music_v1.9.7.3_Source/upstream/src/nxt_dsp.c').decode()
 for a,b in [('NxtDSP_configure','Old_configure'),('NxtDSP_process','Old_process'),('NxtDSP_response','Old_response'),('NxtDSP_peak','Old_peak'),('NxtDSP_clipped','Old_clipped')]:old=old.replace(a,b)
 (p/'old.c').write_text(old)
 results={}
 for rom in ['STOCK','NEXTUI','NXREDUX']:
  exe=p/rom;cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-DPLATFORM_DESKTOP','-fsanitize=address,undefined',*incs,str(R/'qa/v1974_vocal_dsp.c'),str(S/'nxt_vocal.c'),str(S/'nxt_dsp.c'),str(p/'old.c'),'-lm','-lpthread','-o',str(exe)]
  with (R/f'logs/v1974-vocal-{rom.lower()}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  out=subprocess.run([str(exe)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=45);print(rom,out.stdout+out.stderr);out.check_returncode();results[rom]={'exit':0,'synthetic_signal_tests':True,'off_byte_identical_to_baseline_DSP':True,'ASan_UBSan':True,'physical_device_or_real_song_tested':False}
 for rom in ['STOCK','NEXTUI','NXREDUX']:
  exe=p/('policy-'+rom);cmd=['gcc','-no-pie','-O1','-g','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_'+rom,'-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(R/'qa/v1974_vocal_policy.c'),str(S/'nxt_vocal.c'),str(S/'nxt_vocal_ui.c'),str(S/'nxt_audio_mode.c'),str(S/'nxt_dsd.c'),'-Wl,--gc-sections','-lm','-lpthread','-o',str(exe)]
  with (R/f'logs/v1974-vocal-policy-{rom.lower()}-build.log').open('w') as f:subprocess.run(cmd,stdout=f,stderr=f,check=True)
  out=subprocess.run([str(exe)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=30);print(rom,out.stdout+out.stderr);out.check_returncode();results[rom]['production_audio_policy_and_controller']=True
 (R/'logs/v1974-vocal.json').write_text(json.dumps(results,indent=2)+'\n')
