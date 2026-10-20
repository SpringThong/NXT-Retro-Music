#!/usr/bin/env python3
"""Real original-lyrics pipeline -> translator -> both disk caches offline."""
from pathlib import Path
import subprocess,tempfile,os,json,re,shutil,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/nxredux-build/headers';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';Q=R/'.cache/v191-host'
with tempfile.TemporaryDirectory(prefix='nxt191-original-') as td:
 w=Path(td);data=w/'sd';data.mkdir();hdr=w/'headers';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2');bin_=w/'bin';bin_.mkdir();shutil.copy2(R/'qa/fake_v189_translation_curl.py',bin_/'curl');(bin_/'curl').chmod(0o755);control=w/'control';control.write_text('ok');requests=w/'requests.jsonl'
 source=(R/'qa/v189_translation.c').read_text().replace('int main(void){','int legacy_main(void){')
 for name in ['Lyrics_revision','Lyrics_lineCount','Lyrics_lineText','Lyrics_currentIndex','Lyrics_status','Lyrics_retry']:
  source=re.sub(r'^(?:unsigned|int|const char\s*\*|void)\s*'+name+r'\([^\n]*\}\s*$', '',source,flags=re.M)
 source+='\n#undef SDCARD_PATH\n#define SDCARD_PATH '+json.dumps(str(data))+'\n#define url_encode original_url_encode\n#include "../upstream/src/lyrics.c"\n#undef url_encode\n'
 source+='''
static atomic_int net_calls=0;static bool offline;
void PLAT_pinToCores(int t){}
int radio_net_fetch(const char*url,uint8_t*buf,int cap,char*ct,int cn){atomic_fetch_add(&net_calls,1);assert(!strncmp(url,"https://lrclib.net/",18));if(offline)return -1;fprintf(stderr,"fixture lookup: %s\\n",url);assert(strstr(url,"artist_name=Test+Artist")&&strstr(url,"track_name=Test+Song"));const char*json="{\\"trackName\\":\\"Test Song\\",\\"artistName\\":\\"Test Artist\\",\\"duration\\":20,\\"syncedLyrics\\":\\"[00:00.000]The morning light is warm.\\\\n[00:01.500]A peaceful day.\\"}";int n=strlen(json);assert(n<cap);memcpy(buf,json,n);return n;}
int main(void){
 FetchArgs args={0};strcpy(args.title,"01. Test Artist - Test Song.mp3");strcpy(args.track_path,"/Missing/01. Test Artist - Test Song.mp3");filename_lookup(&args);assert(!strcmp(args.artist,"Test Artist")&&!strcmp(args.title,"Test Song"));
 FetchArgs tagged={0};strcpy(tagged.artist,"Keep Artist");strcpy(tagged.title,"Keep Title");strcpy(tagged.track_path,"/Music/Other - Song.mp3");filename_lookup(&tagged);assert(!strcmp(tagged.title,"Keep Title")&&!strcmp(tagged.artist,"Keep Artist"));
 FetchArgs wanted={.duration_sec=20};strcpy(wanted.artist,"Test Artist");strcpy(wanted.title,"Test Song");JSON_Value*v=json_parse_string("[{\\"trackName\\":\\"Test Song\\",\\"artistName\\":\\"Wrong Artist\\",\\"duration\\":20,\\"plainLyrics\\":\\"Wrong song\\"}]");assert(!search_text(v,&wanted));json_value_free(v);
 strcpy(wanted.title,"Test Song (cover)");v=json_parse_string("[{\\"trackName\\":\\"Test Song\\",\\"artistName\\":\\"Original Artist\\",\\"duration\\":20,\\"syncedLyrics\\":\\"[00:00.000]Different recording timing\\",\\"plainLyrics\\":\\"Matching cover words\\"}]");assert(!strcmp(search_text(v,&wanted),"Matching cover words"));json_value_free(v);
 Lyrics_init();nxt.language=1;nxt.translation_mode=2;provider=PROVIDER_GROQ;strcpy(groq_key,"FAKE_GROQ_SECRET_189");assert(save_service());service_loaded=true;
 Lyrics_fetch("","01. Test Artist - Test Song.mp3",20,"/Missing/01. Test Artist - Test Song.mp3",NULL);ready();assert(Lyrics_lineCount()==2&&Lyrics_isSynced());assert(!strcmp(Lyrics_lineText(0),"The morning light is warm."));assert(strstr(NxtTranslate_line(0),"vi:")&&Lyrics_currentIndex(1500)==1);assert(atomic_load(&net_calls)==1);int n=requests();assert(n>0);
 NxtTranslate_quit();Lyrics_cleanup();Lyrics_init();offline=true;control("network");Lyrics_fetch("","01. Test Artist - Test Song.mp3",20,"/Missing/01. Test Artist - Test Song.mp3",NULL);NxtTranslate_tick();ready();assert(atomic_load(&net_calls)==1&&requests()==n&&Lyrics_lineCount()==2&&strstr(NxtTranslate_line(0),"vi:"));
 NxtTranslate_quit();Lyrics_cleanup();puts("PASS real lyrics -> translation: filename normalization, tagged metadata preserved, wrong artist rejected, explicit cover plain-only, downloaded original LRC triggers translation, original+translation cold offline caches use no network/API calls.");return 0;}
'''
 driver=R/'qa/.v191-original-driver.c';driver.write_text(source);b=w/'test';incs=['-I'+str(p) for p in [hdr,H,H/'utf8proc',S,S/'include',S/'include/mbedtls_lib',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(driver),str(S/'lyric_window.c'),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),str(S/'include/mbedtls_lib/platform_util.c'),str(Q/'utf8proc.o'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(b)]
 try:
  p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr
  p=subprocess.run([str(b)],cwd=R/'upstream',env={**os.environ,'QA_DATA':str(data),'QA_CONTROL':str(control),'QA_REQUESTS':str(requests),'PATH':str(bin_)+':'+os.environ['PATH'],'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},capture_output=True,text=True,timeout=35);print(p.stdout+p.stderr);assert p.returncode==0,p.stdout+p.stderr
 finally:driver.unlink(missing_ok=True)
(R/'logs/v191-original-translation-test.json').write_text(json.dumps({'exit':0,'production_original_and_translation_workers':True,'filename_metadata_normalization':True,'known_wrong_artist_rejected':True,'cover_plain_only':True,'fetched_LRC_triggers_translation':True,'cold_original_and_translation_offline_caches':True,'network_and_service_fixtures':True,'ASan_UBSan':True,'live_sources_tested':False,'hardware_tested':False,'sources':{n:hashlib.sha256((S/n).read_bytes()).hexdigest() for n in ['lyrics.c','nxt_translate.c']}},indent=2)+'\n')
