(function () {
  const root = document.getElementById('nxt18-preview');
  if(!root) return;
  const carousel=root.querySelector('#nxt18-theme-carousel'),device=root.querySelector('#nxt18-flow-device');
  const assets=__NXT_EMOJI_JSON__;
  const nativeIcons=__NXT_ICON_JSON__;
  const E=NXTDesign.esc;
  const state={locale:'en',page:'intro',entered:false,profile:0,mood:'calm',genre:'Jazz',country:'Worldwide',source:'Local',text:'',raw:'',layout:'native',group:'',query:'',help:'',wifi:false,usb:false,led:'#93cbb4',draft:null,avatarType:'emoji',iconPage:0,ko:{l:-1,v:-1,t:0}};
  const profiles=[{id:0,name:'Spring Thông',avatar:'sunglasses',type:'emoji'},{id:1,name:'Listener',avatar:'cat',type:'emoji'},{id:2,name:'Guest',avatar:'moon',type:'emoji'}];
  const langs=[['en','English'],['vi','Tiếng Việt'],['fr','Français'],['ru','Русский'],['ko','한국어'],['ja','日本語'],['th','ไทย'],['zh-Hans','简体中文'],['zh-Hant','繁體中文'],['de','Deutsch'],['es','Español'],['pt-BR','Português (Brasil)'],['it','Italiano'],['id','Bahasa Indonesia']];
  const keys=['choose','chooseSub','mood','genre','country','next','back','forYou','profile','settings','help','about','transfer','languages','keyboard','save','today','source','edit','reenter','search','audio','appearance','data','updates','support','idea','nickname','avatar','space','erase','clear'];
  const values={
    en:['Choose your listener','Your music. Your space.','How are you feeling today?','What would you like to hear?','Music from where?','Continue','Back','For you','Profile','Settings','Guide','About','File transfer','Language & keyboard','Try your keyboard','Save','Today’s listening','Source','Edit profile','Choose listener','Find a setting','Playback & audio','Appearance & LED','Library & files','Updates','Support via PayPal','Idea & implementation','Nickname','Avatar','Space','Delete','Clear'],
    vi:['Chọn người nghe','Âm nhạc của bạn. Không gian của bạn.','Hôm nay bạn thấy thế nào?','Bạn muốn nghe thể loại gì?','Bạn muốn nghe nhạc nước nào?','Tiếp tục','Quay lại','Dành cho bạn','Hồ sơ','Cài đặt','Hướng dẫn','Thông tin','Chuyển file','Ngôn ngữ & bàn phím','Thử bàn phím','Lưu','Lựa chọn nghe hôm nay','Nguồn nhạc','Sửa hồ sơ','Chọn người nghe','Tìm tùy chọn','Phát nhạc & âm thanh','Giao diện & LED','Thư viện & file','Cập nhật','Ủng hộ qua PayPal','Ý tưởng & thực hiện','Biệt danh','Avatar','Dấu cách','Xóa','Xóa hết'],
    fr:['Choisir un profil','Votre musique. Votre espace.','Comment vous sentez-vous ?','Quel genre voulez-vous écouter ?','De quel pays ?','Continuer','Retour','Pour vous','Profil','Réglages','Guide','À propos','Transfert de fichiers','Langue et clavier','Essayer le clavier','Enregistrer','Votre écoute du jour','Source','Modifier le profil','Choisir un profil','Rechercher un réglage','Lecture et audio','Apparence et LED','Bibliothèque et fichiers','Mises à jour','Soutenir via PayPal','Idée et réalisation','Pseudonyme','Avatar','Espace','Supprimer','Effacer'],
    ru:['Выберите профиль','Ваша музыка. Ваше пространство.','Какое у вас сегодня настроение?','Какой жанр вы хотите слушать?','Музыку какой страны?','Продолжить','Назад','Для вас','Профиль','Настройки','Руководство','О приложении','Передача файлов','Язык и клавиатура','Проверить клавиатуру','Сохранить','Музыка на сегодня','Источник','Изменить профиль','Выбрать профиль','Найти настройку','Воспроизведение и звук','Оформление и LED','Библиотека и файлы','Обновления','Поддержать через PayPal','Идея и разработка','Псевдоним','Аватар','Пробел','Удалить','Очистить'],
    ko:['프로필 선택','나만의 음악, 나만의 공간','오늘 기분은 어떤가요?','어떤 장르를 듣고 싶으세요?','어느 나라 음악인가요?','계속','뒤로','추천','프로필','설정','사용 안내','앱 정보','파일 전송','언어 및 키보드','키보드 사용해 보기','저장','오늘의 음악','음악 소스','프로필 편집','프로필 선택','설정 검색','재생 및 오디오','테마 및 LED','라이브러리 및 파일','업데이트','PayPal로 후원','기획 및 제작','닉네임','아바타','공백','삭제','모두 지우기'],
    ja:['プロフィールを選択','あなたの音楽、あなたの空間','今日の気分はどうですか？','どんなジャンルを聴きたいですか？','どの国の音楽ですか？','続ける','戻る','おすすめ','プロフィール','設定','使い方','アプリ情報','ファイル転送','言語とキーボード','キーボードを試す','保存','今日の音楽','音楽ソース','プロフィール編集','プロフィールを選択','設定を検索','再生と音声','テーマとLED','ライブラリとファイル','更新','PayPalで支援','企画・制作','ニックネーム','アバター','スペース','削除','クリア'],
    th:['เลือกโปรไฟล์','เพลงของคุณ พื้นที่ของคุณ','วันนี้คุณรู้สึกอย่างไร?','อยากฟังเพลงแนวไหน?','อยากฟังเพลงจากประเทศไหน?','ต่อไป','กลับ','สำหรับคุณ','โปรไฟล์','ตั้งค่า','คู่มือ','เกี่ยวกับแอป','โอนไฟล์','ภาษาและแป้นพิมพ์','ลองแป้นพิมพ์','บันทึก','เพลงสำหรับวันนี้','แหล่งเพลง','แก้ไขโปรไฟล์','เลือกโปรไฟล์','ค้นหาการตั้งค่า','การเล่นและเสียง','ธีมและ LED','คลังเพลงและไฟล์','อัปเดต','สนับสนุนผ่าน PayPal','แนวคิดและการพัฒนา','ชื่อเล่น','รูปโปรไฟล์','เว้นวรรค','ลบ','ล้างทั้งหมด'],
    'zh-Hans':['选择个人资料','你的音乐，你的空间','今天心情怎么样？','想听什么类型的音乐？','想听哪个国家的音乐？','继续','返回','为你推荐','个人资料','设置','使用指南','关于','文件传输','语言与键盘','试用键盘','保存','今天的音乐','音乐来源','编辑个人资料','选择个人资料','搜索设置','播放与音频','主题与LED','音乐库与文件','更新','通过PayPal支持','创意与制作','昵称','头像','空格','删除','清空'],
    'zh-Hant':['選擇個人資料','你的音樂，你的空間','今天心情怎麼樣？','想聽什麼類型的音樂？','想聽哪個國家的音樂？','繼續','返回','為你推薦','個人資料','設定','使用指南','關於','檔案傳輸','語言與鍵盤','試用鍵盤','儲存','今天的音樂','音樂來源','編輯個人資料','選擇個人資料','搜尋設定','播放與音訊','主題與LED','音樂庫與檔案','更新','透過PayPal支持','創意與製作','暱稱','頭像','空格','刪除','清空'],
    de:['Profil auswählen','Deine Musik. Dein Raum.','Wie fühlst du dich heute?','Welches Genre möchtest du hören?','Musik aus welchem Land?','Weiter','Zurück','Für dich','Profil','Einstellungen','Anleitung','Über die App','Dateiübertragung','Sprache und Tastatur','Tastatur ausprobieren','Speichern','Musik für heute','Quelle','Profil bearbeiten','Profil auswählen','Einstellung suchen','Wiedergabe und Audio','Design und LED','Bibliothek und Dateien','Updates','Mit PayPal unterstützen','Idee und Umsetzung','Spitzname','Avatar','Leerzeichen','Löschen','Leeren'],
    es:['Elige tu perfil','Tu música. Tu espacio.','¿Cómo te sientes hoy?','¿Qué género quieres escuchar?','¿Música de qué país?','Continuar','Volver','Para ti','Perfil','Ajustes','Guía','Acerca de','Transferir archivos','Idioma y teclado','Probar el teclado','Guardar','Tu música de hoy','Fuente','Editar perfil','Elegir perfil','Buscar un ajuste','Reproducción y audio','Apariencia y LED','Biblioteca y archivos','Actualizaciones','Apoyar con PayPal','Idea y realización','Apodo','Avatar','Espacio','Borrar','Limpiar'],
    'pt-BR':['Escolha seu perfil','Sua música. Seu espaço.','Como você se sente hoje?','Que gênero quer ouvir?','Música de qual país?','Continuar','Voltar','Para você','Perfil','Configurações','Guia','Sobre','Transferir arquivos','Idioma e teclado','Testar o teclado','Salvar','Sua música de hoje','Fonte','Editar perfil','Escolher perfil','Buscar configuração','Reprodução e áudio','Aparência e LED','Biblioteca e arquivos','Atualizações','Apoiar pelo PayPal','Ideia e realização','Apelido','Avatar','Espaço','Apagar','Limpar'],
    it:['Scegli il profilo','La tua musica. Il tuo spazio.','Come ti senti oggi?','Quale genere vuoi ascoltare?','Musica di quale paese?','Continua','Indietro','Per te','Profilo','Impostazioni','Guida','Informazioni','Trasferisci file','Lingua e tastiera','Prova la tastiera','Salva','La musica di oggi','Sorgente','Modifica profilo','Scegli il profilo','Cerca un’impostazione','Riproduzione e audio','Aspetto e LED','Libreria e file','Aggiornamenti','Supporta con PayPal','Idea e realizzazione','Soprannome','Avatar','Spazio','Elimina','Cancella'],
    id:['Pilih profil','Musikmu. Ruangmu.','Bagaimana perasaanmu hari ini?','Genre apa yang ingin didengar?','Musik dari negara mana?','Lanjut','Kembali','Untukmu','Profil','Pengaturan','Panduan','Tentang','Transfer berkas','Bahasa & papan ketik','Coba papan ketik','Simpan','Musik hari ini','Sumber','Edit profil','Pilih profil','Cari pengaturan','Pemutaran & audio','Tema & LED','Pustaka & berkas','Pembaruan','Dukung melalui PayPal','Ide & pengembangan','Nama panggilan','Avatar','Spasi','Hapus','Bersihkan']
  };
  const T=k=>values[state.locale][keys.indexOf(k)]||values.en[keys.indexOf(k)]||k;
  const vi=(en,v)=>state.locale==='vi'?v:en;
  const icon=name=>'<i data-lucide="'+E(name)+'" aria-hidden="true"></i>';
  const iconNames=['headphones','disc-3','heart','music-2','gamepad-2','mic-2','radio','podcast','moon','sun-moon','book-open','zap','camera','coffee','flower-2','leaf','mountain','waves','rocket','star','sparkles','smile','cat','dog','bird','fish','rabbit','turtle','squirrel','bug','butterfly','paw-print','apple','banana','cherry','grape','citrus','carrot','salad','pizza','cake','cookie','ice-cream-bowl','cup-soda','utensils','chef-hat','umbrella','cloud','cloud-rain','cloud-sun','cloud-moon','snowflake','wind','sun','sunrise','sunset','rainbow','droplets','flame','tree-pine','trees','sprout','clover','cactus','palm-tree','tent','compass','map','map-pin','navigation','globe','plane','train-front','tram-front','bus','car','bike','ship','sailboat','anchor','footprints','backpack','luggage','ticket','building','house','castle','church','landmark','store','hotel','school','library','graduation-cap','book','notebook','notebook-pen','pencil','pen-tool','paintbrush','palette','brush','scissors','ruler','eraser','highlighter','lightbulb','lamp','lamp-desk','lightbulb-off','flashlight','battery','battery-charging','plug','usb','bluetooth','wifi','wifi-off','cable','monitor','laptop','smartphone','tablet','watch','keyboard','mouse','printer','cpu','hard-drive','memory-stick','save','folder','folder-open','folder-heart','folder-music','file','file-audio','file-image','file-text','file-music','files','download','upload','cloud-download','cloud-upload','package','box','gift','shopping-bag','shopping-cart','wallet','credit-card','coins','banknote','piggy-bank','gem','crown','medal','award','trophy','badge','shield','shield-check','lock','unlock','key','fingerprint','scan-face','user','users','user-round','contact','id-card','person-standing','accessibility','hand','thumbs-up','thumbs-down','handshake','heart-handshake','heart-pulse','activity','stethoscope','syringe','pill','thermometer','bone','brain','eye','ear','smile-plus','laugh','frown','meh','angry','annoyed','circle-user','circle-user-round','baby','alarm-clock','clock','timer','hourglass','calendar','calendar-days','calendar-check','calendar-heart','calendar-clock','check','check-check','circle-check','circle-x','circle-help','info','circle-alert','triangle-alert','circle-plus','circle-minus','plus','minus','x','search','zoom-in','zoom-out','filter','list-filter','sliders-horizontal','sliders-vertical','settings','settings-2','wrench','hammer','screwdriver','nut','cog','gauge','speedometer','volume','volume-1','volume-2','volume-x','speaker','audio-lines','audio-waveform','mic','mic-vocal','mic-off','headset','headphone-off','play','pause','skip-back','skip-forward','rewind','fast-forward','repeat','repeat-1','shuffle','list-music','list-video','list','list-checks','layout-grid','layout-dashboard','layout-panel-left','layout-template','columns-2','rows-2','grid-2x2','grid-3x3','square','circle','triangle','diamond','hexagon','octagon','pentagon','heart-crack','heart-off','disc','disc-2','disc-album','drum','drumstick','guitar','piano','music','music-3','music-4','cassette-tape','vinyl','film','video','clapperboard','camera-off','image','images','image-plus','image-play','image-up','scan','focus','crosshair','target','flag','flag-triangle-left','flag-triangle-right','puzzle','dices','dice-1','dice-2','dice-3','dice-4','dice-5','dice-6','joystick','sword','swords','wand','wand-sparkles','sparkle','orbit','satellite','satellite-dish','telescope','atom','flask-conical','flask-round','test-tube','beaker','dna','magnet','triangle-right','circle-dot','circle-dashed','circle-stop','square-play','square-pause'];
  const iconCatalog=Object.keys(nativeIcons).slice(0,300);
  function hydrateIcons(){
    if(globalThis.lucide){globalThis.lucide.createIcons({attrs:{width:22,height:22}});}
    root.querySelectorAll('i[data-lucide]').forEach(n=>{const name=n.getAttribute('data-lucide'),src=nativeIcons[name];if(src){const im=document.createElement('img');im.src=src;im.alt='';im.setAttribute('aria-hidden','true');n.replaceWith(im);}else n.textContent='○';});
  }
  function avatar(p,cls='') {return '<span class="nxt18-avatar '+cls+'">'+(p.type==='icon'?icon(p.avatar):'<img src="'+E(assets[p.avatar]||assets.sunglasses)+'" alt="">')+'</span>';}
  function action(a,label,extra='',primary=false) {return '<button type="button" class="nxt18-action cursor-interaction '+(primary?'nxt18-primary':'')+'" data-action="'+a+'" '+extra+'>'+label+'</button>';}
  function nativeNav(){return state.entered?'<nav class="nxt18-native-nav" aria-label="NXT">'+[['forYou','forYou'],['profile','profile'],['settings','settings'],['help','help'],['about','about']].map(([p,k])=>'<button type="button" class="cursor-interaction '+(state.page===p?'is-selected':'')+'" data-action="page" data-page="'+p+'">'+E(T(k))+'</button>').join('')+'</nav>':'';}
  function title(k,sub=''){return '<h2 class="nxt18-flow-title">'+E(T(k))+'</h2>'+(sub?'<p class="nxt18-subtitle">'+E(sub)+'</p>':'');}
  function bodyHead(){
    const p=profiles[state.profile];
    return '<header class="nxt18-flow-head"><div class="nxt18-flow-brand"><span>NXT</span><span class="nxt18-brand-caption">RETRO MUSIC · 1.8</span></div><div class="nxt18-user" data-tooltip="'+E(p.name)+'">'+avatar(p)+'<span class="nxt18-user-name">'+E(p.name)+'</span></div></header>';
  }
  function choices(field,options){return '<div class="nxt18-options">'+options.map(o=>'<button type="button" class="nxt18-choice cursor-interaction '+(state[field]===o[0]?'is-selected':'')+'" data-action="choice" data-field="'+field+'" data-value="'+E(o[0])+'" aria-pressed="'+(state[field]===o[0])+'">'+(o[2]?'<img src="'+assets[o[2]]+'" alt="">':'')+'<span>'+E(o[1])+'</span></button>').join('')+'</div>';}
  const moods=()=>[['calm',vi('Relaxed','Thư thái'),'calm'],['happy',vi('Happy','Vui vẻ'),'smile'],['focus',vi('Focused','Tập trung'),'sunglasses'],['low',vi('Low energy','Hơi mệt'),'sleep'],['dreamy',vi('Dreamy','Mơ màng'),'moon'],['adventure',vi('Ready to explore','Muốn khám phá'),'rocket']];
  const genres=()=>['Jazz','Acoustic','Lo-fi','Pop','Rock','Electronic'].map(x=>[x,x,'music']);
  const countries=()=>[['Worldwide',vi('Worldwide','Toàn cầu')],['Vietnam',vi('Vietnam','Việt Nam')],['Korea',vi('Korea','Hàn Quốc')],['Japan',vi('Japan','Nhật Bản')],['USA',vi('United States','Mỹ')],['UK',vi('United Kingdom','Anh')],['France',vi('France','Pháp')],['Thailand',vi('Thailand','Thái Lan')],['Germany',vi('Germany','Đức')]];
  function onboarding(page){
    const order=['mood','genre','country'],i=order.indexOf(page),options=page==='mood'?moods():page==='genre'?genres():countries();
    return title(page,T('today'))+'<div class="nxt18-steps" aria-label="'+(i+1)+' / 3">'+order.map((_,j)=>'<span class="nxt18-step '+(i===j?'is-current':'')+'"></span>').join('')+'</div>'+choices(page,options)+'<div class="nxt18-bottom-row">'+action('onboarding-back',E(T('back')))+action('onboarding-next',E(T('next')),'',true)+'</div>';
  }
  let albumId=0;
  function albumThumb(art='art'){const z=NXTDesign.renderTheme(0).svg,p='flow-art-'+(++albumId);const defs=z.slice(z.indexOf('<defs>'),z.indexOf('</defs>')+7).replaceAll('nxt18-0',p);return '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" aria-hidden="true">'+defs+'<use href="#'+p+'-'+art+'" width="512" height="512"/></svg>';}
  function forYou(){
    return nativeNav()+title('forYou',moods().find(x=>x[0]===state.mood)?.[1]+' · '+state.genre+' · '+(countries().find(x=>x[0]===state.country)?.[1]||state.country))+
      '<div class="nxt18-source-row" aria-label="'+E(T('source'))+'">'+['Local','Spotify','SoundCloud'].map(x=>'<button type="button" class="cursor-interaction '+(state.source===x?'is-selected':'')+'" data-action="source" data-value="'+x+'">'+E(x==='Local'?vi('On your SD card','Nhạc trong thẻ'):x)+'</button>').join('')+'</div>'+
      '<div class="nxt18-content-grid">'+[
        [state.genre+' · '+(countries().find(x=>x[0]===state.country)?.[1]||state.country),vi('Your mix for today','Bản phối cho hôm nay'),'art'],
        [vi('A softer day','Một ngày dịu hơn'),vi('A playlist for your mood','Playlist theo tâm trạng'),'gold'],
        ['Blue Hour','Spring Sessions','art'],
        ['Golden Afternoon','Spring Sessions','gold']
      ].map((x,i)=>'<button type="button" class="nxt18-album-tile cursor-interaction" data-action="sample-play" data-value="'+i+'">'+albumThumb(x[2])+'<span><strong>'+E(x[0])+'</strong><small>'+E(x[1])+'</small><small>'+E(state.source==='Local'?vi('Local music','Nhạc trong thẻ'):state.source)+'</small></span></button>').join('')+'</div>'+
      '<div class="nxt18-bottom-row">'+action('edit-today',E(T('today')))+action('page',E(T('profile')),'data-page="profile"')+'</div>';
  }
  const groups=()=>[['audio','audio','USB DAC · EQ · ReplayGain'],['appearance','appearance','36 themes · RGB · LED'],['profile','profile',T('avatar')+' · '+T('today')],['languages','languages',langs.find(x=>x[0]===state.locale)[1]],['transfer','data','USB-C · Wi-Fi'],['about','about',T('help')+' · OTA · PayPal']];
  function settings(){
    const q=state.query.toLocaleLowerCase();return nativeNav()+title('settings')+
      '<input type="text" aria-label="'+E(T('search'))+'" placeholder="'+E(T('search'))+'" data-input="search" value="'+E(state.query)+'">'+
      '<div class="nxt18-setting-list" id="nxt18-settings-results">'+groups().filter(x=>(T(x[1])+' '+x[2]).toLocaleLowerCase().includes(q)).map(x=>'<button type="button" class="nxt18-setting-row cursor-interaction" data-action="group" data-value="'+x[0]+'"><span>'+E(T(x[1]))+'</span><span>'+E(x[2])+'</span></button>').join('')+'</div>';
  }
  function settingsGroup(){
    let rows=[],extra='';
    if(state.group==='audio')rows=[['USB DAC',vi('Audio output','Đầu ra âm thanh')],['Jazz · 5 bands',vi('EQ & DSP','EQ & DSP')],['Album', 'ReplayGain'],['On',vi('Gapless & resume','Gapless & mở lại phiên')],['Off',vi('Sleep timer','Hẹn giờ')]];
    if(state.group==='appearance') {
      rows=[['36',vi('Full interfaces','Giao diện hoàn chỉnh')],['Original NXT',vi('RGB & wallpaper','RGB & hình nền')],[vi('Balanced','Cân bằng'),vi('Motion','Chuyển động')]];
      extra='<div class="nxt18-led"><label for="nxt18-led-color">LED</label><input type="color" id="nxt18-led-color" value="'+state.led+'" data-input="led" aria-label="LED color"><span class="nxt18-led-preview" style="--nxt-led:'+state.led+'"></span><span>'+vi('Live color','Đổi ngay')+'</span></div>';
    }
    return nativeNav()+title(state.group)+
      '<div class="nxt18-setting-list">'+rows.map(x=>'<div class="nxt18-setting-row"><span>'+E(x[1])+'</span><span>'+E(x[0])+'</span></div>').join('')+'</div>'+extra+action('page',E(T('back')),'data-page="settings"');
  }
  function profile(){
    const p=profiles[state.profile];return nativeNav()+title('profile')+'<div class="nxt18-about">'+
      '<div><strong>'+E(p.name)+'</strong><p class="nxt18-note">'+E(T('today'))+' · '+E(state.genre)+' · '+E(state.country)+'</p>'+action('edit-profile',E(T('edit')),'',true)+'</div>'+avatar(p)+'</div>'+
      '<div class="nxt18-inline-actions">'+action('reenter',E(T('reenter')))+action('edit-today',E(T('today')))+action('page',E(T('languages')),'data-page="languages"')+'</div>';
  }
  function editProfile(){
    const p=state.draft;return title('edit')+'<label class="nxt18-subtitle" for="nxt18-nickname">'+E(T('nickname'))+'</label>'+
      '<input type="text" id="nxt18-nickname" data-input="nickname" value="'+E(p.name)+'" maxlength="48">'+
      '<div class="nxt18-source-row" style="margin-top:18px">'+action('avatar-type','3D emoji','data-value="emoji"')+action('avatar-type',vi('Icons','Biểu tượng'),'data-value="icon"')+'</div>'+
      '<div class="nxt18-avatar-grid">'+(state.avatarType==='emoji'?Object.keys(assets).map(name=>'<button type="button" class="nxt18-avatar-option cursor-interaction '+(p.type==='emoji'&&p.avatar===name?'is-selected':'')+'" data-action="avatar" data-value="'+name+'"><img src="'+assets[name]+'" alt="'+E(name)+'"></button>').join(''):
        iconCatalog.slice(state.iconPage*16,state.iconPage*16+16).map(name=>'<button type="button" class="nxt18-avatar-option cursor-interaction '+(p.type==='icon'&&p.avatar===name?'is-selected':'')+'" data-action="avatar" data-value="'+E(name)+'" aria-label="'+E(name)+'">'+icon(name)+'</button>').join(''))+'</div>'+
      (state.avatarType==='icon'?'<div class="nxt18-inline-actions">'+action('icon-prev',E(T('back')))+'<span>'+E((state.iconPage+1)+' / '+Math.ceil(iconCatalog.length/16))+'</span>'+action('icon-next',E(T('next')))+'</div>':'')+
      '<div class="nxt18-bottom-row">'+action('cancel-profile',E(T('back')))+action('save-profile',E(T('save')),'',true)+'</div>';
  }
  const helpNames=()=>[
    [vi('Local music','Nhạc trong thẻ'),[vi('Copy music into your music folder.','Chép nhạc vào thư mục nhạc.'),vi('Open Library and scan new files.','Mở Thư viện và quét file mới.'),vi('Choose an album or a track to play.','Chọn album hoặc bài hát để phát.')]],
    [vi('Online music','Nghe nhạc online'),[vi('Connect Wi-Fi, then open Online music.','Kết nối Wi-Fi, rồi mở Nhạc online.'),vi('For Spotify, sign in with a Premium account.','Với Spotify, đăng nhập tài khoản Premium.'),vi('Connect SoundCloud and choose available full tracks.','Kết nối SoundCloud và chọn bài đầy đủ khả dụng.')]],
    [T('profile'),[vi('Choose your listener when starting NXT.','Chọn người nghe khi mở NXT.'),vi('Choose mood, genre and country.','Chọn tâm trạng, thể loại và quốc gia.'),vi('Open For you to see your mix and suggestions.','Mở Dành cho bạn để xem playlist và bài gợi ý.')]],
    [vi('Buttons','Phím điều khiển'),[vi('D-pad: navigate. A: confirm. B: back.','D-pad: điều hướng. A: xác nhận. B: quay lại.'),'SELECT: EQ · L2: Spectrum · R2: Favorite',vi('Turn the display off and keep listening.','Tắt màn hình vẫn tiếp tục nghe nhạc.')]],
    [vi('Audio & EQ','Âm thanh & EQ'),[vi('Open Settings / Playback & audio.','Mở Cài đặt / Phát nhạc & âm thanh.'),vi('Choose a preset or adjust five bands.','Chọn preset hoặc chỉnh năm dải.'),vi('The player shows the active audio output.','Trình phát hiển thị đầu ra đang dùng.')]],
    [vi('Themes & LED','Giao diện & LED'),[vi('Open Settings / Appearance & LED.','Mở Cài đặt / Giao diện & LED.'),vi('Preview a theme before applying it.','Xem trước theme rồi áp dụng.'),vi('LED color and mode changes apply immediately.','LED đổi màu và chế độ ngay khi chọn.')]],
    [T('transfer'),[vi('Use a data cable for USB-C transfers.','Dùng cáp dữ liệu cho chuyển file USB-C.'),vi('Eject the drive on your computer before returning.','Tháo ổ an toàn trên máy tính trước khi quay lại.'),vi('For Wi-Fi, open the pairing address on the same network.','Với Wi-Fi, mở địa chỉ ghép nối trên cùng mạng.')]],
    [T('languages'),[vi('Choose your language in Settings.','Chọn ngôn ngữ trong Cài đặt.'),vi('The keyboard follows the selected language.','Bàn phím theo ngôn ngữ đã chọn.'),vi('Use Latin mode for addresses and filenames.','Dùng chế độ Latin cho địa chỉ và tên file.')]],
    [T('updates'),[vi('Open About / Updates.','Mở Thông tin / Cập nhật.'),vi('Use the update link for your ROM.','Dùng link cập nhật đúng ROM.'),vi('Keep power connected during installation.','Giữ nguồn điện ổn định khi cài đặt.')]]
  ];
  function help(){
    const data=helpNames();let detail=state.help!==''?data[Number(state.help)]:null;
    return nativeNav()+title('help')+'<div class="nxt18-options">'+data.map((x,i)=>'<button type="button" class="nxt18-choice cursor-interaction" data-action="help-detail" data-value="'+i+'">'+E(x[0])+'</button>').join('')+'</div>'+
      (detail?'<section class="nxt18-help-detail" aria-live="polite"><h3 style="font-weight:500;margin:0">'+E(detail[0])+'</h3><ol>'+detail[1].map(x=>'<li>'+E(x)+'</li>').join('')+'</ol></section>':'');
  }
  function about(){
    return nativeNav()+title('about')+'<div class="nxt18-about"><div><p class="nxt18-note">'+E(T('idea'))+'</p><strong>Nguyễn Xuân Thông</strong><p style="font-size:20px;color:#bbcfb5">Spring Thông</p><p class="nxt18-note">NXT Retro Music · v1.8<br>Vibe Code</p></div><img src="'+assets.music+'" alt=""></div>'+
      '<div class="nxt18-setting-row"><span>'+E(T('support'))+'</span><span>thongkhung2@gmail.com</span></div>'+
      '<div class="nxt18-inline-actions" style="margin-top:18px">'+action('page',E(T('help')),'data-page="help"')+action('updates',E(T('updates')))+
      '<a class="nxt18-action cursor-interaction" href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&amp;business=thongkhung2%40gmail.com&amp;item_name=NXT%20Retro%20Music&amp;currency_code=USD" target="_blank" rel="noopener noreferrer">PayPal</a></div>';
  }
  function transfer(){
    return nativeNav()+title('transfer')+'<div class="nxt18-transfer-grid"><section class="nxt18-transfer"><h3>USB-C</h3><p class="nxt18-note">'+vi('Use a data cable to copy music from your computer.','Dùng cáp dữ liệu để chép nhạc từ máy tính.')+'</p>'+action('usb',state.usb?vi('Return to NXT','Quay lại NXT'):vi('Open USB transfer','Mở chuyển file USB'),'',true)+
      (state.usb?'<p class="nxt18-note" aria-live="polite">'+vi('Save playback → USB transfer → eject on computer → return to NXT.','Lưu phiên nghe → chuyển USB → tháo ổ an toàn trên máy tính → trở lại NXT.')+'</p>':'')+'</section>'+
      '<section class="nxt18-transfer"><h3>Wi-Fi</h3><p class="nxt18-note">'+vi('Copy from a phone or computer on the same network.','Chép từ điện thoại hoặc máy tính cùng mạng.')+'</p>'+action('wifi',state.wifi?vi('Stop sharing','Tắt chia sẻ'):vi('Start sharing','Bật chia sẻ'),'',true)+
      (state.wifi?'<p class="nxt18-note" aria-live="polite">http://192.168.1.42:8090<br>PIN · 482 619</p>':'')+'</section></div>'+
      '<p class="nxt18-note">'+vi('Music · album covers · lyrics','Nhạc · ảnh bìa · lời bài hát')+'</p>';
  }
  function languages(){
    return nativeNav()+title('languages')+'<div class="nxt18-lang-grid">'+langs.map(([code,name])=>'<button type="button" class="cursor-interaction '+(code===state.locale?'is-selected':'')+'" lang="'+code+'" data-action="language" data-value="'+code+'" aria-pressed="'+(code===state.locale)+'">'+E(name)+'</button>').join('')+'</div>'+
      '<div class="nxt18-bottom-row">'+action('page',E(T('back')),'data-page="'+(state.entered?'settings':'intro')+'"')+action('page',E(T('keyboard')),'data-page="keyboard"',true)+'</div>';
  }
  const letters={
    en:'qwertyuiopasdfghjklzxcvbnm',vi:'qwertyuiopasdfghjklzxcvbnmăâđêôơưáàảãạéèẻẽẹíìỉĩịóòỏõọúùủũụýỳỷỹỵ',
    fr:'azertyuiopqsdfghjklmwxcvbnéèêëàâçùûîïôœ',ru:'йцукенгшщзхъфывапролджэячсмитьбюё',
    ko:'ㅂㅈㄷㄱㅅㅛㅕㅑㅐㅔㅁㄴㅇㄹㅎㅗㅓㅏㅣㅋㅌㅊㅍㅠㅜㅡ',
    ja:'あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをんー',
    th:'ๆไำพะัีรนยบลฟหกดเ้่าสวงผปแอิืทมใฝฤฆธณญฐษศซฮฉ',
    'zh-Hans':'qwertyuiopasdfghjklzxcvbnm','zh-Hant':'qwertyuiopasdfghjklzxcvbnm',
    de:'qwertzuiopasdfghjklyxcvbnmäöüß',es:'qwertyuiopasdfghjklñzxcvbnmáéíóúü', 'pt-BR':'qwertyuiopasdfghjklçzxcvbnmáâãàéêíóôõú',
    it:'qwertyuiopasdfghjklzxcvbnmàèéìòù',id:'qwertyuiopasdfghjklzxcvbnm'
  };
  const chinese={ni:['你','妳'],hao:['好','号','浩'],nihao:['你好'],yinyue:['音乐'],xin:['心','新'],ting:['听'],wo:['我'],ai:['爱'],zhongguo:['中国'],riben:['日本'],hanguo:['韩国'],faguo:['法国'],jazz:['爵士'],pingjing:['平静'],gaoxing:['高兴'],pengyou:['朋友'],tong:['通','同','童']};
  const traditional={'音乐':'音樂','听':'聽','爱':'愛','中国':'中國','韩国':'韓國','法国':'法國','高兴':'高興','号':'號'};
  const japanese={'おんがく':['音楽'],'さくら':['桜'],'にほん':['日本'],'こんにちは':['こんにちは'],'わたし':['私'],'ともだち':['友達'],'かんこく':['韓国']};
  const romaji={a:'あ',i:'い',u:'う',e:'え',o:'お',ka:'か',ki:'き',ku:'く',ke:'け',ko:'こ',sa:'さ',shi:'し',su:'す',se:'せ',so:'そ',ta:'た',chi:'ち',tsu:'つ',te:'て',to:'と',na:'な',ni:'に',nu:'ぬ',ne:'ね',no:'の',ha:'は',hi:'ひ',fu:'ふ',he:'へ',ho:'ほ',ma:'ま',mi:'み',mu:'む',me:'め',mo:'も',ya:'や',yu:'ゆ',yo:'よ',ra:'ら',ri:'り',ru:'る',re:'れ',ro:'ろ',wa:'わ',wo:'を',ga:'が',gi:'ぎ',gu:'ぐ',ge:'げ',go:'ご',za:'ざ',ji:'じ',zu:'ず',ze:'ぜ',zo:'ぞ',da:'だ',de:'で',do:'ど',ba:'ば',bi:'び',bu:'ぶ',be:'べ',bo:'ぼ',pa:'ぱ',pi:'ぴ',pu:'ぷ',pe:'ぺ',po:'ぽ',kya:'きゃ',kyu:'きゅ',kyo:'きょ',sha:'しゃ',shu:'しゅ',sho:'しょ',cha:'ちゃ',chu:'ちゅ',cho:'ちょ'};
  const L=Array.from('ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ'),V=Array.from('ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ'),F=['',...Array.from('ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ')];
  function koChar(){const k=state.ko;return k.l<0?'':k.v<0?L[k.l]:String.fromCodePoint(0xac00+(k.l*21+k.v)*28+k.t);}
  function flushKo(){state.text+=koChar();state.ko={l:-1,v:-1,t:0};}
  function typeKo(ch){
    const k=state.ko,l=L.indexOf(ch),v=V.indexOf(ch),f=F.indexOf(ch);
    if(k.l<0){if(l>=0)k.l=l;else state.text+=ch;return;}
    if(k.v<0){if(v>=0)k.v=v;else{flushKo();if(l>=0)state.ko.l=l;else state.text+=ch;}return;}
    if(v>=0){
      if(k.t){let tail=F[k.t];k.t=0;flushKo();state.ko.l=L.indexOf(tail);state.ko.v=v;if(state.ko.l<0){state.text+=ch;state.ko.v=-1;}}
      else {const joins={'ㅗㅏ':'ㅘ','ㅗㅐ':'ㅙ','ㅗㅣ':'ㅚ','ㅜㅓ':'ㅝ','ㅜㅔ':'ㅞ','ㅜㅣ':'ㅟ','ㅡㅣ':'ㅢ'};const combined=joins[V[k.v]+ch];if(combined)k.v=V.indexOf(combined);else{flushKo();state.text+=ch;}}
    } else if(l>=0){if(k.t===0&&f>0)k.t=f;else{flushKo();state.ko.l=l;}}else {flushKo();state.text+=ch;}
  }
  function toKana(raw){
    let out='',i=0;while(i<raw.length){if(raw[i]===raw[i+1]&&!/[aeioun]/.test(raw[i])){out+='っ';i++;continue;}let found=false;
      for(let n=3;n>=1;n--){const x=raw.slice(i,i+n);if(romaji[x]){out+=romaji[x];i+=n;found=true;break;}}
      if(found)continue;if(raw[i]==='n'&&(i===raw.length-1||!/[aeiouy]/.test(raw[i+1]))){out+='ん';i++;continue;}out+=raw[i++];}return out;
  }
  function composed(){
    if(state.locale==='ko'&&state.layout==='native')return state.text+koChar();
    if(state.locale==='ja'&&state.layout==='romaji')return state.text+toKana(state.raw);
    return state.text+state.raw;
  }
  function candidates(){
    if(state.locale.startsWith('zh')&&state.layout==='native'){const matches=chinese[state.raw.toLowerCase()]||[];return state.locale==='zh-Hant'?matches.map(x=>traditional[x]||x):matches;}
    if(state.locale==='ja'){const s=state.layout==='romaji'?toKana(state.raw):state.text;return japanese[s]||[];}
    return [];
  }
  function keyboard(){
    const chars=state.layout==='latin'||state.layout==='romaji'?letters.en:letters[state.locale];
    return title('keyboard',langs.find(x=>x[0]===state.locale)[1])+
      '<input type="text" data-input="typed" aria-label="'+E(T('keyboard'))+'" value="'+E(composed())+'" lang="'+state.locale+'">'+
      '<div class="nxt18-candidates">'+candidates().map(x=>'<button type="button" class="cursor-interaction" data-action="candidate" data-value="'+E(x)+'">'+E(x)+'</button>').join('')+'</div>'+
      '<div class="nxt18-keyboard" aria-label="'+E(T('keyboard'))+'">'+Array.from(chars).map(ch=>'<button type="button" class="nxt18-key cursor-interaction" data-action="key" data-value="'+E(ch)+'">'+E(ch)+'</button>').join('')+'</div>'+
      '<div class="nxt18-inline-actions">'+action('space',E(T('space')))+action('erase',E(T('erase')))+action('clear',E(T('clear')))+action('layout',state.locale==='ja'?(state.layout==='romaji'?'かな':'Romaji'):(state.layout==='latin'?langs.find(x=>x[0]===state.locale)[1]:'Latin'))+'</div>'+
      '<div class="nxt18-bottom-row">'+action('page',E(T('back')),'data-page="languages"')+action('page',E(T('save')),'data-page="'+(state.entered?'profile':'intro')+'"',true)+'</div>';
  }
  function renderFlow(){
    let html='';
    if(state.page==='intro')html=title('choose',T('chooseSub'))+'<div class="nxt18-profile-grid">'+profiles.map((p,i)=>'<button type="button" class="nxt18-profile cursor-interaction" data-action="choose" data-value="'+i+'" aria-label="'+E(p.name)+'" data-tooltip="'+E(p.name)+'">'+avatar(p)+'<span>'+E(p.name)+'</span></button>').join('')+'</div>'+action('page',E(T('languages')),'data-page="languages"');
    else if(['mood','genre','country'].includes(state.page))html=onboarding(state.page);
    else if(state.page==='forYou')html=forYou();
    else if(state.page==='settings')html=settings();
    else if(state.page==='group')html=settingsGroup();
    else if(state.page==='profile')html=profile();
    else if(state.page==='editProfile')html=editProfile();
    else if(state.page==='help')html=help();
    else if(state.page==='about')html=about();
    else if(state.page==='transfer')html=transfer();
    else if(state.page==='languages')html=languages();
    else if(state.page==='keyboard')html=keyboard();
    else if(state.page==='updates')html=nativeNav()+title('updates')+'<div class="nxt18-setting-list"><div class="nxt18-setting-row"><span>Stock / NextUI</span><span>v1.8</span></div><div class="nxt18-setting-row"><span>Google Drive OTA</span><span>ZIP + update.json</span></div></div>'+action('page',E(T('back')),'data-page="about"');
    else if(state.page==='playingSample')html=title('forYou')+'<div class="nxt18-about"><div><strong>Blue Hour</strong><p class="nxt18-note">Spring Sessions · '+E(state.genre)+'</p></div><div class="nxt18-mini-album">'+albumThumb()+'</div></div>'+action('page',E(T('back')),'data-page="forYou"');
    device.innerHTML=bodyHead()+'<main class="nxt18-flow-body" lang="'+state.locale+'">'+html+'</main>';
    hydrateIcons();
  }
  const playState=Array.from({length:36},()=>({playing:false,index:0,shuffle:false,repeat:0,timer:0,favorites:[false,false,false],volume:60,panel:'',spectrum:true,eq:'Jazz',bands:[2,1,0,1,2],history:[]})),songs=['Blue Hour','Golden Afternoon','Rainlight'];
  function playerButton(id,key,label,iconName,extra='',contents='',keycap=''){
    return '<button type="button" class="nxt18-player-button cursor-interaction '+(contents?'nxt18-wide-button ':'')+(keycap?'nxt18-keyed-button ':'')+'" data-player="'+id+'" data-control="'+key+'" aria-label="'+E(label)+'" data-tooltip="'+E(label)+'" '+extra+'><span class="nxt18-button-content">'+icon(iconName)+contents+'</span>'+(keycap?'<span class="nxt18-key-label">'+E(keycap)+'</span>':'')+'</button>';
  }
  function playerPanel(id,st){
    if(!st.panel)return '';
    const heading={timer:'Sleep timer',queue:'Now playing queue',volume:'Volume',eq:'Equalizer',lyrics:'Lyrics',menu:'NXT menu',options:'Playback options'}[st.panel];let content='';
    const option=(key,value,label,on=false)=>'<button type="button" class="nxt18-panel-option cursor-interaction" data-player="'+id+'" data-control="'+key+'" data-value="'+E(value)+'" aria-pressed="'+on+'">'+E(label)+'</button>';
    if(st.panel==='timer')content='<div class="nxt18-panel-options">'+[0,15,30,45,60,90,120].map(n=>option('timer-set',n,n?n+' min':'Off',st.timer===n)).join('')+'</div><p class="nxt18-panel-caption">'+(st.timer?'Stop music after '+st.timer+' minutes.':'No sleep timer.')+'</p>';
    if(st.panel==='queue')content='<p class="nxt18-panel-caption" aria-live="polite">Track '+(st.index+1)+' of '+songs.length+' · '+(st.shuffle?'Shuffle on':'In order')+' · '+['Repeat off','Repeat all','Repeat one'][st.repeat]+'</p><div class="nxt18-queue-list">'+songs.map((s,i)=>'<button type="button" class="nxt18-panel-option nxt18-queue-track cursor-interaction" data-player="'+id+'" data-control="queue-track" data-value="'+i+'" aria-pressed="'+(i===st.index)+'"><span>'+String(i+1).padStart(2,'0')+'</span><span>'+E(s)+'</span><span>'+(['03:16','04:08','02:52'][i])+'</span></button>').join('')+'</div>';
    if(st.panel==='volume')content='<label for="nxt18-volume-'+id+'">Output volume · <output>'+st.volume+'%</output></label><input type="range" id="nxt18-volume-'+id+'" min="0" max="100" step="5" value="'+st.volume+'" class="nxt18-volume-slider" data-player-input="volume" data-player-id="'+id+'"><div class="nxt18-panel-options">'+option('volume-step',-5,'− 5%')+option('volume-step',5,'+ 5%')+'</div>';
    if(st.panel==='eq')content='<div class="nxt18-panel-options">'+['Flat','Jazz','Vocal'].map(n=>option('eq-preset',n,n,st.eq===n)).join('')+'</div><div class="nxt18-eq-bands">'+['60 Hz','250 Hz','1 kHz','4 kHz','12 kHz'].map((name,i)=>'<label>'+name+'<input type="range" min="-12" max="12" step="1" value="'+st.bands[i]+'" data-player-input="eq" data-player-id="'+id+'" data-band="'+i+'"><output>'+st.bands[i]+' dB</output></label>').join('')+'</div><p class="nxt18-panel-caption">SELECT · EQ</p>';
    if(st.panel==='lyrics')content='<p class="nxt18-panel-caption">'+E(songs[st.index])+' · Spring Sessions</p><div class="nxt18-panel-lyrics">Let the music stay with you<br>A little light, a softer day<br>And the world slows down</div>';
    if(st.panel==='menu')content='<div class="nxt18-panel-options">'+option('panel-close','','Return to player')+option('menu-destination','profile','Personal')+option('menu-destination','settings','Settings')+'</div>';
    if(st.panel==='options')content='<div class="nxt18-panel-options">'+option('shuffle','',st.shuffle?'Shuffle on':'Shuffle off',st.shuffle)+option('repeat','',['Repeat off','Repeat all','Repeat one'][st.repeat],st.repeat>0)+option('timer','','Sleep timer'+(st.timer?' · '+st.timer+'m':''))+option('volume','','Volume · '+st.volume+'%')+option('eq','','Equalizer · '+st.eq)+option('menu-destination','settings','Settings')+'</div>';
    return '<section class="nxt18-player-panel '+(st.panel==='lyrics'?'nxt18-lyrics-panel':'')+'" role="region" aria-label="'+E(heading)+'"><header><h3>'+E(heading)+'</h3><button type="button" class="nxt18-panel-close cursor-interaction" data-player="'+id+'" data-control="panel-close" aria-label="Back · B">Back <span class="nxt18-key-label">B</span></button></header>'+content+'</section>';
  }
  function renderPlayer(id,replace=false){
    const r=NXTDesign.renderTheme(id),st=playState[id],t=r.theme;
    let svg=r.svg;if(st.index)svg=svg.replaceAll('Blue Hour',E(songs[st.index]));
    const repeatLabel=['Repeat off','Repeat all','Repeat one'][st.repeat];
    const inside='<div class="nxt18-player" data-spectrum="'+st.spectrum+'" style="--nxt-bg:'+t.bg+';--nxt-panel:'+t.panel+';--nxt-ink:'+t.ink+';--nxt-muted:'+t.muted+';--nxt-accent:'+t.accent+';--nxt-icon-filter:'+(t.light?'brightness(.3)':'none')+';--nxt-active-icon-filter:'+(t.buttonInk==='#f3ede2'?'none':'brightness(.3)')+';--nxt-button-ink:'+t.buttonInk+'">'+svg+
      playerButton(id,'menu','Back to menu · B','arrow-left','','','B').replace('nxt18-player-button','nxt18-player-button nxt18-back-player')+'<span class="nxt18-mobile-title">NXT · '+E(t.name)+'</span>'+
      playerButton(id,'options','Playback options · START','settings','','<span>Options</span>','START').replace('nxt18-player-button','nxt18-player-button nxt18-options-player')+
      '<div class="nxt18-primary-dock">'+playerButton(id,'volume','Volume: '+st.volume+'%','volume-2','','<span data-volume-readout>'+st.volume+'%</span>').replace('nxt18-wide-button','nxt18-wide-button nxt18-volume-button')+
      '<div class="nxt18-transport">'+
      playerButton(id,'shuffle',st.shuffle?'Shuffle on':'Shuffle off','shuffle','aria-pressed="'+st.shuffle+'"')+
      playerButton(id,'prev','Previous track · L1','skip-back','','','L1')+
      playerButton(id,'play',(st.playing?'Pause':'Play')+' · A',st.playing?'pause':'play','aria-pressed="'+st.playing+'"','','A').replace('nxt18-player-button','nxt18-player-button nxt18-play')+
      playerButton(id,'next','Next track · R1','skip-forward','','','R1')+
      playerButton(id,'repeat',repeatLabel,st.repeat===2?'repeat-1':'repeat','aria-pressed="'+(st.repeat>0)+'" data-repeat="'+st.repeat+'"','<span class="nxt18-mode-tag">'+['Off','All','1'][st.repeat]+'</span>').replace('nxt18-wide-button','nxt18-wide-button nxt18-repeat-button')+'</div>'+
      '<div class="nxt18-queue-controls">'+playerButton(id,'favorite','Favorite · R2','heart','aria-pressed="'+st.favorites[st.index]+'"','','R2')+
      playerButton(id,'queue','Queue: track '+(st.index+1)+' of '+songs.length+' · X','list-music','','<span class="nxt18-track-count" aria-live="polite">'+(st.index+1)+' / '+songs.length+'</span>','X')+'</div></div>'+
      '<div class="nxt18-secondary-dock"><span class="nxt18-audio-caption">FLAC · 44.1 kHz / 24-bit</span><div class="nxt18-tools">'+playerButton(id,'eq','Equalizer · SELECT · '+st.eq,'sliders-horizontal','','<span>EQ</span>','SELECT')+
      playerButton(id,'spectrum','Spectrum · L2','audio-waveform','aria-pressed="'+st.spectrum+'"','','L2')+
      playerButton(id,'lyrics','Fullscreen lyrics · Y','file-text','','<span>Lyrics</span>','Y')+
      playerButton(id,'timer','Sleep timer: '+(st.timer?st.timer+' minutes':'Off'),'timer','aria-pressed="'+(st.timer>0)+'"','<span class="nxt18-timer-readout" aria-live="polite">'+(st.timer?st.timer+'m':'Off')+'</span>')+'</div><span class="nxt18-output-caption">USB DAC</span></div>'+
      playerPanel(id,st)+'</div>'+
      '<div class="nxt18-accessible-readout"><strong>'+E(songs[st.index])+' · Spring Sessions</strong><span>FLAC · USB DAC</span></div>';
    if(replace){root.querySelector('#nxt18-player-'+id).innerHTML=inside;hydrateIcons();}
    else return '<section data-variant="'+E(t.name)+'" aria-label="'+E(t.name)+'"'+(id?' hidden':'')+'><div class="nxt18-stage" id="nxt18-player-'+id+'">'+inside+'</div></section>';
  }
  carousel.innerHTML=Array.from({length:36},(_,id)=>renderPlayer(id)).join('');
  root.addEventListener('click',event=>{
    const p=event.target.closest('[data-player]');
    if(p){
      const id=Number(p.dataset.player),st=playState[id],c=p.dataset.control,v=p.dataset.value;
      if(c==='play')st.playing=!st.playing;
      else if(c==='next'){st.history.push(st.index);if(st.history.length>50)st.history.shift();st.index=(st.index+(st.shuffle?1+Math.floor(Math.random()*2):1))%songs.length;}
      else if(c==='prev')st.index=st.history.length?st.history.pop():(st.index+songs.length-1)%songs.length;
      else if(c==='shuffle')st.shuffle=!st.shuffle;
      else if(c==='repeat')st.repeat=(st.repeat+1)%3;
      else if(c==='favorite')st.favorites[st.index]=!st.favorites[st.index];
      else if(['timer','queue','volume','eq','lyrics','menu','options'].includes(c))st.panel=st.panel===c?'':c;
      else if(c==='panel-close')st.panel='';
      else if(c==='timer-set'){st.timer=Number(v);st.panel='';}
      else if(c==='queue-track'){st.history.push(st.index);st.index=Number(v);st.panel='';}
      else if(c==='volume-step')st.volume=Math.max(0,Math.min(100,st.volume+Number(v)));
      else if(c==='eq-preset'){st.eq=v;st.bands={Flat:[0,0,0,0,0],Jazz:[2,1,0,1,2],Vocal:[-2,0,3,2,0]}[v].slice();}
      else if(c==='spectrum')st.spectrum=!st.spectrum;
      else if(c==='menu-destination'){st.panel='';state.page=v;state.entered=true;root.querySelector('#nxt18-flows-tab').click();renderFlow();}
      renderPlayer(id,true);return;
    }
    const b=event.target.closest('[data-action]');if(!b)return;const a=b.dataset.action,v=b.dataset.value;
    if(a==='page'){state.page=b.dataset.page;state.group='';state.query='';}
    else if(a==='choose'){state.profile=Number(v);const pp=profiles[state.profile];state.mood=pp.mood||'calm';state.genre=pp.genre||'Jazz';state.country=pp.country||'Worldwide';state.page='mood';}
    else if(a==='choice')state[b.dataset.field]=v;
    else if(a==='onboarding-next'){if(state.page==='country'){Object.assign(profiles[state.profile],{mood:state.mood,genre:state.genre,country:state.country});state.entered=true;}state.page=state.page==='mood'?'genre':state.page==='genre'?'country':'forYou';}
    else if(a==='onboarding-back')state.page=state.page==='country'?'genre':state.page==='genre'?'mood':'intro';
    else if(a==='source')state.source=v;
    else if(a==='group'){if(['profile','languages','transfer','about'].includes(v))state.page=v;else{state.group=v;state.page='group';}}
    else if(a==='edit-profile'){state.draft={...profiles[state.profile]};state.page='editProfile';state.avatarType=state.draft.type;state.iconPage=0;}
    else if(a==='avatar-type')state.avatarType=v;
    else if(a==='avatar'){state.draft.avatar=v;state.draft.type=state.avatarType;}
    else if(a==='save-profile'){profiles[state.profile]={...state.draft,name:state.draft.name.trim()||profiles[state.profile].name};state.draft=null;state.page='profile';}
    else if(a==='cancel-profile'){state.draft=null;state.page='profile';}
    else if(a==='reenter'){state.entered=false;state.page='intro';}
    else if(a==='edit-today'){state.page='mood';}
    else if(a==='icon-next')state.iconPage=(state.iconPage+1)%Math.ceil(iconCatalog.length/16);
    else if(a==='icon-prev')state.iconPage=(state.iconPage+Math.ceil(iconCatalog.length/16)-1)%Math.ceil(iconCatalog.length/16);
    else if(a==='help-detail')state.help=v;
    else if(a==='updates')state.page='updates';
    else if(a==='wifi')state.wifi=!state.wifi;
    else if(a==='usb')state.usb=!state.usb;
    else if(a==='language'){flushKo();state.text+=state.locale==='ja'&&state.layout==='romaji'?toKana(state.raw):state.raw;state.raw='';state.locale=v;state.layout='native';}
    else if(a==='layout'){flushKo();state.text+=state.raw;state.raw='';state.layout=state.layout==='native'?(state.locale==='ja'?'romaji':'latin'):'native';}
    else if(a==='key'){
      if(state.locale==='ko'&&state.layout==='native')typeKo(v);
      else if(state.locale.startsWith('zh')&&state.layout==='native'||state.locale==='ja'&&state.layout==='romaji')state.raw+=v;
      else state.text+=v;
    }else if(a==='candidate'){state.text=state.locale==='ja'&&state.layout==='native'?v:state.text+v;state.raw='';}
    else if(a==='space'){flushKo();state.text+=(state.locale==='ja'&&state.layout==='romaji'?toKana(state.raw):state.raw)+' ';state.raw='';}
    else if(a==='erase'){const k=state.ko;if(k.t)k.t=0;else if(k.v>=0)k.v=-1;else if(k.l>=0)k.l=-1;else if(state.raw)state.raw=Array.from(state.raw).slice(0,-1).join('');else state.text=Array.from(state.text).slice(0,-1).join('');}
    else if(a==='clear'){state.text='';state.raw='';state.ko={l:-1,v:-1,t:0};}
    else if(a==='sample-play')state.page='playingSample';
    renderFlow();
  });
  root.addEventListener('input',event=>{
    const n=event.target,key=n.dataset.input;
    if(n.dataset.playerInput){
      const id=Number(n.dataset.playerId),st=playState[id];
      if(n.dataset.playerInput==='volume'){st.volume=Number(n.value);n.previousElementSibling.querySelector('output').textContent=st.volume+'%';const b=root.querySelector('#nxt18-player-'+id+' [data-control="volume"]');b.querySelector('[data-volume-readout]').textContent=st.volume+'%';b.setAttribute('aria-label','Volume: '+st.volume+'%');b.dataset.tooltip='Volume: '+st.volume+'%';}
      else {st.bands[Number(n.dataset.band)]=Number(n.value);st.eq='Custom';n.nextElementSibling.textContent=n.value+' dB';const b=root.querySelector('#nxt18-player-'+id+' [data-control="eq"]');b.setAttribute('aria-label','Equalizer · SELECT · Custom');b.dataset.tooltip='Equalizer · SELECT · Custom';}
    }
    else if(key==='nickname'&&state.draft)state.draft.name=n.value;
    else if(key==='typed'){state.ko={l:-1,v:-1,t:0};if(state.locale.startsWith('zh')&&state.layout==='native'||state.locale==='ja'&&state.layout==='romaji'){state.text='';state.raw=n.value;}else{state.text=n.value;state.raw='';}const area=device.querySelector('.nxt18-candidates');if(area)area.innerHTML=candidates().map(x=>'<button type="button" class="cursor-interaction" data-action="candidate" data-value="'+E(x)+'">'+E(x)+'</button>').join('');}
    else if(key==='led'){state.led=n.value;device.querySelector('.nxt18-led-preview').style.setProperty('--nxt-led',state.led);}
    else if(key==='search'){state.query=n.value;device.querySelector('#nxt18-settings-results').innerHTML=groups().filter(x=>(T(x[1])+' '+x[2]).toLocaleLowerCase().includes(state.query.toLocaleLowerCase())).map(x=>'<button type="button" class="nxt18-setting-row cursor-interaction" data-action="group" data-value="'+x[0]+'"><span>'+E(T(x[1]))+'</span><span>'+E(x[2])+'</span></button>').join('');}
  });
  renderFlow();hydrateIcons();
  /* A read-only design inspector lets the QA harness exercise the same UI. */
  root.nxtDesignState=()=>({locale:state.locale,page:state.page,profile:state.profile,avatar:profiles[state.profile].avatar,avatarType:profiles[state.profile].type,text:composed(),genre:state.genre,country:state.country,mood:state.mood,iconCount:iconCatalog.length});
  root.nxtPlayerState=id=>{const st=playState[id];return {index:st.index+1,total:songs.length,shuffle:st.shuffle,repeat:st.repeat,timer:st.timer,volume:st.volume,eq:st.eq,spectrum:st.spectrum,favorite:st.favorites[st.index],panel:st.panel};};
})();
