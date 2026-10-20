#!/usr/bin/env python3
"""Exercise the actual native LED writer against the production NXT owner."""
from pathlib import Path
import json, os, subprocess, tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'upstream/src'
render = ROOT / '.cache/render-sp'
api = (ROOT / 'upstream/NextUI/workspace/all/common/api.c').read_text()
start = api.index('void LEDS_updateLeds(bool indicator_only)')
opening = api.index('{', start)
depth = 1
end = opening + 1
while depth:
    depth += (api[end] == '{') - (api[end] == '}')
    end += 1
function = api[start:end]
fixture = '''#include "nxt.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
NxtConfig nxt;
typedef struct {int unused;} LightSettings;
typedef LightSettings Array[5];
static Array settings;static Array *lights=&settings;
static int lights_initialized=1,calls;
#define LOG_error(...) ((void)0)
static int exactMatch(const char *a,const char *b){return b&&!strcmp(a,b);}
static void PLAT_setLedInbrightness(LightSettings *s){calls++;}
static void PLAT_setLedBrightness(LightSettings *s){calls++;}
static void PLAT_setLedEffectCycles(LightSettings *s){calls++;}
static void PLAT_setLedEffectSpeed(LightSettings *s){calls++;}
static void PLAT_setLedColor(LightSettings *s){calls++;}
static void PLAT_setLedEffect(LightSettings *s){calls++;}
''' + function + '''
int main(void){
 setenv("DEVICE","brickpro",1);nxt.led=1;
 assert(NxtHardware_controlsLEDs());LEDS_updateLeds(false);LEDS_updateLeds(true);assert(calls==0);
 nxt.led=0;assert(!NxtHardware_controlsLEDs());LEDS_updateLeds(false);assert(calls==25);
 calls=0;LEDS_updateLeds(true);assert(calls==25);
 nxt.led=1;LEDS_updateLeds(false);assert(calls==25);
 nxt.led=0;lights_initialized=0;calls=0;LEDS_updateLeds(false);assert(!calls);
 lights_initialized=1;lights=NULL;LEDS_updateLeds(false);assert(!calls);
 return 0;
}
'''
includes = (render / 'includes.txt').read_text().splitlines() + ['-I'+str(p) for p in [SRC, SRC/'include', ROOT/'upstream/NextUI/workspace/all/common', ROOT/'upstream/NextUI/workspace/tg5040/platform', ROOT/'upstream/NextUI/workspace/tg5040/libmsettings']]
with tempfile.TemporaryDirectory(prefix='nxt-led-owner-') as temp:
    folder=Path(temp);source=folder/'fixture.c';binary=folder/'test';source.write_text(fixture)
    command=['gcc','-O1','-g','-no-pie','-D_GNU_SOURCE','-DUSE_SDL2','-DNXT_ROM_STOCK','-DPLATFORM_DESKTOP','-ffunction-sections','-fdata-sections','-fsanitize=address,undefined',*includes,str(source),str(SRC/'nxt_hardware.c'),'-Wl,--gc-sections','-lm','-o',str(binary)]
    with (ROOT/'logs/v151-led-owner-build.log').open('w') as log:subprocess.run(command,stdout=log,stderr=log,check=True)
    result=subprocess.run([str(binary)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=10);result.check_returncode()
    # The same fixture must catch the original writer overwriting app colors.
    original=function.replace('    if (NxtHardware_controlsLEDs()) return;', '')
    source.write_text(fixture.replace(function,original));subprocess.run(command,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
    rejected=subprocess.run([str(binary)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0:halt_on_error=1'},timeout=10)
    assert rejected.returncode != 0
report={'native_LED_ownership':{'exit':0,'production_common_writer':True,'production_NXT_owner':True,'native_writes_when_enabled':0,'native_writes_when_disabled':25,'charging_and_ambient_guard':True,'regression_without_guard_fails':True,'asan_ubsan':True,'physical_hardware_tested':False}}
(ROOT/'logs/v151-led-owner-test.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS actual LEDS_updateLeds and NxtHardware_controlsLEDs: prevent native RGB/effect overwrite while NXT owns LEDs; restore native writer when disabled; original regression fails.')
