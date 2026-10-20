#!/usr/bin/env python3
from pathlib import Path
import subprocess,tempfile,os,json,tarfile,hashlib
R=Path(__file__).resolve().parents[1];S=R/'upstream/src';H=R/'.cache/nxredux-build/headers';SDK=R/'.nxt-sdk/tg5040/compiler/aarch64-nextui-linux-gnu/libc';Q=R/'.cache/v191-host';Q.mkdir(parents=True,exist_ok=True)
def host_includes(w):
 hdr=w/'headers';hdr.mkdir();(hdr/'SDL2').symlink_to(SDK/'usr/include/SDL2')
 return ['-I'+str(p) for p in [hdr,H,H/'utf8proc',S,S/'include',S/'include/mbedtls_lib',R/'upstream/NextUI/workspace/all/common',R/'upstream/NextUI/workspace/tg5040/platform',R/'upstream/NextUI/workspace/tg5040/libmsettings']]
archive=R/'sources/utf8proc-2.9.0.tar.gz';assert hashlib.sha256(archive.read_bytes()).hexdigest()==archive.with_suffix('.gz.sha256').read_text().split()[0]
with tarfile.open(archive) as t:
 for m in t.getmembers():
  if m.isfile() and Path(m.name).name in ['utf8proc.c','utf8proc.h','utf8proc_data.c']:(Q/Path(m.name).name).write_bytes(t.extractfile(m).read())
subprocess.run(['gcc','-O1','-g','-fsanitize=address,undefined','-c',str(Q/'utf8proc.c'),'-o',str(Q/'utf8proc.o')],check=True)
with tempfile.TemporaryDirectory(prefix='nxt191-cache-') as td:
 w=Path(td);data=w/'data';data.mkdir();cache=data/'translation-cache';cache.mkdir();incs=host_includes(w)
 source=(R/'qa/v189_translation.c').read_text().replace('int main(void){','int legacy_main(void){')
 source+='''
static off_t cache_total(Job*j,int*count){DIR*d=opendir(j->cache_dir);assert(d);off_t total=0;*count=0;struct dirent*e;char p[1600];while((e=readdir(d))){if(strlen(e->d_name)!=69)continue;snprintf(p,sizeof(p),"%s/%s",j->cache_dir,e->d_name);struct stat st;if(!lstat(p,&st)&&S_ISREG(st.st_mode)){total+=st.st_size;(*count)++;}}closedir(d);return total;}
int main(int argc,char**argv){assert(CACHE_BYTES==256LL*1024*1024);assert(sizeof(CacheEntry)*CACHE_PRUNE_BATCH<=32*1024);Job*j=calloc(1,sizeof(*j));assert(j);j->generation=atomic_load(&generation);snprintf(j->cache_dir,sizeof(j->cache_dir),"%s",argv[1]);memset(j->hash,'0',64);j->hash[64]=0;int count=0;
 assert(prune(j));off_t bytes=cache_total(j,&count);assert(count==atoi(argv[2])&&bytes<=CACHE_BYTES);assert(j->cache_room==CACHE_BYTES-bytes);
 char p[1600];snprintf(p,sizeof(p),"%s/%s.json",j->cache_dir,j->hash);assert(!access(p,F_OK));
 // Changed generation cancels cleanup without deleting anything.
 atomic_fetch_add(&generation,1);assert(!prune(j));int again;assert(cache_total(j,&again)==bytes&&again==count);j->generation=atomic_load(&generation);
 j->provider=PROVIDER_GROQ;j->target=1;j->source=0;j->source_auto=true;j->count=j->done=2;strcpy(j->model,"openai/gpt-oss-120b");j->input[0]=strdup("The morning light is warm.");j->input[1]=strdup("A peaceful day.");j->output[0]=strdup("Ánh sáng buổi sáng ấm áp.");j->output[1]=strdup("Một ngày bình yên.");digest(j);assert(save_cache(j));size_t saved=j->cache_saved_bytes;off_t room=j->cache_room;assert(j->pruned&&saved>0);assert(save_cache(j)&&j->cache_saved_bytes==saved&&j->cache_room==room);
 // Larger partial commit reuses headroom accounting, not a whole-folder RAM list.
 free(j->output[1]);j->output[1]=strdup("Một ngày bình yên và một bản dịch dài hơn trước.");assert(save_cache(j)&&j->cache_saved_bytes>saved);assert(j->cache_room==room-(off_t)(j->cache_saved_bytes-saved));
 Job*k=calloc(1,sizeof(*k));assert(k);k->generation=j->generation;k->count=2;k->provider=PROVIDER_GROQ;k->target=1;k->source=0;strcpy(k->model,j->model);strcpy(k->cache_dir,j->cache_dir);k->input[0]=strdup(j->input[0]);k->input[1]=strdup(j->input[1]);strcpy(k->curl_path,"/definitely-no-network-helper");
 process(k);assert(k->state==ST_READY&&k->done==2&&!strcmp(k->output[0],j->output[0])&&!strcmp(k->output[1],j->output[1]));free_job(k);free_job(j);for(int i=0;i<LYRICS_MAX_LINES;i++){free(pending[i]);pending[i]=NULL;}
 puts("PASS production 256 MiB cache: no 512-file cap, full-folder oldest eviction, protected current file, bounded metadata heap, cancellation, growing atomic commits and cold offline Groq cache without an API key/network.");return 0;}
'''
 driver=R/'qa/.v191-cache-driver.c';driver.write_text(source);bin_=w/'cache'
 cmd=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-DMBEDTLS_CONFIG_FILE=<mbedtls_config.h>','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*incs,str(driver),str(S/'include/parson/parson.c'),str(S/'include/mbedtls_lib/sha256.c'),str(S/'include/mbedtls_lib/platform_util.c'),str(Q/'utf8proc.o'),'-Wl,--gc-sections','-lpthread','-lm','-o',str(bin_)]
 try:
  p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr
  env={**os.environ,'QA_DATA':str(data),'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
  for files,size,expected in [(700,4096,700),(1200,512*1024,512)]:
   for p in cache.iterdir():p.unlink()
   for i in range(files):
    p=cache/f'{i:064x}.json'
    with p.open('wb') as f:f.truncate(size)
    os.utime(p,(1000000+i,1000000+i))
   (cache/'README.txt').write_text('KEEP_UNRELATED');(cache/('f'*64+'.json')).symlink_to(cache/'README.txt')
   p=subprocess.run([str(bin_),str(cache),str(expected)],env=env,capture_output=True,text=True,timeout=30);print(p.stdout+p.stderr);assert p.returncode==0,p.stdout+p.stderr
   assert (cache/'README.txt').read_text()=='KEEP_UNRELATED' and (cache/('f'*64+'.json')).is_symlink()
   if files==1200:
    assert not (cache/f'{688:064x}.json').exists() and (cache/f'{1199:064x}.json').exists()
 finally:driver.unlink(missing_ok=True)
(R/'logs/v191-cache-test.json').write_text(json.dumps({'exit':0,'budget_bytes':256*1024*1024,'more_than_512_retained':True,'multi_batch_oldest_eviction':True,'current_file_and_unrelated_files_preserved':True,'bounded_metadata_heap_max_32KiB':True,'generation_cancellation':True,'growing_commit_headroom_accounting':True,'cache_hash_format_compatible':True,'cold_offline_Groq_cache_without_key_or_network':True,'ASan_UBSan':True,'hardware_tested':False,'source_sha256':hashlib.sha256((S/'nxt_translate.c').read_bytes()).hexdigest()},indent=2)+'\n')
