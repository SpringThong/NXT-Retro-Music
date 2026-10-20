#include "nxt_lyric_words.h"
#define _GNU_SOURCE
#include "lyrics.h"
#include "nxt.h"
#include "lyric_window.h"
#include "radio_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdatomic.h>

#include "defines.h"
#include "api.h"
#include "include/parson/parson.h"

// Lyrics cache directory path on SD card
#define LYRICS_CACHE_DIR SDCARD_PATH "/.cache/lyrics"
#define CACHE_PARENT_DIR SDCARD_PATH "/.cache"

// Lyrics state (main thread only - written by thread when done)
static LyricLine lyrics_lines[LYRICS_MAX_LINES];
static int lyrics_line_count = 0;
static bool lyrics_synced = false;  // false: plain text with estimated times
static bool lyrics_available = false;
static unsigned display_revision=1;
enum {LS_NONE,LS_FETCHING,LS_READY,LS_NOT_FOUND,LS_NETWORK};
static atomic_int original_state=LS_NONE;

// Dedup tracking
static char last_artist[256] = "";
static char last_title[256] = "";
static char last_path[512] = "";
static int last_duration_sec;

// Background thread state - uses generation counter instead of pthread_join
// to avoid blocking the main thread on network timeouts
static atomic_int fetch_generation = 0;
static pthread_mutex_t pending_lock=PTHREAD_MUTEX_INITIALIZER;
static LyricLine pending_lines[LYRICS_MAX_LINES];
static int pending_count,pending_generation;static bool pending_synced;
static void apply_pending(void){
    pthread_mutex_lock(&pending_lock);
    if(pending_count&&pending_generation==fetch_generation){memcpy(lyrics_lines,pending_lines,sizeof(LyricLine)*pending_count);lyrics_line_count=pending_count;lyrics_synced=pending_synced;lyrics_available=true;display_revision++;}
    pending_count=0;pthread_mutex_unlock(&pending_lock);
}

// Simple hash function for cache filename (DJB2)
static unsigned int simple_hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Ensure cache directory exists
static void ensure_cache_dir(void) {
    mkdir(CACHE_PARENT_DIR, 0755);
    mkdir(LYRICS_CACHE_DIR, 0755);
}

// Get cache file path for artist+title
static void get_cache_filepath(const char* artist, const char* title, char* path, int path_size) {
    char combined[512];
    snprintf(combined, sizeof(combined), "%s - %s", artist ? artist : "", title ? title : "");
    unsigned int hash = simple_hash(combined);
    snprintf(path, path_size, "%s/%08x.lrc", LYRICS_CACHE_DIR, hash);
}

// URL encode a string for use in query parameters
static void url_encode(const char* src, char* dst, int dst_size) {
    const char* hex = "0123456789ABCDEF";
    int j = 0;
    for (int i = 0; src[i] && j < dst_size - 4; i++) {
        unsigned char c = (unsigned char)src[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            dst[j++] = c;
        } else if (c == ' ') {
            dst[j++] = '+';
        } else {
            dst[j++] = '%';
            dst[j++] = hex[c >> 4];
            dst[j++] = hex[c & 0x0F];
        }
    }
    dst[j] = '\0';
}

/* Strict, bounded timestamp reader. Enhanced LRC <mm:ss.xxx> tokens supply
 * actual word boundaries. Plain LRC never acquires invented word timings. */
