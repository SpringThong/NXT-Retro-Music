#!/usr/bin/env python3
"""Render original line-art SVG profile icons using the repo's icon style."""
from pathlib import Path
import ctypes
import ctypes.util

ROOT = Path(__file__).resolve().parents[1]
paths = {
    'cat': '<path d="M5 9V3l5 4h4l5-4v6a8 8 0 1 1-14 0Z"/><path d="m8 15 2 1m6-1-2 1m-4 3 2 1 2-1"/>',
    'dog': '<path d="M7 8 3 4 2 13l5 1m10-6 4-4 1 9-5 1M7 7h10v11l-5 3-5-3Z"/><path d="M9 12h.1M15 12h.1m-5 4 2 1 2-1"/>',
    'rocket': '<path d="M8 15c-1-5 4-11 12-12 0 8-6 13-11 12ZM8 9H4l-2 6 6-1m6 2-1 6 6-2v-4m-14 3-2 3m4-2-1 3"/><circle cx="15" cy="8" r="2"/>',
    'star': '<path d="m12 2 3 6 7 1-5 5 1 8-6-4-6 4 1-8-5-5 7-1Z"/>',
    'flower': '<path d="M10 6c-5-6-10 2-4 5-7 1-5 9 2 6 0 7 8 7 8 0 7 3 9-5 2-6 6-3 1-11-4-5 1-7-5-7-4 0Z"/><circle cx="12" cy="12" r="3"/>',
    'leaf': '<path d="M20 3C9 1 2 6 4 15c2 9 19 7 16-12Zm-14 15 11-11m-8 8 1-5m3 1 4 1"/>',
    'mountain': '<path d="m2 20 7-14 4 7 3-10 6 17Zm4-8 3 2 3-2m2-3 2 2 2-2"/>',
    'waves': '<path d="M2 6c3-4 5 4 8 0s5 4 8 0 4 0 4 0M2 12c3-4 5 4 8 0s5 4 8 0 4 0 4 0M2 18c3-4 5 4 8 0s5 4 8 0 4 0 4 0"/>',
    'coffee': '<path d="M4 9h13v8a4 4 0 0 1-4 4H8a4 4 0 0 1-4-4Zm13 1h3a3 3 0 0 1 0 6h-3M7 2v3m5-3v3m5-3v3"/>',
    'crown': '<path d="m3 6 5 5 4-8 4 8 5-5-2 14H5Zm2 10h14"/>',
    'sparkles': '<path d="m13 2 3 7 7 3-7 3-3 7-3-7-7-3 7-3ZM4 2v5m-2-3h5m-3 14v5m-2-3h5"/>',
    'smile': '<circle cx="12" cy="12" r="9"/><path d="M8 9h.1M16 9h.1M7 14c2 5 8 5 10 0"/>',
}
rsvg = ctypes.CDLL(ctypes.util.find_library('rsvg-2'))
cairo = ctypes.CDLL(ctypes.util.find_library('cairo'))
gobject = ctypes.CDLL(ctypes.util.find_library('gobject-2.0'))
def bind(lib, name, args, result):
    fn = getattr(lib, name); fn.argtypes = args; fn.restype = result; return fn
new = bind(rsvg, 'rsvg_handle_new_from_data', [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p], ctypes.c_void_p)
render = bind(rsvg, 'rsvg_handle_render_cairo', [ctypes.c_void_p, ctypes.c_void_p], ctypes.c_int)
surface_new = bind(cairo, 'cairo_image_surface_create', [ctypes.c_int, ctypes.c_int, ctypes.c_int], ctypes.c_void_p)
context_new = bind(cairo, 'cairo_create', [ctypes.c_void_p], ctypes.c_void_p)
write = bind(cairo, 'cairo_surface_write_to_png', [ctypes.c_void_p, ctypes.c_char_p], ctypes.c_int)
context_free = bind(cairo, 'cairo_destroy', [ctypes.c_void_p], None)
surface_free = bind(cairo, 'cairo_surface_destroy', [ctypes.c_void_p], None)
unref = bind(gobject, 'g_object_unref', [ctypes.c_void_p], None)
source = ROOT / 'artwork/avatar-icons'
target = ROOT / 'upstream/res/icons'
source.mkdir(parents=True, exist_ok=True)
for name, body in paths.items():
    svg = source / ('avatar-' + name + '.svg')
    svg.write_text('<svg xmlns="http://www.w3.org/2000/svg" width="96" height="96" viewBox="0 0 24 24"><g fill="none" stroke="white" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">' + body + '</g></svg>\n')
    data = svg.read_bytes(); handle = new(data, len(data), None)
    assert handle, svg
    surface = surface_new(0, 96, 96); context = context_new(surface)
    assert render(handle, context), svg
    assert write(surface, str(target / (svg.stem + '.png')).encode()) == 0, svg
    context_free(context); surface_free(surface); unref(handle)
print('Rendered 12 original SVG profile icons.')
