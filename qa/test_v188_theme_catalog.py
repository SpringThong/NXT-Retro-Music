from pathlib import Path
import hashlib,json,re,subprocess
from PIL import Image
R=Path(__file__).resolve().parents[1];S=R/'upstream/src'
palette=(S/'nxt_v18_palette.inc').read_text()
names=json.loads('['+palette.split('names[]={',1)[1].split('};',1)[0]+']')
colors=[re.findall(r'0x([0-9a-f]{6})',row) for row in palette.split('={\n',1)[1].split('\n};',1)[0].splitlines()]
scene=(S/'nxt_v18_scene_data.inc').read_text().split('={\n',1)[1].split('\n};',1)[0].splitlines()
assert len(names)==len(set(names))==len(colors)==len(scene)==50
assert '#define NXT_THEME_COUNT 50' in (S/'nxt_theme.h').read_text()
def lum(h):
 c=[int(h[i:i+2],16)/255 for i in (0,2,4)]
 c=[x/12.92 if x<=.04045 else ((x+.055)/1.055)**2.4 for x in c]
 return sum(x*y for x,y in zip(c,(.2126,.7152,.0722)))
def ratio(a,b):
 a,b=sorted((lum(a),lum(b)));return (b+.05)/(a+.05)
assets={};contrasts={}
for i in range(50):
 p=R/f'upstream/res/themes/v18/theme-{i:02d}.png'
 with Image.open(p) as im:assert im.size==(1024,768);assert im.mode in ('RGB','RGBA');im.load()
 assets[str(i)]=hashlib.sha256(p.read_bytes()).hexdigest()
 if i>=36:
  contrasts[str(i)]={f'{a}-{b}':ratio(colors[i][a],colors[i][b]) for a,b in [(0,2),(1,2),(1,3),(1,5),(5,6)]}
  assert min(contrasts[str(i)].values())>=4.5,(names[i],contrasts[str(i)])
  boxes=[tuple(map(int,m)) for m in re.findall(r'\{(\d+),(\d+),(\d+),(\d+)\}',scene[i])]
  song,_,wave,_,art=boxes
  assert art[0]+art[2]<=980 and art[1]+art[3]<=580
  assert song[0]+song[2]<=980 and song[1]+song[3]<=548
  assert wave[0]+wave[2]<=980 and wave[1]+wave[3]<=548
  def overlaps(a,b):return a[0]<b[0]+b[2] and b[0]<a[0]+a[2] and a[1]<b[1]+b[3] and b[1]<a[1]+a[3]
  assert not overlaps(art,song) and not overlaps(art,wave) and not overlaps(song,wave)
assert len({assets[str(i)] for i in range(36,50)})==14
# Build-time generator is deterministic and doesn't drift palette/geometry.
paths=[S/'nxt_v18_palette.inc',S/'nxt_v18_scene_data.inc',*[R/f'upstream/res/themes/v18/theme-{i:02d}.png' for i in range(50)]]
before={p:hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
subprocess.run(['python3',str(R/'tools/add-v187-themes.py')],check=True)
assert all(hashlib.sha256(p.read_bytes()).hexdigest()==h for p,h in before.items())
# Both actual menu labels must use the release constant; legacy labels are gone.
screens=(S/'nxt_screens.c').read_text()
assert 'v1.8.4' not in screens
assert '"What’s new · v" NXT_VERSION' in screens
assert '"X · Main features & changes in v" NXT_VERSION' in screens
assert '"Features & changes\\nWhat is new in v" NXT_VERSION' in screens
report={'exit':0,'themes':50,'original_included':True,'new_distinct_chassis':14,'new_theme_contrast':contrasts,'all_assets_sha256':assets,'all_assets_1024x768':True,'new_live_art_title_analyzer_no_overlap':True,'deterministic_generator':True,'both_current_release_labels':True,'physical_temperature_or_latency_measured':False}
(R/'logs/v188-theme-catalog-test.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS 50 native themes, 14 distinct chassis, all new text/accent contrast >=4.5, live areas separated, deterministic generation and current release labels')

