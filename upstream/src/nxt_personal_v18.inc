typedef struct {int mode,icon,emoji;char text[32],path[512];} ProfileAvatar;
static int avatar_number(JSON_Object *o,const char *name,int fallback,int max){double v=json_object_has_value_of_type(o,name,JSONNumber)?json_object_get_number(o,name):fallback;return isfinite(v)&&v>=0&&v<=max?(int)v:fallback;}
static void read_profile_avatar(int id,int old,ProfileAvatar *a){
 memset(a,0,sizeof(*a));a->icon=old;char path[1400];profile_path(id,"preferences.json",path,sizeof(path));JSON_Value *v=json_parse_file(path);JSON_Object *o=json_value_get_object(v);
 if(o){a->mode=avatar_number(o,"avatar_mode",0,3);a->icon=avatar_number(o,"avatar_icon",old,299);a->emoji=avatar_number(o,"avatar_emoji",0,179);const char *s=json_object_get_string(o,"avatar_text");if(s)NxtText_copy(a->text,s,sizeof(a->text));s=json_object_get_string(o,"avatar_path");if(s)NxtText_copy(a->path,s,sizeof(a->path));}json_value_free(v);
}
static void profile_name(SDL_Surface *s,const char *name,SDL_Rect r,SDL_Color ink){
 // Fixed visible glyph bounds prevent tall fallback fonts escaping cards.
 NxtSkin_compactText(s,21,name,r.x+16*s->w/1024,r.y+72*s->w/1024,r.w-32*s->w/1024,36*s->w/1024,ink);
}
static const char *countries[]={"Worldwide","Vietnam","South Korea","Japan","United States","United Kingdom","France","Thailand","Germany","China","Taiwan","Brazil","Italy","Indonesia","Russia","Spain"};
static const char *countries_vi[]={"Toàn cầu","Việt Nam","Hàn Quốc","Nhật Bản","Mỹ","Anh","Pháp","Thái Lan","Đức","Trung Quốc","Đài Loan","Brazil","Ý","Indonesia","Nga","Tây Ban Nha"};
const char *NxtPersonal_country(int i){return Nxt_text(countries[i>=0&&i<16?i:0],countries_vi[i>=0&&i<16?i:0]);}
ModuleExitReason NxtPersonal_checkIn(DisplayContext *d){
 int step=0,choices[]={nxt.mood,nxt.genre,nxt.country};while(step<3){const char *rows[20];int n=step==0?10:step==1?20:16;
  for(int i=0;i<n;i++)rows[i]=step==0?NxtCustom_mood(i):step==1?NxtCustom_genre(i):NxtPersonal_country(i);
  const char *titles[]={Nxt_text("How are you feeling today?","Hôm nay tâm trạng bạn thế nào?"),Nxt_text("What genre would you like?","Bạn thích nghe thể loại gì?"),Nxt_text("Music from which country?","Bạn muốn nghe nhạc nước nào?")};
  NxtPick p=Nxt_pick(d,titles[step],rows,n,&choices[step],Nxt_text("A Continue · B Back","A Tiếp tục · B Quay lại"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action==2){if(step)step--;else return MODULE_EXIT_TO_MENU;}if(p.action==1)step++;
 }nxt.mood=choices[0];nxt.genre=choices[1];nxt.country=choices[2];if(!Nxt_save())Toast_show(Nxt_text("Cannot save today's choices","Không lưu được lựa chọn hôm nay"),2500);return MODULE_EXIT_TO_MENU;
}