static bool stamp(const char *p,const char *end,char close,int *ms,const char **after){
 if(p>=end||*p<'0'||*p>'9')return false;long minute=0,second=0,fraction=0;int digits=0;
 while(p<end&&*p>='0'&&*p<='9'){minute=minute*10+*p++-'0';if(minute>35791)return false;}
 if(p>=end||*p++!=':'||p>=end||*p<'0'||*p>'9')return false;
 while(p<end&&*p>='0'&&*p<='9'){second=second*10+*p++-'0';if(second>59)return false;}
 if(p<end&&(*p=='.'||*p==':')){p++;while(p<end&&*p>='0'&&*p<='9'){if(digits<3)fraction=fraction*10+*p-'0';digits++;p++;}if(!digits)return false;}
 if(p>=end||*p++!=close)return false;if(digits==1)fraction*=100;else if(digits==2)fraction*=10;
 long value=minute*60000+second*1000+fraction;if(value>2147483647)return false;*ms=(int)value;*after=p;return true;
}
static void enhanced_text(const char *p,const char *end,LyricLine *line){
 int used=0,pending=-1,last=-1;bool valid=true,overflow=false;
 while(p<end){
  int at;const char *after;
  if(*p=='<'&&stamp(p+1,end,'>',&at,&after)){
   if(at<line->time_ms||at<last)valid=false;last=at;
   if(pending>=0){LyricWord *w=&line->words[pending];w->length=used-w->offset;w->end_ms=at;if(!w->length){line->word_count--;pending=-1;}}
   if(line->word_count<LYRICS_MAX_WORDS){pending=line->word_count++;line->words[pending]=(LyricWord){.time_ms=at,.end_ms=at,.offset=used};}
   else {overflow=true;pending=-1;}
   p=after;continue;
  }
  unsigned char ch=(unsigned char)*p;int n=ch<0x80?1:(ch&0xe0)==0xc0?2:(ch&0xf0)==0xe0?3:(ch&0xf8)==0xf0?4:1;if(p+n>end)n=1;
  if(used+n<(int)sizeof(line->text)&&!overflow){memcpy(line->text+used,p,n);used+=n;}else overflow=true;
  p+=n;
 }
 if(pending>=0){LyricWord *w=&line->words[pending];w->length=used-w->offset;if(!w->length)line->word_count--;}
 /* A bounded truncation must end at a UTF-8 boundary. */
 while(used&&p<end&&((unsigned char)*p&0xc0)==0x80)used--;
 line->text[used]=0;
 if(!valid||overflow)line->word_count=0;
 for(unsigned i=0;i<line->word_count;i++){LyricWord *w=&line->words[i];if(w->offset+w->length>used||((unsigned char)line->text[w->offset]&0xc0)==0x80)line->word_count=0;}
}
static int parse_lrc_text(const char* input,LyricLine* lines,int max_lines){
 int count=0;const char *p=input;
 while(*p&&count<max_lines){
  const char *end=p;while(*end&&*end!='\n'&&*end!='\r')end++;
  const char *next=end;while(*next=='\n'||*next=='\r')next++;
  while(p<end&&(*p==' '||*p=='\t'))p++;
  int time;const char *text;
  if(p<end&&*p=='['&&stamp(p+1,end,']',&time,&text)){
   /* Repeated line timestamps remain supported, with equal relative word timing. */
   int times[16],n=0;times[n++]=time;const char *a=text;int more;
   while(n<16&&a<end&&*a=='['&&stamp(a+1,end,']',&more,&text)){times[n++]=more;a=text;}
   for(int k=0;k<n&&count<max_lines;k++){
    LyricLine *line=&lines[count];memset(line,0,sizeof(*line));line->time_ms=times[0];enhanced_text(a,end,line);line->time_ms=times[k];
    if(k&&line->word_count){int delta=times[k]-times[0];for(unsigned j=0;j<line->word_count;j++){line->words[j].time_ms+=delta;line->words[j].end_ms+=delta;}}
    if(line->text[0])count++;
   }
  }
  p=next;
 }
 /* Stable insertion sort keeps duplicate timestamp lines deterministic. */
 for(int i=1;i<count;i++){LyricLine value=lines[i];int j=i;while(j>0&&lines[j-1].time_ms>value.time_ms){lines[j]=lines[j-1];j--;}lines[j]=value;}
 return count;
}

