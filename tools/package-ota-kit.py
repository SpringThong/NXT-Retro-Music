#!/usr/bin/env python3
"""Bundle exact native installer and offline OTA publisher tools for one ROM."""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,zipfile
R=Path(__file__).resolve().parents[1]

def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()

def public_file(p,stage):
 return p.is_file() and not any(part.startswith('.') for part in p.relative_to(stage).parts)

def main():
 rom=os.environ.get('NXT_PACKAGE_ROM') or ('stock' if json.loads((R/'SOURCE.json').read_text())['build_define']=='NXT_ROM_STOCK' else 'nextui')
 assert rom in ['stock','nextui','nxredux'];label={'stock':'Stock','nextui':'NextUI','nxredux':'NXRedux'}[rom]
 config=json.loads((R/({'stock':'stock','nextui':'upstream','nxredux':'nxredux'}[rom]+'/config.json')).read_text());version=config['version']
 out=R/f'dist/{rom}-release';installer=out/config['release_filename'];kit_name=f'OTA_Google_Drive_v{version}_{label}';stage=R/'dist'/kit_name
 if stage.exists():shutil.rmtree(stage)
 stage.mkdir(parents=True);templates=stage/'templates';templates.mkdir()
 notes=R/f'CHANGELOG_v{version}_{label}.md';manifest_path=templates/'update.json'
 subprocess.run([sys.executable,str(R/'tools/make-ota-manifest.py'),'--rom',rom,'--zip',str(installer),'--url','https://drive.google.com/file/d/REPLACE_WITH_PUBLIC_ZIP_FILE_ID/view','--version',version,'--notes',str(notes),'--out',str(manifest_path)],check=True)
 manifest=json.loads(manifest_path.read_text());assert manifest['sha256']==sha(installer) and manifest['size']==installer.stat().st_size
 shutil.copy2(manifest_path,templates/f'update-v{version}.json')
 source=R/({'stock':'stock/ota-source-stock.url','nextui':'upstream/ota-source-nextui.url','nxredux':'nxredux/ota-source-nxredux.url'}[rom]);publisher=next(iter(source.read_text().splitlines()),'').strip()
 if not publisher.startswith('https://'):publisher=''
 data=dict(label=label,installer=installer.name,manifest=manifest,publisherSource=publisher)
 docs=R/f'docs/OTA_Google_Drive_v{version}'
 html=(docs/'CHUAN_BI_OTA.template.html').read_text();assert html.count('__OTA_RELEASE_DATA__')==1
 encoded=json.dumps(data,ensure_ascii=False).replace('<','\\u003c');(stage/'CHUAN_BI_OTA.html').write_text(html.replace('__OTA_RELEASE_DATA__',encoded))
 note=(label+' có nguồn chính thức đi kèm gói cài:\n\n'+publisher+'\n\nTrên chính tệp JSON này, dùng Quản lý phiên bản → Tải phiên bản mới để thay nội dung bằng update.json mới và giữ nguyên ID/link. App tự chuyển nguồn cũ sang nguồn chính thức; người dùng chỉ cần vào Cập nhật phần mềm.') if publisher else label+' chưa có nguồn chính thức trong gói này; không phát hành khi chưa cấu hình nguồn.' 
 app={'stock':'Apps/NXT_Retro_Music','nextui':'Tools/tg5040/NXT Retro Music.pak','nxredux':'Tools/NXT Retro Music.pak'}[rom]
 guide=(docs/'HUONG_DAN.template.md').read_text().replace('__LABEL__',label).replace('__ROM__',rom).replace('__PUBLISHER_NOTE__',note).replace('__APP_PATH__',app)
 for filename in ['HUONG_DAN_DANG_OTA.md','HUONG_DAN_DANG_OTA.txt']:(stage/filename).write_text(guide)
 (stage/'README.md').write_text(f'Mở CHUAN_BI_OTA.html trên máy tính để tạo update.json và tệp nguồn OTA.\n\nBộ dành riêng cho {label} v{version}, chứa ZIP cài đặt nguyên bản.\n\nĐọc HUONG_DAN_DANG_OTA.md để đăng lên Drive. JSON trong templates có chỗ điền link, không đăng mẫu chưa sửa.\n')
 shutil.copy2(notes,stage/notes.name);shutil.copy2(R/'tools/make-ota-manifest.py',stage/'make-ota-manifest.py');shutil.copy2(installer,stage/installer.name)
 (stage/(installer.name+'.sha256')).write_text(manifest['sha256']+'  '+installer.name+'\n')
 assert sha(stage/installer.name)==manifest['sha256']
 (stage/'SHA256_FILES.txt').write_text(''.join(sha(p)+'  '+p.relative_to(stage).as_posix()+'\n' for p in sorted(stage.rglob('*')) if public_file(p,stage)))
 archive=out/f'NXT_OTA_Google_Drive_v{version}_{label}.zip';temp=archive.with_suffix('.zip.building')
 with zipfile.ZipFile(temp,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
  for p in sorted(stage.rglob('*')):
   if public_file(p,stage):z.write(p,kit_name+'/'+p.relative_to(stage).as_posix(),compress_type=zipfile.ZIP_STORED if p.name==installer.name else zipfile.ZIP_DEFLATED)
 with zipfile.ZipFile(temp) as z:
  assert z.testzip() is None;payload=z.read(kit_name+'/'+installer.name)
  assert len(payload)==manifest['size'] and hashlib.sha256(payload).hexdigest()==manifest['sha256']
  assert kit_name+'/CHUAN_BI_OTA.html' in z.namelist() and kit_name+'/update.json' not in z.namelist()
 with temp.open('rb') as f:os.fsync(f.fileno())
 os.replace(temp,archive);digest=sha(archive);archive.with_suffix('.zip.sha256').write_text(digest+'  '+archive.name+'\n')
 value={'version':version,'rom':rom,'exit':0,'kit':archive.name,'kit_sha256':digest,'size':archive.stat().st_size,'installer':installer.name,'installer_sha256':manifest['sha256'],'exact_installer_included':True,'offline_generator_included':True,'placeholder_only_in_templates':True,'public_source_published':False}
 (R/f'logs/v{version.replace(".","")}-ota-publisher-kit-{rom}.json').write_text(json.dumps(value,indent=2)+'\n');print(json.dumps(value,indent=2),flush=True)

if __name__=='__main__':main()
