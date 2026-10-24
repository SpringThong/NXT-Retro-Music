#include "nxt_translate.h"
#include "nxt_theme.h"
#include "nxt_text.h"
#include "nxt_transfer.h"
#include "nxt_locale.h"
#include "nxt_appearance.h"
#include "nxt_monitor.h"
#include "nxt_performance.h"
#include "nxt_custom.h"
#include "nxt_personal.h"
#include "nxt_ota.h"
#include "nxt_log.h"
#include "nxt_spotify.h"
#include "nxt_stock.h"
#include "nxt_drive.h"
#include "nxt_sort.h"
#include "browser.h"
/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt.h"
#include "nxt_bitperfect.h"
#include "nxt_controls.h"
#include "nxt_led.h"
#include "nxt_playback.h"
#include "nxt_skin.h"
#include "api.h"
#include "msettings.h"
#include "player.h"
#include "module_player.h"
#include "module_radio.h"
#include "module_podcast.h"
#include "module_downloader.h"
#include "playlist_m3u.h"
#include "file_utils.h"
#include "keyboard.h"
#include "ui_fonts.h"
#include "settings.h"
#include "module_settings.h"
#include "toast.h"
#include "resume.h"
#include "nxt_audio_mode.h"
#include "nxt_volume.h"
#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
extern ModuleExitReason Nxt_systemScreen(DisplayContext *);
extern void Nxt_libraryAddTracks(DisplayContext *,const char *);
static const char *on(int value){return value?Nxt_text("On","Bật"):Nxt_text("Off","Tắt");}
static int cycle(int v,int d,int n){return (v+d+n)%n;}
static bool number(const char *prompt,int *value,int low,int high){char *s=Keyboard_open(prompt,12);if(!s)return false;char *end;errno=0;long v=strtol(s,&end,10);bool ok=*s&&!*end&&!errno&&v>=low&&v<=high;free(s);if(ok)*value=v;else Toast_show(Nxt_text("Invalid number","Số không hợp lệ"),1500);return ok;}
static ModuleExitReason info(DisplayContext *d,const char *title,const char **rows,int n){return Nxt_document(d,title,rows,n);}