// Read a whole text file. Returns a string that the caller frees, or NULL.
static char* read_text_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 256 * 1024) {
        fclose(f);
        return NULL;
    }

    char* data = (char*)malloc(size + 1);
    if (!data) {
        fclose(f);
        return NULL;
    }

    if (fread(data, 1, size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    data[size] = '\0';
    return data;
}

// Weight of a plain line: its character count, with a floor so a short line
// ("Oh", "Yeah") still gets some time
#define PLAIN_MIN_WEIGHT 8
// The words of a song usually start after an intro and end before an outro
#define PLAIN_START_PERMILLE 80
#define PLAIN_END_PERMILLE   920
// Time per line where the duration is not known
#define PLAIN_LINE_MS 4000

// Parse plain lyrics text, and give each line an estimated time: the lines share
// the middle of the track by their length, and a blank line (a break between
// verses) takes the time of an average line. Lines like "[Chorus]" are skipped.
static int parse_plain_text(const char* text, int duration_ms, LyricLine* lines, int max_lines) {
    // The weight of the lines before each line, and the count of the breaks before it
    int* lines_before = (int*)malloc(sizeof(int) * max_lines);
    int* breaks_before = (int*)malloc(sizeof(int) * max_lines);
    if (!lines_before || !breaks_before) {
        free(lines_before);
        free(breaks_before);
        return 0;
    }

    int count = 0;
    int total = 0;
    int breaks = 0;
    bool pending_break = false;
    const char* p = text;

    while (*p && count < max_lines) {
        const char* end = p;
        while (*end && *end != '\n') end++;
        const char* next = *end ? end + 1 : end;

        // Trim the line
        while (p < end && (*p == ' ' || *p == '\t')) p++;
        const char* e = end;
        while (e > p && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r')) e--;
        int len = (int)(e - p);

        if (len == 0) {
            pending_break = (count > 0);
        } else if (p[0] == '[' && e[-1] == ']') {
            // A section label
        } else {
            if (pending_break) {
                breaks++;
                pending_break = false;
            }
            if (len > (int)sizeof(lines[count].text)-1){len=sizeof(lines[count].text)-1;while(len&&((unsigned char)p[len]&0xc0)==0x80)len--;}
            memset(&lines[count],0,sizeof(lines[count]));
            memcpy(lines[count].text, p, len);
            lines[count].text[len] = '\0';

            // Count characters, not bytes: skip UTF-8 continuation bytes
            int chars = 0;
            for (int i = 0; i < len; i++) {
                if (((unsigned char)p[i] & 0xC0) != 0x80) chars++;
            }
            lines_before[count] = total;
            breaks_before[count] = breaks;
            total += chars > PLAIN_MIN_WEIGHT ? chars : PLAIN_MIN_WEIGHT;
            count++;
        }
        p = next;
    }

    if (count > 0) {
        int break_weight = total / count;
        int64_t sum = (int64_t)total + (int64_t)breaks * break_weight;
        int64_t start = (int64_t)duration_ms * PLAIN_START_PERMILLE / 1000;
        int64_t span = (int64_t)duration_ms * (PLAIN_END_PERMILLE - PLAIN_START_PERMILLE) / 1000;
        for (int i = 0; i < count; i++) {
            int64_t before = lines_before[i] + (int64_t)breaks_before[i] * break_weight;
            lines[i].time_ms = duration_ms > 0 ? (int)(start + span * before / sum) : i * PLAIN_LINE_MS;
        }
    }

    free(lines_before);
    free(breaks_before);
    return count;
}

// Parse lyrics text of either kind: LRC if it has timestamps, else plain text.
// Sets *synced to tell which one it was.
static int parse_lyrics_text(const char* text, int duration_ms, LyricLine* lines, int max_lines,
                             bool* synced) {
    int count = parse_lrc_text(text, lines, max_lines);
    *synced = (count > 0);
    if (count == 0) count = parse_plain_text(text, duration_ms, lines, max_lines);
    return count;
}

// Read a lyrics file (a cache entry, or an .lrc file next to the track) into the line array.
// Returns line count, or 0 on failure.
static int load_lyrics_file(const char* path, int duration_ms, LyricLine* lines, int max_lines,
                            bool* synced) {
    char* data = read_text_file(path);
    if (!data) return 0;

    int count = parse_lyrics_text(data, duration_ms, lines, max_lines, synced);
    free(data);
    return count;
}

// Save LRC text to cache file
static void save_lyrics_to_cache(const char* cache_path, const char* lrc_text) {
    FILE* f = fopen(cache_path, "w");
    if (!f) return;
    fputs(lrc_text, f);
    fclose(f);
}

// Thread argument
typedef struct {
    char artist[256];
    char title[256];
    char track_path[512];
    char* embedded;  // lyrics from the tags of the file, owned (NULL if none)
    int duration_sec;
    int generation;  // to detect if this fetch is still current
} FetchArgs;

static void free_fetch_args(FetchArgs* args) {
    free(args->embedded);
    free(args);
}

// Put the parsed lines into the shared state, if this fetch is still current
static void publish_lines(const LyricLine* lines, int count, bool synced, int generation) {
    if(count<=0||count>LYRICS_MAX_LINES)return;
    pthread_mutex_lock(&pending_lock);
    if(fetch_generation==generation){memcpy(pending_lines,lines,sizeof(LyricLine)*count);pending_count=count;pending_synced=synced;pending_generation=generation;atomic_store(&original_state,LS_READY);}
    pthread_mutex_unlock(&pending_lock);
}

static void fetch_state(int gen,int state){pthread_mutex_lock(&pending_lock);if(fetch_generation==gen)atomic_store(&original_state,state);pthread_mutex_unlock(&pending_lock);}
static void base_title(const char *input,char out[256]){
 snprintf(out,256,"%s",input);char *cut=strpbrk(out,"([");char *wide=strstr(out,"（");if(wide&&(!cut||wide<cut))cut=wide;if(cut)*cut=0;
 size_t n=strlen(out);while(n&&out[n-1]==' ')out[--n]=0;
}
/* Only normalize a filename fallback; never overwrite actual artist tags. */
static void filename_lookup(FetchArgs *args){
 if(args->artist[0]||!args->track_path[0])return;const char *name=strrchr(args->track_path,'/');name=name?name+1:args->track_path;
 if(strcmp(args->title,name))return;char *ext=strrchr(args->title,'.');if(!ext)return;const char *types[]={".mp3",".flac",".wav",".m4a",".aac",".ogg",".opus",".alac",".mp4",".dsf",".dff"};bool audio=false;for(unsigned i=0;i<sizeof(types)/sizeof(*types);i++)if(!strcasecmp(ext,types[i]))audio=true;if(!audio)return;*ext=0;
 char *start=args->title;int digits=0;while(digits<4&&start[digits]>='0'&&start[digits]<='9')digits++;if(digits>0&&digits<4&&start[digits]=='.'&&start[digits+1]==' ')start+=digits+2;
 while(*start==' ')start++;if(start!=args->title)memmove(args->title,start,strlen(start)+1);size_t n=strlen(args->title);while(n&&args->title[n-1]==' ')args->title[--n]=0;
 char *split=strstr(args->title," - ");if(split&&split>args->title&&split[3]){size_t count=split-args->title;memcpy(args->artist,args->title,count);args->artist[count]=0;memmove(args->title,split+3,strlen(split+3)+1);}
}
static bool explicit_cover(const char *title){return strcasestr(title,"(cover")||strcasestr(title,"[cover")||strcasestr(title," cover ")||strcasestr(title,"（cover")||strstr(title,"翻唱");}
/* Never borrow synchronized timing from a different cover/remix. Plain text
 * may be used for a matching base title; it is explicitly marked unsynced. */
static const char *search_text(JSON_Value *root,FetchArgs *args){
 JSON_Array *a=json_value_get_array(root);const char *best=NULL;int best_score=-1;char wanted[256];base_title(args->title,wanted);
 for(size_t i=0;i<json_array_get_count(a);i++){
  JSON_Object *item=json_array_get_object(a,i);const char *title=json_object_get_string(item,"trackName"),*artist=json_object_get_string(item,"artistName");if(!title)continue;
  char candidate[256];base_title(title,candidate);bool exact=!strcasecmp(title,args->title),same_artist=artist&&!strcasecmp(artist,args->artist);
  if(args->artist[0]&&!same_artist&&!explicit_cover(args->title))continue;
  if(!exact&&(!*wanted||strcasecmp(candidate,wanted)))continue;
  double duration=json_object_get_number(item,"duration");double delta=duration-args->duration_sec;if(delta<0)delta=-delta;
  if(duration>0&&args->duration_sec>0&&delta>60)continue;
  bool correct_timing=exact&&same_artist&&(duration<=0||args->duration_sec<=0||delta<=12);
  const char *text=correct_timing?json_object_get_string(item,"syncedLyrics"):NULL;
  if(!text||!*text)text=json_object_get_string(item,"plainLyrics");if(!text||!*text)continue;
  int score=(exact?100:20)+(same_artist?30:0)+(correct_timing?20:0);if(score>best_score){best_score=score;best=text;}
 }
 return best;
}
// The path of the .lrc file next to a track: the extension of the track changed to .lrc
static void get_sidecar_filepath(const char* track_path, char* path, int path_size) {
    snprintf(path, path_size, "%s", track_path);
    char* dot = strrchr(path, '.');
    char* slash = strrchr(path, '/');
    if (dot && (!slash || dot > slash)) *dot = '\0';
    size_t len = strlen(path);
    snprintf(path + len, path_size - len, ".lrc");
}

// Background fetch thread function (detached — must not touch shared state if stale)
static void* fetch_thread_func(void* arg) {
    PWR_pinToCores(CPU_CORE_EFFICIENCY);
    FetchArgs* args = (FetchArgs*)arg;
    int my_gen = args->generation;

    // Temporary buffer for parsing (thread-local, not shared)
    LyricLine* tmp_lines = (LyricLine*)malloc(sizeof(LyricLine) * LYRICS_MAX_LINES);
    if (!tmp_lines) {
        fetch_state(my_gen,LS_NETWORK);free_fetch_args(args);
        return NULL;
    }

    int duration_ms = args->duration_sec * 1000;
    bool synced = false;
    int count = 0;

    // The lyrics of the file come first: an .lrc file next to the track, then the tags
    if (args->track_path[0]) {
        char sidecar_path[768];
        get_sidecar_filepath(args->track_path, sidecar_path, sizeof(sidecar_path));
        count = load_lyrics_file(sidecar_path, duration_ms, tmp_lines, LYRICS_MAX_LINES, &synced);
    }
    if (count == 0 && args->embedded) {
        count = parse_lyrics_text(args->embedded, duration_ms, tmp_lines, LYRICS_MAX_LINES, &synced);
    }

    if(count==0)filename_lookup(args);
    // The cache and LRCLIB find lyrics by artist and title, thus a track with neither stops here
    if (count == 0 && !args->artist[0] && !args->title[0]) {
        fetch_state(my_gen,LS_NOT_FOUND);free(tmp_lines);
        free_fetch_args(args);
        return NULL;
    }

    ensure_cache_dir();

    char cache_path[768];
    get_cache_filepath(args->artist, args->title, cache_path, sizeof(cache_path));
    // Then the disk cache of earlier downloads
    if (count == 0) {
        count = load_lyrics_file(cache_path, duration_ms, tmp_lines, LYRICS_MAX_LINES, &synced);
    }
    if (count > 0) {
        publish_lines(tmp_lines, count, synced, my_gen);
        free(tmp_lines);
        free_fetch_args(args);
        return NULL;
    }

    // URL-encode artist and title separately
    char encoded_artist[512];
    char encoded_title[512];
    url_encode(args->artist, encoded_artist, sizeof(encoded_artist));
    url_encode(args->title, encoded_title, sizeof(encoded_title));

    // Build LRCLIB exact match URL: /api/get?artist_name=X&track_name=Y&duration=Z
    char url[2048];
    snprintf(url, sizeof(url),
        "https://lrclib.net/api/get?artist_name=%s&track_name=%s&duration=%d",
        encoded_artist, encoded_title, args->duration_sec);
    // Fetch LRCLIB API response
    uint8_t* response_buf = (uint8_t*)malloc(64 * 1024+1);
    if (!response_buf) {
        free(tmp_lines);
        free_fetch_args(args);
        return NULL;
    }

    const char* synced_lyrics = NULL;
    JSON_Value* root = NULL;

    // Try exact match first
    int bytes = radio_net_fetch(url, response_buf, 64 * 1024, NULL, 0);

    if (bytes > 0) {
        response_buf[bytes] = '\0';
        root = json_parse_string((const char*)response_buf);
        if (root) {
            JSON_Object* obj = json_value_get_object(root);
            if (obj) {
                synced_lyrics = json_object_get_string(obj, "syncedLyrics");
                if (!synced_lyrics || !synced_lyrics[0]) synced_lyrics=json_object_get_string(obj,"plainLyrics");
                if (!synced_lyrics || !synced_lyrics[0]) synced_lyrics = NULL;
            }
            if (!synced_lyrics) {
                json_value_free(root);
                root = NULL;
            }
        }
    }

    // Check if we've been superseded before doing fallback fetch
    if (fetch_generation != my_gen) {
        free(response_buf);
        if (root) json_value_free(root);
        free(tmp_lines);
        free_fetch_args(args);
        return NULL;
    }

    // Fallback: fuzzy search if exact match failed
    if (!synced_lyrics) {
        char query[512];
        snprintf(query, sizeof(query), "%s %s", args->artist, args->title);
        char encoded_query[1024];
        url_encode(query, encoded_query, sizeof(encoded_query));
        snprintf(url, sizeof(url), "https://lrclib.net/api/search?q=%s", encoded_query);

        bytes = radio_net_fetch(url, response_buf, 64 * 1024, NULL, 0);

        if (bytes > 0) {
            response_buf[bytes] = '\0';
            root = json_parse_string((const char*)response_buf);
            if (root) {
                JSON_Array* results = json_value_get_array(root);
                if (results) synced_lyrics=search_text(root,args);
                if (!synced_lyrics) {
                    json_value_free(root);
                    root = NULL;
                }
            }
        }
    }

    /* Cover/version suffixes often fail the artist+title lookup. A final
     * base-title query may recover plain lyrics without claiming exact timing. */
    char base[256];base_title(args->title,base);
    if(!synced_lyrics&&*base&&strcmp(base,args->title)&&fetch_generation==my_gen){
        char encoded[1024];url_encode(base,encoded,sizeof(encoded));snprintf(url,sizeof(url),"https://lrclib.net/api/search?q=%s",encoded);
        bytes=radio_net_fetch(url,response_buf,64*1024,NULL,0);
        if(bytes>0){response_buf[bytes]=0;root=json_parse_string((const char*)response_buf);if(root)synced_lyrics=search_text(root,args);if(!synced_lyrics){json_value_free(root);root=NULL;}}
    }
    if(fetch_generation!=my_gen){free(response_buf);json_value_free(root);free(tmp_lines);free_fetch_args(args);return NULL;}
    free(response_buf);

    if (!synced_lyrics) {
        fetch_state(my_gen,bytes>0?LS_NOT_FOUND:LS_NETWORK);fprintf(stderr,"[NXT lyrics] no original lyrics generation=%d state=%d\n",my_gen,bytes>0?LS_NOT_FOUND:LS_NETWORK);
        if (root) json_value_free(root);
        free(tmp_lines);
        free_fetch_args(args);
        return NULL;
    }

    // Save raw LRC text to cache
    save_lyrics_to_cache(cache_path, synced_lyrics);

    // Parse into temp buffer
    count = parse_lyrics_text(synced_lyrics,duration_ms,tmp_lines,LYRICS_MAX_LINES,&synced);
    json_value_free(root);

    if(!count)fetch_state(my_gen,LS_NOT_FOUND);
    publish_lines(tmp_lines, count, synced, my_gen);

    free(tmp_lines);
    free_fetch_args(args);
    return NULL;
}

void Lyrics_init(void) {
    display_revision++;
    atomic_store(&original_state,LS_NONE);
    lyrics_line_count = 0;
    lyrics_available = false;
    last_artist[0] = '\0';
    last_title[0] = '\0';
    last_path[0] = '\0';
    fetch_generation = 0;
}

void Lyrics_cleanup(void) {
    display_revision++;
    fetch_generation++;  // invalidate any running thread
    lyrics_line_count = 0;
    lyrics_available = false;
    last_artist[0] = '\0';
    last_title[0] = '\0';
    last_path[0] = '\0';
}

void Lyrics_clear(void) {
    display_revision++;
    pthread_mutex_lock(&pending_lock);fetch_generation++;pending_count=0;atomic_store(&original_state,LS_NONE);pthread_mutex_unlock(&pending_lock);
    lyrics_line_count = 0;
    lyrics_available = false;
    last_artist[0] = '\0';
    last_title[0] = '\0';
    last_path[0] = '\0';
}

bool Lyrics_loadTextFile(const char *path,int duration_ms){
    Lyrics_clear();last_duration_sec=duration_ms/1000;char *text=read_text_file(path);if(!text)return false;
    lyrics_line_count=parse_lyrics_text(text,duration_ms,lyrics_lines,LYRICS_MAX_LINES,&lyrics_synced);free(text);lyrics_available=lyrics_line_count>0;atomic_store(&original_state,lyrics_available?LS_READY:LS_NOT_FOUND);display_revision++;return lyrics_available;
}

void Lyrics_fetch(const char* artist, const char* title, int duration_sec,
                  const char* track_path, const char* embedded) {
    if (!artist) artist = "";
    if (!title) title = "";
    if (!track_path) track_path = "";

    // A track with no tags can still have lyrics in its file, or next to it
    bool has_tags = artist[0] || title[0];
    if (!has_tags && !track_path[0] && !embedded) {
        return;
    }

    // Dedup check
    if (strcmp(last_artist, artist) == 0 &&
        strcmp(last_title, title) == 0 &&
        strcmp(last_path, track_path) == 0) {
        return;
    }

    // Invalidate old replies and update status atomically with their publish gate.
    pthread_mutex_lock(&pending_lock);fetch_generation++;atomic_store(&original_state,LS_FETCHING);pthread_mutex_unlock(&pending_lock);

    // Reset state
    display_revision++;
    strncpy(last_artist, artist, sizeof(last_artist) - 1);
    last_artist[sizeof(last_artist) - 1] = '\0';
    strncpy(last_title, title, sizeof(last_title) - 1);
    last_title[sizeof(last_title) - 1] = '\0';
    snprintf(last_path, sizeof(last_path), "%s", track_path);
    lyrics_line_count = 0;
    lyrics_available = false;

    // Prepare thread args
    FetchArgs* args = (FetchArgs*)malloc(sizeof(FetchArgs));
    if (!args) return;
    strncpy(args->artist, artist, sizeof(args->artist) - 1);
    args->artist[sizeof(args->artist) - 1] = '\0';
    strncpy(args->title, title, sizeof(args->title) - 1);
    args->title[sizeof(args->title) - 1] = '\0';
    snprintf(args->track_path, sizeof(args->track_path), "%s", track_path);
    args->embedded = embedded ? strdup(embedded) : NULL;
    args->duration_sec = duration_sec;last_duration_sec=duration_sec;
    args->generation = fetch_generation;

    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&thread, &attr, fetch_thread_func, args) != 0) {
        fetch_state(args->generation,LS_NETWORK);free_fetch_args(args);
    }
    pthread_attr_destroy(&attr);
}

