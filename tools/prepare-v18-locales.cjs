const fs=require('fs'),path=require('path'),vm=require('vm');const r=path.resolve(__dirname,'..');
const source=fs.readFileSync(path.join(r,'design/v1.8/approved-flows.js'),'utf8');
const start=source.indexOf('const keys='),end=source.indexOf('  const T=',start);const box={};vm.createContext(box);vm.runInContext(source.slice(start,end)+';globalThis.locales={keys,values};',box);
const base=box.locales.values.en,all={};for(const [lang,v] of Object.entries(box.locales.values)){all[lang]={};for(let i=0;i<base.length;i++)all[lang][base[i]]=v[i];}
const aliases={'Choose your listener':0,'How are you feeling today?':2,'What genre would you like?':3,'Music from which country?':4,'For you':7,'Personal':8,'Usage guide':10,'About, help & updates':11,'Language & keyboard':13,'Test keyboard':14,'Today’s listening choices':16,'Search settings':20,'Audio & DAC':21,'Appearance & LEDs':22,'Music folder':23,'Updates & help':11,'Idea & implementation':26};
for(const [lang,dict] of Object.entries(all))for(const [key,index] of Object.entries(aliases))dict[key]=box.locales.values[lang][index];
const strings=new Map();for(const file of fs.readdirSync(path.join(r,'upstream/src')).filter(n=>/\.(c|inc)$/.test(n))){const text=fs.readFileSync(path.join(r,'upstream/src',file),'utf8');const rex=/Nxt_text\(\s*("(?:[^"\\]|\\.)*")\s*,\s*("(?:[^"\\]|\\.)*")\s*\)/g;for(const m of text.matchAll(rex)){try{strings.set(JSON.parse(m[1]),JSON.parse(m[2]));}catch(e){}}}
for(const [en,vi] of Object.entries({'Discover':'Khám phá','Chill':'Thư giãn','Focus':'Tập trung','Happy':'Vui vẻ','Sad':'Buồn','Sleep':'Ngủ','Workout':'Tập luyện','Travel':'Du lịch','Romantic':'Lãng mạn','Energy':'Năng lượng','Vietnam pop':'Nhạc Việt','Thai pop':'Nhạc Thái','Acoustic':'Mộc','Classical':'Cổ điển','Ambient':'Không gian','Folk':'Dân gian','Retro':'Hoài cổ','Worldwide':'Toàn cầu','Vietnam':'Việt Nam','South Korea':'Hàn Quốc','Japan':'Nhật Bản','United States':'Mỹ','United Kingdom':'Anh','France':'Pháp','Thailand':'Thái Lan','Germany':'Đức','China':'Trung Quốc','Taiwan':'Đài Loan','Italy':'Ý','Indonesia':'Indonesia','Russia':'Nga','Spain':'Tây Ban Nha'}))strings.set(en,vi);
all.en={};all.vi={};for(const [en,vi] of strings){all.en[en]=en;all.vi[en]=vi;}
const extras={
 'Play':['Lire','Играть','재생','再生','เล่น','播放','播放','Abspielen','Reproducir','Reproduzir','Riproduci','Putar'],
 'Pause':['Pause','Пауза','일시 정지','一時停止','หยุดชั่วคราว','暂停','暫停','Pause','Pausa','Pausa','Pausa','Jeda'],
 'Previous':['Précédent','Назад','이전','前へ','ก่อนหน้า','上一首','上一首','Zurück','Anterior','Anterior','Precedente','Sebelumnya'],
 'Next':['Suivant','Далее','다음','次へ','ถัดไป','下一首','下一首','Weiter','Siguiente','Próximo','Successivo','Berikutnya'],
 'Queue':['File d’attente','Очередь','재생 목록','再生キュー','คิวเพลง','播放队列','播放佇列','Warteschlange','Cola','Fila','Coda','Antrean'],
 'Lyrics':['Paroles','Текст песни','가사','歌詞','เนื้อเพลง','歌词','歌詞','Liedtext','Letra','Letras','Testo','Lirik'],
 'Options':['Options','Параметры','옵션','オプション','ตัวเลือก','选项','選項','Optionen','Opciones','Opções','Opzioni','Opsi'],
 'Shuffle':['Aléatoire','Случайный порядок','셔플','シャッフル','สุ่มเพลง','随机播放','隨機播放','Zufällig','Aleatorio','Aleatório','Casuale','Acak'],
 'Repeat':['Répétition','Повтор','반복','リピート','เล่นซ้ำ','循环播放','循環播放','Wiederholen','Repetir','Repetir','Ripeti','Ulangi'],
 'Sleep timer':['Minuterie','Таймер сна','취침 타이머','スリープタイマー','ตั้งเวลาปิดเพลง','定时停止','定時停止','Sleep-Timer','Temporizador','Temporizador','Timer di spegnimento','Timer tidur'],
 'On':['Activé','Вкл.','켜짐','オン','เปิด','开启','開啟','Ein','Activado','Ligado','Attivo','Aktif'],
 'Off':['Désactivé','Выкл.','꺼짐','オフ','ปิด','关闭','關閉','Aus','Desactivado','Desligado','Disattivo','Nonaktif'],
 'Library':['Bibliothèque','Библиотека','라이브러리','ライブラリ','คลังเพลง','音乐库','音樂庫','Bibliothek','Biblioteca','Biblioteca','Libreria','Pustaka'],
 'LEDs':['LED','Светодиоды','LED','LED','LED','LED','LED','LED','LED','LED','LED','LED'],
 'Favorite':['Favori','Избранное','즐겨찾기','お気に入り','รายการโปรด','收藏','收藏','Favorit','Favorito','Favorito','Preferito','Favorit']};
const langs=Object.keys(box.locales.values).filter(l=>!['en','vi'].includes(l));for(const [key,vs] of Object.entries(extras))for(let i=0;i<langs.length;i++)all[langs[i]][key]=vs[i];
const genfile=path.join(r,'design/v1.8/locale-generated.json');if(fs.existsSync(genfile)){const gen=JSON.parse(fs.readFileSync(genfile));for(const [lang,dict] of Object.entries(gen))all[lang]={...dict,...all[lang]};}
fs.mkdirSync(path.join(r,'upstream/res/locales'),{recursive:true});for(const [lang,dict] of Object.entries(all))fs.writeFileSync(path.join(r,'upstream/res/locales',lang+'.json'),JSON.stringify(dict,null,2)+'\n');
fs.writeFileSync(path.join(r,'design/v1.8/locale-source.json'),JSON.stringify([...strings.keys()],null,2));console.log('Prepared',Object.keys(all).length,'locales and',strings.size,'source strings');
