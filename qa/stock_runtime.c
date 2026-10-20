/* Exercise the production stock adapter against real Linux processes/files. */
#include <assert.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "nxt_stock.h"

int InitializedSettings(void) { return 0; }
int GetJack(void) { return 0; }
void SetJack(int value) { (void)value; }
static Uint32 clock_ms;
Uint32 SDL_GetTicks(void) { return clock_ms; }

static struct ifaddrs fake_wifi;
static struct sockaddr_in wifi_address;
int getifaddrs(struct ifaddrs **out) {*out=&fake_wifi;return 0;}
void freeifaddrs(struct ifaddrs *p) {assert(p==&fake_wifi);}
static void wifi_case(const char *name,const char *address,unsigned flags,bool connected) {
 fake_wifi.ifa_name=(char *)name;fake_wifi.ifa_addr=(struct sockaddr *)&wifi_address;
 fake_wifi.ifa_flags=flags;wifi_address.sin_family=AF_INET;
 assert(inet_pton(AF_INET,address,&wifi_address.sin_addr)==1);clock_ms+=1000;
 assert(NxtStock_wifiConnected()==connected);
}
static pid_t spawn_keymon(void) {
    int ready[2]; assert(!pipe(ready));
    pid_t pid=fork(); assert(pid>=0);
    if (!pid) {
        close(ready[0]);
        assert(!prctl(PR_SET_PDEATHSIG, SIGKILL));
        assert(!prctl(PR_SET_NAME, "keymon"));
        assert(write(ready[1],"1",1)==1);
        close(ready[1]);
        for (;;) pause();
    }
    close(ready[1]); char byte;
    assert(read(ready[0], &byte, 1)==1); close(ready[0]);
    return pid;
}
static void stopped(pid_t pid) {
    int status; assert(waitpid(pid,&status,WUNTRACED)==pid && WIFSTOPPED(status));
}
static void continued(pid_t pid) {
    int status; assert(waitpid(pid,&status,WCONTINUED)==pid && WIFCONTINUED(status));
}
static void writefile(const char *path,const char *text) {
    FILE *f=fopen(path,"w"); assert(f); assert(fputs(text,f)>=0); assert(!fclose(f));
}
static void proc_state(const char *tmp,pid_t pid,const char *state) {
    /* The executor gives each command a PID namespace but keeps a host /proc.
     * Supply proc entries with namespace PIDs; signals still reach real children. */
    char dir[512], path[560];
    snprintf(dir,sizeof(dir),"%s/proc/%ld",tmp,(long)pid);
    mkdir(dir,0700);
    snprintf(path,sizeof(path),"%s/comm",dir); writefile(path,"keymon\n");
    snprintf(path,sizeof(path),"%s/status",dir); writefile(path,state);
}
int main(void) {
    wifi_case("wlan0","192.168.1.9",IFF_UP|IFF_RUNNING,true);
    wifi_case("wlan0","0.0.0.0",IFF_UP|IFF_RUNNING,false);
    wifi_case("wlan0","169.254.1.9",IFF_UP|IFF_RUNNING,false);
    wifi_case("wlan0","127.0.0.1",IFF_UP|IFF_RUNNING,false);
    wifi_case("eth0","192.168.1.9",IFF_UP|IFF_RUNNING,false);
    wifi_case("wlan0","192.168.1.9",0,false);
    wifi_case("wlan0","192.168.1.9",IFF_UP|IFF_RUNNING,true);clock_ms=0;

    char awake[512], alive[512], newfile[512], ready[512], address[512], route[2048];
    const char *tmp=getenv("QA_STOCK_TMP"); assert(tmp);
    snprintf(awake,sizeof(awake),"%s/stay_awake",tmp);
    snprintf(alive,sizeof(alive),"%s/stay_alive",tmp);
    snprintf(newfile,sizeof(newfile),"%s/replacement",tmp);
    snprintf(ready,sizeof(ready),"%s/bluetooth_ready",tmp);
    snprintf(address,sizeof(address),"%s/btdev_addr",tmp);
    pid_t owned=spawn_keymon(), existing=spawn_keymon();
    assert(!kill(existing,SIGSTOP)); stopped(existing);
    proc_state(tmp,owned,"State:\tS (sleeping)\n");
    proc_state(tmp,existing,"State:\tT (stopped)\n");
    writefile(alive,"another app");
    NxtStock_init(); stopped(owned);
    assert(!access(awake,F_OK) && !access(alive,F_OK));
    /* Use the real Stock file protocol; opening OSD must not resume keymon. */
    assert(!NxtStock_requestPanel());
    char osd[512],shown[560],request[512];
    snprintf(osd,sizeof(osd),"%s/trimui_osd",tmp);assert(!mkdir(osd,0700));
    snprintf(shown,sizeof(shown),"%s/osdd_show_up",osd);
    snprintf(request,sizeof(request),"%s/show_osdd",tmp);
    assert(NxtStock_requestPanel() && !access(request,F_OK));
    assert(NxtStock_panelActive());clock_ms=1500;assert(!NxtStock_panelActive());
    assert(NxtStock_requestPanel());writefile(shown,"1");
    clock_ms=5000;assert(NxtStock_panelActive());
    int panel_status;assert(waitpid(owned,&panel_status,WCONTINUED|WNOHANG)==0);
    unlink(shown);assert(!NxtStock_panelActive());unlink(request);
    NxtStock_visible(false); continued(owned);
    assert(!NxtStock_requestPanel());
    assert(!access(awake,F_OK)); /* Inhibitors belong to the audio session. */
    NxtStock_visible(true); stopped(owned);
    writefile(newfile,"new owner");
    assert(!rename(newfile,awake));
    NxtStock_quit(); continued(owned);
    assert(!access(awake,F_OK) && !access(alive,F_OK)); /* Preserve other owners. */
    NxtStock_quit();
    int status;
    assert(waitpid(existing,&status,WCONTINUED|WNOHANG)==0);
    assert(!kill(existing,SIGCONT)); continued(existing); /* Was never resumed by NXT. */
    proc_state(tmp,existing,"State:\tS (sleeping)\n");
    unlink(awake); unlink(alive);
    NxtStock_init(); stopped(owned); stopped(existing);
    NxtStock_quit(); continued(owned); continued(existing);
    assert(access(awake,F_OK) && access(alive,F_OK));
    assert(!NxtStock_bluetoothRoute(route,sizeof(route)));
    writefile(ready,"1"); writefile(address,"AA:BB:CC:DD:EE:FF\n");
    assert(NxtStock_bluetoothRoute(route,sizeof(route)));
    assert(strstr(route,"AA:BB:CC:DD:EE:FF") && strstr(route,"bluealsa"));
    assert(strstr(route,"profile \"a2dp\"") && strstr(route,"ctl.!default { type bluealsa device \"AA:BB:CC:DD:EE:FF\" }"));
    writefile(address,"untrusted $(command)\n");
    assert(!NxtStock_bluetoothRoute(route,sizeof(route)));
    unlink(address);
    assert(!NxtStock_bluetoothRoute(route,sizeof(route)));
    writefile(address,"00:00:00:00:00:00\n");assert(!NxtStock_bluetoothRoute(route,sizeof(route)));
    writefile(address,"AA:BB:CC:DD:EE:FF\n");
    assert(!NxtStock_bluetoothRoute(route,8));
    kill(owned,SIGKILL); kill(existing,SIGKILL);
    waitpid(owned,NULL,0); waitpid(existing,NULL,0);
    puts("PASS: Stock Wi-Fi interface/IP detection, Home OSD request/acknowledgement/timeout, keymon stays stopped during OSD, daemon handoff, background inhibitors, inode ownership, quit cleanup and Bluetooth validation.");
    return 0;
}
