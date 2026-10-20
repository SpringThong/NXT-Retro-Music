static int ime_language(const KeyboardState *s){int i=s->layout_index;return i==4||i==5||i==7||i==8?i:-1;}
static bool ime_refresh(KeyboardState *s){char composed[KEYBOARD_MAX_INPUT],value[KEYBOARD_MAX_INPUT];int lang=ime_language(s);if(!NxtIME_compose(lang,s->ime_raw,composed,sizeof(composed)))return false;int n=snprintf(value,sizeof(value),"%s%s",s->ime_prefix,composed);if(n<0||(size_t)n>s->text_limit||(size_t)n>=sizeof(value))return false;memcpy(s->text,value,n+1);s->text_version++;return true;}
static void ime_append(KeyboardState *s,const char *text){char addition[8];int lang=ime_language(s);if(lang==4){char c=s->current_key->ansi;if(!c)return;addition[0]=s->shift!=SHIFT_OFF&&strchr("reqtwop",c)?toupper((unsigned char)c):c;addition[1]=0;text=addition;}
 size_t n=strlen(s->ime_raw),k=strlen(text);if(n+k>=sizeof(s->ime_raw))return;memcpy(s->ime_raw+n,text,k+1);if(!ime_refresh(s))s->ime_raw[n]=0;
}
static void ime_convert(KeyboardState *s){
 int lang=ime_language(s);char key[KEYBOARD_MAX_INPUT],values[32][128];if(!NxtIME_compose(lang,s->ime_raw,key,sizeof(key)))return;int n=NxtIME_candidates(lang,key,values);
 if(n){const char *rows[32];for(int i=0;i<n;i++)rows[i]=values[i];int selected=0;NxtPick p=Nxt_pick(DisplayHelper_current(),Nxt_text("Choose characters","Chọn chữ"),rows,n,&selected,Nxt_text("A Apply · B Back","A Áp dụng · B Về"));if(p.action!=1)return;char value[KEYBOARD_MAX_INPUT];int len=snprintf(value,sizeof(value),"%s%s",s->ime_prefix,values[selected]);if(len<0||(size_t)len>s->text_limit||(size_t)len>=sizeof(value))return;memcpy(s->text,value,len+1);}
 else {size_t len=strlen(s->text);if(len<s->text_limit){s->text[len]=' ';s->text[len+1]=0;}}
 snprintf(s->ime_prefix,sizeof(s->ime_prefix),"%s",s->text);s->ime_raw[0]=0;s->text_version++;
}
static void ime_backspace(KeyboardState *s){size_t n=strlen(s->ime_raw);if(n){s->ime_raw[n-1]=0;ime_refresh(s);return;}const char *p=s->ime_prefix,*last=p;while(*p){last=p;p=NxtText_nextCluster(p);}s->ime_prefix[last-s->ime_prefix]=0;ime_refresh(s);}
