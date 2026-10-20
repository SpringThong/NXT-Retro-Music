const fs=require('fs'),path=require('path'),vm=require('vm'),assert=require('assert').strict;
const {webcrypto}=require('crypto');
const ROOT=path.resolve(__dirname,'..');
const WORK=path.join(ROOT,'logs');
(async()=>{
 const results=[];
 const variants=process.env.NXT_TEST_ROM==='nxredux'?[['NXRedux','nxredux']]:[['Stock','stock'],['NextUI','nextui']];
 for(const [label,rom] of variants){
  const dir=path.join(ROOT,'dist','OTA_Google_Drive_v1.8.3_'+label);
  const html=fs.readFileSync(path.join(dir,'CHUAN_BI_OTA.html'),'utf8');
  const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
  const downloads=[],blobs=new Map(),elements=new Map();
  class Element {
   constructor(){this.value='';this.textContent='';this.files=[];this.disabled=false;this.events={};this.classes=new Set();
    this.classList={toggle:(name,yes)=>yes?this.classes.add(name):this.classes.delete(name)};}
   addEventListener(name,fn){this.events[name]=fn;}
   remove(){}
   click(){downloads.push({name:this.download,blob:blobs.get(this.href)});}
  }
  const get=id=>{if(!elements.has(id))elements.set(id,new Element());return elements.get(id);};
  const document={getElementById:get,createElement:()=>new Element(),body:{appendChild:()=>{}}};
  class TestURL extends URL {static createObjectURL(blob){const key='blob:test-'+blobs.size;blobs.set(key,blob);return key;}static revokeObjectURL(){}}
  const sandbox={document,Blob,URL:TestURL,crypto:webcrypto,Uint8Array,setTimeout:()=>0,console};
  vm.createContext(sandbox); vm.runInContext(script,sandbox,{filename:'CHUAN_BI_OTA-'+rom+'.html'});
  assert.equal(get('romBadge').textContent,label);assert.match(html,/V1\.8\.3/);assert.match(get('identity').textContent,/1\.8\.3/);
  for(const bad of ['http://example.com/installer.zip','https://drive.google.com/drive/folders/FOLDER_183','https://drive.google.com/file/d/REPLACE_WITH_PUBLIC_ZIP_FILE_ID/view','https://user:pass@example.com/file.zip','https://example.com/file.zip?value=@']){
   get('zipUrl').value=bad; get('makeManifest').events.click();
   assert.equal(get('manifestStatus').classes.has('error'),true);
  }
  const zipId='NXT_'+rom.toUpperCase()+'_INSTALLER_FIXTURE_183';
  const zipUrl='https://drive.google.com/file/d/'+zipId+'/view';
  const source='https://drive.google.com/file/d/NXT_'+rom.toUpperCase()+'_MANIFEST_FIXTURE_183/view';
  get('zipUrl').value=zipUrl; get('manifestUrl').value=source; get('makeManifest').events.click();
  const download=downloads.pop();assert.equal(download.name,'update.json');
  const actual=JSON.parse(await download.blob.text());
  const expected=JSON.parse(fs.readFileSync(path.join(dir,'templates','update.json'),'utf8'));
  assert.deepEqual(actual,{...expected,download_url:zipUrl});
  fs.writeFileSync(path.join(WORK,'generated-update-'+rom+'.json'),JSON.stringify(actual,null,2)+'\n');
  get('makeSource').events.click();
  const txt=downloads.pop();assert.equal(txt.name,'NXT_OTA_'+rom+'.txt');assert.equal(await txt.blob.text(),source+'\n');
  get('manifestUrl').value='https://drive.google.com/uc?export=download&id='+zipId;
  get('makeSource').events.click();assert.equal(get('sourceStatus').classes.has('error'),true);
  get('manifestUrl').value=source;
  get('makeManifest').events.click();assert.equal(get('manifestStatus').classes.has('error'),false);downloads.pop();
  const installer=path.join(dir,'NXT_Retro_Music_v1.8.3_'+label+'.zip');
  get('zipFile').files=[{name:path.basename(installer),size:fs.statSync(installer).size,arrayBuffer:async()=>{const b=fs.readFileSync(installer);return b.buffer.slice(b.byteOffset,b.byteOffset+b.byteLength);}}];
  await get('verifyZip').events.click();
  assert.match(get('verifyStatus').textContent,/đều khớp/);assert.equal(get('verifyZip').disabled,false);
  assert.match(html,/connect-src 'none'/);
  assert.equal(/<script[^>]+src=/.test(html),false);
  assert.match(html,/width:min\(880px,calc\(100% - 32px\)\)/);
  results.push({rom,generated_manifest_matches_actual_release:true,real_installer_sha_checked:true,source_txt_format_correct:true,invalid_and_same_file_links_rejected:true,scripts_and_assets_self_contained:true,network_disabled_by_csp:true,execution_environment:'Node VM with DOM event/download stubs; not a rendered browser'});
 }
 fs.writeFileSync(path.join(WORK,process.env.NXT_TEST_ROM==='nxredux'?'nxredux-publisher-generator-qa.json':'v183-publisher-generator-qa.json'),JSON.stringify(results,null,2)+'\n');
 console.log(JSON.stringify(results,null,2));
})().catch(e=>{console.error(e);process.exit(1)});

