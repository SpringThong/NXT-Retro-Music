#!/usr/bin/env python3
"""Rebuild existing monochrome icons from pinned Lucide/original SVG vectors.
Original coloured provider badges and emoji assets are intentionally preserved.
Requires librsvg, cairo and Pillow on the build host; runtime uses cached PNGs.
"""
from pathlib import Path
import ctypes,ctypes.util,tarfile,hashlib,json,re,io
from PIL import Image
R=Path(__file__).resolve().parents[1]
archive=R/'sources/lucide-static-0.468.0.tgz'
SHA=hashlib.sha256(archive.read_bytes()).hexdigest()
assert SHA=="d87c47325f0b2ac3ad2d6f056889da32b8f1216eafe4ed6534bc02786effe8cc", "Pinned Lucide archive mismatch"
rsvg=ctypes.CDLL(ctypes.util.find_library('rsvg-2'));cairo=ctypes.CDLL(ctypes.util.find_library('cairo'));gobject=ctypes.CDLL(ctypes.util.find_library('gobject-2.0'))
def bind(lib,name,args,result):
 f=getattr(lib,name);f.argtypes=args;f.restype=result;return f
ptr=ctypes.c_void_p;integer=ctypes.c_int
new=bind(rsvg,'rsvg_handle_new_from_data',[ctypes.c_char_p,ctypes.c_size_t,ptr],ptr)
render=bind(rsvg,'rsvg_handle_render_cairo',[ptr,ptr],integer)
surface=bind(cairo,'cairo_image_surface_create',[integer,integer,integer],ptr)
context=bind(cairo,'cairo_create',[ptr],ptr)
write=bind(cairo,'cairo_surface_write_to_png',[ptr,ctypes.c_char_p],integer)
freec=bind(cairo,'cairo_destroy',[ptr],None);frees=bind(cairo,'cairo_surface_destroy',[ptr],None);unref=bind(gobject,'g_object_unref',[ptr],None)
with tarfile.open(archive) as tar:
 vectors={Path(m.name).stem:tar.extractfile(m).read().decode() for m in tar.getmembers() if '/icons/' in m.name and m.name.endswith('.svg')}
alias={'mic-2':'mic-vocal','unlock-keyhole':'lock-keyhole-open'};changed=[]
for path in sorted((R/'upstream/res/icons').glob('*.png')):
 local=R/'artwork/avatar-icons'/(path.stem+'.svg')
 svg=local.read_text() if local.exists() else vectors.get(alias.get(path.stem,path.stem))
 if not svg:continue
 svg=re.sub(r'\b(width|height)="[^"]*"',lambda m:m.group(1)+'="384"',svg,count=2)
 svg=svg.replace('currentColor','white').replace('stroke="black"','stroke="white"')
 data=svg.encode();handle=new(data,len(data),None);assert handle,path
 image=surface(0,384,384);ctx=context(image);assert render(handle,ctx)
 tmp=R/'.cache/icon-render.png';assert write(image,str(tmp).encode())==0
 with Image.open(tmp) as im:out=im.convert('RGBA').resize((192,192),Image.Resampling.LANCZOS)
 # White RGB outside coverage prevents dark fringes in older SDL upscalers too.
 white=Image.new('RGBA',out.size,'white');white.putalpha(out.getchannel('A'));white.save(path,optimize=True)
 freec(ctx);frees(image);unref(handle);changed.append(path.name)
(R/'artwork/icon-refinement-v1977.json').write_text(json.dumps({'source':'lucide-static 0.468.0 + original avatar SVGs','archive_sha256':SHA,'output_pixels':192,'offline_supersampling_pixels':384,'icons':changed},indent=2)+'\n')
print(f'Rebuilt {len(changed)} existing vector icons at 192 px; coloured badges preserved; source SHA256 {SHA}')
