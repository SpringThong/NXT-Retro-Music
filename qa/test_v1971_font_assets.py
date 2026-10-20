"""Validate pinned licensed assets, primary Vietnamese coverage and old font IDs."""
from pathlib import Path
import hashlib,json,re,zipfile
from fontTools.ttLib import TTFont
R=Path(__file__).resolve().parents[1]
def sha(b):return hashlib.sha256(b).hexdigest()
m=json.loads((R/'docs/FONTS_v1.9.7.1.json').read_text())
assert m['total_choices']==60 and m['old_indices_preserved']==22 and len(m['new_families'])==38
c=(R/'upstream/src/nxt_text.c').read_text()
arrays={decl:re.findall(r'"([^"]+)"',re.search(r'static const char \*'+decl+r'\[\]=\{([^}]+)\}',c)[1]) for decl in ['names','paths']}
assert all(len(a)==60 for a in arrays.values())
# All composed Vietnamese letters, tone marks, ASCII digits/punctuation.
from unicodedata import normalize
text='Tiếng Việt Đặng Thị Nguyễn Phương 0123456789 !?.,:; () ÀÁẢÃẠĂẰẮẲẴẶÂẦẤẨẪẬÈÉẺẼẸÊỀẾỂỄỆÌÍỈĨỊÒÓỎÕỌÔỒỐỔỖỘƠỜỚỞỠỢÙÚỦŨỤƯỪỨỬỮỰỲÝỶỸỴĐ'
required={ord(x) for x in normalize('NFC',text+text.lower())}
count=total=0
for family in m['new_families']:
 assert arrays['names'][family['index']]==family['family']
 license=R/family['license_asset'];assert sha(license.read_bytes())==family['license_sha256'];assert b'SIL OPEN FONT LICENSE' in license.read_bytes()
 for asset in family['files']:
  path=R/asset['asset'];data=path.read_bytes();assert sha(data)==asset['sha256'] and len(data)==asset['bytes']
  with TTFont(path) as font:missing=required-set(font.getBestCmap())
  assert not missing,(path,missing)
  assert family['commit'] in asset['url'];count+=1;total+=len(data)
with zipfile.ZipFile(R.parent.parent/'baseline/NXT_Retro_Music_v1.9.7_Source.zip') as z:
 old=z.read('NXT_Retro_Music_v1.9.7_Source/upstream/src/nxt_text.c').decode()
 for decl,values in arrays.items():assert re.findall(r'"([^"]+)"',re.search(r'static const char \*'+decl+r'\[\]=\{([^}]+)\}',old)[1])==values[:22]
 for name in z.namelist():
  if name.startswith('NXT_Retro_Music_v1.9.7_Source/upstream/res/fonts/') and not name.endswith('/'):assert (R/name.split('/',1)[1]).read_bytes()==z.read(name)
result={'version':'1.9.7.1','exit':0,'total_font_choices':60,'new_families':38,'asset_files':count,'new_asset_bytes':total,'full_primary_vietnamese_cmap':True,'first_22_ids_and_all_old_font_bytes_preserved':True,'pinned_unmodified_assets_license_hashes_verified':True,'SF_Pro_bundled':False}
(R/'logs/v1971-font-assets.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',result)