int Lyrics_currentIndex(int position_ms) {
    apply_pending();
    position_ms -= nxt.lyric_offset;
    if (!lyrics_available) return -1;
    return LyricWindow_currentIndex(lyrics_lines, lyrics_line_count, position_ms);
}

int Lyrics_lineCount(void) {
    apply_pending();
    return lyrics_available ? lyrics_line_count : 0;
}

const char* Lyrics_lineText(int index) {
    apply_pending();
    if (!lyrics_available || index < 0 || index >= lyrics_line_count) return NULL;
    return lyrics_lines[index].text;
}

bool Lyrics_isAvailable(void) {
    apply_pending();
    return lyrics_available;
}

bool Lyrics_isSynced(void) {
    apply_pending();
    return lyrics_available && lyrics_synced;
}

unsigned Lyrics_revision(void){apply_pending();return display_revision;}

const char *Lyrics_status(void){
 apply_pending();int state=atomic_load(&original_state);
 return state==LS_READY?Nxt_text("Original lyrics ready","Lời gốc đã sẵn sàng"):state==LS_FETCHING?Nxt_text("Finding original lyrics…","Đang tìm lời gốc…"):state==LS_NOT_FOUND?Nxt_text("No original lyrics found · add a .lrc file","Không tìm thấy lời gốc · thêm file .lrc"):state==LS_NETWORK?Nxt_text("Cannot fetch original lyrics · check Wi-Fi","Chưa tải được lời gốc · kiểm tra Wi-Fi"):Nxt_text("No original lyrics loaded","Chưa có lời gốc");
}

