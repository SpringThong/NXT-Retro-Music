#!/usr/bin/env python3
"""Real player queue controller/playlist/M3U/Drive records; decoder is a fixture."""
from pathlib import Path
import json
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "upstream/src"
render = ROOT / ".cache/render-sp"
includes = (render / "includes.txt").read_text().splitlines()
includes += ["-I" + str(p) for p in [SRC, SRC / "include",
    ROOT / "upstream/NextUI/workspace/all/common",
    ROOT / "upstream/NextUI/workspace/tg5040/platform",
    ROOT / "upstream/NextUI/workspace/tg5040/libmsettings"]]
with tempfile.TemporaryDirectory(prefix="nxt-drive-queue-") as directory:
    work = Path(directory)
    records = work / "drive/streams"
    records.mkdir(parents=True)
    paths = []
    for identity, name in [("1QueueFixture02", "2 Nhạc.wav"), ("1QueueFixture10", "10 Nhạc.wav")]:
        (records / (identity + ".json")).write_text(json.dumps({
            "url": "https://drive.google.com/file/d/" + identity + "/view", "name": name}))
        paths.append("nxtdrive://" + identity + "/audio.wav")
    queue = work / "folder.m3u"
    queue.write_text("#EXTM3U\n" + "\n".join(paths) + "\n")
    source = work / "queue_harness.c"
    source.write_text(r'''
#include "nxt_text.h"
#include "module_player.c"
#include <assert.h>
NxtConfig nxt;int is_brick=0,is_brickpro=1;
static char loaded[512],saved_queue[512];static int starts,stops,prepared,saved_index;
static TrackInfo info;
const BoolSetting SETTING_LYRICS_ENABLED={0};
static int display_callbacks_removed;
void DisplayHelper_removeRecreatedCallback(DisplayRecreatedCallback callback){assert(callback==display_recreated);display_callbacks_removed++;}
void LOG_note(int level,const char *fmt,...){}
const char *Nxt_text(const char *a,const char *b){return b;}
int userdata_snpath(const char *relative,char *out,size_t size){return snprintf(out,size,"%s/%s",getenv("QA_DATA"),relative);}
bool __wrap_NxtDrive_preparePlayback(const char *path){NxtDriveEntry e;assert(NxtDrive_streamEntry(path,&e));prepared++;return true;}
void Player_setPlaybackSpeed(float value){assert(value==1);}
int Player_load(const char *path){NxtDriveEntry e;assert(NxtDrive_streamEntry(path,&e));snprintf(loaded,sizeof(loaded),"%s",path);memset(&info,0,sizeof(info));NxtDrive_metadata(path,&info);return 0;}
int Player_play(void){starts++;return 0;}void Player_stop(void){stops++;}
const TrackInfo *Player_getTrackInfo(void){return &info;}
const char *Player_getCurrentFile(void){return loaded;}
SDL_Surface *Player_getAlbumArt(void){return (SDL_Surface*)1;}
const char *Player_getEmbeddedLyrics(void){return NULL;}
void album_art_fetch(const char *artist,const char *title){assert(0);}
void Lyrics_clear(void){}void Lyrics_fetch(const char *a,const char *b,int sec,const char *path,const char *embedded){assert(0);}
bool Settings_getBool(const BoolSetting *key){return false;}
void Resume_saveFiles(const char *folder,const char *path,const char *name,int index,int position){assert(0);}
void Resume_savePlaylist(const char *queue,const char *path,const char *name,int index,int position){
 assert(!strcmp(path,loaded)&&index==playlist.current_index&&position==0);
 snprintf(saved_queue,sizeof(saved_queue),"%s",queue);saved_index=index;
}
Uint32 SDL_GetTicks(void){return 1000;}
AudioFormat Player_detectFormat(const char *path){const char *ext=strrchr(path,'.');return ext&&!strcasecmp(ext,".wav")?AUDIO_FORMAT_WAV:AUDIO_FORMAT_UNKNOWN;}
int main(int argc,char **argv){
 assert(argc==2);Playlist_init(&playlist);assert(playlist.tracks);
 assert(!M3U_loadTracks(argv[1],playlist.tracks,PLAYLIST_MAX_TRACKS,&playlist.track_count)&&playlist.track_count==2);
 assert(!strcmp(playlist.tracks[0].name,"2 Nhạc.wav")&&!strcmp(playlist.tracks[1].name,"10 Nhạc.wav"));
 playlist_active=true;playlist.current_index=0;PlayerModule_setResumePlaylistPath(argv[1]);
 assert(playlist_try_play(0)&&strstr(loaded,"1QueueFixture02"));assert(saved_index==0&&!strcmp(saved_queue,argv[1]));
 assert(handle_track_ended()&&playlist.current_index==1&&strstr(loaded,"1QueueFixture10")&&saved_index==1);
 int before=starts;assert(!handle_track_ended()&&starts==before); /* Repeat off stops at the last song. */
 repeat_enabled=2;assert(handle_track_ended()&&playlist.current_index==0&&strstr(loaded,"1QueueFixture02"));
 repeat_enabled=1;assert(handle_track_ended()&&playlist.current_index==0&&strstr(loaded,"1QueueFixture02"));
 repeat_enabled=0;shuffle_enabled=true;assert(handle_track_ended()&&playlist.current_index==1&&strstr(loaded,"1QueueFixture10"));shuffle_enabled=false;
 PlayerModule_nextTrack();assert(playlist.current_index==0&&strstr(loaded,"1QueueFixture02"));
 PlayerModule_prevTrack();assert(playlist.current_index==1&&strstr(loaded,"1QueueFixture10"));
 before=starts;manually_stopped=true;assert(!handle_track_ended()&&starts==before);manually_stopped=false;
 assert(prepared==starts&&stops==2);
 /* Reopen the URI M3U as resume does, resolve the current song's index, and
  * start that song while retaining the complete folder queue. */
 char current[512];snprintf(current,sizeof(current),"%s",loaded);Playlist_free(&playlist);Playlist_init(&playlist);
 assert(!M3U_loadTracks(argv[1],playlist.tracks,PLAYLIST_MAX_TRACKS,&playlist.track_count)&&playlist.track_count==2);
 playlist.current_index=-1;for(int i=0;i<playlist.track_count;i++)if(!strcmp(current,playlist.tracks[i].path))playlist.current_index=i;
 assert(playlist.current_index==1&&playlist_try_play(-1)&&strstr(loaded,"1QueueFixture10"));
 assert(prepared==starts&&saved_index==1);
 /* Changing listener stops first, then releases the module; its next start
  * must not reuse the previous listener's queue, browser or resume path. */
 initialized=true;browser.entries=calloc(1,sizeof(*browser.entries));assert(browser.entries);browser.entry_count=1;
 strcpy(browser.current_path,"/previous/listener/music");strcpy(resume_playlist_path,"/previous/listener/queue.m3u");
 restore_position=45000;queue_generation=44;next_candidate=1;last_mode=10;nav_stack_top=3;manually_stopped=true;
 PlayerModule_quit();assert(!initialized&&!playlist_active&&!playlist.tracks&&!browser.entries&&!browser.current_path[0]);
 assert(!resume_playlist_path[0]&&restore_position==-1&&queue_generation==0&&next_candidate==-1&&last_mode==-1&&nav_stack_top==0&&!manually_stopped);
 assert(display_callbacks_removed==1);PlayerModule_quit();assert(display_callbacks_removed==1);
 /* A pending queue created before opening a player UI is also released. */
 Playlist_init(&playlist);assert(playlist.tracks);playlist_active=true;playlist.track_count=1;
 PlayerModule_quit();assert(!playlist.tracks&&!playlist_active&&playlist.track_count==0);
 puts("PASS real queue controller + playlist + saved Drive URI/M3U: automatic next/end, repeat one/all, shuffle, manual next/previous wrap, stopped playback and resume at the selected song. Audio loading is a fixture.");
 return 0;
}
''')
    binary = work / "queue"
    command = ["gcc", "-O2", "-D_GNU_SOURCE", "-DUSE_SDL2", "-DNXT_ROM_STOCK", "-ffunction-sections", "-fdata-sections",
        *includes, str(source), *[str(SRC / f) for f in ["playlist.c", "playlist_m3u.c", "browser.c",
        "nxt_drive.c", "nxt_drive_parser.c", "nxt_sort.c", "nxt_text.c", "include/parson/parson.c"]],
        str(render / "ttf.o"), str(render / "utf8proc.o"), "-Wl,--gc-sections", "-Wl,--wrap=NxtDrive_preparePlayback",
        "-l:libSDL2-2.0.so.0", "-l:libfreetype.so.6", "-l:libharfbuzz.so.0", "-l:libfribidi.so.0",
        "-lm", "-o", str(binary)]
    subprocess.run(command, check=True)
    subprocess.run([str(binary), str(queue)], env={**os.environ, "QA_DATA": str(work)}, timeout=15, check=True)
(ROOT / "logs/stock-drive-queue-test.json").write_text(json.dumps({"queue_controller": {
    "exit": 0, "production_module_player_and_playlist": True, "auto_advance_repeat_shuffle_manual_navigation": True,
    "saved_drive_uri_m3u_resume": True, "module_profile_reset": True, "audio_load_fixture": True}}, indent=2) + "\n")
