#!/usr/bin/env python3
"""Optional public Google Drive service check; no private login or live music claim."""
from pathlib import Path
import ctypes,json,subprocess,tempfile,urllib.request
R=Path(__file__).resolve().parents[1]
folder='https://drive.google.com/embeddedfolderview?id=15uNXeRBIhVvZJIhL4yTw4IsStMhUaaxl'
file='https://drive.usercontent.google.com/download?id=1cKSdgtWrPgvEsBGmjWALOH33taGyVXKb&export=download&confirm=t'
def get(url,limit):
 with urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'NXT-Retro-Music'}),timeout=20) as r:return r.read(limit)
with tempfile.TemporaryDirectory(prefix='nxt-drive-live-') as directory:
 lib=Path(directory)/'parser.so';subprocess.run(['gcc','-O2','-shared','-fPIC',str(R/'upstream/src/nxt_drive_parser.c'),'-o',str(lib)],check=True);l=ctypes.CDLL(str(lib))
 class Entry(ctypes.Structure):_fields_=[('id',ctypes.c_char*128),('resource_key',ctypes.c_char*128),('name',ctypes.c_char*256),('folder',ctypes.c_bool)]
 entries=(Entry*256)();title=ctypes.create_string_buffer(256)
 l.NxtDrive_parseFolder.argtypes=[ctypes.c_char_p,ctypes.POINTER(Entry),ctypes.c_int,ctypes.c_char_p,ctypes.c_size_t]
 n=l.NxtDrive_parseFolder(get(folder,8*1024*1024),entries,256,title,256);assert n>=7
 data=get(file,1024*1024);assert data==b'spam\n'
 report={'live_public_folder':{'exit':0,'entries':n,'url':folder},'public_text_download':{'exit':0,'bytes':len(data),'audio':False}}
 (R/'logs/stock-drive-live-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
 print('PASS live public Google Drive folder and known 5-byte text download; no live audio/device claim.')