void Lyrics_retry(void){
 int state=atomic_load(&original_state);if(state==LS_FETCHING||state==LS_READY)return;
 char artist[256],title[256],path[512];snprintf(artist,sizeof(artist),"%s",last_artist);snprintf(title,sizeof(title),"%s",last_title);snprintf(path,sizeof(path),"%s",last_path);
 if(!*artist&&!*title&&!*path)return;last_artist[0]=last_title[0]=last_path[0]=0;Lyrics_fetch(artist,title,last_duration_sec,path,NULL);
}

/* Only one active line is estimated, once per lyric revision; no worker,
 * audio decode, network request or per-frame allocation is needed. */
static LyricLine estimate;static unsigned estimate_revision;static int estimate_index=-1;
static const LyricLine *word_line(int index){
 apply_pending();if(!lyrics_available||index<0||index>=lyrics_line_count)return NULL;
 const LyricLine *l=&lyrics_lines[index];if(l->word_count||!nxt.karaoke_estimate)return l;
 if(estimate_revision!=display_revision||estimate_index!=index){estimate=*l;int end=index+1<lyrics_line_count?lyrics_lines[index+1].time_ms:last_duration_sec*1000;NxtWords_estimate(&estimate,end);estimate_revision=display_revision;estimate_index=index;}return &estimate;
}
bool Lyrics_canFollow(void){return Lyrics_isAvailable()&&(Lyrics_isSynced()||nxt.karaoke_estimate);}
bool Lyrics_copyLine(int index,LyricLine *out){apply_pending();if(!out||!lyrics_available||index<0||index>=lyrics_line_count)return false;*out=lyrics_lines[index];return true;}
bool Lyrics_wordEstimated(int index){const LyricLine*l=word_line(index);return l&&l->estimated;}
int Lyrics_wordCount(int index){const LyricLine*l=word_line(index);return l?l->word_count:0;}
const LyricWord *Lyrics_wordAt(int index,int word){const LyricLine*l=word_line(index);return l&&word>=0&&word<l->word_count?&l->words[word]:NULL;}
bool Lyrics_wordMotion(int index,int position_ms){position_ms-=nxt.lyric_offset;for(int i=0;i<Lyrics_wordCount(index);i++){const LyricWord *w=Lyrics_wordAt(index,i);if(position_ms>=w->time_ms&&position_ms<w->end_ms)return true;}return false;}

