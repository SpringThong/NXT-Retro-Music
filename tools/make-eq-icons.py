#!/usr/bin/env python3
"""Render original line-art SVG profile icons using the repo's icon style."""
from pathlib import Path
import ctypes
import ctypes.util

ROOT = Path(__file__).resolve().parents[1]
paths = {'flat': '<path d="M3 12h18"/><circle cx="12" cy="12" r="2"/>', 'pop': '<path d="m12 2 3 6 7 1-5 5 1 7-6-3-6 3 1-7-5-5 7-1Z"/>', 'rock': '<path d="m14 11 6-8 2 2-7 8m-4-2c-4-4-9 1-6 4-5 3-1 8 3 5 5 2 8-3 4-7Z"/><circle cx="9" cy="16" r="1.5"/>', 'jazz': '<path d="M16 3h4v3h-3v9c0 8-11 8-11 0H3l3-5 3 5H6c0 4 7 5 7 0V5l3-2Z"/><path d="M13 8h4m-4 3h4m-4 3h4"/>', 'classical': '<path d="M3 4h18v16H3ZM3 13h18M7 13v7m5-7v7m5-7v7"/><path d="M6 4v10h2V4m3 0v10h2V4m3 0v10h2V4"/>', 'vocal': '<rect x="9" y="2" width="6" height="12" rx="3"/><path d="M5 10v2a7 7 0 0 0 14 0v-2m-7 9v3m-4 0h8"/>', 'edm': '<path d="m13 2-9 12h7l-1 8 10-13h-7Z"/>', 'acoustic': '<path d="m14 12 7-9 1 1-7 10m-3-4c-4-3-6 0-6 3-4 0-4 5-2 7 3 3 8 1 8-3 4-2 4-5 0-7Z"/><circle cx="10" cy="15" r="2"/>', 'bass': '<rect x="5" y="2" width="14" height="20" rx="2"/><circle cx="12" cy="7" r="2"/><circle cx="12" cy="16" r="4"/>', 'treble': '<path d="M5 4v16m0-8 5-5v10Zm8-5c5 2 5 6 0 8m3-11c7 3 7 11 0 14"/>', 'lofi': '<rect x="2" y="5" width="20" height="14" rx="3"/><circle cx="7" cy="11" r="3"/><circle cx="17" cy="11" r="3"/><path d="m7 17 2-3h6l2 3M10 10h4"/>', 'loudness': '<path d="m3 9 5-5v16l-5-5ZM12 8v8m4-11v14m4-17v20"/>'}
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
source = ROOT / 'artwork/eq-icons'
target = ROOT / 'upstream/res/icons'
source.mkdir(parents=True, exist_ok=True)
for name, body in paths.items():
    svg = source / ('eq-' + name + '.svg')
    svg.write_text('<svg xmlns="http://www.w3.org/2000/svg" width="96" height="96" viewBox="0 0 24 24"><g fill="none" stroke="white" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">' + body + '</g></svg>\n')
    data = svg.read_bytes(); handle = new(data, len(data), None)
    assert handle, svg
    surface = surface_new(0, 96, 96); context = context_new(surface)
    assert render(handle, context), svg
    assert write(surface, str(target / (svg.stem + '.png')).encode()) == 0, svg
    context_free(context); surface_free(surface); unref(handle)
print('Rendered 12 original SVG profile icons.')
