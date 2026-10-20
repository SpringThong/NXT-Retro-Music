from pathlib import Path
R=Path(__file__).resolve().parents[1]
for name in ['package-nxredux.py','package-nextui.py','package-stock.py']:
 p=R/'tools'/name;s=p.read_text();s='from release_zip_entry import add_file\n'+s
 if name=='package-stock.py':
  assert 'z.write(path,path.relative_to(STAGE))' in s;s=s.replace('z.write(path,path.relative_to(STAGE))','add_file(z,path,STAGE)')
 else:
  assert 'z.write(p,p.relative_to(STAGE))' in s;s=s.replace('z.write(p,p.relative_to(STAGE))','add_file(z,p,STAGE)')
 p.write_text(s)
