from pathlib import Path
R=Path(__file__).resolve().parents[1]
s=(R/'qa/v182_render.c').read_text()
s='#define PlayerModule_isActive Fixture_old_isActive\n#define Player_getPosition Fixture_old_position\n#define userdata_snpath Fixture_flat_snpath\n'+s
s=s.replace('static const char *quick', 'static const char *quick')
key='static const char *qa_output=';i=s.index(key);s=s[:i]+'''#undef PlayerModule_isActive
#undef Player_getPosition
#undef userdata_snpath
#include "nxt_startup.h"
#include "nxt_bitperfect.h"
static bool qa_resumed,qa_play;
static int qa_pos,qa_saved_position;
bool PlayerModule_isActive(void){return qa_resumed;}
int Player_getPosition(void){return qa_pos;}
int userdata_snpath(const char *rel,char *out,size_t size){int n=NxtPersonal_route(rel,out,size);return n>=0?n:snprintf(out,size,"%s/%s",getenv("QA_DATA"),rel);}
ModuleExitReason PlayerModule_runResumeMode(DisplayContext *d,const ResumeState *r,bool play){qa_resumed=true;qa_play=play;qa_saved_position=r->position_ms;fixture_state=play?PLAYER_STATE_PLAYING:PLAYER_STATE_PAUSED;return MODULE_EXIT_TO_MENU;}
bool NxtDrive_isStream(const char *p){return false;}
bool NxtBit_ready(void){return false;}
const char *NxtBit_status(void){return "Bit-Perfect requested · paused / unavailable";}
const char *NxtBit_error(void){return "Connect USB DAC";}
''' +s[i:]
s=s.replace('pressed_keys=frame_no<=sequence_count?sequence[frame_no-1]:BTN_B;released_keys=0;', 'int key=frame_no<=sequence_count?sequence[frame_no-1]:BTN_B;pressed_keys=key>0?key:0;released_keys=key<0?-key:0;')
# New startup flow exercises the real startup UI/persistence. Audio remains an explicit fixture.
i=s.index('int main(int argc')
s=s[:i]+r'''
static void v184_controls(void){
 NxtConfig before=nxt;nxt.bit_perfect=0;int preset=nxt.eq_preset;float bands[31];memcpy(bands,nxt.eq,sizeof(bands));
 const int select[]={0,-BTN_SELECT};input(select,2);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no==2&&nxt.eq_preset==preset&&!memcmp(bands,nxt.eq,sizeof(bands)));
 const int nested[]={BTN_START,0,-BTN_SELECT};input(nested,3);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no<=3&&nxt.eq_preset==preset);
 nxt.bit_perfect=1;input(select,2);assert(Nxt_dspScreen((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&frame_no==2&&nxt.eq_preset==preset&&!memcmp(bands,nxt.eq,sizeof(bands)));shot("bit-perfect-EQ-bypass");
 nxt=before;
 const int audio[]={BTN_A,0,BTN_B,BTN_B};input(audio,4);assert(Nxt_settings((DisplayContext*)1)==MODULE_EXIT_TO_MENU);shot("audio-settings-v184");
 const int mode[]={BTN_DOWN,BTN_A,BTN_DOWN,BTN_A,BTN_B,BTN_B};nxt.bit_perfect=0;input(mode,6);assert(Nxt_quickOptions((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&nxt.bit_perfect==1&&nxt.eq_preset==preset);shot("quick-options-v184");nxt=before;
 puts("PASS actual SELECT release: presets close unchanged, nested EQ closes fully, Bit-Perfect EQ closes unchanged; native settings and START mode selection.");
}
static void v184_startup(void){
 int old_child=child;bool old_track=track_loaded,old_playing=playing;PlayerState old_state=fixture_state;
 child=-1;track_loaded=playing=false;fixture_state=PLAYER_STATE_STOPPED;fixture_path[0]=0;qa_resumed=false;nxt.bit_perfect=0;nxt.auto_play=0;
 char file[1600];userdata_snpath("last-playback.json",file,sizeof(file));unlink(file);int mood=nxt.mood,genre=nxt.genre,country=nxt.country;
 const int back[]={0,BTN_B};input(back,2);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&!NxtStartup_preview()&&frame_no==2);shot("startup-empty-now-playing");assert(nxt.mood==mood&&nxt.genre==genre&&nxt.country==country);
 FILE *f=fopen(file,"w");assert(f);fputs("{\"path\":\"/missing/music.flac\",\"title\":\"Last song · Nhạc lần trước\",\"artist\":\"Saved artist\",\"source\":\"Local\",\"position_ms\":81723,\"duration_ms\":192000,\"sample_rate\":44100,\"bit_depth\":24,\"channels\":2}",f);fclose(f);
 input(back,2);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&!qa_resumed&&!NxtStartup_preview());shot("startup-missing-saved-track");JSON_Value *v=json_parse_file(file);assert(json_object_get_number(json_value_get_object(v),"position_ms")==81723);json_value_free(v);
 char song[1600];snprintf(song,sizeof(song),"%s/saved.flac",getenv("QA_DATA"));f=fopen(song,"w");assert(f);fputs("explicit audio fixture",f);fclose(f);
 v=json_value_init_object();JSON_Object *o=json_value_get_object(v);json_object_set_string(o,"path",song);json_object_set_string(o,"title","Saved song");json_object_set_string(o,"source","Local");json_object_set_number(o,"position_ms",81723);assert(json_serialize_to_file(v,file)==JSONSuccess);json_value_free(v);
 input(NULL,0);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&qa_resumed&&!qa_play&&qa_saved_position==81723&&!NxtStartup_preview());
 qa_resumed=false;nxt.auto_play=1;input(NULL,0);assert(NxtStartup_run((DisplayContext*)1)==MODULE_EXIT_TO_MENU&&qa_resumed&&qa_play&&qa_saved_position==81723);nxt.auto_play=0;qa_resumed=false;
 snprintf(fixture_path,sizeof(fixture_path),"%s",song);fixture_info=(TrackInfo){.duration_ms=192000,.sample_rate=44100,.channels=2,.bit_depth=24};snprintf(fixture_info.title,sizeof(fixture_info.title),"Saved native song");qa_pos=94567;fixture_state=PLAYER_STATE_PAUSED;NxtStartup_save();v=json_parse_file(file);assert(json_object_get_number(json_value_get_object(v),"position_ms")==94567);json_value_free(v);
 int old_listener=listener;listener=1;char profile_file[1600];userdata_mkdir("listeners/1");userdata_snpath("last-playback.json",profile_file,sizeof(profile_file));assert(strstr(profile_file,"listeners/1/"));qa_pos=128000;NxtStartup_save();v=json_parse_file(profile_file);assert(v&&json_object_get_number(json_value_get_object(v),"position_ms")==128000);json_value_free(v);v=json_parse_file(file);assert(v&&json_object_get_number(json_value_get_object(v),"position_ms")==94567);json_value_free(v);listener=old_listener;
 NxtStartup_quit();child=old_child;track_loaded=old_track;playing=old_playing;fixture_state=old_state;
 puts("PASS real startup card: empty Now Playing, no questionnaire, missing track kept, paused restore, optional auto-play, atomic position save and profile isolation. Audio playback is a fixture.");
}
''' +s[i:]
s=s.replace('original_preserved();original_cover_fit();galleries();profile_flow();eq_flow();icons_and_emoji();new_menu_flows();','original_preserved();original_cover_fit();galleries();profile_flow();eq_flow();icons_and_emoji();new_menu_flows();v184_controls();')
s=s.replace('SDL_FreeSurface(page);page=NULL;drawing=NULL;}', 'v184_startup();SDL_FreeSurface(page);page=NULL;drawing=NULL;}')
(R/'qa/v184_render.c').write_text(s)
s=(R/'qa/test_v182_ui.py').read_text().replace('v182','v184').replace('v1.8.2','v1.8.4')
s=s.replace("'include/parson/parson.c']","'include/parson/parson.c','nxt_startup.c']")
s=s.replace("'physical_device_tested':False", "'select_toggle_preserves_preset':True,'startup_restores_paused_without_questionnaire':True,'profile_local_startup_card':True,'physical_device_tested':False")
(R/'qa/test_v184_ui.py').write_text(s)
