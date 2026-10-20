from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,json,time,sys
R=Path(__file__).resolve().parents[1]
checks=['decoder','normal_audio','bit_worker','bluetooth','audio_core','volume','volume_keys','stock_volume_audio','spotify_ipc','soundcloud_ipc','drive_decode','loader','baseline','sleep_timer','theme_catalog','translation']
def run(n):
 start=time.monotonic()
 with (R/f'logs/run-v1882-{n}.log').open('w') as f:r=subprocess.run(['python3',str(R/f'qa/test_v1882_{n}.py')],cwd=R,stdout=f,stderr=f)
 value={'test':n,'exit':r.returncode,'seconds':round(time.monotonic()-start,2)}
 print(json.dumps(value),flush=True);return value
requested=sys.argv[1:] or checks
assert all(n in checks for n in requested)
previous=json.loads((R/'logs/v1882-regression-runner.json').read_text()) if sys.argv[1:] else []
with ThreadPoolExecutor(max_workers=3) as pool:fresh=list(pool.map(run,requested))
merged={r['test']:r for r in previous};merged.update({r['test']:r for r in fresh})
results=[merged[n] for n in checks if n in merged]
(R/'logs/v1882-regression-runner.json').write_text(json.dumps(results,indent=2)+'\n')
assert all(x['exit']==0 for x in results),results

