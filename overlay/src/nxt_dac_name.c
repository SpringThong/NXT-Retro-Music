/* SPDX-License-Identifier: GPL-3.0-only
 * Explicit user labels, never a guessed model or a capability whitelist. */
#include "nxt_dac_name.h"
#include "parson/parson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static const char*fields[]={"vid","pid","manufacturer","product"};
static const char*identity(const NxtDACIdentity*i,int k){const char*v[]={i->vid,i->pid,i->manufacturer,i->product};return v[k]?v[k]:"";}
static bool matches(JSON_Object*o,const NxtDACIdentity*i){for(int k=0;k<4;k++){const char*v=json_object_get_string(o,fields[k]);if(!v||strcmp(v,identity(i,k)))return false;}return true;}
static JSON_Value*load(const char*path){struct stat st;if(lstat(path,&st)||!S_ISREG(st.st_mode)||st.st_size>32768)return NULL;return json_parse_file(path);}
static bool valid(const char*n){if(!n||strlen(n)>96)return false;for(const unsigned char*p=(const unsigned char*)n;*p;p++)if(*p<32||*p==127)return false;return true;}
void NxtDACName_read(const char*path,const NxtDACIdentity*i,char*out,size_t size){if(!size)return;out[0]=0;if(!path||!i)return;JSON_Value*v=load(path);JSON_Array*a=json_value_get_array(v);size_t count=json_array_get_count(a);if(count>32)count=32;
 for(size_t n=0;n<count;n++){JSON_Object*o=json_array_get_object(a,n);const char*name=json_object_get_string(o,"name");if(matches(o,i)&&valid(name)){snprintf(out,size,"%s",name);break;}}json_value_free(v);}
bool NxtDACName_save(const char*path,const NxtDACIdentity*i,const char*name){if(!path||!i||!valid(name)||(!*identity(i,0)&&!*identity(i,2)&&!*identity(i,3)))return false;
 JSON_Value*v=load(path);JSON_Array*a=json_value_get_array(v);if(!a){json_value_free(v);v=json_value_init_array();a=json_value_get_array(v);}size_t count=json_array_get_count(a);if(count>32){json_value_free(v);return false;}
 size_t n=0;for(;n<count;n++)if(matches(json_array_get_object(a,n),i))break;
 if(n<count){json_array_remove(a,n);count--;}if(*name){if(count==32)json_array_remove(a,0);JSON_Value*row=json_value_init_object();JSON_Object*o=json_value_get_object(row);for(int k=0;k<4;k++)json_object_set_string(o,fields[k],identity(i,k));json_object_set_string(o,"name",name);json_array_append_value(a,row);}
 char tmp[1200];if(snprintf(tmp,sizeof(tmp),"%s.tmp.XXXXXX",path)>=(int)sizeof(tmp)){json_value_free(v);return false;}int fd=mkstemp(tmp);char*text=json_serialize_to_string(v);bool ok=fd>=0&&text;FILE*f=fd>=0?fdopen(fd,"w"):NULL;if(!f){if(fd>=0)close(fd);ok=false;}else{if(!text||fputs(text,f)<0||fflush(f)||fsync(fd))ok=false;if(fclose(f))ok=false;}if(ok)ok=!rename(tmp,path);if(!ok)unlink(tmp);json_free_serialized_string(text);json_value_free(v);return ok;
}
