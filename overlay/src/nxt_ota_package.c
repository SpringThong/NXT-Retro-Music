/* SPDX-License-Identifier: GPL-3.0-only */
#include "nxt_ota_package.h"
#include "nxt_log.h"
#include "file_utils.h"
#include "include/parson/parson.h"
#include "mbedtls/sha256.h"
#include <zip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/syscall.h>
static bool fail(char *error,size_t size,const char *message){snprintf(error,size,"%s",message);return false;}
static bool version(const char *s,int parts[4]){
    if(!s||!*s)return false;for(int i=0;i<4;i++)parts[i]=0;int n=0;
    for(;;){if(!isdigit((unsigned char)*s)||n>=4)return false;int v=0,digits=0;while(isdigit((unsigned char)*s)){if(++digits>4)return false;v=v*10+*s++-'0';}parts[n++]=v;if(!*s)return true;if(*s++!='.')return false;}
}
int NxtOTA_compareVersion(const char *a,const char *b){int x[4],y[4];if(!version(a,x)||!version(b,y))return -2;for(int i=0;i<4;i++)if(x[i]!=y[i])return x[i]>y[i]?1:-1;return 0;}
bool NxtOTA_https(const char *url){
    if(!url||strncmp(url,"https://",8)||!url[8]||strlen(url)>2047)return false;const char *end=strchr(url+8,'/');if(!end)end=url+strlen(url);if(end==url+8)return false;
    for(const char *p=url;*p;p++)if(isspace((unsigned char)*p)||(unsigned char)*p<32||*p=='@'||*p=='\\')return false;return true;
}
static bool string(JSON_Object *o,const char *key,char *out,size_t size){const char *s=json_object_get_string(o,key);if(!s||strlen(s)>=size)return false;snprintf(out,size,"%s",s);return true;}
bool NxtOTA_parseManifest(const char *text,NxtOTAManifest *manifest,char *error,size_t size){
    memset(manifest,0,sizeof(*manifest));JSON_Value *v=json_parse_string(text);JSON_Object *o=json_value_get_object(v);bool ok=false;
    if(!o||json_object_get_number(o,"schema")!=1)fail(error,size,"Manifest schema must be 1");
    else if(!json_object_get_string(o,"package")||strcmp(json_object_get_string(o,"package"),NXT_OTA_PACKAGE))fail(error,size,"Wrong app package");
    else if(!json_object_get_string(o,"rom")||strcmp(json_object_get_string(o,"rom"),NXT_OTA_ROM))fail(error,size,"Wrong ROM for this app");
    else if(!json_object_get_string(o,"device")||strcmp(json_object_get_string(o,"device"),NXT_OTA_DEVICE))fail(error,size,"Wrong device; require TrimUI Brick Pro");
    else if(!string(o,"version",manifest->version,sizeof(manifest->version))||NxtOTA_compareVersion(manifest->version,"0")==-2)fail(error,size,"Invalid numeric version");
    else if(!string(o,"download_url",manifest->url,sizeof(manifest->url))||!NxtOTA_https(manifest->url))fail(error,size,"Require public HTTPS download URL");
    else if(!string(o,"sha256",manifest->sha256,sizeof(manifest->sha256))||strlen(manifest->sha256)!=64)fail(error,size,"Require SHA-256 with 64 hex digits");
    else{bool valid=true;for(int i=0;i<64;i++){if(!isxdigit((unsigned char)manifest->sha256[i]))valid=false;manifest->sha256[i]=tolower((unsigned char)manifest->sha256[i]);}double bytes=json_object_get_number(o,"size");
        if(!valid)fail(error,size,"Invalid SHA-256");else if(!json_object_has_value_of_type(o,"size",JSONNumber)||!isfinite(bytes)||bytes<1||bytes>256*1024*1024||floor(bytes)!=bytes)fail(error,size,"Invalid package size; maximum 256 MiB");
        else{manifest->size=bytes;const char *notes=json_object_get_string(o,"changelog");snprintf(manifest->notes,sizeof(manifest->notes),"%s",notes?notes:"");ok=true;}
    }json_value_free(v);return ok;
}
void NxtOTA_removeTree(const char *path){struct stat st;if(lstat(path,&st))return;if(!S_ISDIR(st.st_mode)){unlink(path);return;}DIR *dir=opendir(path);if(!dir)return;struct dirent *entry;while((entry=readdir(dir))){if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,".."))continue;char child[4096];if(snprintf(child,sizeof(child),"%s/%s",path,entry->d_name)<(int)sizeof(child))NxtOTA_removeTree(child);}closedir(dir);rmdir(path);}
static bool safe_path(const char *path){
    if(!path||!*path||*path=='/'||strchr(path,'\\')||strlen(path)>1023)return false;const char *p=path;
    while(*p){const char *end=strchr(p,'/');size_t n=end?(size_t)(end-p):strlen(p);if(!n||(n==1&&*p=='.')||(n==2&&p[0]=='.'&&p[1]=='.'))return false;for(size_t i=0;i<n;i++)if((unsigned char)p[i]<32)return false;if(!end)break;p=end+1;}
    return true;
}
static bool elf(const char *path){unsigned char data[20];FILE *f=fopen(path,"rb");if(!f)return false;bool ok=fread(data,1,sizeof(data),f)==sizeof(data)&&!memcmp(data,"\x7f" "ELF\2\1",6)&&data[18]==183&&data[19]==0;fclose(f);return ok;}
static bool staged_app(const char *stage,const NxtOTAManifest *manifest){
    char path[4096];snprintf(path,sizeof(path),"%s/config.json",stage);JSON_Value *v=json_parse_file(path);JSON_Object *o=json_value_get_object(v);bool ok=o;const char *keys[]={"package","rom","device","version","launch"},*values[]={NXT_OTA_PACKAGE,NXT_OTA_ROM,NXT_OTA_DEVICE,manifest->version,"launch.sh"};
    for(int i=0;i<5&&ok;i++){const char *s=json_object_get_string(o,keys[i]);ok=s&&!strcmp(s,values[i]);}json_value_free(v);
    snprintf(path,sizeof(path),"%s/launch.sh",stage);struct stat st;ok=ok&&!lstat(path,&st)&&S_ISREG(st.st_mode)&&st.st_size>0;
    const char *bins[]={"musicplayer.elf","nxt-spotify-backend","nxt-soundcloud-backend"};for(int i=0;i<3&&ok;i++){snprintf(path,sizeof(path),"%s/bin/tg5040/%s",stage,bins[i]);ok=elf(path);}
    return ok;
}
bool NxtOTA_verifyZip(const char *archive,const NxtOTAManifest *manifest,const char *stage,char *error,size_t size){
    struct stat st;if(lstat(archive,&st)||!S_ISREG(st.st_mode)||(uint64_t)st.st_size!=manifest->size)return fail(error,size,"Downloaded size does not match manifest");
    FILE *f=fopen(archive,"rb");if(!f)return fail(error,size,"Cannot open downloaded ZIP");mbedtls_sha256_context sha;mbedtls_sha256_init(&sha);bool ok=mbedtls_sha256_starts(&sha,0)==0;unsigned char buffer[65536],digest[32]={0};size_t n;
    while(ok&&(n=fread(buffer,1,sizeof(buffer),f)))ok=mbedtls_sha256_update(&sha,buffer,n)==0;if(ferror(f))ok=false;fclose(f);if(ok)ok=mbedtls_sha256_finish(&sha,digest)==0;mbedtls_sha256_free(&sha);char hex[65];for(int i=0;i<32;i++)snprintf(hex+i*2,3,"%02x",digest[i]);
    if(!ok||strcmp(hex,manifest->sha256))return fail(error,size,"SHA-256 mismatch; do not install");NxtLog_step("ota","sha256",true,"Downloaded bytes verified");
    int code=0;zip_t *zip=zip_open(archive,ZIP_RDONLY,&code);if(!zip)return fail(error,size,"Cannot open ZIP");zip_int64_t count=zip_get_num_entries(zip,0);uint64_t total=0;const char *prefix=NXT_OTA_ZIP_PREFIX;size_t prefix_len=strlen(prefix);ok=count>0&&count<=8000;
    /* Inspect every archive entry before creating any payload file. */
    for(zip_int64_t i=0;i<count&&ok;i++){zip_stat_t entry;zip_stat_init(&entry);if(zip_stat_index(zip,i,0,&entry)||!safe_path(entry.name)){ok=false;break;}
        if(entry.size>256*1024*1024||(total+=entry.size)>1024ULL*1024*1024){ok=false;break;}zip_uint8_t system;zip_uint32_t attrs;if(zip_file_get_external_attributes(zip,i,0,&system,&attrs)){ok=false;break;}mode_t type=(attrs>>16)&S_IFMT;if(system==3&&type&&type!=S_IFREG&&type!=S_IFDIR){ok=false;break;}
        for(zip_int64_t j=0;j<i;j++)if(!strcmp(entry.name,zip_get_name(zip,j,0))){ok=false;break;}
    }
    if(ok)ok=mkdir_p(stage);
    for(zip_int64_t i=0;i<count&&ok;i++){zip_stat_t entry;zip_stat_index(zip,i,0,&entry);if(strncmp(entry.name,prefix,prefix_len))continue;const char *relative=entry.name+prefix_len;if(!*relative)continue;char target[4096];if(snprintf(target,sizeof(target),"%s/%s",stage,relative)>=(int)sizeof(target)){ok=false;break;}
        if(relative[strlen(relative)-1]=='/'){ok=mkdir_p(target);continue;}char parent[4096];snprintf(parent,sizeof(parent),"%s",target);char *slash=strrchr(parent,'/');if(slash)*slash=0;if(!mkdir_p(parent)){ok=false;break;}
        zip_file_t *input=zip_fopen_index(zip,i,0);int fd=input?open(target,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW,0600):-1;if(fd<0){if(input)zip_fclose(input);ok=false;break;}uint64_t copied=0;zip_int64_t got;
        while(ok&&(got=zip_fread(input,buffer,sizeof(buffer)))>0){size_t pos=0;while(pos<(size_t)got){ssize_t wrote=write(fd,buffer+pos,(size_t)got-pos);if(wrote<0&&errno==EINTR)continue;if(wrote<=0){ok=false;break;}pos+=wrote;}copied+=got;}
        if(got<0||copied!=entry.size)ok=false;if(zip_fclose(input))ok=false;bool executable=!strcmp(relative,"launch.sh")||!strcmp(relative,"bin/usb-transfer.sh")||(!strncmp(relative,"bin/",4)&&(!strcmp(strrchr(relative,'/')+1,"musicplayer.elf")||!strcmp(strrchr(relative,'/')+1,"nxt-spotify-backend")||!strcmp(strrchr(relative,'/')+1,"nxt-soundcloud-backend")||!strcmp(strrchr(relative,'/')+1,"yt-dlp")));
        if(fchmod(fd,executable?0755:0644)||fsync(fd))ok=false;if(close(fd))ok=false;
    }
    if(zip_close(zip))ok=false;if(ok)ok=staged_app(stage,manifest);if(!ok){NxtOTA_removeTree(stage);return fail(error,size,"Invalid or unsafe installer ZIP for this ROM");}NxtLog_step("ota","stage",true,"Paths, ROM, version and ARM64 binaries checked");return true;
}
bool NxtOTA_install(const char *app,const char *stage,char *error,size_t size){
    char parent[4096],backup[4096];if(snprintf(parent,sizeof(parent),"%s",app)>=(int)sizeof(parent))return fail(error,size,"App path too long");char *slash=strrchr(parent,'/');if(!slash||slash==parent)return fail(error,size,"Invalid app directory");*slash=0;if(snprintf(backup,sizeof(backup),"%s/" NXT_OTA_BACKUP_NAME,parent)>=(int)sizeof(backup))return fail(error,size,"Backup path too long");if(!strcmp(app,backup))return fail(error,size,"App directory is the backup; restore it first");struct stat st;if(lstat(app,&st)||!S_ISDIR(st.st_mode)||lstat(stage,&st)||!S_ISDIR(st.st_mode))return fail(error,size,"App or staging directory unavailable");
#ifdef SYS_renameat2
    if(!syscall(SYS_renameat2,AT_FDCWD,app,AT_FDCWD,stage,2)){
        NxtOTA_removeTree(backup);if(rename(stage,backup))NxtLog_step("ota","backup-rename",false,"Previous app retained in staging directory");sync();NxtLog_step("ota","install",true,"Atomic app exchange; user data untouched");return true;
    }
#endif
    NxtOTA_removeTree(backup);if(rename(app,backup))return fail(error,size,"Cannot back up current app");
    if(rename(stage,app)){bool restored=rename(backup,app)==0;NxtLog_step("ota","rollback",restored,"Installation rename failed");return fail(error,size,restored?"Install failed; previous app restored":"Install failed; restore the previous-app backup manually");}
    sync();NxtLog_step("ota","install",true,"App replaced with previous-app backup; user data untouched");return true;
}
