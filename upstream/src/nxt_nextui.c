/* SPDX-License-Identifier: GPL-3.0-only
 * NextUI owns the global audio route. Keep this player's PCM private and
 * query connected audio sinks directly, rather than the status-bar cache. */
#include "nxt_nextui.h"
#if defined(NXT_ROM_NEXTUI) && !defined(PLATFORM_DESKTOP)
#include "file_utils.h"
#include <dbus/dbus.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/stat.h>
#define AUDIO_SINK_UUID "0000110b-0000-1000-8000-00805f9b34fb"
static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wake=PTHREAD_COND_INITIALIZER;
static pthread_t thread;
static bool running,started,wifi_ready;
static char bt_address[18],config_path[1024],base_config[1024];
static char *saved_alsa;
static bool had_alsa;
static bool mac(const char *s){
    if(!s||strlen(s)!=17)return false;
    for(int i=0;i<17;i++)if(i%3==2?s[i]!=':':!isxdigit((unsigned char)s[i]))return false;
    return true;
}
/* ObjectManager returns a{oa{sa{sv}}}; Connected alone also matches gamepads.
 * Only Device1 objects advertising the remote A2DP Audio Sink qualify. */
static void audio_sink(DBusMessage *reply,const char *preferred,char out[18]){
    out[0]=0;DBusMessageIter root,objects;
    if(!reply||!dbus_message_iter_init(reply,&root)||dbus_message_iter_get_arg_type(&root)!=DBUS_TYPE_ARRAY)return;
    dbus_message_iter_recurse(&root,&objects);
    for(;dbus_message_iter_get_arg_type(&objects)==DBUS_TYPE_DICT_ENTRY;dbus_message_iter_next(&objects)){
        DBusMessageIter object,interfaces;dbus_message_iter_recurse(&objects,&object);
        if(!dbus_message_iter_next(&object)||dbus_message_iter_get_arg_type(&object)!=DBUS_TYPE_ARRAY)continue;
        dbus_message_iter_recurse(&object,&interfaces);
        for(;dbus_message_iter_get_arg_type(&interfaces)==DBUS_TYPE_DICT_ENTRY;dbus_message_iter_next(&interfaces)){
            DBusMessageIter entry,properties;const char *iface=NULL;dbus_message_iter_recurse(&interfaces,&entry);
            if(dbus_message_iter_get_arg_type(&entry)!=DBUS_TYPE_STRING)continue;
            dbus_message_iter_get_basic(&entry,&iface);
            if(strcmp(iface,"org.bluez.Device1")||!dbus_message_iter_next(&entry)||dbus_message_iter_get_arg_type(&entry)!=DBUS_TYPE_ARRAY)continue;
            dbus_message_iter_recurse(&entry,&properties);const char *address=NULL;dbus_bool_t connected=FALSE;bool a2dp=false;
            for(;dbus_message_iter_get_arg_type(&properties)==DBUS_TYPE_DICT_ENTRY;dbus_message_iter_next(&properties)){
                DBusMessageIter property,value;const char *key=NULL;dbus_message_iter_recurse(&properties,&property);
                if(dbus_message_iter_get_arg_type(&property)!=DBUS_TYPE_STRING)continue;
                dbus_message_iter_get_basic(&property,&key);
                if(!dbus_message_iter_next(&property)||dbus_message_iter_get_arg_type(&property)!=DBUS_TYPE_VARIANT)continue;
                dbus_message_iter_recurse(&property,&value);int type=dbus_message_iter_get_arg_type(&value);
                if(!strcmp(key,"Connected")&&type==DBUS_TYPE_BOOLEAN)dbus_message_iter_get_basic(&value,&connected);
                else if(!strcmp(key,"Address")&&type==DBUS_TYPE_STRING)dbus_message_iter_get_basic(&value,&address);
                else if(!strcmp(key,"UUIDs")&&type==DBUS_TYPE_ARRAY){
                    DBusMessageIter uuids;dbus_message_iter_recurse(&value,&uuids);
                    for(;dbus_message_iter_get_arg_type(&uuids)==DBUS_TYPE_STRING;dbus_message_iter_next(&uuids)){
                        const char *uuid=NULL;dbus_message_iter_get_basic(&uuids,&uuid);if(!strcasecmp(uuid,AUDIO_SINK_UUID))a2dp=true;
                    }
                }
            }
            if(connected&&a2dp&&mac(address)){
                if(!out[0]||!strcasecmp(preferred,address))snprintf(out,18,"%s",address);
                if(!strcasecmp(preferred,address))return;
            }
        }
    }
}
static bool wifi_probe(void){
    struct ifaddrs *all=NULL;if(getifaddrs(&all))return false;bool connected=false;
    for(struct ifaddrs *i=all;i;i=i->ifa_next){
        if(!i->ifa_addr||!i->ifa_name||(i->ifa_flags&(IFF_UP|IFF_RUNNING))!=(IFF_UP|IFF_RUNNING)||(i->ifa_flags&IFF_LOOPBACK))continue;
        char wireless[256];snprintf(wireless,sizeof(wireless),"/sys/class/net/%s/wireless",i->ifa_name);
        if(strncmp(i->ifa_name,"wlan",4)&&strncmp(i->ifa_name,"wlp",3)&&access(wireless,F_OK))continue;
        if(i->ifa_addr->sa_family==AF_INET){uint32_t a=ntohl(((struct sockaddr_in*)i->ifa_addr)->sin_addr.s_addr);connected=a&&(a>>24)!=127&&(a>>16)!=0xa9fe;}
        else if(i->ifa_addr->sa_family==AF_INET6){struct in6_addr *a=&((struct sockaddr_in6*)i->ifa_addr)->sin6_addr;connected=!IN6_IS_ADDR_UNSPECIFIED(a)&&!IN6_IS_ADDR_LOOPBACK(a)&&!IN6_IS_ADDR_LINKLOCAL(a);}
        if(connected)break;
    }
    freeifaddrs(all);return connected;
}
static void *monitor(void *unused){
    (void)unused;DBusConnection *bus=NULL;int misses=0;
    for(;;){
        pthread_mutex_lock(&lock);bool active=running;char preferred[18];snprintf(preferred,sizeof(preferred),"%s",bt_address);pthread_mutex_unlock(&lock);if(!active)break;
        DBusError error;dbus_error_init(&error);char address[18]="";bool valid=false;
        if(!bus)bus=dbus_bus_get_private(DBUS_BUS_SYSTEM,&error);
        if(bus){
            dbus_connection_set_exit_on_disconnect(bus,FALSE);
            DBusMessage *request=dbus_message_new_method_call("org.bluez","/","org.freedesktop.DBus.ObjectManager","GetManagedObjects");
            DBusMessage *reply=request?dbus_connection_send_with_reply_and_block(bus,request,1200,&error):NULL;
            if(reply){valid=dbus_message_get_type(reply)==DBUS_MESSAGE_TYPE_METHOD_RETURN;if(valid)audio_sink(reply,preferred,address);dbus_message_unref(reply);}
            if(request)dbus_message_unref(request);
            if(!dbus_connection_get_is_connected(bus)){dbus_connection_close(bus);dbus_connection_unref(bus);bus=NULL;}
        }
        dbus_error_free(&error);bool wifi=wifi_probe();
        pthread_mutex_lock(&lock);
        /* A transient D-Bus timeout must not abruptly send music to speakers. */
        if(valid){misses=0;snprintf(bt_address,sizeof(bt_address),"%s",address);}else if(++misses>=3)bt_address[0]=0;
        wifi_ready=wifi;
        struct timespec until;clock_gettime(CLOCK_REALTIME,&until);until.tv_nsec+=750000000;if(until.tv_nsec>=1000000000){until.tv_sec++;until.tv_nsec-=1000000000;}
        if(running)pthread_cond_timedwait(&wake,&lock,&until);pthread_mutex_unlock(&lock);
    }
    if(bus){dbus_connection_close(bus);dbus_connection_unref(bus);}return NULL;
}
void NxtNextUI_init(void){
    if(started)return;
    const char *existing=getenv("ALSA_CONFIG_PATH");had_alsa=existing!=NULL;saved_alsa=existing?strdup(existing):NULL;
    const char *base=existing&&*existing?existing:"/usr/share/alsa/alsa.conf";
    if(base[0]=='/'&&!strpbrk(base,"<>\r\n")&&strlen(base)<sizeof(base_config)&&!access(base,R_OK))snprintf(base_config,sizeof(base_config),"%s",base);
    if(base_config[0]&&userdata_mkdir("audio"))
#ifdef NXT_ROM_NXREDUX
        userdata_snpath("audio/nxredux-asound.conf",config_path,sizeof(config_path));
#else
        userdata_snpath("audio/nextui-asound.conf",config_path,sizeof(config_path));
#endif
    pthread_mutex_lock(&lock);wifi_ready=wifi_probe();running=true;pthread_mutex_unlock(&lock);
    if(dbus_threads_init_default()&&pthread_create(&thread,NULL,monitor,NULL)==0)started=true;
    else {pthread_mutex_lock(&lock);running=false;pthread_mutex_unlock(&lock);}
}
void NxtNextUI_quit(void){
    if(started){pthread_mutex_lock(&lock);running=false;pthread_cond_signal(&wake);pthread_mutex_unlock(&lock);pthread_join(thread,NULL);started=false;}
    if(had_alsa&&saved_alsa)setenv("ALSA_CONFIG_PATH",saved_alsa,1);else unsetenv("ALSA_CONFIG_PATH");free(saved_alsa);saved_alsa=NULL;
    pthread_mutex_lock(&lock);bt_address[0]=0;wifi_ready=false;pthread_mutex_unlock(&lock);config_path[0]=base_config[0]=0;
}
bool NxtNextUI_bluetoothConnected(void){pthread_mutex_lock(&lock);bool connected=bt_address[0]!=0;pthread_mutex_unlock(&lock);return connected;}
bool NxtNextUI_bluetoothRoute(char *out,size_t n){
    pthread_mutex_lock(&lock);char address[18];snprintf(address,sizeof(address),"%s",bt_address);pthread_mutex_unlock(&lock);
    if(!address[0])return false;
    int written=snprintf(out,n,"# NXT Retro Music shared PCM / NextUI Bluetooth\ndefaults.bluealsa.device \"%s\"\npcm.nxt_music_output { type plug slave.pcm { type bluealsa device \"%s\" profile \"a2dp\" } }\npcm.!default { type plug slave.pcm \"nxt_music_output\" }\nctl.!default { type bluealsa device \"%s\" }\n",address,address,address);
    return written>=0&&(size_t)written<n;
}
bool NxtNextUI_wifiConnected(void){pthread_mutex_lock(&lock);bool value=wifi_ready;pthread_mutex_unlock(&lock);return value;}
bool NxtNextUI_audioConfig(char *out,size_t n){if(!config_path[0]||strlen(config_path)>=n)return false;snprintf(out,n,"%s",config_path);return true;}
bool NxtNextUI_wrapRoute(const char *body,char *out,size_t n){if(!base_config[0])return false;int written=snprintf(out,n,"<%s>\npcm.!nxt_music_mix {}\npcm.!nxt_music_output {}\n%s",base_config,body);return written>=0&&(size_t)written<n;}
#else
void NxtNextUI_init(void){}
void NxtNextUI_quit(void){}
bool NxtNextUI_bluetoothConnected(void){return false;}
bool NxtNextUI_bluetoothRoute(char *out,size_t n){if(n)out[0]=0;return false;}
bool NxtNextUI_wifiConnected(void){return false;}
bool NxtNextUI_audioConfig(char *out,size_t n){if(n)out[0]=0;return false;}
bool NxtNextUI_wrapRoute(const char *body,char *out,size_t n){(void)body;if(n)out[0]=0;return false;}
#endif
