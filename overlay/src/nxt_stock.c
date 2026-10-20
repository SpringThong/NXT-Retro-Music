#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
/* SPDX-License-Identifier: GPL-3.0-only
 * Official TrimUI firmware adapter; no system files are installed or replaced. */
#include "nxt_stock.h"
#include "nxt.h"
#ifdef NXT_ROM_STOCK
#include "defines.h"
#include "msettings.h"
#include <SDL2/SDL.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef NXT_STOCK_PROC_PATH
#define NXT_STOCK_PROC_PATH "/proc"
#endif
#ifndef NXT_STOCK_TMP_PATH
#define NXT_STOCK_TMP_PATH "/tmp"
#endif
#ifndef NXT_STOCK_INPUT_PATH
#define NXT_STOCK_INPUT_PATH "/dev/input"
#endif

static bool active, shown;
static pid_t paused[16];
static size_t paused_count;
static char runtime[512];
static const char *power_flags[] = {"stay_awake", "stay_alive"};
static bool panel_pending, panel_seen;
static Uint32 panel_deadline;

/* Stock keymon uses this same file to request trimui_osdd. Its daemon grabs
 * the input and releases osdd_show_up when closed. Keep our keymon paused:
 * replaying its queued volume events would break the app's DAC volume path. */
bool NxtStock_requestPanel(void) {
    if(!active||!shown||access(NXT_STOCK_TMP_PATH "/trimui_osd",F_OK))return false;
    int fd=open(NXT_STOCK_TMP_PATH "/show_osdd",O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC|O_NOFOLLOW,0644);
    if(fd<0)return false;
    close(fd);panel_pending=true;panel_seen=false;
    panel_deadline=SDL_GetTicks()+1500;
    return true;
}
bool NxtStock_panelActive(void) {
    bool shown_now=access(NXT_STOCK_TMP_PATH "/trimui_osd/osdd_show_up",F_OK)==0;
    if(shown_now){panel_seen=true;panel_pending=true;}
    else if(panel_seen||(panel_pending&&(Sint32)(SDL_GetTicks()-panel_deadline)>=0)) {
        panel_pending=false;panel_seen=false;
    }
    return panel_pending;
}

static bool is_keymon(pid_t pid) {
    char path[640], name[64];
    snprintf(path, sizeof(path), NXT_STOCK_PROC_PATH "/%ld/comm", (long)pid);
    FILE *f = fopen(path, "r");
    if (!f) return false;
    bool valid = fgets(name, sizeof(name), f) && !strcmp(name, "keymon\n");
    fclose(f);
    return valid;
}

static bool already_stopped(pid_t pid) {
    char path[640], line[256];
    snprintf(path, sizeof(path), NXT_STOCK_PROC_PATH "/%ld/status", (long)pid);
    FILE *f = fopen(path, "r");
    if (!f) return true;
    bool stopped = false;
    while (fgets(line, sizeof(line), f)) {
        if (!strncmp(line, "State:", 6)) {
            char *state = line + 6;
            while (*state && isspace((unsigned char)*state)) state++;
            stopped = *state == 'T' || *state == 't';
            break;
        }
    }
    fclose(f);
    return stopped;
}

static void pause_keymon(void) {
    DIR *dir = opendir(NXT_STOCK_PROC_PATH);
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir)) && paused_count < 16) {
        char *end;
        long number = strtol(entry->d_name, &end, 10);
        if (*end || number <= 1 || number == getpid()) continue;
        pid_t pid = (pid_t)number;
        if (!is_keymon(pid) || already_stopped(pid)) continue;
        /* Write the recovery record BEFORE stopping a daemon. The launcher
         * can resume it even if this process is killed unexpectedly. */
        char path[640];
        snprintf(path, sizeof(path), "%s/keymon-%ld", runtime, number);
        int marker = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
        if (marker < 0) continue;
        close(marker);
        if (!kill(pid, SIGSTOP)) paused[paused_count++] = pid;
        else unlink(path);
    }
    closedir(dir);
}

