#!/usr/bin/env python3
"""Exercise the production Telex, Drive parser and natural-sort implementations."""
from pathlib import Path
import ctypes
import functools
import json
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'upstream/src'
checks={}
with tempfile.TemporaryDirectory(prefix='nxt-controls-') as directory:
    work=Path(directory)
    lib=work/'input.so'
    subprocess.run(['gcc','-O2','-shared','-fPIC','-D_GNU_SOURCE','-Wall','-Wextra',
                    '-Werror','-Wno-misleading-indentation',str(SRC/'nxt_telex.c'),
                    str(SRC/'nxt_drive_parser.c'),'-o',str(lib)],check=True)
    native=ctypes.CDLL(str(lib))
    class Telex(ctypes.Structure):
        _fields_=[('raw',ctypes.c_char*1024),('literal_bytes',ctypes.c_size_t)]
    native.NxtTelex_append.argtypes=[ctypes.POINTER(Telex),ctypes.c_char_p,ctypes.c_size_t,ctypes.c_char_p]
    native.NxtTelex_append.restype=ctypes.c_bool
    native.NxtTelex_backspace.argtypes=[ctypes.POINTER(Telex),ctypes.c_char_p,ctypes.c_size_t]
    native.NxtTelex_backspace.restype=ctypes.c_bool
    native.NxtTelex_reset.argtypes=[ctypes.POINTER(Telex),ctypes.c_char_p]
    pairs={
        'tieengs Vieetj':'tiếng Việt','Tooi yeeu Vieetj Nam':'Tôi yêu Việt Nam',
        'ddaay laf banr nhacj':'đây là bản nhạc',
        'thuowng':'thương','thuwowng':'thương','dduwowngf':'đường',
        'nguyeenx xuaan thoong':'nguyễn xuân thông','quyeenj':'quyện',
        'gias':'giá','gif':'gì','hoaf':'hòa','hoasn':'hoán',
        'TIENGS VIEETJ':'TIÉNG VIỆT','aa aw ee oo ow uw dd':'â ă ê ô ơ ư đ',
        'as af ar ax aj':'á à ả ã ạ','aas aaf aar aax aaj':'ấ ầ ẩ ẫ ậ',
        'aaa eee ooo ddd ass aff arr axx ajj':'aa ee oo dd as af ar ax aj',
        'aaz asz ddz':'a a d','123 https://drive.google.com':'123 https://drive.google.com',
    }
    # URL fields deliberately use Latin mode in Keyboard_openLatin, not Telex.
    pairs.pop('123 https://drive.google.com')
    for raw,wanted in pairs.items():
        state=Telex();out=ctypes.create_string_buffer(512)
        for ch in raw:
            assert native.NxtTelex_append(ctypes.byref(state),out,512,ch.encode())
            out.value.decode('utf-8')
        actual=out.value.decode();assert actual==wanted,(raw,actual,wanted)
    state=Telex();out=ctypes.create_string_buffer(512)
    for ch in 'tieengs':assert native.NxtTelex_append(ctypes.byref(state),out,512,ch.encode())
    assert native.NxtTelex_backspace(ctypes.byref(state),out,512) and out.value.decode()=='tiêng'
    assert native.NxtTelex_backspace(ctypes.byref(state),out,512) and out.value.decode()=='tiên'
    for _ in range(10):native.NxtTelex_backspace(ctypes.byref(state),out,512)
    assert out.value==b''
    state=Telex();out=ctypes.create_string_buffer(4)
    assert native.NxtTelex_append(ctypes.byref(state),out,4,b'aaa') and out.value==b'aa'
    before=state.raw;assert not native.NxtTelex_append(ctypes.byref(state),out,4,b'bbb')
    assert state.raw==before and out.value==b'aa'
    state=Telex();out=ctypes.create_string_buffer(512)
    native.NxtTelex_reset(ctypes.byref(state),b'https://drive.google.com/AbCd')
    assert native.NxtTelex_append(ctypes.byref(state),out,512,b' tieengs')
    assert out.value.decode()=='https://drive.google.com/AbCd tiếng'
    checks['telex']={'exit':0,'cases':len(pairs),'utf8_limits_and_undo':True}

    class Entry(ctypes.Structure):
        _fields_=[('id',ctypes.c_char*128),('resource_key',ctypes.c_char*128),
                  ('name',ctypes.c_char*256),('folder',ctypes.c_bool)]
    native.NxtDrive_parseURL.argtypes=[ctypes.c_char_p,ctypes.POINTER(Entry)]
    native.NxtDrive_parseURL.restype=ctypes.c_bool
    native.NxtDrive_url.argtypes=[ctypes.POINTER(Entry),ctypes.c_bool,ctypes.c_char_p,ctypes.c_size_t]
    native.NxtDrive_url.restype=ctypes.c_bool
    native.NxtDrive_parseFolder.argtypes=[ctypes.c_char_p,ctypes.POINTER(Entry),ctypes.c_int,ctypes.c_char_p,ctypes.c_size_t]
    native.NxtDrive_parseFolder.restype=ctypes.c_int
    native.NxtDrive_confirmation.argtypes=[ctypes.c_char_p,ctypes.POINTER(Entry),ctypes.c_char_p,ctypes.c_size_t]
    native.NxtDrive_confirmation.restype=ctypes.c_bool
    fid='1AbCdEfGhIJ_0123456789-x'
    for url,folder in [(f'https://drive.google.com/file/d/{fid}/view?usp=sharing',False),
                       (f'https://drive.google.com/drive/folders/{fid}?resourcekey=0-AbCdE',True),
                       (f'https://drive.google.com/drive/u/2/folders/{fid}',True),
                       (f'https://drive.google.com/embeddedfolderview?id={fid}',True),
                       (f'https://docs.google.com/file/d/{fid}/edit',False),
                       (f'https://drive.usercontent.google.com/download?id={fid}',False)]:
        entry=Entry();assert native.NxtDrive_parseURL(url.encode(),ctypes.byref(entry)),url
        assert entry.id.decode()==fid and entry.folder==folder
        out=ctypes.create_string_buffer(768);assert native.NxtDrive_url(ctypes.byref(entry),True,out,768)
        assert b'confirm=t' in out.value and fid.encode() in out.value
    for url in ['http://drive.google.com/file/d/'+fid,'https://drive.google.com.evil.test/file/d/'+fid,
                'https://drive.google.com@evil.test/file/d/'+fid,'file:///etc/passwd',
                'https://drive.google.com/file/d/../../etc/passwd',
                f'https://drive.google.com/file/d/{fid}?resourcekey=bad%00key',
                f'https://drive.google.com/file/d/{fid}?resourcekey=$(bad)']:
        assert not native.NxtDrive_parseURL(url.encode(),ctypes.byref(Entry())),url
    html=f'''<html><title>Nhạc &amp; Mix - Google Drive</title><div class="flip-entries">
       <a href="https://drive.google.com/file/d/{fid}/view?resourcekey=0-AbCdE"><div>02 - Đường &amp; mưa.mp3</div></a>
       <a href='https://drive.google.com/drive/folders/{fid}Folder'><b>10 Thư mục</b></a>
       <a href="https://evil.test/file/d/{fid}">bad.mp3</a>
       <a href="https://drive.google.com/file/d/{fid}/view">duplicate</a></div></html>'''
    entries=(Entry*8)();title=ctypes.create_string_buffer(256)
    assert native.NxtDrive_parseFolder(html.encode(),entries,8,title,256)==2
    assert title.value.decode()=='Nhạc & Mix'
    assert entries[0].name.decode()=='02 - Đường & mưa.mp3' and entries[0].resource_key==b'0-AbCdE'
    assert entries[1].folder and entries[1].id.decode()==fid+'Folder'
    assert native.NxtDrive_parseFolder(html.encode(),entries,1,title,256)==-2
    assert native.NxtDrive_parseFolder(b'<title>Google Drive</title>ServiceLogin',entries,8,title,256)==-1
    assert native.NxtDrive_parseFolder(b'<title>Google Drive</title>You need access',entries,8,title,256)==-1
    assert native.NxtDrive_parseFolder(b'<title>Empty - Google Drive</title><div class="flip-entries"></div>',entries,8,title,256)==0
    entry=Entry();assert native.NxtDrive_parseURL(f'https://drive.google.com/file/d/{fid}/view'.encode(),ctypes.byref(entry))
    confirmation=f'''<form action="https://drive.usercontent.google.com/download">
      <input name="id" value="{fid}"><input name="confirm" value="t"><input name="uuid" value="abc-123"></form>'''
    out=ctypes.create_string_buffer(768)
    assert native.NxtDrive_confirmation(confirmation.encode(),ctypes.byref(entry),out,768)
    assert b'uuid=abc-123' in out.value
    for bad in [confirmation.replace('drive.usercontent.google.com','evil.test'),confirmation.replace(fid,fid+'other')]:
        assert not native.NxtDrive_confirmation(bad.encode(),ctypes.byref(entry),out,768)
    checks['drive_parser']={'exit':0,'public_links_resource_keys_unicode_and_confirmation':True}

    # Use the production Unicode key function, not a replacement comparator.
    rendering=ROOT/'.cache/render-sp'
    if not (rendering/'includes.txt').exists():subprocess.run(['python3',str(ROOT/'qa/setup_render_spotify.py')],check=True)
    includes=(rendering/'includes.txt').read_text().splitlines()
    sortlib=work/'sort.so'
    subprocess.run(['gcc','-O2','-shared','-fPIC','-D_GNU_SOURCE','-ffunction-sections','-fdata-sections',
        *includes,'-I'+str(SRC),str(SRC/'nxt_sort.c'),str(SRC/'nxt_text.c'),
        str(rendering/'utf8proc.o'),str(rendering/'ttf.o'),'-Wl,--gc-sections',
        '-l:libSDL2-2.0.so.0','-l:libfreetype.so.6','-l:libharfbuzz.so.0','-l:libfribidi.so.0',
        '-lm','-o',str(sortlib)],check=True)
    sorting=ctypes.CDLL(str(sortlib));sorting.NxtSort_compare.argtypes=[ctypes.c_char_p,ctypes.c_char_p]
    sorting.NxtSort_compare.restype=ctypes.c_int
    names=['10 Remix','2 Remix','Zebra','Ắn 12','an 2','Đường 3','duong 2','1 Remix','100 Remix']
    ordered=sorted(names,key=functools.cmp_to_key(lambda a,b:sorting.NxtSort_compare(a.encode(),b.encode())))
    assert ordered==['1 Remix','2 Remix','10 Remix','100 Remix','an 2','Ắn 12','duong 2','Đường 3','Zebra'],ordered
    for a in names:
        assert sorting.NxtSort_compare(a.encode(),a.encode())==0
        for b in names:
            assert (sorting.NxtSort_compare(a.encode(),b.encode())>0)==(sorting.NxtSort_compare(b.encode(),a.encode())<0)
    checks['natural_sort']={'exit':0,'numbers_case_and_vietnamese':True,'order':ordered}

(ROOT/'logs/stock-controls-drive-test.json').write_text(json.dumps(checks,ensure_ascii=False,indent=2)+'\n')
print('PASS Telex composition/undo/UTF-8 limits, public Drive parsing and natural Vietnamese sorting.')