/* UI-thread publication: an old recognition job cannot replace a newer song. */
int Lyrics_lineTime(int index){apply_pending();return index>=0&&index<lyrics_line_count?lyrics_lines[index].time_ms:0;}
bool Lyrics_applyWordTimings(unsigned revision,const LyricLine *lines,int count,bool synced){
 apply_pending();if(!lines||revision!=display_revision||count!=lyrics_line_count||count<1||count>LYRICS_MAX_LINES)return false;
 int last=0;for(int i=0;i<count;i++){const LyricLine*l=&lines[i];if(strcmp(l->text,lyrics_lines[i].text)||l->time_ms<last||l->word_count>LYRICS_MAX_WORDS)return false;last=l->time_ms;size_t len=strlen(l->text);for(int k=0;k<l->word_count;k++){const LyricWord*w=&l->words[k];if(w->time_ms<l->time_ms||w->end_ms<=w->time_ms||w->offset+w->length>len||!w->length||(k&&w->time_ms<l->words[k-1].time_ms))return false;}}
 for(int i=0;i<count;i++){lyrics_lines[i].time_ms=lines[i].time_ms;lyrics_lines[i].word_count=lines[i].word_count;lyrics_lines[i].estimated=lines[i].estimated;lyrics_lines[i].aligned=lines[i].aligned;memcpy(lyrics_lines[i].words,lines[i].words,sizeof(lines[i].words));}lyrics_synced=synced;display_revision++;return true;
}