static void resume_keymon(void) {
    for (size_t i = 0; i < paused_count; i++) {
        if (is_keymon(paused[i])) kill(paused[i], SIGCONT);
        char path[640];
        snprintf(path, sizeof(path), "%s/keymon-%ld", runtime, (long)paused[i]);
        unlink(path);
    }
    paused_count = 0;
}

static void hold_power_flag(const char *name) {
    char path[640], marker[640];
    snprintf(path, sizeof(path), NXT_STOCK_TMP_PATH "/%s", name);
    snprintf(marker, sizeof(marker), "%s/owned-%s", runtime, name);
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    if (fd < 0) return; /* Leave another app's flag untouched. */
    if (write(fd, "1\n", 2) != 2 || link(path, marker)) unlink(path);
    close(fd);
}

static void release_power_flag(const char *name) {
    char path[640], marker[640];
    struct stat live, owned;
    snprintf(path, sizeof(path), NXT_STOCK_TMP_PATH "/%s", name);
    snprintf(marker, sizeof(marker), "%s/owned-%s", runtime, name);
    if (!lstat(path, &live) && !lstat(marker, &owned) &&
        live.st_dev == owned.st_dev && live.st_ino == owned.st_ino) unlink(path);
    unlink(marker);
}

void NxtStock_visible(bool visible) {
    if (!active || shown == visible) return;
    shown = visible;
    if (visible) pause_keymon();
    else resume_keymon();
}

void NxtStock_init(void) {
    if (active) return;
    const char *dir = getenv("NXT_STOCK_RUNTIME_DIR");
    if (dir && *dir) snprintf(runtime, sizeof(runtime), "%s", dir);
    else snprintf(runtime, sizeof(runtime), NXT_STOCK_TMP_PATH "/nxt-stock-%lu",
                  (unsigned long)getuid());
    if (mkdir(runtime, 0700) && errno != EEXIST) return;
    active = true;
    for (size_t i = 0; i < 2; i++) hold_power_flag(power_flags[i]);
    atexit(NxtStock_quit);
    NxtStock_visible(true);
}

void NxtStock_tick(void) {
    if (!active || !InitializedSettings()) return;
    /* The stock keymon does not publish NextUI's shared jack state. Query
     * the Linux switch state directly; this also works after a UI reattach. */
    static Uint32 next_probe;
    Uint32 now = SDL_GetTicks();
    if ((Sint32)(now - next_probe) < 0) return;
    next_probe = now + 500;
    DIR *dir = opendir(NXT_STOCK_INPUT_PATH);
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strncmp(entry->d_name, "event", 5)) continue;
        char path[640];
        snprintf(path, sizeof(path), NXT_STOCK_INPUT_PATH "/%s", entry->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) continue;
        unsigned long supported = 0, state = 0;
        int valid = ioctl(fd, EVIOCGBIT(EV_SW, sizeof(supported)), &supported);
        if (valid >= 0 && (supported & (1UL << SW_HEADPHONE_INSERT)) &&
            ioctl(fd, EVIOCGSW(sizeof(state)), &state) >= 0) {
            int jack = !!(state & (1UL << SW_HEADPHONE_INSERT));
            if (GetJack() != jack) SetJack(jack);
            close(fd);
            break;
        }
        close(fd);
    }
    closedir(dir);
}

void NxtStock_quit(void) {
    if (!active) return;
    resume_keymon();
    for (size_t i = 0; i < 2; i++) release_power_flag(power_flags[i]);
    active = shown = false;
    panel_pending=panel_seen=false;
}

