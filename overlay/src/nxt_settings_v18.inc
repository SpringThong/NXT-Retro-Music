/* v1.8.1: device settings; personal appearance belongs to the active profile. */
static ModuleExitReason v181_settings_group(DisplayContext *d,int group){
 if(group==0)return audio_settings(d);
 int selected=0;for(;;){const char *rows[3];int n=0;
 if(group==0){rows[n++]=Nxt_text("Audio & DAC","Âm thanh & DAC");rows[n++]=Nxt_text("Playback","Phát nhạc");}
 else {rows[n++]=Nxt_text("Music folder","Thư mục nhạc");rows[n++]=Nxt_text("Wi-Fi file transfer","Chuyển file qua Wi-Fi");}
 NxtPick p=Nxt_pick(d,group==0?Nxt_text("Playback & audio","Phát nhạc & âm thanh"):Nxt_text("Library & files","Thư viện & tệp"),rows,n,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));
 if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;ModuleExitReason r=MODULE_EXIT_TO_MENU;
 if(group==0)r=setting_group(d,selected==0?1:3);
 else if(selected==0)r=setting_group(d,2);else if(selected==1)r=NxtTransfer_run(d);
 if(r==MODULE_EXIT_QUIT)return r;
 }}
ModuleExitReason Nxt_LEDSettings(DisplayContext *d){return setting_group(d,5);}
ModuleExitReason Nxt_settings(DisplayContext *d){int selected=0;char query[128]="";for(;;){
 const char *labels[]={Nxt_text("Playback & audio","Phát nhạc & âm thanh"),Nxt_text("Library & files","Thư viện & tệp"),Nxt_text("Device performance","Hiệu năng thiết bị"),Nxt_text("Key settings","Cài đặt phím"),Nxt_text("Language","Ngôn ngữ")};
 const char *terms[]={Nxt_text("Audio DAC Bluetooth Bit-Perfect bit perfect EQ ReplayGain playback sleep timer volume crossfade","Âm thanh DAC Bluetooth Bit-Perfect bit perfect EQ Phát nhạc Hẹn giờ Âm lượng Chuyển bài"),Nxt_text("Library folder files scan transfer Wi-Fi copy","Thư viện Thư mục Tệp Quét Chuyển Wi-Fi Sao chép"),Nxt_text("Performance CPU RAM battery temperature device","Hiệu năng CPU RAM Pin Nhiệt độ Thiết bị"),Nxt_text("Buttons shortcuts mapping controls keys","Phím Nút Phím tắt Ánh xạ Điều khiển"),Nxt_text("Language keyboard French Russian Korean Japanese Thai Chinese German Spanish Portuguese Italian Indonesian","Ngôn ngữ Bàn phím Pháp Nga Hàn Nhật Thái Trung Đức Tây Ban Nha Bồ Đào Nha Ý Indonesia")};
 const char *rows[5];int ids[5],n=0;char *needle=NxtText_searchKey(query);
 for(int i=0;i<5;i++){char hay[1024];snprintf(hay,sizeof(hay),"%s %s",labels[i],NxtLocale_text(terms[i]));char *key=NxtText_searchKey(hay);bool match=!needle||!*needle||(key&&strstr(key,needle));free(key);if(match){rows[n]=labels[i];ids[n++]=i;}}free(needle);
 if(!n){rows[0]=Nxt_text("No settings found · X search again","Không tìm thấy · X tìm lại");ids[0]=-1;n=1;}if(selected>=n)selected=0;
 NxtPick p=Nxt_pick(d,Nxt_text("Settings","Cài đặt"),rows,n,&selected,Nxt_text("A Open · X Search · Y Clear · B Back","A Mở · X Tìm · Y Xóa tìm · B Về"));
 if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
 if(p.action==3){char *text=Keyboard_open(Nxt_text("Search settings","Tìm tùy chọn"),127);if(text){NxtText_copy(query,text,sizeof(query));free(text);}selected=0;}
 if(p.action==4){query[0]=0;selected=0;}
 if(p.action==1&&ids[selected]>=0){int id=ids[selected];ModuleExitReason r=id<2?v181_settings_group(d,id):id==2?NxtPerformance_settings(d):id==3?NxtControl_settings(d):NxtLocale_run(d);if(r==MODULE_EXIT_QUIT)return r;}
 }}
