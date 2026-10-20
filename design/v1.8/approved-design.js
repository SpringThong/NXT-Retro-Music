/* NXT Retro Music v1.8: inspectable design scenes at the Brick Pro 4:3 ratio.
 * Artwork and chassis are vector assets; audio/device values are preview data.
 * These scenes are a design specification, not an SDL release build.
 */
const NXTDesign = (() => {
  const names = ['Original NXT','Cassette C60','Vintage Radio','Walkman','Reel to Reel','Hi-Fi Rack','CD Deck','Jukebox','CRT Terminal','Turntable','Modern Focus','Modern Ambient','Boombox','MiniDisc','Pocket Wheel','Classic 90s','Tube Amplifier','Gramophone','Car Audio','Synth Studio','Oscilloscope','Bento','E-Ink','Hologram','Nordic Studio','Night Drive','Jazz Lounge','Soft Paper','Aurora','Ocean Blue','Lo-fi Desk','Film Gallery','Studio Meter','Mono Slate','Rose Quartz','Gallery Noir'];
  const palettes = [
    ['#102b31','#1d4148','#dce9e7','#acc8c6','#63cdb4'],
    ['#ded0ad','#d0bc8f','#44382c','#584937','#9c522b'],
    ['#342024','#63373d','#f0e2d3','#d2b4ad','#d4a15b'],
    ['#102e50','#214b78','#e1edf4','#b1c9e0','#efb062'],
    ['#143e40','#285658','#e5eddf','#bfd4c8','#e2be72'],
    ['#301f36','#52334f','#f0e5ee','#d5b8cc','#db9dbd'],
    ['#202655','#383c78','#e7eafa','#b9bfe0','#91b6ef'],
    ['#48202e','#70364a','#f5e2da','#e0b9b4','#e7b36f'],
    ['#142b22','#214236','#d1efcc','#aad1a9','#7ec886'],
    ['#33281b','#544027','#f0e5ce','#d0bfa0','#e2af65'],
    ['#2c2046','#453263','#ede6fa','#cbbadb','#bda5f0'],
    ['#302a53','#514470','#f0e8f8','#cdc0e0','#e3aed1'],
    ['#402721','#683e34','#f5e7db','#ddbcad','#f0a179'],
    ['#d3d7ef','#b5bddf','#303b5b','#3e4c6c','#4c62a5'],
    ['#c9e0df','#a8cecf','#24474b','#33585c','#276c7d'],
    ['#292345','#4a3e70','#e7e7fa','#c6bddd','#aaa0e8'],
    ['#3c2b22','#5c4230','#f0e2cd','#d5bba0','#e8b263'],
    ['#302823','#554236','#eee1cd','#d2b99c','#d5b570'],
    ['#351f29','#502b3a','#f5e4e8','#d8b2bd','#f295a3'],
    ['#132f3e','#205163','#deedf0','#a8ced3','#78d6c9'],
    ['#2b321b','#46512a','#e8edcf','#c7d2a6','#c3d477'],
    ['#d0e1d3','#b3cbb9','#2b4b3f','#365749','#326f56'],
    ['#dfdccf','#cec8b8','#434740','#50544a','#5b6252'],
    ['#261c4a','#463061','#e9e1f7','#c4b3dd','#bea1e8'],
    ['#d6e3e6','#b8cfd5','#2f4852','#3d5863','#366477'],
    ['#0f2345','#263b61','#e1e9f2','#acbfd8','#e08e99'],
    ['#392124','#60393a','#f1dfca','#d7b79c','#e3b571'],
    ['#e6d1c7','#d9b9ac','#554239','#5f483e','#984e45'],
    ['#113b36','#215e51','#dcefe3','#acd5bf','#92d29c'],
    ['#c5dbec','#a6c5e1','#244767','#2d5172','#2b6597'],
    ['#dfceba','#cbb090','#554236','#554236','#96593c'],
    ['#20333e','#344f5c','#e5eee7','#bdd0c8','#d9b481'],
    ['#e1d9b4','#cdc08d','#4a4a31','#4f4f36','#766638'],
    ['#273039','#424d58','#e6eaf0','#bdc9d4','#a7bdce'],
    ['#e6cbd4','#d6aeba','#5a3449','#643c51','#9c446c'],
    ['#1f252b','#363f47','#e7e9e4','#bfcbc3','#c4d296']
  ];
  const esc = s => String(s).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&apos;'}[c]));
  const text = (x,y,s,n,c,a='start',family='NXT Manrope,DejaVu Sans,sans-serif') =>
    '<text x="'+x+'" y="'+y+'" fill="'+c+'" font-size="'+n+'" font-weight="500" font-family="'+family+'" text-anchor="'+a+'">'+esc(s)+'</text>';
  const rect = (x,y,w,h,c,r=0,extra='') => '<rect x="'+x+'" y="'+y+'" width="'+w+'" height="'+h+'" rx="'+r+'" fill="'+c+'" '+extra+'/>';
  const circle = (x,y,r,c,extra='') => '<circle cx="'+x+'" cy="'+y+'" r="'+r+'" fill="'+c+'" '+extra+'/>';
  const line = (x1,y1,x2,y2,c,w=1,extra='') => '<line x1="'+x1+'" y1="'+y1+'" x2="'+x2+'" y2="'+y2+'" stroke="'+c+'" stroke-width="'+w+'" '+extra+'/>';
  const path = (d,c,extra='') => '<path d="'+d+'" fill="'+c+'" '+extra+'/>';
  const luminance = c => c.slice(1).match(/../g).map(h=>parseInt(h,16)/255).map(v=>v<=.04045?v/12.92:Math.pow((v+.055)/1.055,2.4)).reduce((n,v,i)=>n+v*[.2126,.7152,.0722][i],0);
  const contrast = (a,b) => (Math.max(luminance(a),luminance(b))+.05)/(Math.min(luminance(a),luminance(b))+.05);
  const theme = id => {const [bg,panel,ink,muted,accent]=palettes[id];return {id,name:names[id],bg,panel,ink,muted,accent,buttonInk:contrast(accent,'#f3ede2')>contrast(accent,'#182027')?'#f3ede2':'#182027',light:[1,13,14,21,22,24,27,29,30,32,34].includes(id)};};
  function defs(t,p) {
    return '<defs>'+
      '<linearGradient id="'+p+'-body" x1="0" y1="0" x2="0" y2="1"><stop stop-color="'+t.panel+'"/><stop offset="1" stop-color="'+t.bg+'"/></linearGradient>'+
      '<linearGradient id="'+p+'-metal" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#d0c8b4"/><stop offset=".15" stop-color="#a8a292"/><stop offset=".5" stop-color="#d2cbb9"/><stop offset="1" stop-color="#7e7f70"/></linearGradient>'+
      '<linearGradient id="'+p+'-wood" x1="0" y1="0" x2="1" y2=".5"><stop stop-color="#71553b"/><stop offset=".48" stop-color="#97734d"/><stop offset="1" stop-color="#4e3d2d"/></linearGradient>'+
      '<linearGradient id="'+p+'-brass" x1="0" y1="0" x2="1" y2="1"><stop stop-color="#d4b16d"/><stop offset=".24" stop-color="#7f6135"/><stop offset=".55" stop-color="#b18a4f"/><stop offset="1" stop-color="#493721"/></linearGradient>'+
      '<radialGradient id="'+p+'-disc"><stop stop-color="#33393b"/><stop offset=".45" stop-color="#11181b"/><stop offset=".7" stop-color="#343b3c"/><stop offset="1" stop-color="#0e1518"/></radialGradient>'+
      '<radialGradient id="'+p+'-glass"><stop stop-color="#ddc691" stop-opacity=".26"/><stop offset="1" stop-color="#c5c6a6" stop-opacity=".03"/></radialGradient>'+
      '<linearGradient id="'+p+'-sky" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#0d2a40"/><stop offset=".56" stop-color="#587b87"/><stop offset=".82" stop-color="#d2b296"/><stop offset="1" stop-color="#d8ba93"/></linearGradient>'+
      '<linearGradient id="'+p+'-water" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#52747e"/><stop offset="1" stop-color="#102d41"/></linearGradient>'+
      '<linearGradient id="'+p+'-after" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#796063"/><stop offset="1" stop-color="#d6a78a"/></linearGradient>'+
      '<filter id="'+p+'-mono"><feColorMatrix type="saturate" values="0"/></filter>'+
      '<symbol id="'+p+'-art" viewBox="0 0 512 512">'+
        rect(0,0,512,512,'url(#'+p+'-sky)')+
        circle(344,161,63,'#e3c49e')+
        path('M0 251 L62 206 L105 230 L187 173 L258 236 L313 194 L417 272 L512 218 L512 390 L0 390 Z','#294b60')+
        path('M0 291 L87 250 L144 284 L215 242 L291 291 L391 251 L512 298 L512 390 L0 390 Z','#173b50')+
        rect(0,320,512,192,'url(#'+p+'-water)')+
        path('M0 335 Q138 349 232 335 T512 336 L512 343 Q355 336 223 345 T0 342 Z','#afaba0','opacity=".45"')+
        path('M280 340 L416 340 L387 347 L308 347 Z M307 357 L388 357 L375 361 L320 361 Z M323 377 L376 377 L369 381 L332 381 Z','#d1b694','opacity=".4"')+
        path('M0 412 Q90 387 158 417 Q218 438 265 458 Q401 431 512 452 L512 512 L0 512 Z','#112837')+
        path('M0 474 Q96 446 158 470 L191 512 L0 512 Z','#0d2232')+
        line(342,143,342,181,'#d7b890',1)+
        text(35,50,'SPRING SESSIONS',16,'#d8dfe0')+
        text(34,470,'BLUE HOUR',43,'#d5ddd9')+
        text(37,495,'VOL. 01  /  AFTER THE DAY',13,'#b7c8cc')+
      '</symbol>'+
      '<symbol id="'+p+'-gold" viewBox="0 0 512 512">'+rect(0,0,512,512,'url(#'+p+'-after)')+circle(186,205,106,'#ecd3a0')+path('M0 371 Q134 254 266 356 T512 352 L512 512 L0 512 Z','#7f685a')+path('M0 433 Q177 324 358 430 T512 436 L512 512 L0 512 Z','#514d43')+text(38,74,'GOLDEN AFTERNOON',28,'#eadfcc')+text(38,471,'SPRING SESSIONS',19,'#e4dcc9')+'</symbol>'+
    '</defs>';
  }
  function cover(t,p,x,y,size,r=8,art='art',mono=false) {
    const id=p+'-clip-'+x+'-'+y;
    return '<defs><clipPath id="'+id+'">'+rect(x,y,size,size,'#fff',r)+'</clipPath></defs>'+
      rect(x-6,y-6,size+12,size+12,t.light?'#bdb8a9':'#10191d',r+3)+
      '<g clip-path="url(#'+id+')"'+(mono?' filter="url(#'+p+'-mono)"':'')+'><use href="#'+p+'-'+art+'" x="'+x+'" y="'+y+'" width="'+size+'" height="'+size+'"/></g>';
  }
  function grid(x,y,w,h,c,step=40) {
    let a='';for(let i=x;i<=x+w;i+=step)a+=line(i,y,i,y+h,c,1,'opacity=".12"');for(let j=y;j<=y+h;j+=step)a+=line(x,j,x+w,j,c,1,'opacity=".12"');return a;
  }
  function wave(t,x,y,w,h,kind='bars') {
    let a='';
    if(kind==='scope'){
      let points=[];for(let i=0;i<=120;i++){let px=x+w*i/120,py=y+h/2+Math.sin(i*.19)*h*.26+Math.sin(i*.59)*h*.055;points.push(px.toFixed(1)+','+py.toFixed(1));}
      return '<g class="nxt18-waveform"><polyline points="'+points.join(' ')+'" fill="none" stroke="'+t.accent+'" stroke-width="3" stroke-linejoin="round"/></g>';
    }
    for(let i=0;i<40;i++){let v=(.15+Math.abs(Math.sin(i*.73)*Math.cos(i*.27))*.72)*h;a+=rect(x+i*w/40,y+h-v,w/40-4,v,t.accent,2,'opacity="'+(.42+(i%5)*.1)+'"');}return '<g class="nxt18-waveform">'+a+'</g>';
  }
  function speaker(t,p,cx,cy,r) {
    let a=circle(cx,cy,r,'#121c21')+circle(cx,cy,r-10,t.panel)+circle(cx,cy,r-17,'#1a262a')+circle(cx,cy,r*.58,'url(#'+p+'-disc)')+circle(cx,cy,r*.24,'#313b3c');
    for(let i=-5;i<=5;i++){let dy=i*r/7,dx=Math.sqrt(Math.max(0,(r-8)*(r-8)-dy*dy));a+=line(cx-dx,cy+dy,cx+dx,cy+dy,t.muted,1,'opacity=".13"');}return a;
  }
  function reel(t,p,x,y,r) {
    let a=circle(x,y,r,'url(#'+p+'-metal)')+circle(x,y,r-7,'#1f2b2d')+circle(x,y,r-15,'#8e958b');
    for(let i=0;i<3;i++){let q=i*Math.PI*2/3;a+=circle(x+Math.cos(q)*r*.45,y+Math.sin(q)*r*.45,r*.22,'#233032');}
    return a+circle(x,y,r*.2,'#566362')+circle(x,y,r*.085,'#c1c8b8');
  }
  function disc(t,p,x,y,r) {
    let a=circle(x,y,r,'url(#'+p+'-disc)');for(let i=0;i<8;i++)a+=circle(x,y,r-10-i*r/15,'none','stroke="#778485" stroke-opacity=".14" stroke-width="1"');
    a+=circle(x,y,r*.36,t.accent)+circle(x,y,r*.325,t.panel)+text(x,y-5,'NXT',r*.13,t.ink,'middle')+text(x,y+24,'BLUE HOUR',r*.065,t.muted,'middle')+circle(x,y,4,'#c9cec2');return a;
  }
  function knob(t,p,x,y,r=30) {return circle(x,y,r,'#111d21')+circle(x,y,r-4,'url(#'+p+'-metal)')+line(x,y-r+10,x,y-r+23,'#273333',3);}
  function vu(t,p,x,y,w=232,h=122) {
    let a=rect(x,y,w,h,'#c9c0a6',7)+path('M'+(x+20)+' '+(y+h-22)+' Q'+(x+w/2)+' '+(y-14)+' '+(x+w-20)+' '+(y+h-22),'none','stroke="#595b4c" stroke-width="2"');
    for(let i=0;i<9;i++){let q=Math.PI*(.86-i*.72/8),cx=x+w/2,cy=y+h+20,rx=Math.cos(q)*w*.43,ry=-Math.sin(q)*h*.92;a+=line(cx+rx,cy+ry,cx+rx*.89,cy+ry*.9,'#565a4d',2);}
    a+=line(x+w/2,y+h-12,x+w*.65,y+h*.31,'#795638',3)+circle(x+w/2,y+h-12,4,'#565848')+text(x+w/2,y+h-34,'LEVEL',18,'#575b4c','middle');return a;
  }
  function info(t,x,y,w=450,size=49,family) {
    return text(x,y,'Blue Hour',size,t.ink,'start',family)+text(x,y+46,'Spring Sessions',32,t.muted,'start',family);
  }
  function lyrics(t,x,y,w=430,center=false,compact=false) {
    let anchor=center?'middle':'start',xx=center?x+w/2:x;
    return text(xx,y,compact?'Let the music stay':'Let the music stay with you',compact?31:32,t.muted,anchor)+
      text(xx,y+48,compact?'A little light, a softer day':'A little light, a softer day',34,t.ink,anchor)+
      (compact?'':text(xx,y+89,'And the world slows down',29,t.muted,anchor));
  }
  function progress(t,x,y,w) {
    y=Math.min(y,608);
    return rect(x,y,w,4,t.muted,2,'opacity=".24"')+rect(x,y,w*.43,4,t.accent,2)+circle(x+w*.43,y+2,5,t.accent)+
      text(x,y+33,'01:24',24,t.muted)+text(x+w,y+33,'03:16',24,t.muted,'end');
  }
  function header(t) {
    return '<g class="nxt18-svg-header">'+text(100,64,'NXT',31,t.ink)+text(187,63,t.name.toUpperCase(),21,t.muted)+
      text(805,63,'LOCAL MUSIC',18,t.muted,'end')+line(48,95,976,95,t.muted,1,'opacity=".16"')+'</g>';
  }
  function frame(t,x,y,w,h,p,r=26) {return rect(x,y,w,h,'#111c21',r+3,'opacity=".35"')+rect(x+3,y+3,w-6,h-6,'url(#'+p+'-body)',r);}
  function quality(t) {return '';}
  function renderTheme(id,opts={}) {
    const t=theme(id),p='nxt18-'+id;
    let s=defs(t,p)+rect(0,0,1024,768,t.bg)+header(t),py=608,controls={x:512,y:677};
    const cov=(x,y,n,r=8,art='art',mono=false)=>cover(t,p,x,y,n,r,art,mono);
    const fr=(x,y,w,h,r=26)=>frame(t,x,y,w,h,p,r);
    switch(id) {
      case 0:
        s+=cov(64,153,365,14)+info(t,489,207)+lyrics(t,489,345,450)+wave(t,490,484,440,48)+progress(t,64,575,896);controls={x:512,y:663};break;
      case 1:
        s+=fr(46,154,932,414,30)+rect(68,174,888,347,t.bg,21)+cov(94,196,286,3)+info(t,421,233,480,44)+
          rect(420,308,484,116,'#7f7c6d',16)+reel(t,p,528,366,47)+reel(t,p,797,366,47)+rect(578,336,169,57,'#3c4540',5)+
          text(662,369,'NXT  /  C60',21,'#dfdfcd','middle')+text(425,471,'A little light, a softer day',31,t.ink)+
          rect(298,527,428,33,'#111d21',11)+progress(t,72,595,880);controls={x:512,y:677};break;
      case 2:
        s+=rect(48,147,928,425,'url(#'+p+'-wood)',27)+rect(66,165,892,389,t.panel,17)+cov(94,190,306,5)+
          rect(443,189,480,102,'#d1c9af',7)+text(468,221,'NXT RADIO',22,'#4c5145')+
          Array.from({length:22},(_,i)=>line(468+i*20,243,468+i*20,263,'#6e705b',i%5===0?2:1)).join('')+
          line(763,234,763,275,'#947343',3)+info(t,449,347,420,45)+lyrics(t,449,434,440,false,true)+
          Array.from({length:8},(_,i)=>line(95,513+i*4,397,513+i*4,'#8d9b86',1,'opacity=".35"')).join('')+progress(t,76,595,872);
        controls={x:512,y:677};break;
      case 3:
        s+=fr(48,153,928,418,24)+rect(69,175,356,358,'#10212a',16)+cov(86,192,322,9)+info(t,470,230,452,46)+lyrics(t,470,335,440)+
          rect(457,463,465,52,t.bg,8)+wave(t,473,476,432,23)+text(480,550,'STEREO  /  PERSONAL AUDIO',20,t.muted)+
          progress(t,72,595,880);controls={x:512,y:677};break;
      case 4:
        s+=fr(48,157,928,416,18)+cov(72,186,288,8)+reel(t,p,532,292,99)+reel(t,p,798,292,99)+
          line(465,364,513,429,'#101b20',7)+line(849,367,797,429,'#101b20',7)+line(513,429,797,429,'#101b20',7)+
          info(t,412,492,524,42)+text(80,525,'A little light,',30,t.muted)+text(80,564,'a softer day',31,t.ink)+
          progress(t,72,606,880);controls={x:512,y:682};break;
      case 5:
        s+=fr(48,154,928,420,14)+rect(65,171,908,205,t.panel,9)+cov(78,185,302,4)+info(t,422,237,482,43)+
          vu(t,p,419,337,242,144)+vu(t,p,684,337,242,144)+text(444,536,'A little light, a softer day',30,t.ink)+
          knob(t,p,148,538,25)+knob(t,p,226,538,25)+knob(t,p,304,538,25)+progress(t,72,607,880);controls={x:512,y:684};break;
      case 6:
        s+=fr(48,148,928,428,25)+disc(t,p,270,355,180)+cov(623,197,288,8)+
          info(t,68,536,500,40)+text(620,522,'A little light,',28,t.muted)+text(620,566,'a softer day',30,t.ink)+
          progress(t,72,609,880);controls={x:512,y:682};break;
      case 7:
        s+=path('M85 557 L85 355 Q85 144 512 144 Q939 144 939 355 L939 557 Z','url(#'+p+'-brass)')+
          path('M111 545 L111 360 Q111 174 512 174 Q913 174 913 360 L913 545 Z',t.panel)+
          path('M141 528 L141 365 Q141 204 512 204 Q883 204 883 365 L883 528 Z',t.bg)+
          cov(159,246,275,8)+info(t,481,302,378,41)+text(488,403,'A little light,',29,t.muted)+text(488,446,'a softer day',34,t.ink)+
          text(512,581,'NXT  /  SELECT YOUR MOMENT',22,t.muted,'middle')+progress(t,80,614,864);controls={x:512,y:684};break;
      case 8:
        s+=fr(47,144,930,432,22)+rect(65,162,894,395,'#14231e',12)+grid(67,164,890,390,t.accent,36)+
          text(86,199,'> NOW PLAYING',24,t.accent,'start','NXT Mono,DejaVu Sans Mono,monospace')+cov(83,224,292,2)+
          info(t,426,272,487,47,'NXT Mono,DejaVu Sans Mono,monospace')+lyrics(t,429,378,463)+
          text(430,526,'[ LOCAL ]  [ LOSSLESS ]',23,t.muted,'start','NXT Mono,DejaVu Sans Mono,monospace')+progress(t,72,607,880);
        controls={x:512,y:684};break;
      case 9:
        s+=rect(47,151,930,430,'url(#'+p+'-wood)',24)+rect(64,168,896,396,t.panel,17)+disc(t,p,279,367,180)+
          line(470,200,512,344,'#b6b5a5',10)+line(512,344,451,462,'#b6b5a5',10)+rect(431,451,34,38,'#c7c1ad',6)+
          circle(469,199,22,'#777f75')+cov(611,197,294,4)+text(625,532,'Blue Hour',33,t.ink)+text(625,569,'Spring Sessions',24,t.muted)+
          text(77,583,'A little light, a softer day',29,t.ink)+progress(t,72,638,880);controls={x:512,y:694};break;
      case 10:
        s+=cov(58,153,422,17)+info(t,545,244,430,51)+lyrics(t,548,373,425)+wave(t,548,491,414,43)+progress(t,548,570,414);
        controls={x:755,y:670};break;
      case 11:
        s+=path('M0 266 Q260 73 528 236 T1024 202 L1024 768 L0 768 Z',t.panel,'opacity=".5"')+
          rect(52,147,920,461,t.bg,28,'opacity=".65"')+cov(83,181,368,17)+info(t,507,240,440,47)+lyrics(t,510,352,439)+
          wave(t,510,493,408,44)+progress(t,83,613,856);controls={x:512,y:686};break;
      case 12:
        s+=fr(45,161,934,414,28)+speaker(t,p,187,386,113)+speaker(t,p,837,386,113)+
          cov(362,207,299,7)+text(512,550,'Blue Hour · Spring Sessions',30,t.ink,'middle')+
          rect(90,126,182,17,t.panel,6)+rect(752,126,181,17,t.panel,6)+text(512,127,'NXT BOOMBOX',24,t.muted,'middle')+
          text(512,593,'A little light, a softer day',29,t.ink,'middle')+progress(t,72,638,880);controls={x:512,y:694};break;
      case 13:
        s+=rect(55,162,403,416,t.panel,29)+rect(71,178,371,385,t.bg,20)+cov(91,199,330,3)+
          rect(305,496,111,51,'#7e968e',4)+line(346,502,346,539,'#bdc8b9',4)+
          info(t,511,234,435,46)+lyrics(t,514,350,440)+wave(t,515,495,412,56)+progress(t,72,612,880);controls={x:512,y:686};break;
      case 14:
        s+=rect(62,151,370,485,t.panel,29)+rect(69,158,356,472,t.bg,25)+cov(94,184,306,8)+
          circle(247,559,61,'#b8c1b3')+circle(247,559,29,'#d4dbc9')+
          text(247,522,'MENU',15,'#566452','middle')+text(247,604,'NXT',15,'#566452','middle')+
          info(t,501,251,450,47)+lyrics(t,505,368,438)+wave(t,505,505,420,56)+progress(t,500,607,427);
        controls={x:722,y:686};break;
      case 15:
        s+=rect(48,150,928,432,t.bg,5)+rect(54,156,916,420,t.panel,3)+rect(58,160,908,41,t.bg,2)+
          text(76,188,'NXT  /  AUDIO PLAYER',20,'#c3d8dc')+cov(80,226,286,1)+info(t,412,273,514,43,'NXT Mono,DejaVu Sans Mono,monospace')+
          rect(410,360,515,157,t.bg,3)+wave(t,427,378,481,68)+text(430,495,'A little light, a softer day',28,t.ink)+
          progress(t,72,611,880);controls={x:512,y:686};break;
      case 16:
        s+=fr(48,159,928,420,17)+cov(78,191,319,6)+info(t,449,220,480,43);
        for(let i=0;i<3;i++){let x=510+i*170;s+=rect(x-45,295,90,160,'url(#'+p+'-glass)',43,'stroke="#b8bb9b" stroke-opacity=".38" stroke-width="2"')+rect(x-24,329,48,71,'#93845b',8)+line(x-17,407,x-17,431,'#bea47a',3)+line(x+17,407,x+17,431,'#bea47a',3)+rect(x-47,449,94,30,'#111c20',7);}
        s+=text(451,542,'A little light, a softer day',28,t.ink)+progress(t,72,611,880);controls={x:512,y:686};break;
      case 17:
        s+=cov(76,172,335,6)+info(t,76,544,432,36)+
          path('M613 365 Q514 307 542 200 Q595 137 908 149 Q884 285 764 322 L687 384 Z','url(#'+p+'-brass)')+
          '<ellipse cx="738" cy="196" rx="167" ry="58" transform="rotate(-11 738 196)" fill="#574023"/>'+
          '<ellipse cx="738" cy="197" rx="149" ry="45" transform="rotate(-11 738 197)" fill="#281f18"/>'+
          path('M649 331 Q609 388 674 420','none','stroke="#967044" stroke-width="17"')+
          disc(t,p,738,468,112)+rect(556,580,376,30,'url(#'+p+'-wood)',8)+text(526,635,'A little light, a softer day',27,t.ink)+
          progress(t,72,632,391);controls={x:708,y:704};break;
      case 18:
        s+=fr(48,158,928,432,28)+rect(69,180,886,359,t.panel,14)+cov(90,201,305,6)+
          info(t,442,246,478,43)+lyrics(t,445,362,471)+wave(t,444,491,461,24)+
          text(87,572,'NXT AUDIO',21,t.muted)+knob(t,p,894,565,31)+progress(t,72,619,880);controls={x:512,y:690};break;
      case 19:
        s+=fr(48,153,928,429,17)+cov(75,184,299,4)+info(t,430,230,498,45)+lyrics(t,432,330,494)+wave(t,432,448,493,75);
        for(let i=0;i<12;i++){let x=77+i*29;s+=rect(x,502,27,58,'#bac2ba',3);if(i%7!==2&&i%7!==6)s+=rect(x+19,502,15,35,'#18242a',2);}
        s+=progress(t,72,612,880);controls={x:512,y:686};break;
      case 20:
        s+=fr(48,154,928,424,19)+cov(75,180,300,6)+info(t,429,234,495,45)+rect(428,326,496,182,'#233a2c',9)+
          grid(433,332,485,169,t.accent,39)+wave(t,443,349,466,127,'scope')+
          text(434,550,'A little light, a softer day',29,t.ink)+progress(t,72,612,880);controls={x:512,y:686};break;
      case 21:
        s+=rect(46,143,437,447,t.panel,25)+cov(68,165,393,15)+rect(508,143,470,275,t.panel,24)+info(t,534,209,416,43)+
          text(536,326,'A little light,',31,t.muted)+text(536,370,'a softer day',35,t.ink)+
          rect(508,441,470,149,t.panel,23)+wave(t,535,470,416,79)+progress(t,70,619,884);controls={x:512,y:690};break;
      case 22:
        s+=line(60,145,964,145,t.muted,1)+cov(89,184,340,0,'art',true)+info(t,504,263,428,44,'NXT Serif,Georgia,serif')+
          lyrics(t,507,383,438)+line(503,527,938,527,t.muted,1,'opacity=".4"')+progress(t,72,612,880);
        controls={x:512,y:686};break;
      case 23:
        s+=path('M65 153 L410 153 L465 214 L465 563 L119 563 L65 507 Z',t.panel,'opacity=".7"')+cov(99,197,326,2)+
          line(65,153,185,153,t.accent,2,'opacity=".8"')+line(345,563,465,563,t.accent,2,'opacity=".8"')+
          info(t,519,254,424,46)+lyrics(t,522,367,425)+wave(t,523,513,421,50)+progress(t,72,612,880);controls={x:512,y:686};break;
      case 24:
        s+=rect(67,161,400,417,'#c4b79f',14)+cov(87,180,360,4)+info(t,526,237,414,47,'NXT Serif,Georgia,serif')+
          lyrics(t,529,350,417)+line(530,528,942,528,t.muted,1,'opacity=".3"')+text(526,565,'A quiet place to listen',25,t.muted)+
          progress(t,80,612,864);controls={x:512,y:686};break;
      case 25:
        s+=path('M0 465 L246 243 L542 177 L1024 317 L1024 768 L0 768 Z',t.panel,'opacity=".32"')+
          path('M387 768 L492 280 L519 280 L592 768 Z','#7296a5','opacity=".08"')+
          info(t,68,249,490,53)+lyrics(t,70,364,451)+cov(620,218,321,13)+wave(t,74,529,467,37)+progress(t,72,612,880);
        controls={x:512,y:686};break;
      case 26:
        s+=rect(68,158,412,437,'#6b5041',13)+cov(90,180,367,3)+disc(t,p,762,406,154)+info(t,540,210,415,45,'NXT Serif,Georgia,serif')+
          text(91,588,'A little light, a softer day',29,t.ink)+text(768,601,'JAZZ  /  EVENING SESSION',20,t.muted,'middle')+progress(t,72,637,880);
        controls={x:512,y:694};break;
      case 27:
        s+=text(80,155,'THE LISTENING JOURNAL',23,t.muted)+cov(70,194,380,0)+line(493,180,493,574,t.muted,1,'opacity=".3"')+
          info(t,541,267,409,47,'NXT Serif,Georgia,serif')+lyrics(t,544,384,400)+text(542,566,'01  /  A MOMENT TO YOURSELF',20,t.muted)+progress(t,72,615,880);
        controls={x:512,y:686};break;
      case 28:
        s+=path('M0 260 Q327 29 633 249 T1024 188 L1024 391 Q729 451 495 283 T0 447 Z','#80bdae','opacity=".08"')+
          path('M0 313 Q325 82 643 293 T1024 250','none','stroke="#8ec7ad" stroke-width="1" stroke-opacity=".16"')+
          rect(77,191,382,382,t.panel,27)+cov(95,209,346,14)+info(t,526,273,415,50)+lyrics(t,529,389,422)+wave(t,529,523,412,48)+progress(t,72,616,880);
        controls={x:512,y:686};break;
      case 29:
        s+=cov(316,167,392,14)+text(510,593,'Blue Hour',38,t.ink,'middle')+text(510,634,'Spring Sessions',26,t.muted,'middle')+
          text(64,286,'A little light,',26,t.muted)+text(64,328,'a softer day',30,t.ink)+
          line(79,383,262,383,t.muted,1,'opacity=".2"')+wave(t,78,416,184,119,'scope')+
          text(787,284,'44.1 kHz',27,t.ink)+text(787,326,'24-bit',25,t.muted)+text(787,378,'LOSSLESS',22,t.muted)+wave(t,779,416,170,119)+
          progress(t,78,674,179)+progress(t,777,674,173);controls={x:511,y:700};break;
      case 30:
        s+=rect(59,166,395,430,'#b5a48a',5)+rect(67,158,377,424,'#e2dbca',3)+cov(83,175,345,0)+text(113,557,'a softer day',29,'#686c5a')+
          info(t,510,245,448,44,'NXT Serif,Georgia,serif')+lyrics(t,513,360,430)+
          '<ellipse cx="860" cy="575" rx="61" ry="17" fill="#b8a17e"/>'+circle(850,565,39,'#d9ccb5')+circle(850,565,31,'#76664f')+
          path('M881 549 Q930 544 910 585 Q901 603 877 591','none','stroke="#b9a98a" stroke-width="12"')+progress(t,72,614,880);controls={x:512,y:686};break;
      case 31:
        s+=rect(54,157,399,431,'#171e22',4)+cov(76,178,355,1)+info(t,499,249,392,43,'NXT Serif,Georgia,serif')+
          text(503,382,'A little light,',29,t.muted)+text(503,427,'a softer day',34,t.ink)+text(503,547,'01  /  BLUE HOUR',23,t.muted);
        for(let i=0;i<3;i++){s+=cov(863,193+i*129,82,1,i===1?'gold':'art')+Array.from({length:4},(_,j)=>rect(843,202+i*129+j*20,7,9,t.muted,1,'opacity=".5"')).join('');}
        s+=progress(t,72,615,880);controls={x:512,y:686};break;
      case 32:
        s+=rect(50,148,925,181,t.panel,18)+vu(t,p,81,168,413,140)+vu(t,p,529,168,414,140)+cov(78,358,271,6)+
          info(t,407,413,533,49)+lyrics(t,410,514,510,false,true)+progress(t,409,618,531);
        controls={x:676,y:686};break;
      case 33:
        s+=rect(0,105,480,596,'#1c2932',0)+cov(67,188,346,4)+info(t,538,269,420,46)+lyrics(t,541,384,420)+
          wave(t,65,580,347,37,'scope')+text(540,561,'AUDIO  /  LOSSLESS',22,t.muted)+progress(t,540,609,408);
        controls={x:747,y:686};break;
      case 34:
        s+=rect(51,150,442,442,t.panel,31)+cov(76,175,392,20)+info(t,544,259,407,46,'NXT Serif,Georgia,serif')+
          lyrics(t,547,374,407)+line(548,539,946,539,t.muted,1,'opacity=".28"')+progress(t,76,615,870);
        controls={x:512,y:686};break;
      case 35:
        s+=cov(45,142,455,1)+text(553,201,'SPRING SESSIONS',23,t.muted)+text(549,278,'Blue Hour',54,t.ink,'start','NXT Serif,Georgia,serif')+
          text(554,339,'VOL. 01  /  AFTER THE DAY',22,t.muted)+lyrics(t,554,438,418)+progress(t,554,609,398);
        controls={x:755,y:686};break;
    }
    s+=quality(t);controls={x:512,y:684};
    return {svg:'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 768" width="1024" height="768" role="img" aria-label="'+esc(t.name)+' music player"><title>'+esc(t.name)+' · NXT v1.8</title><desc>Design preview. Album artwork, current song, lyrics, progress and output information.</desc>'+s+'</svg>',theme:t,controls};
  }
  return {names,palettes,theme,renderTheme,esc};
})();
if(typeof module!=='undefined') module.exports=NXTDesign;