bool NxtStock_bluetoothRoute(char *out, unsigned long size) {
    if (access(NXT_STOCK_TMP_PATH "/bluetooth_ready", F_OK)) return false;
    char address[32] = "";
    FILE *f = fopen(NXT_STOCK_TMP_PATH "/btdev_addr", "r");
    if (f) {
        char candidate[64];
        if (fgets(candidate, sizeof(candidate), f)) {
            candidate[strcspn(candidate, "\r\n")] = 0;
            bool valid = strlen(candidate) == 17;
            for (int i = 0; valid && i < 17; i++)
                valid = i % 3 == 2 ? candidate[i] == ':' :
                        isxdigit((unsigned char)candidate[i]) != 0;
            if (!valid) { fclose(f); return false; }
            snprintf(address, sizeof(address), "%s", candidate);
        }
        fclose(f);
    }
    if(!*address||!strcmp(address,"00:00:00:00:00:00"))return false;
    int n = snprintf(out, size,
        "# NXT Retro Music shared PCM / Stock Bluetooth\n"
        "defaults.bluealsa.device \"%s\"\n"
        "pcm.nxt_music_output { type plug slave.pcm { type bluealsa device \"%s\" "
        "profile \"a2dp\" } }\n"
        "pcm.!default { type plug slave.pcm \"nxt_music_output\" }\n"
        "ctl.!default { type bluealsa device \"%s\" }\n", address,address,address);
    return n >= 0 && (unsigned long)n < size;
}

int NxtStock_info(void) {
    SDL_version version;
    SDL_GetVersion(&version);
    printf(NXT_APP_NAME " v" NXT_VERSION " — " NXT_BUILD_REVISION " / TrimUI Brick Pro\n");
    printf("SDL %u.%u.%u\n", version.major, version.minor, version.patch);
    printf("Video drivers:");
    for (int i = 0; i < SDL_GetNumVideoDrivers(); i++)
        printf(" %s", SDL_GetVideoDriver(i));
    printf("\nRender drivers:");
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
        SDL_RendererInfo info;
        if (!SDL_GetRenderDriverInfo(i, &info)) printf(" %s", info.name);
    }
    printf("\nResources: %s\nSettings: %s\n", RES_PATH, USERDATA_PATH);
    const char *required[] = {RES_PATH "/font1.ttf", RES_PATH "/font2.ttf",
                             RES_PATH "/assets@3x.png", RES_PATH "/inputs@3x.png",
                             "./res/icons/spotify-source.png",
                             "./res/icons/soundcloud-source.png"};
    bool valid = true;
    for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); i++) {
        bool exists = access(required[i], R_OK) == 0;
        printf("%s %s\n", exists ? "OK" : "MISSING", required[i]);
        valid = valid && exists;
    }
    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif

#ifdef NXT_ROM_STOCK
bool NxtStock_wifiConnected(void){static Uint32 last;static bool result;Uint32 now=SDL_GetTicks();if(last&&(Uint32)(now-last)<750)return result;last=now;result=false;struct ifaddrs *list=NULL;if(getifaddrs(&list))return false;for(struct ifaddrs *a=list;a;a=a->ifa_next){if(!a->ifa_name||!a->ifa_addr||!(a->ifa_flags&IFF_UP)||!(a->ifa_flags&IFF_RUNNING)||(a->ifa_flags&IFF_LOOPBACK))continue;char path[256];snprintf(path,sizeof(path),"/sys/class/net/%s/wireless",a->ifa_name);if(access(path,F_OK)&&strncmp(a->ifa_name,"wlan",4)&&strncmp(a->ifa_name,"wl",2))continue;if(a->ifa_addr->sa_family==AF_INET){unsigned ip=ntohl(((struct sockaddr_in*)a->ifa_addr)->sin_addr.s_addr);if(ip&&(ip>>24)!=127&&(ip>>16)!=0xa9fe)result=true;}else if(a->ifa_addr->sa_family==AF_INET6){struct in6_addr *ip=&((struct sockaddr_in6*)a->ifa_addr)->sin6_addr;if(!IN6_IS_ADDR_UNSPECIFIED(ip)&&!IN6_IS_ADDR_LOOPBACK(ip)&&!IN6_IS_ADDR_LINKLOCAL(ip))result=true;}}freeifaddrs(list);return result;}
#endif
