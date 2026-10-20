const fs=require('fs'),path=require('path'),vm=require('vm');
const sharp=process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES?require(path.join(process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES,'sharp')):require('sharp');
const root=path.resolve(__dirname,'..'),source=path.join(root,'design/v1.8/approved-design.js');
let code=fs.readFileSync(source,'utf8');
code=code.replace('const NXTDesign =','globalThis.NXTDesign =');
code=code.replace('function cover(t,p,x,y,size,r=8,art=\'art\',mono=false) {','function cover(t,p,x,y,size,r=8,art=\'art\',mono=false) { globalThis.meta.arts.push([x,y,size,size]); if(globalThis.meta.arts.length>1)return "";');
code=code.replace('function info(t,x,y,w=450,size=49,family) {','function info(t,x,y,w=450,size=49,family) { globalThis.meta.song=[x,y-size,w,92]; return "";');
code=code.replace('function lyrics(t,x,y,w=430,center=false,compact=false) {','function lyrics(t,x,y,w=430,center=false,compact=false) { globalThis.meta.lyrics=[x,y-30,w,compact?86:124]; return "";');
code=code.replace('function wave(t,x,y,w,h,kind=\'bars\') {','function wave(t,x,y,w,h,kind=\'bars\') { globalThis.meta.wave=[x,y,w,h]; return "";');
code=code.replace('function progress(t,x,y,w) {','function progress(t,x,y,w) { globalThis.meta.progress=[x,Math.min(y,608),w,32]; return "";');
code=code.replace('function header(t) {','function header(t) { return "";');
vm.runInThisContext(code);
const song={9:[623,506,294,74],12:[310,516,470,54],17:[76,509,432,58],29:[316,560,392,46],35:[549,225,415,107],31:[499,206,335,105]};
const lyric={1:[425,442,480,50],4:[80,492,290,86],5:[444,506,480,52],6:[620,496,299,84],7:[488,372,366,97],9:[77,552,455,42],12:[270,564,490,42],15:[430,467,485,42],16:[451,512,470,52],17:[526,590,433,42],20:[434,517,484,43],21:[536,298,410,95],26:[91,556,376,44],29:[64,254,209,87],31:[503,352,333,96],35:[554,409,418,124]};
const waves={6:[620,582,299,18],9:[632,584,288,18],12:[725,575,218,24],17:[72,572,395,24],26:[550,576,380,24]};
(async()=>{let data=[];const out=path.join(root,'upstream/res/themes/v18');fs.mkdirSync(out,{recursive:true});for(let id=0;id<36;id++){
 globalThis.meta={arts:[],song:null,lyrics:null,wave:null,progress:null};let r=NXTDesign.renderTheme(id);let m=globalThis.meta;
 m.song=song[id]||m.song;m.lyrics=lyric[id]||m.lyrics;m.wave=waves[id]||m.wave||[0,0,0,0];m.progress=m.progress||[72,608,880,32];m.art=m.arts[0];
 let svg=r.svg.replace(/<text\b[\s\S]*?<\/text>/g,'').replace(/<symbol\b[\s\S]*?<\/symbol>/g,'').replace(/<use\b[^>]*\/>/g,'');
 // Physical meters have live needles; remove the demo needle only.
 if(id===5||id===32)svg=svg.replace(/<line\b[^>]*stroke="#795638"[^>]*\/>/g,'');
 const name='theme-'+String(id).padStart(2,'0');fs.writeFileSync(path.join(out,name+'.svg'),svg);await sharp(Buffer.from(svg)).png().toFile(path.join(out,name+'.png'));data.push({...m,theme:r.theme});
 }
 fs.writeFileSync(path.join(out,'scenes.json'),JSON.stringify(data,null,2));
 fs.writeFileSync(path.join(root,'upstream/src/nxt_v18_scene_data.inc'),'static const Scene scenes[36]={\n'+data.map(d=>'{'+['song','lyrics','wave','progress','art'].map(k=>'{'+d[k].map(Math.round).join(',')+'}').join(',')+'}').join(',\n')+'\n};\n');
 fs.writeFileSync(path.join(root,'upstream/src/nxt_v18_palette.inc'),'static const char *names[]={'+NXTDesign.names.map(JSON.stringify).join(',')+'};\nstatic const unsigned colors[36][7]={\n'+data.map(d=>{let t=d.theme;return '{'+[t.bg,t.panel,t.ink,t.muted,t.panel,t.accent,t.buttonInk].map(c=>'0x'+c.slice(1)).join(',')+'}';}).join(',\n')+'\n};\n');
 require('child_process').execFileSync('python3',[path.join(root,'tools/add-v187-themes.py')],{stdio:'inherit'});
 console.log('Exported 50 opaque native chassis and live content geometry');
})().catch(e=>{console.error(e);process.exit(1)});
