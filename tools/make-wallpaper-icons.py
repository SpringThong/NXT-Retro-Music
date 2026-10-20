#!/usr/bin/env python3
"""Render original gallery SVG icons in the existing NXT line style."""
from pathlib import Path
import ctypes,ctypes.util
ROOT=Path(__file__).resolve().parents[1]
paths={
'image':'<rect x="3" y="3" width="18" height="18" rx="3"/><circle cx="8" cy="8" r="2"/><path d="m3 18 5-5 4 4 4-6 5 7"/>',
'images':'<path d="M6 3h12a3 3 0 0 1 3 3v12M3 6h13a2 2 0 0 1 2 2v11a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2Z"/><circle cx="6" cy="11" r="1.5"/><path d="m2 19 5-5 3 3 4-5 4 6"/>',
'flower-2':'<circle cx="12" cy="12" r="3"/><path d="M9 8C4 1 1 10 7 11c-7 4 1 11 4 5 1 8 11 4 6-2 8-1 4-11-2-6 0-8-10-7-6 0Z"/>',
 'trees':'<path d="m8 3-5 7h2l-3 6h12l-3-6h2Zm0 13v6m9-16-4 6h2l-3 6h10l-3-6h2Zm0 12v4"/>',
 'building-2':'<rect x="3" y="3" width="11" height="19" rx="1"/><path d="M14 9h6v13M7 7h3m-3 4h3m-3 4h3m7-2h1m-1 4h1M7 22v-4h3v4"/>',
 'orbit':'<circle cx="12" cy="12" r="3"/><ellipse cx="12" cy="12" rx="11" ry="5" transform="rotate(-35 12 12)"/><path d="M7 3c6 1 11 10 9 17m-8 0c-4-3-6-11-4-14"/><circle cx="18" cy="6" r="2"/>',
 'shapes':'<path d="m7 2 5 9H2Z"/><rect x="13" y="3" width="8" height="8" rx="1"/><circle cx="7" cy="18" r="4"/><path d="m17 14 5 4-5 4-5-4Z"/>'}
def bind(lib,name,args,result):
 f=getattr(lib,name);f.argtypes=args;f.restype=result;return f
r=ctypes.CDLL(ctypes.util.find_library('rsvg-2'));c=ctypes.CDLL(ctypes.util.find_library('cairo'));g=ctypes.CDLL(ctypes.util.find_library('gobject-2.0'))
new=bind(r,'rsvg_handle_new_from_data',[ctypes.c_char_p,ctypes.c_size_t,ctypes.c_void_p],ctypes.c_void_p)
render=bind(r,'rsvg_handle_render_cairo',[ctypes.c_void_p,ctypes.c_void_p],ctypes.c_int)
surface=bind(c,'cairo_image_surface_create',[ctypes.c_int,ctypes.c_int,ctypes.c_int],ctypes.c_void_p);context=bind(c,'cairo_create',[ctypes.c_void_p],ctypes.c_void_p)
write=bind(c,'cairo_surface_write_to_png',[ctypes.c_void_p,ctypes.c_char_p],ctypes.c_int)
freec=bind(c,'cairo_destroy',[ctypes.c_void_p],None);frees=bind(c,'cairo_surface_destroy',[ctypes.c_void_p],None);unref=bind(g,'g_object_unref',[ctypes.c_void_p],None)
folder=ROOT/'artwork/wallpaper-icons';folder.mkdir(parents=True,exist_ok=True)
for name,body in paths.items():
 svg=folder/(name+'.svg');svg.write_text('<svg xmlns="http://www.w3.org/2000/svg" width="96" height="96" viewBox="0 0 24 24"><g fill="none" stroke="white" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">'+body+'</g></svg>\n')
 data=svg.read_bytes();h=new(data,len(data),None);s=surface(0,96,96);ctx=context(s);assert h and render(h,ctx);assert not write(s,str(ROOT/'upstream/res/icons'/(name+'.png')).encode());freec(ctx);frees(s);unref(h)
print('Seven original gallery icons rendered.')
