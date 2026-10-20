/* Run the real Bluetooth volume helper without an external mixer or headset. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int commands, scans;
static int controls_ready=1;
static char last_command[256];
static FILE *qa_popen(const char *command,const char *mode) {
    assert(!strcmp(command,"amixer scontrols")&&!strcmp(mode,"r"));scans++;
    FILE *f=tmpfile();assert(f);
    fputs("Simple mixer control 'Battery',0\nSimple mixer control 'SCO',0\n",f);
    if(controls_ready)fputs("Simple mixer control 'A2DP',0\n",f);
    rewind(f);return f;
}
static int qa_pclose(FILE *f){return fclose(f);}
static int qa_system(const char *command){commands++;snprintf(last_command,sizeof(last_command),"%s",command);return 0;}
#define popen qa_popen
#define pclose qa_pclose
#define system qa_system
#include "msettings.c"
#undef popen
#undef pclose
#undef system
struct mixer *mixer_open(unsigned int card){assert(!"Bluetooth must not open a soundcard mixer");return NULL;}
void mixer_close(struct mixer *m){}
unsigned int mixer_get_num_ctls(const struct mixer *m){return 0;}
struct mixer_ctl *mixer_get_ctl(struct mixer *m,unsigned int id){return NULL;}
const char *mixer_ctl_get_name(const struct mixer_ctl *c){return NULL;}
enum mixer_ctl_type mixer_ctl_get_type(const struct mixer_ctl *c){return MIXER_CTL_TYPE_INT;}
int mixer_ctl_get_range_min(const struct mixer_ctl *c){return 0;}
int mixer_ctl_get_range_max(const struct mixer_ctl *c){return 100;}
unsigned int mixer_ctl_get_num_values(const struct mixer_ctl *c){return 0;}
int mixer_ctl_set_value(struct mixer_ctl *c,unsigned int id,int value){return 0;}
int mixer_ctl_set_percent(struct mixer_ctl *c,unsigned int id,int value){return 0;}
int main(void) {
    static Settings current;settings=&current;current.audiosink=AUDIO_SINK_BLUETOOTH;
    const int values[]={0,1,37,100};
    for(int i=0;i<4;i++) {
        SetRawVolume(values[i]);char expected[256];
        snprintf(expected,sizeof(expected),"amixer sset \"A2DP\" -M %d%% unmute > /dev/null 2>&1",values[i]);
        assert(!strcmp(last_command,expected)&&commands==i+1&&scans==i+1);
    }
    controls_ready=0;SetRawVolume(50);assert(commands==4&&scans==5);
    controls_ready=1;SetRawVolume(50);assert(commands==5&&strstr(last_command,"50% unmute"));
    current.mute=1;current.toggled_volume=0;SetRawVolume(80);
    assert(strstr(last_command,"0% unmute")&&commands==6);
    puts("PASS real Bluetooth mixer helper: A2DP control selected instead of Battery/SCO, explicit unmute with requested volume, absent control does not issue a command, later control recovers, system mute still outputs zero volume. No physical headset test.");
}

#include "v1881_bit_stub.inc"
