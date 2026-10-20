#!/usr/bin/env python3
"""Exercise actual OTA manifest, hash, ZIP, ARM64 target and replacement code."""
from pathlib import Path
import ctypes,hashlib,json,os,subprocess,tempfile,zipfile
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';q=R/'.cache/render-sp'
incs=(q/'includes.txt').read_text().splitlines()+['-I'+str(p) for p in [S,S/'include',S/'include/mbedtls_lib',R/'.host-tools/usr/include',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
class Manifest(ctypes.Structure):
    _fields_=[('version',ctypes.c_char*48),('url',ctypes.c_char*2048),('sha256',ctypes.c_char*65),('notes',ctypes.c_char*8192),('size',ctypes.c_uint64)]
checks={}
with tempfile.TemporaryDirectory(prefix='nxt-ota-qa-') as td:
    w=Path(td);stub=w/'stub.c';stub.write_text('#include <stdbool.h>\n#include <stddef.h>\nvoid NxtLog_step(const char*c,const char*s,bool ok,const char*f,...){}\nint NxtPersonal_route(const char*s,char*out,size_t n){return -1;}\nvoid mbedtls_platform_zeroize(void*p,size_t n){volatile unsigned char*b=p;while(n--)*b++=0;}\n')
    so=w/'ota.so';cmd=['gcc','-O1','-g','-shared','-fPIC','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>',*incs,str(S/'nxt_ota_package.c'),str(S/'file_utils.c'),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),str(stub),'-l:libzip.so.4','-lm','-o',str(so)]
    subprocess.run(cmd,check=True);lib=ctypes.CDLL(str(so));error=ctypes.create_string_buffer(400)
    lib.NxtOTA_compareVersion.argtypes=[ctypes.c_char_p,ctypes.c_char_p];lib.NxtOTA_compareVersion.restype=ctypes.c_int
    for a,b,want in [('1.5','1.3',1),('1.10','1.9',1),('1.5.0','1.5',0),('1.2','1.3',-1),('bad','1.3',-2),('1.','1.3',-2),('1.2.3.4.5','1.3',-2)]:assert lib.NxtOTA_compareVersion(a.encode(),b.encode())==want
    valid={'schema':1,'package':'vn.nguyenxuanthong.nxtretromusic','rom':'stock','device':'trimui-brick-pro','version':'1.6','download_url':'https://drive.google.com/file/d/PUBLIC_ZIP_ID/view','sha256':'a'*64,'size':100,'changelog':'Thử nghiệm'}
    lib.NxtOTA_parseManifest.argtypes=[ctypes.c_char_p,ctypes.POINTER(Manifest),ctypes.c_char_p,ctypes.c_size_t];lib.NxtOTA_parseManifest.restype=ctypes.c_bool
    m=Manifest();assert lib.NxtOTA_parseManifest(json.dumps(valid).encode(),ctypes.byref(m),error,len(error));assert m.version==b'1.6'
    for field,value in [('schema',2),('rom','nextui'),('device','wrong'),('package','wrong'),('version','beta'),('download_url','http://evil.test/a'),('download_url','https://a@evil.test/a'),('sha256','G'*64),('size',0),('size',268435457),('size',1.5)]:
        bad={**valid,field:value};assert not lib.NxtOTA_parseManifest(json.dumps(bad).encode(),ctypes.byref(m),error,len(error)),(field,value)
    checks['manifest']={'exit':0,'numeric_version_cases':7,'bad_manifest_cases':11}
    # Upstream's non-Stock build must never inherit the Stock OTA target.
    next_so=w/'nextui-target.so';next_cmd=[str(next_so) if x==str(so) else x for x in cmd if x!='-DNXT_ROM_STOCK'];subprocess.run(next_cmd,check=True)
    next_lib=ctypes.CDLL(str(next_so));next_lib.NxtOTA_parseManifest.argtypes=lib.NxtOTA_parseManifest.argtypes;next_lib.NxtOTA_parseManifest.restype=ctypes.c_bool
    next_valid={**valid,'rom':'nextui'};assert next_lib.NxtOTA_parseManifest(json.dumps(next_valid).encode(),ctypes.byref(m),error,len(error));assert not next_lib.NxtOTA_parseManifest(json.dumps(valid).encode(),ctypes.byref(m),error,len(error))
    target_c=w/'target.c';target_c.write_text('#include "nxt_ota_package.h"\n#include <stdio.h>\nint main(void){puts(NXT_OTA_ROM);puts(NXT_OTA_DEFAULT_FILE);puts(NXT_OTA_DATA_DIR);return 0;}\n')
    for flags,expected in [(['-DNXT_ROM_STOCK'],['stock','ota-source-stock.url','ota/stock']),([],['nextui','ota-source-nextui.url','ota/nextui']),(['-DNXT_ROM_NEXTUI'],['nextui','ota-source-nextui.url','ota/nextui'])]:
        target=w/'target';subprocess.run(['gcc',*flags,'-I'+str(S),str(target_c),'-o',str(target)],check=True);assert subprocess.check_output([str(target)],text=True).splitlines()==expected
    conflict=subprocess.run(['gcc','-DNXT_ROM_STOCK','-DNXT_ROM_NEXTUI','-I'+str(S),str(target_c),'-o',str(w/'invalid')],capture_output=True,text=True);assert conflict.returncode and 'Choose exactly one OTA ROM target' in conflict.stderr
    checks['rom_source_separation']={'exit':0,'stock_and_implicit_explicit_nextui_defaults':True,'no_shared_default_file_or_userdata_path':True,'cross_rom_manifest_rejected_both_directions':True,'ambiguous_build_flags_rejected':True,'nextui_runtime_build_tested':False}

    config={k:valid[k] for k in ('package','rom','device','version')};config['launch']='launch.sh'
    elf=bytearray(64);elf[:6]=b'\x7fELF\2\1';elf[18]=183
    prefix='Apps/NXT_Retro_Music/'
    baseline={'config.json':json.dumps(config).encode(),'launch.sh':b'#!/bin/sh\nexit 0\n',**{'bin/tg5040/'+n:bytes(elf) for n in ['musicplayer.elf','nxt-spotify-backend','nxt-soundcloud-backend']},'res/test.txt':b'resource'}
    def archive(label,changes=None,extra=None):
        path=w/(label+'.zip')
        with zipfile.ZipFile(path,'w',zipfile.ZIP_DEFLATED) as z:
            for name,data in {**baseline,**(changes or {})}.items():z.writestr(prefix+name,data)
            for name,data in extra or []:z.writestr(name,data)
        update={**valid,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'size':path.stat().st_size};manifest=Manifest();assert lib.NxtOTA_parseManifest(json.dumps(update).encode(),ctypes.byref(manifest),error,len(error));return path,manifest
    lib.NxtOTA_verifyZip.argtypes=[ctypes.c_char_p,ctypes.POINTER(Manifest),ctypes.c_char_p,ctypes.c_char_p,ctypes.c_size_t];lib.NxtOTA_verifyZip.restype=ctypes.c_bool
    def verify(path,m,label):return lib.NxtOTA_verifyZip(os.fsencode(path),ctypes.byref(m),os.fsencode(w/label),error,len(error))
    path,m=archive('good');assert verify(path,m,'stage'),error.value;assert (w/'stage/res/test.txt').read_bytes()==b'resource';assert (w/'stage/launch.sh').stat().st_mode&0o111
    for name,value in [('rom','nextui'),('device','wrong'),('version','1.7')]:
        bad={**config,name:value};path,m=archive('bad-'+name,{'config.json':json.dumps(bad).encode()});assert not verify(path,m,'stage-'+name)
    for label,name in [('traversal',prefix+'../outside'),('absolute','/tmp/outside'),('backslash',prefix+'..\\outside'),('duplicate',prefix+'launch.sh')]:
        path,m=archive(label,extra=[(name,b'unsafe')]);assert not verify(path,m,'stage-'+label),label
    symlink=zipfile.ZipInfo(prefix+'res/link');symlink.create_system=3;symlink.external_attr=0o120777<<16;path,m=archive('symlink',extra=[(symlink,b'/tmp/outside')]);assert not verify(path,m,'stage-symlink')
    wrong=bytearray(elf);wrong[18]=62;path,m=archive('wrong-elf',{'bin/tg5040/musicplayer.elf':bytes(wrong)});assert not verify(path,m,'stage-elf')
    path,m=archive('hash');m.sha256=b'0'*64;assert not verify(path,m,'stage-hash');m.size+=1;assert not verify(path,m,'stage-size')
    checks['zip']={'exit':0,'bad_zip_cases':11,'sha_size_checked':True,'only_app_payload':True,'arm64_target_checked':True}
    app=w/'NXT_Retro_Music';app.mkdir();(app/'old.txt').write_text('old');userdata=w/'userdata';userdata.mkdir();(userdata/'credentials.json').write_text('fixture private credentials')
    lib.NxtOTA_install.argtypes=[ctypes.c_char_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_size_t];lib.NxtOTA_install.restype=ctypes.c_bool
    assert lib.NxtOTA_install(os.fsencode(app),os.fsencode(w/'stage'),error,len(error)),error.value
    assert (app/'config.json').exists() and (w/'.NXT_Retro_Music.previous/old.txt').read_text()=='old';assert (userdata/'credentials.json').read_text()=='fixture private credentials'
    checks['install']={'exit':0,'previous_app_backup':True,'userdata_untouched':True,'physical_device_tested':False}
    print('PASS production OTA: versions/manifests, checksum, unsafe paths/duplicates/symlink, ROM/device/ARM64, install/backup and preserved user data.')
(R/'logs/v15-ota-test.json').write_text(json.dumps(checks,indent=2)+'\n')
