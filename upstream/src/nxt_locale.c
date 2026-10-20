/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_locale.h"
#include "parson/parson.h"
#include "keyboard.h"
#include "toast.h"
#include <stdio.h>
static const char *codes[]={"en","vi","fr","ru","ko","ja","th","zh-Hans","zh-Hant","de","es","pt-BR","it","id"};
static const char *names[]={"English","Tiếng Việt","Français","Русский","한국어","日本語","ไทย","简体中文","繁體中文","Deutsch","Español","Português (Brasil)","Italiano","Bahasa Indonesia"};
static JSON_Value *catalog;static int loaded=-1;
const char *NxtLocale_name(int i){return names[i>=0&&i<14?i:0];}
const char *NxtLocale_text(const char *text){
 if(!text)return "";int lang=nxt.language>=0&&nxt.language<14?nxt.language:0;if(lang<2)return text;
 if(loaded!=lang){json_value_free(catalog);char path[96];snprintf(path,sizeof(path),"./res/locales/%s.json",codes[lang]);catalog=json_parse_file(path);loaded=lang;}
 const char *value=json_object_get_string(json_value_get_object(catalog),text);return value&&*value?value:text;
}
ModuleExitReason NxtLocale_run(DisplayContext *d){int selected=nxt.language;for(;;){const char *rows[15];for(int i=0;i<14;i++)rows[i]=names[i];rows[14]=Nxt_text("Test keyboard","Thử bàn phím");NxtPick p=Nxt_pick(d,Nxt_text("Language & keyboard","Ngôn ngữ & bàn phím"),rows,15,&selected,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;if(selected<14){nxt.language=selected;Nxt_save();}else{char *value=Keyboard_open(Nxt_text("Test keyboard","Thử bàn phím"),512);if(value){Toast_show(value,3000);free(value);}}}}