static ModuleExitReason timer(DisplayContext *d){const char *rows[]={Nxt_text("Cancel timer","Hủy hẹn giờ"),"15 min","30 min","45 min","60 min","90 min","120 min",Nxt_text("Custom minutes…","Nhập số phút…")};int values[]={0,15,30,45,60,90,120};int sel=0;NxtPick p=Nxt_pick(d,Nxt_text("Sleep timer","Hẹn giờ dừng"),rows,8,&sel,Nxt_text("A Set • B Back","A Đặt • B Quay lại"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==1){int m=0;if(sel<7)m=values[sel];else if(!number(Nxt_text("Minutes (0 cancels)","Số phút (0 để hủy)"),&m,0,INT_MAX))return MODULE_EXIT_TO_MENU;Nxt_sleepTimer(m);Toast_show(m?Nxt_text("Timer set","Đã đặt hẹn giờ"):Nxt_text("Timer cancelled","Đã hủy hẹn giờ"),1500);}return MODULE_EXIT_TO_MENU;}
static ModuleExitReason audio_mode(DisplayContext *d);
static ModuleExitReason dsd_output(DisplayContext*d);
static ModuleExitReason dac_name_settings(DisplayContext*d);
static ModuleExitReason setting_group(DisplayContext *d,int group){int sel=0;
 for(;;){char lines[13][620];const char *rows[13];int n=0;const char *themes[]={"System","Dark","Light"},*themesvi[]={"Theo hệ thống","Tối","Sáng"};const char *trans[]={"Normal","Gapless","Crossfade"},*transvi[]={"Thông thường","Liền mạch","Chồng âm"};const char *rg[]={"Off","Track","Album"},*rgvi[]={"Tắt","Theo bài","Theo album"};const char *colors[]={"Mint","Blue","Amber","Pink","White"},*colorsvi[]={"Xanh mint","Xanh dương","Vàng","Hồng","Trắng"};
#define ROW(en,vi,fmt,v) do{snprintf(lines[n],620,"%s\t" fmt,Nxt_text(en,vi),v);rows[n]=lines[n];n++;}while(0)
  if(group==0){ROW("Theme","Giao diện","%s",nxt.language==1?themesvi[nxt.theme]:themes[nxt.theme]);ROW("Font family","Bộ font","%s",NxtText_fontName(nxt.font_index));ROW("Interface text","Chữ giao diện","%d%%",nxt.font_size);ROW("Lyrics text","Chữ lời bài hát","%d px",nxt.lyric_size);ROW("Lyric delay","Độ trễ lời","%+d ms",nxt.lyric_offset);ROW("Language","Ngôn ngữ","%s",NxtLocale_name(nxt.language));ROW("Script fallback","Font đa ngôn ngữ","%s",Nxt_text("Always enabled · offline","Luôn bật · offline"));ROW("Colors & wallpaper","Màu sắc & hình nền","%s",NxtPersonal_name());}
  else if(group==1){ROW("Audio output","Đầu ra âm thanh","%s",NxtHardware_output());ROW("Sound mode","Chế độ âm thanh","%s",NxtAudio_modeName(nxt.audio_mode));ROW("ReplayGain","ReplayGain","%s",nxt.language==1?rgvi[nxt.replaygain]:rg[nxt.replaygain]);ROW("DAC unplug","Khi rút DAC","%s",nxt.unplug_pause?Nxt_text("Pause","Tạm dừng"):Nxt_text("Speaker","Chuyển loa"));ROW("Auto switch to DAC","Tự chuyển sang DAC","%s",nxt.audio_mode==2?Nxt_text("Automatic mode","Chế độ Tự động"):on(nxt.auto_dac));ROW("Speaker / 3.5 mm volume","Âm lượng loa / 3,5 mm","%d%%",nxt.volume_percent[0]);ROW("Bluetooth volume","Âm lượng Bluetooth","%d%%",nxt.volume_percent[1]);ROW("DAC volume","Âm lượng DAC","%d%%",nxt.volume_percent[2]);ROW("Output information","Thông tin đầu ra","%s",NxtHardware_pcm());ROW("Bottom USB port","Cổng USB dưới","%s",NxtHardware_bottomStatus());ROW("Auto switch to Bluetooth","Tự chuyển sang Bluetooth","%s",on(nxt.auto_bt));
ROW("DSD USB output","Đầu ra DSD USB","%s",nxt.dsd_output==2?Nxt_text("Automatic Native / DoP","Tự động Native / DoP"):nxt.dsd_output?"DoP":"Native DSD");
ROW("DAC display name","Tên hiển thị DAC","%s",*NxtHardware_dacName()?NxtHardware_dacName():Nxt_text("USB descriptor","Theo tên USB"));
}
  else if(group==2){ROW("Music folder","Thư mục nhạc","%s",nxt.music_root);ROW("Include subfolders","Gồm thư mục con","%s",on(nxt.recursive));ROW("Refresh library","Quét lại thư viện","%s","…");}
  else if(group==3){ROW("Auto-play on launch","Tự phát khi mở app","%s",on(nxt.auto_play));ROW("Track transition","Chuyển bài","%s",nxt.language==1?transvi[nxt.transition]:trans[nxt.transition]);ROW("Crossfade duration","Thời gian chồng âm","%.1f s",nxt.crossfade_ms/1000.);ROW("Sleep timer","Hẹn giờ dừng","%d min",(Nxt_timerRemaining()+59)/60);ROW("Background playback","Phát nhạc dưới nền","%s",on(nxt.background));ROW("Auto screen off","Tự tắt màn hình","%s",SettingsModule_getScreenOffDisplayStr());ROW("Lyric translation","Dịch lời bài hát","%s",NxtTranslate_modeName(nxt.translation_mode));ROW("Screen-off controls","Điều khiển khi tắt màn hình","%s",nxt.screen_off_controls?Nxt_text("On · Power wakes screen","Bật · Nguồn bật sáng màn"):Nxt_text("Off · any button wakes screen","Tắt · bấm nút để sáng màn"));}
  else if(group==5){ROW("LED enabled","Bật LED","%s",on(nxt.led));ROW("Mode","Chế độ","%s",nxt.led_mode?Nxt_text("Music reactive","Theo nhạc"):Nxt_text("Constant color","Màu cố định"));ROW("Color","Màu LED","%s",NxtLED_name(nxt.led_color));ROW("Manual RGB palette","Bảng màu RGB tự chỉnh","#%06X",NxtLED_rgb());ROW("Brightness","Độ sáng","%d%%",nxt.led_strength);ROW("Color effect","Hiệu ứng màu","%s",NxtLED_effectName(nxt.led_effect));ROW("Gradient second color","Màu gradient thứ hai","#%06X",nxt.led_rgb2);}
#undef ROW
  const char *titles[]={Nxt_text("Appearance & fonts","Giao diện & font"),Nxt_text("Sound & output","Âm thanh & đầu ra"),Nxt_text("Music folder","Thư mục nhạc"),Nxt_text("Playback","Cách phát"),Nxt_text("Controls","Điều khiển"),Nxt_text("LEDs","LED")};
  NxtPick p=Nxt_pickLive(d,titles[group],rows,n,&sel,Nxt_text("←→ Change • A Open • B Back","←→ Đổi • A Mở • B Quay lại"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1&&p.action!=5&&p.action!=6)continue;int delta=p.action==5?-1:1;ModuleExitReason r=MODULE_EXIT_TO_MENU;
  if(group==0){switch(sel){case 0:nxt.theme=cycle(nxt.theme,delta,3);break;case 1:if(p.action==1){const char *fonts[NXT_FONT_COUNT];for(int i=0;i<NXT_FONT_COUNT;i++)fonts[i]=NxtText_fontName(i);int f=nxt.font_index;NxtPick fp=Nxt_pick(d,Nxt_text("Choose font","Chọn font"),fonts,NXT_FONT_COUNT,&f,Nxt_text("A Apply • B Back","A Áp dụng • B Quay lại"));if(fp.action==1)nxt.font_index=f;}else nxt.font_index=cycle(nxt.font_index,delta,NXT_FONT_COUNT);nxt.font_path[0]=0;Fonts_unload();Fonts_load();break;case 2:nxt.font_size+=delta*5;if(nxt.font_size>120)nxt.font_size=85;if(nxt.font_size<85)nxt.font_size=120;Fonts_unload();Fonts_load();break;case 3:nxt.lyric_size+=delta*2;if(nxt.lyric_size>48)nxt.lyric_size=24;if(nxt.lyric_size<24)nxt.lyric_size=48;break;case 4:nxt.lyric_offset+=delta*100;if(nxt.lyric_offset>30000)nxt.lyric_offset=30000;if(nxt.lyric_offset< -30000)nxt.lyric_offset=-30000;break;case 5:if(NxtLocale_run(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;break;case 7:r=NxtAppearance_run(d);break;}}
  else if(group==1){if(nxt.bit_perfect&&sel==2){Toast_show(Nxt_text("ReplayGain bypassed in Bit-Perfect","Bỏ qua ReplayGain trong Bit-Perfect"),2000);continue;}switch(sel){case 0:{const char *out[]={Nxt_text("Speaker","Loa máy"),"Bluetooth","DAC"};int o=nxt.output<3?nxt.output:0;NxtPick op=Nxt_pick(d,Nxt_text("Audio output","Đầu ra âm thanh"),out,3,&o,Nxt_text("A Select • B Back","A Chọn • B Quay lại"));if(op.action==1){if(NxtHardware_select(o))nxt.output=o;else Toast_show(Nxt_text("Output unavailable; connect it in system settings first","Đầu ra chưa sẵn sàng; hãy kết nối trong cài đặt hệ thống"),2500);}break;}case 1:r=audio_mode(d);break;case 2:nxt.replaygain=cycle(nxt.replaygain,delta,3);break;case 3:nxt.unplug_pause=!nxt.unplug_pause;break;case 4:nxt.auto_dac=!nxt.auto_dac;break;case 5:case 6:case 7:{int o=sel-5;nxt.volume_percent[o]=cycle(nxt.volume_percent[o],delta,101);nxt.volume[o]=(nxt.volume_percent[o]+4)/5;if(GetAudioSink()==o)NxtVolume_setPercent(nxt.volume_percent[o]);break;}case 8:{const char *details[]={NxtHardware_output(),NxtHardware_details(),NxtHardware_pcm(),NxtHardware_score()};r=info(d,Nxt_text("Output information","Thông tin đầu ra"),details,4);break;}case 9:{if(p.action!=1)break;
const char *modes[]={Nxt_text("Listen through DAC\nUnplug charger/PC cable before selecting","Nghe DAC\nRút cáp sạc/PC trước khi chọn"),Nxt_text("Charging mode\nUnplug DAC before selecting","Chế độ sạc\nRút DAC trước khi chọn")};
int mode=0;NxtPick pick=Nxt_pick(d,Nxt_text("Bottom USB port","Cổng USB dưới"),modes,2,&mode,Nxt_text("A Apply • B Back","A Áp dụng • B Quay lại"));
if(pick.action==9)return MODULE_EXIT_QUIT;
if(pick.action==1){int result=NxtHardware_bottomUSB(mode==0);Toast_show(result==3?Nxt_text("Request sent; check USB port status","Đã gửi yêu cầu; kiểm tra trạng thái cổng USB"):result?(mode==0?Nxt_text("DAC mode enabled; connect DAC","Đã bật chế độ DAC; hãy cắm DAC"):Nxt_text("Charging mode selected","Đã chuyển về chế độ sạc")):Nxt_text("USB action failed; see NXT_Input_Log.txt","Lỗi thao tác USB; xem NXT_Input_Log.txt"),3000);}
break;}case 10:nxt.auto_bt=!nxt.auto_bt;break;case 11:r=dsd_output(d);break;case 12:r=dac_name_settings(d);break;}}
  else if(group==2){if(sel==0)Nxt_chooseFolder(d);if(sel==1){nxt.recursive=!nxt.recursive;Nxt_libraryQuit();}if(sel==2){NxtLibrary_request(true);r=Nxt_library(d);}}
  else if(group==3){if(sel==7){int old=nxt.screen_off_controls;nxt.screen_off_controls=!old;if(!Nxt_save()){nxt.screen_off_controls=old;Toast_show(Nxt_text("Cannot save setting","Không lưu được cài đặt"),1800);}continue;}if(sel==6){r=NxtTranslate_settings(d);if(r==MODULE_EXIT_QUIT)return r;continue;}if(sel==0){nxt.auto_play=!nxt.auto_play;Nxt_save();continue;}sel--;if(nxt.bit_perfect&&sel<2){Toast_show(Nxt_text("Bypassed in Bit-Perfect; saved settings retained","Bỏ qua trong Bit-Perfect; giữ cài đặt đã lưu"),2000);continue;}if(sel==0)nxt.transition=cycle(nxt.transition,delta,3);if(sel==1){nxt.crossfade_ms+=delta*500;if(nxt.crossfade_ms>12000)nxt.crossfade_ms=500;if(nxt.crossfade_ms<500)nxt.crossfade_ms=12000;}if(sel==2)r=timer(d);if(sel==3)nxt.background=!nxt.background;if(sel==4){if(delta<0)SettingsModule_cycleScreenOffPrev();else SettingsModule_cycleScreenOffNext();}}
  else if(group==5){if(sel==0)nxt.led=!nxt.led;if(sel==1)nxt.led_mode=!nxt.led_mode;if(sel==2)nxt.led_color=cycle(nxt.led_color,delta,17);if(sel==3){if(NxtLED_palette(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}if(sel==4){nxt.led_strength+=delta*5;if(nxt.led_strength>100)nxt.led_strength=0;if(nxt.led_strength<0)nxt.led_strength=100;}if(sel==5)nxt.led_effect=cycle(nxt.led_effect,delta,6);if(sel==6&&p.action==1)r=NxtCustom_rgb(d,Nxt_text("LED second color","Màu LED thứ hai"),&nxt.led_rgb2);}
  Nxt_save();if(r==MODULE_EXIT_QUIT)return r;
 }
}
static ModuleExitReason song_output_info(DisplayContext *);
#include "nxt_audio_v184.inc"
#include "nxt_settings_v18.inc"
static ModuleExitReason song_output_info(DisplayContext *d){
 const TrackInfo *t=NxtPlayback_info();char text[12][1024];const char *rows[12];int n=0;
 snprintf(text[n],sizeof(text[n]),"%s",t->title);rows[n]=text[n];n++;
 snprintf(text[n],sizeof(text[n]),"%s%s%s",t->artist,t->album[0]?" · ":"",t->album);rows[n]=text[n];n++;
 if(t->codec[0]){snprintf(text[n],sizeof(text[n]),"%s: %s",Nxt_text("Codec","Định dạng"),t->codec);rows[n]=text[n];n++;}
 if(t->sample_rate>0){snprintf(text[n],sizeof(text[n]),"%s: %d Hz",Nxt_text("Source sample rate","Tần số lấy mẫu nguồn"),t->sample_rate);rows[n]=text[n];n++;}
 if(t->bit_depth>0){snprintf(text[n],sizeof(text[n]),"%s: %d bit",Nxt_text("Source bit depth","Độ sâu bit nguồn"),t->bit_depth);rows[n]=text[n];n++;}
 if(t->bitrate>0){snprintf(text[n],sizeof(text[n]),"%s: %d kbps",Nxt_text("Bitrate","Tốc độ bit"),t->bitrate/1000);rows[n]=text[n];n++;}
 if(t->channels>0){snprintf(text[n],sizeof(text[n]),"%s: %d",Nxt_text("Channels","Số kênh"),t->channels);rows[n]=text[n];n++;}
 snprintf(text[n],sizeof(text[n]),"%s: %s",Nxt_text("Output","Đầu ra"),NxtHardware_output());rows[n]=text[n];n++;
 if(NxtHardware_pcm()[0]){snprintf(text[n],sizeof(text[n]),"%s",NxtHardware_pcm());rows[n]=text[n];n++;}
 if(NxtHardware_details()[0]){snprintf(text[n],sizeof(text[n]),"%s",NxtHardware_details());rows[n]=text[n];n++;}
 if(nxt.bit_perfect){snprintf(text[n],sizeof(text[n]),"%s",NxtBit_status());rows[n]=text[n];n++;if(*NxtBit_error()){snprintf(text[n],sizeof(text[n]),"%s",NxtBit_error());rows[n]=text[n];n++;}}
 return Nxt_document(d,Nxt_text("Song & output information","Thông tin bài hát & đầu ra"),rows,n);
}
ModuleExitReason Nxt_quickOptions(DisplayContext *d){int sel=0;for(;;){
 char shuffle[100],repeat[100],sound[100];const char *mode[]={Nxt_text("Off","Tắt"),Nxt_text("One","Một bài"),Nxt_text("All","Tất cả")};
 snprintf(shuffle,100,"%s\t%s",Nxt_text("Shuffle","Ngẫu nhiên"),on(nxt.shuffle));snprintf(repeat,100,"%s\t%s",Nxt_text("Repeat","Lặp lại"),mode[nxt.repeat_mode]);snprintf(sound,100,"%s\t%s",Nxt_text("Sound mode","Chế độ âm thanh"),NxtAudio_modeName(nxt.audio_mode));
 const char *rows[]={Nxt_text("Audio output","Đầu ra âm thanh"),sound,Nxt_text("Sleep timer · stop and close app","Hẹn giờ · dừng và đóng app"),shuffle,repeat,Nxt_text("Song & output information","Thông tin bài hát & đầu ra"),Nxt_text("Favorite current track","Yêu thích bài đang phát"),Nxt_text("Lyrics preview","Lời thu gọn"),Nxt_text("Turn screen off","Tắt màn hình"),Nxt_text("Stop and save position","Dừng và lưu vị trí"),Nxt_text("Play in background","Phát nền"),Nxt_text("Stop and close app","Dừng và đóng app"),Nxt_text("Lyric translation","Dịch lời bài hát")};
 NxtPick p=Nxt_pickLive(d,Nxt_text("Quick options","Tùy chọn nhanh"),rows,13,&sel,Nxt_text("A Select · B Back","A Chọn · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
 ModuleExitReason result=MODULE_EXIT_TO_MENU;
 switch(sel){case 12:result=NxtTranslate_settings(d);break;case 0:result=audio_output(d);break;case 1:result=audio_mode(d);break;case 2:result=timer(d);break;case 3:Nxt_setShuffle(!nxt.shuffle);break;case 4:Nxt_setRepeat((nxt.repeat_mode+1)%3);break;case 5:result=song_output_info(d);break;case 6:NxtPlayback_toggleFavorite();break;case 7:Settings_toggleBool(&SETTING_LYRICS_ENABLED);break;case 8:Nxt_requestScreenOff();return MODULE_EXIT_TO_MENU;case 9:Nxt_stop();return MODULE_EXIT_TO_MENU;case 10:nxt.background=1;Nxt_save();return MODULE_EXIT_QUIT;case 11:Nxt_stop();Nxt_save();ModuleCommon_signalStop();ModuleCommon_requestQuit();return MODULE_EXIT_QUIT;}
 if(result==MODULE_EXIT_QUIT)return result;
}}
ModuleExitReason Nxt_online(DisplayContext *d){int sel=0;for(;;){const char *rows[]={Nxt_text("Spotify\nSearch, playlists and direct playback","Spotify\nTìm bài, playlist và phát trực tiếp"),Nxt_text("SoundCloud\nPublic tracks and playlists · no API key","SoundCloud\nNhạc công khai và playlist · không cần API key"),Nxt_text("Google Drive\nPublic shared folders and audio files","Google Drive\nThư mục và tệp nhạc chia sẻ công khai"),"Radio\nInternet radio","Podcast\nSubscriptions & episodes",Nxt_text("YouTube\nDownload music","YouTube\nTải nhạc về máy")};NxtPick p=Nxt_pickLive(d,Nxt_text("Online music","Nhạc online"),rows,6,&sel,Nxt_text("A Open • B Back","A Mở • B Quay lại"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action==1){ModuleExitReason result=sel==0?NxtSpotify_run(d):sel==1?NxtSoundCloud_run(d):sel==2?NxtDrive_run(d):sel==3?RadioModule_run(d):sel==4?PodcastModule_run(d):DownloaderModule_run(d);if(result==MODULE_EXIT_QUIT)return result;if(Nxt_takeReturnHome()){Nxt_returnHome();return MODULE_EXIT_TO_MENU;}}}}
/* Atomic UTF-8 M3U8 writes preserve names and playlist ordering. */
static bool write_list(const char *path,const PlaylistTrack *tracks,int count){char temp[1200];snprintf(temp,sizeof(temp),"%s.tmp",path);FILE *f=fopen(temp,"wb");if(!f)return false;bool ok=fputs("#EXTM3U\n",f)>=0;for(int i=0;i<count&&ok;i++){char name[256];snprintf(name,sizeof(name),"%s",tracks[i].name);for(char *p=name;*p;p++)if(*p=='\r'||*p=='\n')*p=' ';ok=fprintf(f,"#EXTINF:-1,%s\n%s\n",name,tracks[i].path)>=0;}ok=!ferror(f)&&ok;if(fclose(f))ok=false;if(ok)ok=rename(temp,path)==0;if(!ok)unlink(temp);return ok;}
static int choose_file_order(const void *a,const void *b){
 const FileEntry *x=a,*y=b;if(x->is_dir!=y->is_dir)return x->is_dir?-1:1;return NxtSort_compare(x->name,y->name);
}
bool Nxt_chooseFile(DisplayContext *d,const char *extension1,const char *extension2,char out[512]){
 char path[512];snprintf(path,sizeof(path),"%s",SDCARD_PATH);int selected=0;
 for(;;){
  FileEntry *entries=calloc(2048,sizeof(*entries));const char **names=calloc(2048,sizeof(*names));
  if(!entries||!names){free(entries);free(names);return false;}
  int count=1;snprintf(entries[0].name,sizeof(entries[0].name),"..");
  DIR *dir=opendir(path);if(dir){struct dirent *entry;
   while((entry=readdir(dir))&&count<2048){
    if(entry->d_name[0]=='.')continue;char full[512];
    if(snprintf(full,sizeof(full),"%s/%s",path,entry->d_name)>=(int)sizeof(full))continue;
    struct stat st;if(lstat(full,&st)||S_ISLNK(st.st_mode))continue;
    const char *ext=strrchr(entry->d_name,'.');bool folder=S_ISDIR(st.st_mode);
    if(!folder&&(!S_ISREG(st.st_mode)||!ext||(strcasecmp(ext,extension1)&&(strcasecmp(extension1,".jpg")||strcasecmp(ext,".jpeg"))&&(!extension2||strcasecmp(ext,extension2)))))continue;
    FileEntry *row=&entries[count++];snprintf(row->name,sizeof(row->name),"%s",entry->d_name);snprintf(row->path,sizeof(row->path),"%s",full);row->is_dir=folder;
   }closedir(dir);
  }
  qsort(entries+1,count-1,sizeof(*entries),choose_file_order);for(int i=0;i<count;i++)names[i]=entries[i].name;
  NxtPick choice=Nxt_pick(d,path,names,count,&selected,Nxt_text("A Open / import · B Back","A Mở / nhập · B Quay lại"));bool done=false;
  if(choice.action==1&&selected>=0&&selected<count){
   if(!selected){if(strcmp(path,SDCARD_PATH)){char *slash=strrchr(path,'/');if(slash)*slash=0;}}
   else if(entries[selected].is_dir)snprintf(path,sizeof(path),"%s",entries[selected].path);
   else{snprintf(out,512,"%s",entries[selected].path);done=true;}
   selected=0;
  }
  free(entries);free(names);if(done)return true;if(choice.action==2||choice.action==9)return false;
 }
}
static ModuleExitReason playlist_detail(DisplayContext *d,PlaylistInfo *list){int sel=0,tab=0;for(;;){PlaylistTrack *tracks=calloc(PLAYLIST_MAX_TRACKS,sizeof(*tracks));int count=0;if(!tracks||M3U_loadTracks(list->path,tracks,PLAYLIST_MAX_TRACKS,&count)){free(tracks);return MODULE_EXIT_TO_MENU;}const char **rows=calloc(count?count:1,sizeof(char*));for(int i=0;i<count;i++)rows[i]=tracks[i].name;const char *tabs[]={Nxt_text("Play all","Phát tất cả"),Nxt_text("Add tracks","Thêm bài"),Nxt_text("Rename","Đổi tên"),Nxt_text("Export","Xuất M3U8")};NxtPick p=Nxt_pickTabs(d,list->name,rows,count,&sel,Nxt_text("A Play • X Remove • ←→ Move • L1/R1 Tab • Y Run • B Back","A Phát • X Bỏ • ←→ Đổi chỗ • L1/R1 Mục • Y Chọn • B Quay lại"),tabs,4,tab);free(rows);ModuleExitReason r=MODULE_EXIT_TO_MENU;
 if(p.action==2||p.action==9){free(tracks);return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;}
 if(p.action==11)tab=(tab+3)%4;if(p.action==12)tab=(tab+1)%4;
 if(p.action==1&&count){PlayerModule_setResumePlaylistPath(list->path);r=PlayerModule_runWithPlaylist(d,tracks,count,sel);}
 if(p.action==3&&count&&sel<count){memmove(&tracks[sel],&tracks[sel+1],(count-sel-1)*sizeof(*tracks));if(!write_list(list->path,tracks,count-1))Toast_show(Nxt_text("Cannot save playlist","Không lưu được danh sách"),2000);}
 if((p.action==5||p.action==6)&&count){int to=sel+(p.action==5?-1:1);if(to>=0&&to<count){PlaylistTrack temp=tracks[sel];tracks[sel]=tracks[to];tracks[to]=temp;if(write_list(list->path,tracks,count))sel=to;}}
 if(p.action==4){if(tab==0&&count){PlayerModule_setResumePlaylistPath(list->path);r=PlayerModule_runWithPlaylist(d,tracks,count,0);}if(tab==1)Nxt_libraryAddTracks(d,list->path);if(tab==2){char *name=Keyboard_open(Nxt_text("New playlist name","Tên danh sách mới"),120);if(name){char safe[128],target[1024],rel[180];if(M3U_sanitizeName(name,safe,sizeof(safe))){snprintf(rel,180,"playlists/%s.m3u",safe);userdata_snpath(rel,target,sizeof(target));if(!strcmp(target,list->path)|| (access(target,F_OK)&&!rename(list->path,target))){snprintf(list->path,sizeof(list->path),"%s",target);snprintf(list->name,sizeof(list->name),"%s",safe);}else Toast_show(Nxt_text("Name exists / cannot rename","Tên đã có / không đổi tên được"),2000);}free(name);}}if(tab==3){char dir[1024],out[1200];snprintf(dir,sizeof(dir),"%s/Playlists",nxt.music_root);mkdir(dir,0755);snprintf(out,sizeof(out),"%s/%s.m3u8",dir,list->name);Toast_show(write_list(out,tracks,count)?Nxt_text("Exported UTF-8 M3U8 to Playlists","Đã xuất M3U8 UTF-8 vào Playlists"):Nxt_text("Cannot export playlist","Không xuất được danh sách"),2500);}}
 free(tracks);if(r==MODULE_EXIT_QUIT||Nxt_takeReturnHome()){Nxt_returnHome();return r;}
}}
ModuleExitReason Nxt_playlists(DisplayContext *d){int sel=0;M3U_init();for(;;){PlaylistInfo lists[MAX_PLAYLISTS];int count=M3U_listPlaylists(lists,MAX_PLAYLISTS);const char *rows[MAX_PLAYLISTS+2];char labels[MAX_PLAYLISTS][300];rows[0]=Nxt_text("Create playlist\nName and choose tracks","Tạo danh sách phát\nĐặt tên và chọn bài hát");rows[1]=Nxt_text("Import M3U / M3U8\nBrowse memory card","Nhập M3U / M3U8\nChọn file trên thẻ nhớ");for(int i=0;i<count;i++){snprintf(labels[i],300,"%s\n%d %s",lists[i].name,lists[i].track_count,Nxt_text("tracks","bài hát"));rows[i+2]=labels[i];}NxtPick p=Nxt_pickLive(d,Nxt_text("Playlists","Danh sách phát"),rows,count+2,&sel,Nxt_text("A Open • X Delete • B Back","A Mở • X Xóa • B Quay lại"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
 if(p.action==3&&sel>=2&&sel<count+2){const char *confirm[]={Nxt_text("Cancel","Hủy"),Nxt_text("Delete playlist","Xóa danh sách phát")};int c=0;NxtPick cp=Nxt_pick(d,lists[sel-2].name,confirm,2,&c,Nxt_text("A Confirm • B Back","A Xác nhận • B Quay lại"));if(cp.action==1&&c==1)M3U_delete(lists[sel-2].path);}
 if(p.action==1){if(sel<2){char import[512]="";if(sel==1&&!Nxt_chooseFile(d,".m3u",".m3u8",import))continue;char *name=Keyboard_open(Nxt_text("Playlist name","Tên danh sách phát"),120);if(name){char safe[128],rel[180],target[1024];if(M3U_sanitizeName(name,safe,sizeof(safe))&&M3U_create(safe)==0){snprintf(rel,180,"playlists/%s.m3u",safe);userdata_snpath(rel,target,sizeof(target));if(sel==0)Nxt_libraryAddTracks(d,target);else{PlaylistTrack *tracks=calloc(PLAYLIST_MAX_TRACKS,sizeof(*tracks));int n=0;bool ok=tracks&&!M3U_loadTracks(import,tracks,PLAYLIST_MAX_TRACKS,&n)&&write_list(target,tracks,n);free(tracks);Toast_show(ok?Nxt_text("Playlist imported","Đã nhập danh sách phát"):Nxt_text("Cannot read playlist","Không đọc được danh sách"),2000);}}else Toast_show(Nxt_text("Name exists / cannot create","Tên đã có / không tạo được danh sách"),2000);free(name);}}else if(sel<count+2){ModuleExitReason r=playlist_detail(d,&lists[sel-2]);if(r==MODULE_EXIT_QUIT||Nxt_takeReturnHome())return r;}}
}}
ModuleExitReason Nxt_changes(DisplayContext *d){const char *rows[]={
 Nxt_text("v1.10: Spotify QR session library access; valid search pages; bounded online PCM buffering; B asks background or close while playing, and exits immediately when paused. Three original OTA sources retained.","v1.10: thư viện Spotify theo phiên QR; trang tìm kiếm hợp lệ; bộ đệm âm thanh online có giới hạn; B hỏi nghe nền hay thoát khi đang phát, pause thì thoát hẳn. Giữ ba nguồn OTA cũ."),
 Nxt_text("v1.9.7.7: Cleaner icon edges, slightly larger Auralis controls and a solid light active state on glass. Mirrored transport centres remain aligned at native resolution.","v1.9.7.7: Làm mịn icon, tăng nhẹ nút Auralis; nút bật có nền sáng và icon tối dễ nhận biết. Căn đối xứng cụm điều khiển ở độ phân giải màn hình."),
 Nxt_text("Filtered album covers, frame textures, avatars and vinyl labels; retain original cover detail and aspect ratio. Cache icons by name and size, with bounded memory and no full-screen supersampling during animation.","Lọc mịn ảnh bìa, khung theme, avatar và nhãn đĩa; giữ chi tiết nguồn và đúng tỉ lệ. Lưu icon theo tên/kích thước, giới hạn bộ nhớ; không render siêu mẫu toàn màn hình trong chuyển động."),
 Nxt_text("v1.9.7.6: Fix playable public SoundCloud tracks wrongly rejected as Go+/region restricted. Accept the MONETIZE web policy while retaining private, preview, blocked and subscription checks.","v1.9.7.6: Sửa bài SoundCloud công khai bị nhận nhầm giới hạn Go+/khu vực. Chấp nhận trạng thái MONETIZE; vẫn kiểm tra bài riêng tư, nghe thử, bị chặn và thuê bao."),
 Nxt_text("Do not infer Go+ or region from every stream failure. SoundCloud logs now show the current app version/ROM. Keep the v1.9.7.5 listening features, lyrics and official OTA sources; vocal reduction stays removed.","Không kết luận Go+/khu vực cho mọi lỗi lấy luồng. Log SoundCloud hiển thị đúng phiên bản/ROM. Giữ tính năng nghe nhạc, lời và nguồn OTA của v1.9.7.5; phần giảm giọng vẫn được gỡ."),
 Nxt_text("v1.9.7.5: Removed vocal reduction, its shortcut, settings and K status. Music follows the existing Normal, Bit-Perfect or Auto sound mode without the vocal filter.","v1.9.7.5: Gỡ giảm giọng ca sĩ, phím tắt, cài đặt và ký hiệu K. Nghe nhạc theo chế độ Thường, Bit-Perfect hoặc Tự động hiện có, không còn bộ lọc giọng."),
 Nxt_text("Existing vocal-reduction button assignments become Unassigned; other assignments stay unchanged. Keep bilingual/italic lyrics, lyric effects, themes, fonts, screen-off controls, clean page transitions, online playback, DSD and each ROM's official OTA source.","Nút từng gán giảm giọng được trả về Không gán; các nút khác giữ nguyên. Giữ lời song ngữ/chữ nghiêng, hiệu ứng lời, themes, fonts, điều khiển khi tắt màn hình, chuyển trang sạch, nhạc online, DSD và nguồn OTA chính thức từng ROM."),
 Nxt_text("v1.9.7.3: Navigation switches complete pages without a whole-screen slide. Rebuild the native window from a clean backbuffer for both full and layer-only updates. Keep lyric, vinyl and spectrum motion.","v1.9.7.3: Chuyển trang trực tiếp, bỏ trượt toàn màn hình. Ghép khung mới từ nền sạch cho cả trang và các lớp chuyển động. Giữ hiệu ứng lời, đĩa than và phổ nhạc."),
 Nxt_text("v1.9.7.3: Keeps Screen-off controls, all v1.9.7.2 features and the official OTA source for each ROM.","v1.9.7.3: Giữ điều khiển khi tắt màn hình, toàn bộ tính năng v1.9.7.2 và nguồn OTA chính thức của từng ROM."),
 Nxt_text("v1.9.7.2: Fix repeated page-entrance animation on Original Now Playing. A single page identity prevents horizontal jumping on periodic redraws.","v1.9.7.2: Sửa hiệu ứng vào trang lặp lại trong Đang phát giao diện gốc, gây dịch ngang mỗi lần vẽ lại."),
 Nxt_text("v1.9.7.2: Settings > Playback & audio > Playback > Screen-off controls. When enabled, button mappings and volume work without waking the screen. Power wakes it; holding SELECT still locks/unlocks controls. Default off preserves any-button wake.","v1.9.7.2: Cài đặt > Phát nhạc & âm thanh > Cách phát > Điều khiển khi tắt màn hình. Khi bật, nút và âm lượng vẫn hoạt động, màn hình giữ tắt. Nguồn bật sáng; giữ SELECT vẫn khóa/mở khóa nút. Mặc định tắt để giữ cách đánh thức cũ."),
 Nxt_text("v1.9.7.1: 60 font choices, including 38 new families with Vietnamese support. Preserve previous font selections and multilingual fallback. Choose fonts in Original interface options.","v1.9.7.1: 60 lựa chọn font, thêm 38 họ chữ hỗ trợ tiếng Việt. Giữ lựa chọn font cũ và dự phòng chữ đa ngôn ngữ. Chọn bộ font trong Tùy chọn giao diện gốc."),
 Nxt_text("v1.9.7.1: Preview lyric colours, bold, outline, shadow, size and transitions directly over the current cover. X switches Now Playing / Full lyrics; Y edits RGB on colour fields. A applies; B cancels. Labelled bilingual sample lyrics do not alter the playing song or translation cache.","v1.9.7.1: Xem trước màu lời, chữ đậm, viền, bóng, cỡ chữ và hiệu ứng ngay trên bìa hiện tại. X đổi Đang phát / Full lời; Y chỉnh RGB tại mục màu. A áp dụng; B hủy. Lời mẫu song ngữ có nhãn riêng, không sửa bài đang phát hay cache lời dịch."),
 Nxt_text("v1.9.7.1: Retains v1.9.7 playback, saved profiles, lyrics, shuffle and per-ROM OTA sources for Stock, NextUI and NX Redux.","v1.9.7.1: Giữ phát nhạc, hồ sơ đã lưu, lời bài hát, ngẫu nhiên và nguồn OTA riêng của v1.9.7 cho Stock, NextUI và NX Redux."),
 Nxt_text("v1.9.7: Auralis transport controls centred at both resolutions, four auxiliary icons balanced on either side. Larger song title with two-line fitting; independent title/artist/lyric/translation colours, bold lyrics, cached outline and shadow, saved per listener.","v1.9.7: Căn giữa cụm trước–phát/dừng–sau và cân bốn icon phụ ở cả hai độ phân giải. Tên bài lớn hơn, tự chia hai dòng; chỉnh màu tên bài/ca sĩ/lời/bản dịch riêng, lời đậm, viền và bóng có cache, lưu theo hồ sơ."),
 Nxt_text("v1.9.7: Ten lyric transition choices. AI timing aligns partial words and ignores accent differences instead of dropping a whole sentence. Optional offline estimates for missing timings are labelled ESTIMATED; real Enhanced LRC stays preferred. Groq audio analysis still needs consent; cached results work offline, and manual retry refreshes the analysis.","v1.9.7: 10 lựa chọn hiệu ứng chuyển lời. Ghép mốc AI theo từ khớp, bỏ qua khác dấu thay vì loại cả câu. Có tô chữ ước lượng offline cho phần thiếu mốc, hiển thị ƯỚC LƯỢNG; ưu tiên Enhanced LRC thật. Phân tích âm thanh Groq vẫn cần đồng ý; mốc đã lưu dùng offline, thử lại sẽ nhận diện mới."),
 Nxt_text("v1.9.7: With shuffle on, both previous and next choose randomly in local/Drive/Spotify/SoundCloud queues; with shuffle off, they follow the existing order. Preserves all v1.9.6 features and each ROM's official OTA source. No universal singing-sync or physical 60 FPS/temperature guarantee.","v1.9.7: Bật ngẫu nhiên thì cả bài trước và sau đều chọn ngẫu nhiên cho hàng đợi trên thẻ/Drive/Spotify/SoundCloud; tắt thì theo thứ tự cũ. Giữ các tính năng v1.9.6 và nguồn OTA riêng từng ROM. Không cam kết khớp giọng mọi bài hoặc FPS/nhiệt độ chưa đo trên máy."),
 Nxt_text("v1.9.6: Auralis charcoal background and soft-white type/selection from the reference image, black text/icons in the active selection, HarmonyOS Sans reference font for menus and lyrics. Font family stays in Original theme options; the duplicate Personal appearance entry is removed.","v1.9.6: Auralis xám than, chữ và ô chọn xám trắng theo ảnh mẫu, chữ/icon đen trong ô đang chọn; font HarmonyOS Sans cho menu và lời. Bộ font giữ trong Tùy chọn giao diện gốc, bỏ mục trùng ở Giao diện cá nhân."),
 Nxt_text("v1.9.6: Six lyric transitions: Off, Glide up, Fade, Glide + fade, Slide from right, Soft rise. Original and italic translation move together in Now Playing/full lyrics; effects are cached and stop when idle/hidden. Menu selection responds immediately, without full-screen crossfade ghosts.","v1.9.6: 6 hiệu ứng lời: Tắt, Trượt lên, Mờ nhẹ, Trượt + mờ, Lướt từ phải, Nổi nhẹ. Lời gốc và dịch nghiêng chuyển cùng nhau trên Đang phát/Full lời; dùng cache, ngừng khi đứng yên/ẩn. Chọn menu phản hồi ngay, bỏ bóng chồng toàn màn."),
 Nxt_text("v1.9.6: Fine volume rejects duplicate coarse changes from ROM keymon around app volume presses, retaining one fine level per input event. Independent output volumes, native DAC/DSD and all existing audio routes remain.","v1.9.6: âm lượng từng nấc loại thay đổi thô trùng từ keymon của ROM quanh lần bấm trong app. Giữ âm lượng riêng theo đầu ra, DAC/DSD nguyên bản và các đường phát hiện có."),
 Nxt_text("v1.9.6: Optional Voice-timed karaoke via Groq Whisper for local songs up to 10 minutes. Off by default; explicit audio-upload consent, low-priority cancellable worker, exact-text matching and offline timing cache. Singing recognition can fail; protected online streams are not recorded. Enhanced LRC word timestamps remain supported offline.","v1.9.6: thêm Karaoke theo giọng hát qua Groq Whisper cho nhạc trên thẻ tối đa 10 phút. Mặc định tắt; xác nhận gửi âm thanh, chạy nền ưu tiên thấp/có hủy, chỉ ghép chữ khớp lời và lưu mốc offline. Nhận diện tiếng hát có thể sai; không ghi luồng online được bảo vệ. Giữ Enhanced LRC offline có mốc chữ."),
 Nxt_text("v1.9.5: Auralis Black Glass. Neutral black throughout menus, compact icon-only player controls, clearer native-resolution cover and independent Now Playing / full-lyrics cover brightness saved per listener. Auralis options appear only while this theme is selected.","v1.9.5: Auralis Black Glass, màu đen xuyên suốt menu, nút kính nhỏ chỉ hiện icon, bìa rõ hơn và hai độ sáng bìa Đang phát / Full lời lưu riêng theo hồ sơ. Tùy chỉnh Auralis chỉ hiện khi chọn theme này."),
 Nxt_text("v1.9.5: 60 Hz target for short cached menu transitions across themes, moving lyric pairs and explicitly timed words. Idle/screen-off visuals remain bounded; FFT and CPU modes stay separate. Actual frame rate, heat and battery depend on the ROM/device.","v1.9.5: mục tiêu 60 Hz cho chuyển menu có cache trên các theme, chuyển cặp lời và tô tiếng có mốc thật. Giảm vẽ khi đứng yên / tắt màn; giữ giới hạn FFT và chế độ CPU riêng. FPS, nhiệt và pin thực tế cần kiểm tra trên máy."),
 Nxt_text("v1.9.5: Enhanced LRC karaoke with real word timestamps, no guessed word timings for line-only lyrics. Bilingual translation remains italic. Full lyrics hides spectrum without changing its saved setting. Volume labels show individual levels without a percent sign.","v1.9.5: karaoke từ mốc từng tiếng Enhanced LRC; lời chỉ có mốc câu giữ chạy theo câu. Bản dịch song ngữ vẫn in nghiêng. Full lời ẩn phổ, không đổi lựa chọn phổ đã lưu. Âm lượng hiển thị từng nấc, bỏ dấu phần trăm."),
 Nxt_text("v1.9.5: 22 font choices. Added unmodified Huawei HarmonyOS Sans / Condensed, OPPO Sans 4.0, Google Roboto Condensed and Roboto Slab; existing Xiaomi MiSans and all 17 prior fonts stay at the same indices. Font selection now applies to every theme. Copyright notices and full licenses are included.","v1.9.5: 22 bộ font. Thêm nguyên bản Huawei HarmonyOS Sans / Condensed, OPPO Sans 4.0, Google Roboto Condensed và Roboto Slab; giữ Xiaomi MiSans và đủ 17 font cũ đúng vị trí. Font đã chọn dùng cho mọi theme. Kèm thông tin bản quyền và giấy phép đầy đủ."),
 Nxt_text("v1.9.5 retains the v1.9.4 audio pipeline, DSD Native/DoP, DAC labels, Bit-Perfect/Auto, Bluetooth, Spotify/SoundCloud/Drive, Groq/MyMemory/Google, 256 MiB translation cache, library index, Wi-Fi transfer, profiles, ROM integration and the three official OTA sources.","v1.9.5 giữ luồng âm thanh v1.9.4, DSD Native/DoP, nhãn DAC, Bit-Perfect/Tự động, Bluetooth, Spotify/SoundCloud/Drive, Groq/MyMemory/Google, cache dịch 256 MB, chỉ mục nhạc, chép Wi-Fi, hồ sơ, tích hợp ROM và ba nguồn OTA chính thức."),
 Nxt_text("v1.9.4: Auralis Themes with full-screen album art, paired bilingual lyrics, separate Singing now / Up next cues and vertical transitions in Now Playing and full lyrics. Translations are italic in both views. MiSans Semibold by Xiaomi is added without removing existing fonts/themes.","v1.9.4: thêm Auralis Themes với bìa album toàn màn hình, cặp lời song ngữ, phân biệt Đang hát / Tiếp theo và chuyển dòng dọc ở Đang phát lẫn xem lời đầy đủ. Bản dịch in nghiêng ở cả hai màn hình. Thêm MiSans Semibold của Xiaomi, giữ nguyên font và theme cũ."),
 Nxt_text("v1.9.4: real 1% volume steps with per-output saved levels; remove the overlapping firmware shortcut hints. Retains all v1.9.3 features, indexed library, original DSD/DoP, Bit-Perfect, online sources, translation/cache, ROM integration and official OTA. Cached cover and bounded redraws; no new processing in the audio callback.","v1.9.4: âm lượng tăng/giảm thực từng 1%, nhớ mức riêng cho từng đầu ra; bỏ lớp gợi ý phím hệ thống bị chồng. Giữ toàn bộ tính năng v1.9.3, chỉ mục thư viện, DSD/DoP nguyên bản, Bit-Perfect, nguồn online, dịch/cache, tích hợp ROM và OTA chính thức. Lưu sẵn nền album, giới hạn vẽ lại; không thêm xử lý vào luồng âm thanh."),
 Nxt_text("v1.9.3 Stock / NextUI / NX Redux: open the saved library immediately, check file changes once in the background, skip unchanged media reads and database writes, batch updates, retain library views on idle frames, and refresh after completed Wi-Fi uploads. START library options: check new music or rebuild all metadata.","v1.9.3 Stock / NextUI / NX Redux: mở ngay thư viện đã lưu, kiểm tra thay đổi một lượt ở nền, bỏ đọc lại bài và ghi chỉ mục không đổi, gom cập nhật theo đợt, giữ danh sách khi đứng yên và nhận nhạc sau khi chép Wi-Fi xong. START trong Thư viện: kiểm tra nhạc mới hoặc quét lại toàn bộ thông tin."),
 Nxt_text("v1.9.3 Stock/NextUI also includes the v1.9.2 DSD transport fixes, confirmed Native-to-DoP mode, saved DAC display names and detailed USB/DSD diagnostics.","v1.9.3 Stock/NextUI giữ sửa lỗi DSD của v1.9.2: tự động Native sang DoP có xác nhận, nhớ tên hiển thị DAC và log USB/DSD chi tiết."),
 Nxt_text("v1.9.2 NX Redux: preserve the real DSD hw failure, explicit Native/DoP automatic mode without PCM conversion, saved DAC display names, USB identity and ROM formats in exported diagnostics.","v1.9.2 NX Redux: giữ đúng lỗi mở DSD, chế độ tự động Native/DoP có xác nhận và không chuyển PCM, nhớ tên hiển thị DAC, xuất nhận dạng USB và định dạng driver ROM trong log thiết bị."),
 Nxt_text("v1.9.1 · all three ROMs: 256 MiB translation cache on SD, no 512-song cap, bounded background cleanup. Original USB DSD and Groq TXT import now also on Stock/NextUI. Existing LRCLIB original-lyrics lookup feeds automatic translation when enabled.","v1.9.1 · cả ba ROM: cache lời dịch 256 MB trên thẻ, bỏ giới hạn 512 bài, dọn nền với bộ nhớ giới hạn. Stock/NextUI thêm DSD USB nguyên bản và nhập Groq TXT. Giữ tìm lời gốc LRCLIB rồi tự dịch khi bật dịch."),
 Nxt_text("v1.9 NX Redux: original local DSF/DFF DSD64-512 to compatible USB DAC. Native DSD default, explicit DoP selection, no PCM conversion/DSP/FFT. DAC removal pauses DSD. Retains v1.8.9.1 features and OTA.","v1.9 NX Redux: DSD64-512 nguyên bản từ DSF/DFF trên thẻ đến DAC USB tương thích. Mặc định Native DSD, chọn riêng DoP; không chuyển PCM/DSP/FFT. Rút DAC tạm dừng DSD. Giữ tính năng v1.8.9.1 và OTA."),
 Nxt_text("v1.8.9.1 NX Redux: Tools icon for official v2.1.2, with a verified launcher backup.","v1.8.9.1 NX Redux: icon trong Tools cho ROM chính thức v2.1.2, có sao lưu launcher gốc."),
 Nxt_text("Three effective CPU modes: Battery / Balanced / Performance. CPU limits restore when hidden or exiting.","Ba chế độ CPU có tác dụng thật: Tiết kiệm pin / Cân bằng / Hiệu năng. Trả quyền CPU cho ROM khi nghe nền hoặc thoát."),
 Nxt_text("Home receives NXT track information and controls the current local, Spotify or SoundCloud queue.","Bảng nhạc Home nhận thông tin từ NXT và điều khiển hàng đợi nhạc trong thẻ, Spotify hoặc SoundCloud."),
 Nxt_text("LED colors and effects apply immediately; firmware frame animation is released and restored.","Màu và hiệu ứng LED áp dụng ngay; nhường và phục hồi animation của ROM đúng cách."),
 Nxt_text("Groq key: Type or import groq-api-key.txt from Music / beside launch.sh. Existing services and the official NX OTA source are retained.","Khóa Groq: nhập hoặc lấy từ groq-api-key.txt trong Music / cạnh launch.sh. Giữ các dịch vụ hiện có và nguồn OTA NX chính thức."),
 Nxt_text("v1.8.9: Groq lyric translation alongside retained MyMemory and Google Cloud. Separate API keys, selectable Groq models and checked line IDs for bilingual lyrics.","v1.8.9: thêm dịch lời Groq, giữ MyMemory và Google Cloud. Khóa API riêng, chọn model Groq và kiểm tra ID để ghép đúng từng dòng song ngữ."),
 Nxt_text("v1.8.9: larger Groq groups reduce network/helper starts; repeated lines use saved results. 429 waits for the provider retry/reset time. One background worker; audio/rendering unchanged.","v1.8.9: Groq dịch nhóm lớn hơn, giảm lượt mạng/helper; câu lặp dùng kết quả đã lưu. Lỗi 429 chờ theo dịch vụ. Một worker nền; giữ pipeline âm thanh và độ sắc nét."),
#ifdef NXT_ROM_NXREDUX
 Nxt_text("v1.8.9: the official NX Redux OTA source is included. Legacy sources migrate automatically; open Software update to check releases for NX Redux.","v1.8.9: gắn nguồn OTA chính thức cho NX Redux, tự chuyển nguồn cũ. Vào Cập nhật phần mềm để kiểm tra phiên bản dành cho NX Redux."),
#else
 Nxt_text("v1.8.9: official Stock and NextUI OTA sources are included. Legacy sources migrate automatically; open Software update to check the correct ROM. NX Redux release comes later.","v1.8.9: gắn nguồn OTA chính thức cho Stock và NextUI, tự chuyển nguồn cũ. Vào Cập nhật phần mềm để kiểm tra đúng ROM. NX Redux phát hành sau."),
#endif

 Nxt_text("v1.8.8.2: new release version for OTA upgrades from v1.8.8.1, including all LyricsFix2 fixes and italic translations in the full lyrics screen.","v1.8.8.2: tăng phiên bản để OTA nâng từ v1.8.8.1; giữ các sửa lỗi LyricsFix2 và chữ nghiêng cho bản dịch ở màn hình lời đầy đủ."),
 Nxt_text("Delete listeners in Personal > Profile > Edit profile, or press X on a selected listener. Confirmation required; keeps one listener and preserves music files.","Xóa người nghe trong Cá nhân > Hồ sơ > Sửa hồ sơ, hoặc nhấn X ở màn hình chọn người nghe. Có xác nhận; giữ một hồ sơ và giữ tệp nhạc."),
 Nxt_text("LyricsFix2: original lyric fetching now accepts plain text and matching cover titles. Missing originals / source-network failures have distinct status and retry.","LyricsFix2: nhận lời gốc dạng văn bản và tìm theo tên cơ bản cho bài cover. Phân biệt thiếu lời gốc / lỗi tải nguồn và có thử lại."),
 Nxt_text("LyricsFix2: isolated translation-helper environment and player descriptors, moderate worker priority, provider error diagnostics and orphan endpoint recovery after the old process is dead.","LyricsFix2: tách môi trường và kết nối của helper dịch khỏi trình phát, ưu tiên nền vừa phải, log lỗi dịch vụ và phục hồi endpoint khi tiến trình cũ đã chết."),

 Nxt_text("v" NXT_VERSION " · Smoother translation and bilingual Now Playing.","v" NXT_VERSION " · Tối ưu tác vụ dịch và song ngữ trên Đang phát."),
 Nxt_text("NEW: original and translation appear together as two fixed rows in Now Playing across all 50 themes. Full-screen lyrics remains available with Y; translated rows are italic only there.","MỚI: lời gốc và bản dịch hiện cùng lúc ở hai hàng cố định trên Đang phát, đủ 50 theme. Giữ màn hình lời đầy đủ bằng Y; riêng các hàng dịch ở đây dùng chữ nghiêng."),
 Nxt_text("NEW: small batches reduce translation requests and helper starts. Google uses multi-q; MyMemory checks line boundaries and falls back safely.","MỚI: dịch nhóm nhỏ để giảm lượt mạng và khởi chạy helper. Google dùng multi-q; MyMemory kiểm tra đủ dòng và tự dùng từng câu khi cần."),
 Nxt_text("NEW: temporary network failures automatically resume missing cues. A failed cue does not abandon the rest of the song; service quotas use a timed backoff.","MỚI: lỗi mạng tạm thời tự tiếp tục các câu còn thiếu. Một câu lỗi không bỏ phần còn lại của bài; hết hạn mức sẽ chờ theo thời gian."),
 Nxt_text("NEW: moderate-priority worker, posix_spawn helpers and nonblocking UI result collection. Translation/Bilingual toggles reuse the current translation job.","MỚI: worker ưu tiên nền vừa phải, helper posix_spawn, nhận kết quả không chặn UI. Đổi Bản dịch/Song ngữ dùng tiếp tác vụ hiện tại."),
 Nxt_text("RETAINED: language follows the app; default Off, MyMemory or optional own Google Cloud key. Original v1.8.8 caches, lyric timing/offset and Y screen remain.","GIỮ: dịch sang ngôn ngữ app, mặc định Tắt, MyMemory hoặc Google Cloud với khóa riêng. Dùng lại cache v1.8.8, giữ mốc/độ trễ lời và màn hình Y."),
 Nxt_text("Online speed/completeness depends on Wi-Fi and service quota. Hidden or screen-off playback pauses requests to save work; cached lyrics remain available.","Tốc độ và số câu dịch còn phụ thuộc Wi-Fi/hạn mức. Ẩn app hoặc tắt màn hình tạm dừng yêu cầu dịch; vẫn dùng bản dịch đã lưu."),
 Nxt_text("Retained from v1.8.7: 50 themes including Original NXT, sharp fonts/artwork, saved profiles, compact controls and current-version release notes.",
          "Giữ từ v1.8.7: 50 giao diện gồm NXT gốc, chữ/ảnh sắc nét, hồ sơ đã lưu, phím gọn và ghi chú đúng phiên bản."),
 Nxt_text("Retained from v1.8.6: volume can be changed on the device or DAC. The forced Bit-Perfect / use DAC volume message is removed.",
          "Giữ từ v1.8.6: chỉnh âm lượng trên máy hoặc DAC. Đã bỏ thông báo bắt buộc Bit-Perfect / chỉnh âm lượng trên DAC."),
 Nxt_text("Retained from v1.8.6: online Spotify, SoundCloud, Drive, radio and podcasts use Normal playback. Automatic resumes eligible local Bit-Perfect on USB DAC.",
          "Giữ từ v1.8.6: Spotify, SoundCloud, Drive, radio và podcast dùng Thường. Tự động trở lại Bit-Perfect cho nhạc local phù hợp với DAC USB."),
 Nxt_text("Retained from v1.8.6: Library & files contains Music folder and Wi-Fi file transfer. The redundant System / Wi-Fi entry is removed.",
          "Giữ từ v1.8.6: Thư viện & tệp chỉ có Thư mục nhạc và Chuyển file Wi-Fi. Bỏ mục Hệ thống / Wi-Fi bị trùng."),
 Nxt_text("Retained from v1.8.5: select a listener first, then open their last song and position in Now Playing, paused by default. Listening choices stay in Personal.",
          "Giữ từ v1.8.5: chọn hồ sơ trước, rồi Đang phát mở bài/vị trí nghe dở, mặc định tạm dừng. Lựa chọn nghe nằm trong Cá nhân."),
 Nxt_text("Retained from v1.8.5: Playback & audio has Playback and Sound & output. Normal / Bit-Perfect / Automatic is under Sound & output.",
          "Giữ từ v1.8.5: Phát nhạc & âm thanh có Cách phát và Âm thanh & đầu ra. Thường / Bit-Perfect / Tự động nằm trong Âm thanh & đầu ra."),
 Nxt_text("Retained from v1.8.5: NextUI performance modes apply supported CPUFreq caps and visual budgets, then restore ROM policy on exit. Stock / NX Redux keep ROM CPU control.",
          "Giữ từ v1.8.5: NextUI áp dụng giới hạn CPUFreq hỗ trợ và nhịp vẽ, trả chính sách ROM khi thoát. Stock / NX Redux giữ CPU do ROM quản lý."),
 Nxt_text("Retained: SELECT opens EQ; SELECT again closes it without selecting a preset. B still goes back.",
          "Giữ: SELECT mở EQ; SELECT lần nữa đóng mà không chọn preset. B vẫn quay lại."),
 Nxt_text("Retained: supported local lossless integer PCM can bypass EQ, DSP, limiter, ReplayGain and resampling through USB DAC. Software volume fallback uses Normal.",
          "Giữ: PCM số nguyên lossless local tương thích có thể bỏ qua EQ, DSP, limiter, ReplayGain và resample qua DAC USB. Âm lượng phần mềm dự phòng dùng Thường."),
 Nxt_text("Retained: sleep timer saves position and closes the app and playback helpers. Firmware idle/sleep settings then control device sleep.",
          "Giữ: hẹn giờ lưu vị trí và thoát app cùng tiến trình phát. Cài đặt chờ/ngủ của ROM điều khiển máy ngủ sau đó."),
 Nxt_text("Retained: Original NXT wallpaper/album/vinyl, compact icon controls in other themes, whole-line lyrics, queue count, source quality and output/DAC information.",
          "Giữ: nền/album/đĩa than NXT gốc, nút icon ở giao diện khác, lời nguyên câu, số bài, chất lượng nguồn và thông tin đầu ra/DAC."),
 Nxt_text("Retained: per-profile appearance, instant LEDs, 300 icons, 180 3D emoji, 14 languages and keyboards; English on first launch.",
          "Giữ: giao diện theo hồ sơ, LED đổi ngay, 300 icon, 180 emoji 3D, 14 ngôn ngữ và bàn phím; lần đầu tiếng Anh."),
 Nxt_text("Separate Stock, NextUI and NX Redux installers and OTA packages. OTA checks ROM, version, size and SHA-256 and preserves personal data.",
          "Bộ cài và OTA riêng cho Stock, NextUI và NX Redux. OTA kiểm tra ROM, phiên bản, dung lượng, SHA-256 và giữ dữ liệu cá nhân.")
 };return Nxt_document(d,Nxt_text("What’s new · v" NXT_VERSION,"Những thay đổi · v" NXT_VERSION),rows,sizeof(rows)/sizeof(*rows));}

ModuleExitReason Nxt_help_v171(DisplayContext *d){
 const int logical[]={4,5,6,7,8,9,10,11,14,15};
 const char *en[]={"play / pause","main menu, keep playback state","playing queue","full-screen lyrics","previous track","next track","one press: next spectrum color; last → off → first","favorite / unfavorite current track","EQ / DSP","quick options"};
 const char *vi[]={"phát / tạm dừng","menu chính, giữ trạng thái nghe","hàng đợi đang phát","lời toàn màn hình","bài trước","bài tiếp theo","nhấn một lần: đổi màu phổ; cuối → tắt → màu đầu","thêm / bỏ yêu thích bài hiện tại","EQ / DSP","tùy chọn nhanh"};
 char mapped[16][256];const char *rows[96];int n=0;
rows[n++]=Nxt_text("DSD: Sound & output → DSD USB output. Compatible USB DAC required, Native default or explicit DoP. No DSD to speaker/Bluetooth. Hardware DAC volume only when supported.","DSD: Âm thanh & đầu ra → Đầu ra DSD USB. Cần DAC USB tương thích, mặc định Native hoặc chọn riêng DoP. Không phát DSD qua loa/Bluetooth. Âm lượng phần cứng DAC khi được hỗ trợ.");
 rows[n++]=Nxt_text("Start by choosing or creating a listener. Personal has four tabs: For you, Statistics, History, Profile. L1/R1 switches tabs.","Mở app chọn/tạo người nghe trước. Cá nhân có bốn tab: Dành cho bạn, Thống kê, Lịch sử, Hồ sơ. L1/R1 chuyển tab.");
 rows[n++]=Nxt_text("Personal → Profile → Personal appearance: theme, RGB, fonts and unified Wallpaper gallery. Online images and JPG/PNG from SD are in the same place.","Cá nhân → Hồ sơ → Giao diện cá nhân: giao diện, RGB, font và Kho hình nền. Ảnh online và ảnh JPG/PNG trên thẻ nằm cùng một chỗ.");
 rows[n++]=Nxt_text("Wallpaper gallery: L1/R1 changes five tabs; D-pad/analogs selects; L2/R2 pages; START chooses online source or built-in offline backgrounds.","Kho hình nền: L1/R1 đổi năm tab; D-pad/analog chọn ảnh; L2/R2 đổi trang; START chọn nguồn online hoặc nền có sẵn offline.");
 rows[n++]=Nxt_text("A opens and loads the full picture, A again previews the home menu. X saves offline/applies, Y favorites; L2 in picture detail switches Smooth / Preserve pixels.","A mở và tải ảnh đầy đủ, A lần nữa xem trên menu. X lưu/dùng nền, Y yêu thích; L2 trong trang xem đổi Mượt / Giữ nét pixel.");
 rows[n++]=Nxt_text("In menu preview, left/right changes panel opacity, SELECT toggles dark/light, A saves to this listener and B cancels. Prefer 15–35% cover to show more wallpaper.","Khi xem menu, trái/phải chỉnh độ phủ ô, SELECT đổi sáng/tối, A lưu cho người nghe này, B hủy. Độ phủ 15–35% giúp thấy nền rõ hơn.");
 rows[n++]=Nxt_text("SD images: place JPG/PNG in Pictures/Wallpapers or browse another folder. Images up to 10 MiB and 16 million pixels; background is prepared for the 1024×768 display. Downloads work offline.","Ảnh trên thẻ: chép JPG/PNG vào Pictures/Wallpapers hoặc duyệt thư mục khác. Ảnh tối đa 10 MiB và 16 triệu điểm ảnh; nền được chuẩn bị theo màn hình 1024×768. Ảnh đã tải dùng được offline.");
 rows[n++]=Nxt_text("Online gallery: 24 categories; scroll beyond the last result or R2 to load more pages. Reopen and reapply older downloaded pictures to refresh their quality cache.","Kho online: 24 thể loại; cuộn qua ảnh cuối hoặc R2 tải tiếp nhiều trang. Mở và áp dụng lại ảnh tải ở bản cũ để làm mới chất lượng nền.");
 rows[n++]=Nxt_text("Statistics: left/right changes period; Y opens details. History: up/down selects a track; A plays; L2/R2 pages. Profile: edit nickname/avatar or open personal appearance.","Thống kê: trái/phải đổi thời gian, Y xem chi tiết. Lịch sử: lên/xuống chọn bài, A phát, L2/R2 đổi trang. Hồ sơ: sửa biệt danh/avatar hoặc mở giao diện cá nhân.");
 rows[n++]=Nxt_text("OTA Stock: the publisher source is built in. Open Update to check automatically; see notes, download/verify, then confirm install. B returns while downloading; reopen after successful installation.","OTA Stock: đã nhúng nguồn nhà phát hành. Mở Cập nhật để tự kiểm tra; xem thay đổi, tải/kiểm tra rồi xác nhận cài. B quay lại vẫn tải nền; mở lại sau khi cài xong.");
 rows[n++]=Nxt_text("About START exports NXT_App_Log.txt; recording can be disabled per profile. Listening counts after 30 seconds or half of a shorter track.","START trong Thông tin xuất NXT_App_Log.txt; có thể tắt ghi lịch sử theo hồ sơ. Lượt nghe tính sau 30 giây hoặc nửa bài ngắn.");
 rows[n++]=Nxt_text("X: v" NXT_VERSION " changes; ↑↓ scroll, L1/R1 page, B back.","X: thay đổi v" NXT_VERSION "; ↑↓ cuộn, L1/R1 trang, B quay lại.");
 rows[n++]=Nxt_text("Drive keeps up to two 256 MiB temporary SD buffers; cached seeks reuse bytes and the next track is prefetched. Low SD space or oversized files fall back to HTTPS Range.","Drive giữ tối đa hai đệm tạm 256 MiB trên thẻ; tua dùng lại dữ liệu và tải trước bài kế. Thẻ ít trống/tệp quá lớn dùng HTTPS Range.");
 rows[n++]=Nxt_text("Online results extend as you scroll and while listening; not limited to 20 songs.","Kết quả online tải thêm khi cuộn và khi nghe; không giới hạn 20 bài.");
#ifdef NXT_ROM_STOCK
 rows[n++]=NXT_BUILD_REVISION;
 rows[n++]=Nxt_text("Side Home: Stock quick panel (Task Manager)","Home bên hông: bảng nhanh Stock (Task Manager)");
 rows[n++]=Nxt_text("SELECT + Up/Down: volume; Left/Right: brightness","SELECT + Lên/Xuống: âm lượng; Trái/Phải: độ sáng");
 rows[n++]=Nxt_text("D-pad and both sticks share navigation and shortcuts","D-pad và hai cần analog dùng chung điều hướng và phím tắt");
 rows[n++]=Nxt_text("DAC volume keys: keep the selected output","Nút âm lượng DAC: giữ đầu ra đã chọn");
 rows[n++]=Nxt_text("Auto DAC defaults on; unplug switches to speaker. Audio & DAC settings can change this.","Mặc định tự chuyển sang DAC; rút DAC về loa. Có thể đổi trong Âm thanh & DAC.");
 rows[n++]=Nxt_text("DAC volume works during background playback","Nút âm lượng DAC hoạt động khi phát nền");
#else
 rows[n++]=NXT_BUILD_REVISION;
 rows[n++]=Nxt_text("SELECT + Up/Down: volume; Left/Right: brightness","SELECT + Lên/Xuống: âm lượng; Trái/Phải: độ sáng");
 rows[n++]=Nxt_text("D-pad and both sticks share navigation and shortcuts","D-pad và hai cần analog dùng chung điều hướng và phím tắt");
#endif
 rows[n++]=Nxt_text("Google Drive: add a public folder/audio link; A opens/plays","Google Drive: thêm link thư mục/tệp công khai; A mở/phát");
 rows[n++]=Nxt_text("Drive: stream online with private temporary SD buffers; cleared on stop","Drive: nghe online, đệm tạm riêng trên thẻ; dọn khi dừng");
 rows[n++]=Nxt_text("Drive: Play All queues the folder; selecting a song keeps the full queue","Drive: Nghe cả thư mục; chọn một bài vẫn giữ đầy đủ hàng đợi");
 rows[n++]=Nxt_text("Folders: natural numeric order, then A-Z","Thư mục: xếp số tự nhiên, rồi A-Z");
 rows[n++]=Nxt_text("Keyboard Y: Telex/Latin; A type, B erase, X space","Bàn phím Y: Telex/Latin; A nhập, B xóa, X cách");
 rows[n++]=Nxt_text("Telex aa/aw/ee/oo/ow/uw/dd; s/f/r/x/j tones; z clears","Telex aa/aw/ee/oo/ow/uw/dd; s/f/r/x/j đặt dấu; z bỏ dấu");
 rows[n++]=Nxt_text("Keyboard L1 Shift, R1 Caps, SELECT Done","Bàn phím L1 viết hoa, R1 khóa hoa, SELECT xong");
 rows[n++]=Nxt_text("SoundCloud: public track and playlist search","SoundCloud: tìm nhạc và playlist công khai");
 rows[n++]=Nxt_text("SoundCloud public mode: no API key or Pro account needed","SoundCloud công khai: không cần API key hay tài khoản Pro");
 rows[n++]=Nxt_text("Free listeners: eligible tracks; Go+ limits apply","Người nghe Free: bài được phép; vẫn có giới hạn Go+");
 rows[n++]=Nxt_text("SoundCloud R2: save bookmarks on this device","SoundCloud R2: lưu bài yêu thích trên máy");
 rows[n++]=Nxt_text("SoundCloud source QR is available in its source menu","SoundCloud: QR liên kết gốc nằm trong menu nguồn");
 rows[n++]=Nxt_text("SoundCloud LRCLIB lyrics: matching tracks only","Lời SoundCloud qua LRCLIB: chỉ hiện khi khớp bài");
 rows[n++]=Nxt_text("Spotify: choose music and play on the Brick","Spotify: chọn bài và phát trực tiếp trên Brick");
 rows[n++]=Nxt_text("Spotify needs Premium; local music does not","Spotify cần Premium; nhạc trên thẻ không cần");
 rows[n++]=Nxt_text("QR: Online music → Spotify → Log in by QR","QR: Nhạc online → Spotify → Đăng nhập bằng QR");
 rows[n++]=Nxt_text("Scan and approve at spotify.com/pair","Quét mã và xác nhận tại spotify.com/pair");
 rows[n++]=Nxt_text("Spotify: playback login is separate from library API; use a track link if only library is restricted.","Spotify: đăng nhập phát riêng với API thư viện; có thể phát link nếu chỉ thư viện bị giới hạn.");
 rows[n++]=Nxt_text("Your login is saved for the next launch","App giữ đăng nhập cho lần mở tiếp theo");
 rows[n++]=Nxt_text("Shared Now Playing and mini player interface","Màn hình Đang phát và thanh phát thu gọn dùng chung");
 rows[n++]=Nxt_text("Spotify / LRCLIB lyrics; timed when available","Lời Spotify / LRCLIB; đồng bộ khi có mốc thời gian");
 rows[n++]=Nxt_text("B to menu or screen off: music continues","B về menu hoặc tắt màn hình: nhạc tiếp tục");
 rows[n++]=Nxt_text(NXT_FIRMWARE_NAME ": START → Play in background","Về " NXT_FIRMWARE_NAME ": START → Phát nền");
 rows[n++]=Nxt_text("L2 spectrum colors; R2 Spotify favorites","L2 đổi màu phổ; R2 lưu yêu thích Spotify");
 rows[n++]=Nxt_text("Bottom USB: Audio & DAC → Bottom USB port","USB dưới: Âm thanh & DAC → Cổng USB dưới");
 rows[n++]=Nxt_text("Controls · Your current button mapping","Điều khiển · Ánh xạ nút hiện tại");
 for(int i=0;i<16;i++){snprintf(mapped[i],sizeof(mapped[i]),"%s: %s",Nxt_keyNames[i],NxtControl_name(nxt.playing_keys[i]));rows[n++]=mapped[i];}
 rows[n++]=Nxt_text("Power: short press switches screen; music continues","Nguồn: nhấn ngắn tắt / bật màn hình, nhạc vẫn chạy");
 rows[n++]=Nxt_text("Long Power: " NXT_FIRMWARE_NAME " shutdown; saves local position","Giữ nút nguồn: tắt máy qua " NXT_FIRMWARE_NAME ", lưu vị trí nhạc thẻ");
 rows[n++]=Nxt_text("Any button wakes; the wake gesture is consumed","Nút bất kỳ bật màn hình; thao tác bật không đổi nhạc");
 rows[n++]=Nxt_text("Hold physical SELECT 0.9 s: lock / unlock","Giữ SELECT trên máy 0,9 giây: khóa / mở khóa");
 rows[n++]=Nxt_text("Left / Right: seek; hold to scan local music","Trái / Phải: tua; giữ nút để tua liên tục nhạc trên thẻ");
 rows[n++]=Nxt_text("Library: L1/R1 tabs; X search; Y sort","Thư viện: L1/R1 đổi mục; X tìm; Y sắp xếp");
 rows[n++]=Nxt_text("Playlists: L1/R1 toolbar, Y executes","Playlist: L1/R1 thanh công cụ, Y thực hiện");
 rows[n++]=Nxt_text("Place UTF-8 .lrc beside each local audio file","Nhạc trên thẻ: đặt .lrc UTF-8 cùng tên bên cạnh bài");
 rows[n++]=Nxt_text("Font fallback covers multilingual titles and lyrics","Font dự phòng cho tên bài và lời đa ngôn ngữ");
 rows[n++]=Nxt_text("Sleep timer includes paused and screen-off time","Hẹn giờ tính cả thời gian tạm dừng và tắt màn hình");
 rows[n++]=Nxt_text("Timer ends: save position and close NXT, including background playback","Hết hẹn giờ: lưu vị trí và thoát NXT, kể cả khi phát nền");
 rows[n++]=Nxt_text("System settings control screen-off and sleep after NXT exits","Cài đặt hệ thống điều khiển tắt màn hình và ngủ sau khi NXT thoát");
 rows[n++]=Nxt_text("Front TrimUI logo: stop music and fully exit app","Logo TrimUI mặt trước: dừng nhạc và thoát hẳn app");
 rows[n++]=Nxt_text("Cold local resume stays paused until you press Play","Nhạc trên thẻ: khôi phục sau khởi động ở trạng thái dừng");
 rows[n++]=Nxt_text("Reopen this app to control background playback","Mở lại app này để điều khiển nhạc đang phát nền");
 return info(d,Nxt_text("Usage guide · v" NXT_VERSION "","Hướng dẫn sử dụng · v" NXT_VERSION ""),rows,n);
}

static ModuleExitReason about_details(DisplayContext *d){
 int show=0,dirty=1;uint32_t drawn=0;
 for(;;){
  ModuleCommon_frameBegin();Background_tick();SDL_Surface *s=DisplayHelper_getSurface(d);
  GlobalInputResult g=ModuleCommon_handleGlobalInput(s,&show,HELP_NONE);
  if(g.should_quit)return MODULE_EXIT_QUIT;
  if(g.input_consumed){dirty=1;ModuleCommon_frameEnd(s);continue;}
  if(NxtPad_justPressed(BTN_B))return MODULE_EXIT_TO_MENU;
  if(NxtPad_justPressed(BTN_A)){if(Nxt_help(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;dirty=1;}
  if(NxtPad_justPressed(BTN_X)){if(Nxt_changes(d)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;dirty=1;}
  if(NxtPad_justPressed(BTN_START)){Toast_show(NxtLog_export()?Nxt_text("Saved NXT_App_Log.txt","Đã lưu NXT_App_Log.txt"):Nxt_text("Cannot export app log","Không xuất được log app"),3000);dirty=1;}
  if(NxtPad_justPressed(BTN_Y)){Toast_show(NxtHardware_writeDiagnostic()?Nxt_text("Saved NXT_Device_Log.txt","Đã lưu NXT_Device_Log.txt"):Nxt_text("Cannot save device log","Không lưu được thông tin máy"),3000);dirty=1;}
  if(dirty||g.dirty||SDL_GetTicks()-drawn>=100){
   NxtSkin_begin(s,Nxt_text("About","Thông tin"),NULL,false);
   int u=s->w;int left=32*u/1024,top=145*u/1024;
   NxtSkin_rect(s,(SDL_Rect){left,top,522*u/1024,360*u/1024},NxtSkin_color(NXT_PANEL),12*u/1024);
   NxtSkin_rect(s,(SDL_Rect){578*u/1024,top,414*u/1024,360*u/1024},NxtSkin_color(NXT_PANEL),12*u/1024);
   NxtSkin_text(s,32,NXT_APP_NAME,56*u/1024,168*u/1024,462*u/1024,NxtSkin_color(NXT_INK));
   NxtSkin_text(s,22,"v" NXT_VERSION "  ·  Vibe Code",56*u/1024,221*u/1024,462*u/1024,NxtSkin_color(NXT_ACCENT));
   NxtSkin_text(s,17,NXT_BUILD_REVISION,56*u/1024,254*u/1024,462*u/1024,NxtSkin_color(NXT_MUTED));
   NxtSkin_text(s,19,Nxt_text("Idea & implementation","Ý tưởng & thực hiện"),56*u/1024,285*u/1024,462*u/1024,NxtSkin_color(NXT_MUTED));
   NxtSkin_text(s,25,"Nguyễn Xuân Thông",56*u/1024,316*u/1024,462*u/1024,NxtSkin_color(NXT_INK));
   NxtSkin_text(s,19,Nxt_text("Nickname","Biệt danh"),56*u/1024,382*u/1024,462*u/1024,NxtSkin_color(NXT_MUTED));
   NxtSkin_text(s,25,"Spring Thông",56*u/1024,413*u/1024,462*u/1024,NxtSkin_color(NXT_INK));
   NxtSkin_fitText(s,12,"MiSans · Xiaomi  /  HarmonyOS Sans · Huawei",56*u/1024,462*u/1024,462*u/1024,16*u/1024,NxtSkin_color(NXT_MUTED),false);
   NxtSkin_fitText(s,12,"OPPO Sans · OPPO  /  Google Fonts · licenses/",56*u/1024,486*u/1024,462*u/1024,16*u/1024,NxtSkin_color(NXT_MUTED),false);
   NxtSkin_icon(s,"heart",602*u/1024,170*u/1024,25*u/1024,NxtSkin_color(NXT_ACCENT));
   NxtSkin_text(s,23,Nxt_text("Support development","Ủng hộ phát triển"),642*u/1024,166*u/1024,322*u/1024,NxtSkin_color(NXT_INK));
   NxtSkin_text(s,19,"MoMo",602*u/1024,239*u/1024,366*u/1024,NxtSkin_color(NXT_MUTED));
   NxtSkin_text(s,28,"0339838267",602*u/1024,272*u/1024,366*u/1024,NxtSkin_color(NXT_ACCENT));
   NxtSkin_text(s,19,"Vietcombank (VCB)",602*u/1024,321*u/1024,366*u/1024,NxtSkin_color(NXT_MUTED));
   NxtSkin_text(s,28,"0581000786219",602*u/1024,352*u/1024,366*u/1024,NxtSkin_color(NXT_ACCENT));
   NxtSkin_text(s,19,"PayPal",602*u/1024,415*u/1024,366*u/1024,NxtSkin_color(NXT_MUTED));
   NxtSkin_compactText(s,21,"thongkhung2@gmail.com",602*u/1024,449*u/1024,366*u/1024,30*u/1024,NxtSkin_color(NXT_ACCENT));
   NxtSkin_rect(s,(SDL_Rect){left,520*u/1024,s->w-left*2,60*u/1024},NxtSkin_color(NXT_ACCENT),12*u/1024);
   NxtSkin_icon(s,"book-open",56*u/1024,536*u/1024,27*u/1024,NxtSkin_color(NXT_ACCENT_INK));
   NxtSkin_text(s,25,Nxt_text("Usage guide","Hướng dẫn sử dụng"),104*u/1024,530*u/1024,790*u/1024,NxtSkin_color(NXT_ACCENT_INK));
   NxtSkin_icon(s,"chevron-right",946*u/1024,538*u/1024,24*u/1024,NxtSkin_color(NXT_ACCENT_INK));
   NxtSkin_rect(s,(SDL_Rect){left,598*u/1024,s->w-left*2,60*u/1024},NxtSkin_color(NXT_PANEL),12*u/1024);
   NxtSkin_text(s,24,Nxt_text("X · Main features & changes in v" NXT_VERSION,"X · Những tính năng và thay đổi chính v" NXT_VERSION),56*u/1024,611*u/1024,910*u/1024,NxtSkin_color(NXT_INK));
   NxtSkin_footer(s,Nxt_text("A Guide · X Changes · START Log · Y Device · B Back","A Hướng dẫn · X Thay đổi · START Log · Y Máy · B Về"),false);
   ModuleCommon_markSurfaceDrawn();dirty=0;drawn=SDL_GetTicks();
  }
  ModuleCommon_PWR_update(&dirty,&show);ModuleCommon_frameEnd(s);
 }
}

static ModuleExitReason software_updates(DisplayContext *d){int selected=0;for(;;){const char *rows[]={Nxt_text("Check for updates","Kiểm tra cập nhật"),Nxt_text("Update instructions","Hướng dẫn cập nhật")};NxtPick p=Nxt_pick(d,Nxt_text("Software update","Cập nhật phần mềm"),rows,2,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action==1){ModuleExitReason r=selected==0?NxtOTA_run(d):NxtOTA_guide(d);if(r==MODULE_EXIT_QUIT)return r;}}}
ModuleExitReason Nxt_about(DisplayContext *d){int selected=0;for(;;){const char *rows[]={Nxt_text("Application information\nVersion, credits and support","Thông tin ứng dụng\nPhiên bản, tác giả và ủng hộ"),Nxt_text("Software update\nCheck for updates and instructions","Cập nhật phần mềm\nKiểm tra bản mới và hướng dẫn"),Nxt_text("Usage guide\nControls and music features","Hướng dẫn sử dụng\nCác phím và tính năng nghe nhạc"),Nxt_text("Features & changes\nWhat is new in v" NXT_VERSION,"Tính năng & thay đổi\nNhững điểm mới trong v" NXT_VERSION)};
 NxtPick p=Nxt_pickLive(d,Nxt_text("About, help & updates","Thông tin, hướng dẫn & cập nhật"),rows,4,&selected,Nxt_text("A Open · START Log · Y Device · B Back","A Mở · START Log · Y Máy · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;ModuleExitReason r=MODULE_EXIT_TO_MENU;
 if(p.action==3)r=Nxt_changes(d);else if(p.action==10)Toast_show(NxtLog_export()?Nxt_text("Saved NXT_App_Log.txt","Đã lưu NXT_App_Log.txt"):Nxt_text("Cannot export app log","Không xuất được log app"),3000);else if(p.action==4)Toast_show(NxtHardware_writeDiagnostic()?Nxt_text("Saved NXT_Device_Log.txt","Đã lưu NXT_Device_Log.txt"):Nxt_text("Cannot save device log","Không lưu được thông tin máy"),3000);else if(p.action==1)r=selected==0?about_details(d):selected==1?software_updates(d):selected==2?Nxt_help(d):Nxt_changes(d);if(r==MODULE_EXIT_QUIT)return r;
 }}
#include "nxt_help_v18.inc"
