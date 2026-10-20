"""Record executable permissions explicitly, independent of workspace chmod."""
import zipfile,shutil
def add_file(archive,path,root):
 if any(part.startswith('.') for part in path.relative_to(root).parts):return
 info=zipfile.ZipInfo.from_file(path,path.relative_to(root).as_posix());info.compress_type=zipfile.ZIP_DEFLATED
 with path.open('rb') as stream:
  head=stream.read(4);stream.seek(0)
  if path.suffix=='.sh' or head==b'\x7fELF' or head[:2]==b'#!':
   info.create_system=3;info.external_attr=0o100755<<16
  with archive.open(info,'w') as target:shutil.copyfileobj(stream,target,1024*1024)
