/* audiosysps2.c - audio system timing and master-volume fades. */
#include "rush/rush_protos.h"

static float gMusicMasterFadeStartVol = 1.0f;
static float gMusicMasterFadeEndVol = 1.0f;
static float gSfxMasterFadeStartVol = 1.0f;
static float gSfxMasterFadeEndVol = 1.0f;
float g_SfxVol = 1.0f;
float g_MusicVol = 1.0f;
float gMusicMasterVol;
float gSfxMasterVol;
unsigned char gForceUpdateMusic;
unsigned char gForceUpdateSfx;
static float s_audioLastSystemSeconds;
static float gSfxMasterFadeInterval;
static float gSfxMasterFadeStartTime;
static float gMusicMasterFadeInterval;
static float gMusicMasterFadeStartTime;
signed char gMono;
static float gAudioSeconds;

/* 0x8009ed90 (the first parameter is unnamed in the original: DWARF records only maxSfx, in r4) */
void AudioInitSys(int maxVoices, int maxSfx)
{
    AudioSfxInit(maxSfx);
    AudioSongInit();
    gForceUpdateMusic = 0;
    gAudioSeconds = 0.0f;
    gForceUpdateSfx = 0;
    gMusicMasterVol = 0.0f;
    gSfxMasterVol = 0.0f;
    s_audioLastSystemSeconds = appGetTime();
}

/* 0x8009eddc */
void AudioShutdownSys(void)
{
}

/* 0x8009ede0 */
float GetAudioTime(void)
{
    return gAudioSeconds;
}

/* 0x8009ede8 */
int AudioTimeout(float startTime, float interval)
{
    return gAudioSeconds > startTime + interval;
}

/* 0x8009ee00 */
float AudioTimeoutPerc(float startTime, float interval)
{
    return (gAudioSeconds - startTime) / interval;
}

/* 0x8009ee10 */
void AudioSetMasterVolume(float vol, unsigned int time, int music, int sfx)
{
    if (music) {
        gMusicMasterFadeStartTime = gAudioSeconds;
        gMusicMasterFadeInterval = (float)time / 1000.0f;
        gMusicMasterFadeStartVol = gMusicMasterVol;
        gMusicMasterFadeEndVol = vol * (gMono ? 0.6f : 1.0f);
        g_MusicVol = vol;
    }
    if (sfx) {
        gSfxMasterFadeStartTime = gAudioSeconds;
        gSfxMasterFadeInterval = (float)time / 1000.0f;
        gSfxMasterFadeStartVol = gSfxMasterVol;
        gSfxMasterFadeEndVol = vol;
        g_SfxVol = vol;
    }
}

/* 0x8009eec0 */
void AudioPeriodicMaintenance(void)
{
    float cur_sys_seconds = appGetTime();

    gAudioSeconds += cur_sys_seconds - s_audioLastSystemSeconds;
    s_audioLastSystemSeconds = cur_sys_seconds;
    gForceUpdateSfx = 0;
    gForceUpdateMusic = 0;
    if (gMusicMasterVol != gMusicMasterFadeEndVol) {
        if (AudioTimeout(gMusicMasterFadeStartTime, gMusicMasterFadeInterval))
            gMusicMasterVol = gMusicMasterFadeEndVol;
        else
            gMusicMasterVol = gMusicMasterFadeStartVol + (gMusicMasterFadeEndVol - gMusicMasterFadeStartVol)
                * AudioTimeoutPerc(gMusicMasterFadeStartTime, gMusicMasterFadeInterval);
        gForceUpdateMusic = 1;
    }
    if (gSfxMasterVol != gSfxMasterFadeEndVol) {
        if (AudioTimeout(gSfxMasterFadeStartTime, gSfxMasterFadeInterval))
            gSfxMasterVol = gSfxMasterFadeEndVol;
        else
            gSfxMasterVol = gSfxMasterFadeStartVol + (gSfxMasterFadeEndVol - gSfxMasterFadeStartVol)
                * AudioTimeoutPerc(gSfxMasterFadeStartTime, gSfxMasterFadeInterval);
        gForceUpdateSfx = 1;
    }
    AudioSfxServer();
    AudioStreamServer();
    sndTick();
}
