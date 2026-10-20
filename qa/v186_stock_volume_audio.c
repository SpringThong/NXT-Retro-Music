/* Actual Stock callback: a hardware DAC fader must not be attenuated twice. */
#include "nxt_player_core.c"
#include <assert.h>
NxtConfig nxt;
static bool qa_hardware_volume;
bool NxtVolume_hardware(void){return qa_hardware_volume;}float NxtVolume_gain(void){return 1.f;}
bool Radio_isActive(void){return false;}int Radio_getAudioSamples(int16_t*p,int n){return 0;}
bool NxtSpotify_isPlaying(void){return false;}bool NxtSoundCloud_isPlaying(void){return false;}
int NxtSpotify_audioSamples(float*p,int n,int r){return 0;}int NxtSoundCloud_audioSamples(float*p,int n,int r){return 0;}
void NxtDSP_process(float*p,int n,float gain){}
Uint32 SDL_GetTicks(void){return 0;}
int GetUSBDACSoftwareVolume(void){return 50;}
static void fill(void){e.sink=2;e.volume=1;e.format=AUDIO_F32SYS;e.rate=48000;e.state=PLAYER_STATE_PLAYING;e.read=e.write=0;e.queued=1;e.ring[0].count=32;e.ring[0].used=0;for(int i=0;i<64;i++)e.ring[0].pcm[i]=.25f;}
int main(void){float pcm[64];qa_hardware_volume=false;fill();output_audio(NULL,(Uint8*)pcm,sizeof(pcm));for(int i=0;i<64;i++)assert(fabsf(pcm[i]-.125f)<.00001f);qa_hardware_volume=true;fill();output_audio(NULL,(Uint8*)pcm,sizeof(pcm));for(int i=0;i<64;i++)assert(fabsf(pcm[i]-.25f)<.00001f);puts("PASS production Stock callback: existing software-only volume still applies; hardware volume ownership avoids a second software attenuation when switching raw to online Normal. PCM transport is a fixture.");}
