/* audiosfxps2.c - sound effect playback (PS2 interface over the GameCube AX player).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\audiosfxps2.c; assembly: build/2049LE/asm/GameSource/src/audiosfxps2.s */
#include "rush/rush_protos.h"

/* PS2 SPU pitch (4096 = 44.1 kHz scaled by 3763/4096) for a sample rate */
#define PS2_PITCH(freq) ((int)((freq) << 12) / 44100 * 3763 / 4096)

unsigned int gEffectResourceID[311] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x10000, 0x10002, 0x10004, 0x10006, 0x10008, 0x10009, 0x1000a, 0x1000b, 0x1000c, 0x1000d,
    0x1000e, 0x1000f, 0x10010, 0x20000, 0x20001, 0x20002, 0x20003, 0x20004, 0x20005, 0x20006,
    0x20007, 0x20008, 0x20009, 0x2000a, 0x2000b, 0x2000c, 0x2000d, 0x2000e, 0x30000, 0x30001,
    0x30002, 0xffffffff, 0x30003, 0x40000, 0x40001, 0xffffffff, 0x40002, 0x40003, 0x40004, 0x40005,
    0x40006, 0x40007, 0x40008, 0x40009, 0x4000a, 0x4000b, 0x4000c, 0x4000d, 0x4000e, 0x4000f,
    0x40010, 0x40011, 0x40012, 0x40013, 0x40014, 0x40015, 0x40016, 0x40017, 0x40018, 0x40019,
    0x4001a, 0x4001b, 0x4001c, 0x4001d, 0x4001e, 0x4001f, 0x40020, 0x40021, 0x40022, 0x40023,
    0x40024, 0xffffffff, 0x40025, 0x50000, 0x50001, 0x50002, 0x50003, 0x50004, 0x50005, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x50006, 0x50007,
    0x50008, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0x50009, 0x5000a, 0x5000b, 0xffffffff, 0x5000c, 0x5000d,
    0x5000e, 0xffffffff, 0x50010, 0x50011, 0x50012, 0x50013, 0x50014, 0x50015, 0x50016, 0x50017,
    0x50018, 0x60000, 0x60001, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};
static _tLoadedBanks s_loaded = { 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void get_effect_volume(float vol, float pan, int *left, int *right);

/* 0x8009e200, size 0x4 */
void AudioLoadBank(void)
{

}

/* 0x8009e204, size 0x90 */
void _AudioLoadBank(anon_name_first_24 *bank, int lock)
{
    if (gNoAudio) {
        return;
    }
    if (bank->loaded) {
        return;
    }
    ngcLoadEffectBank(bank->ps2id, 0, 0);
    bank->loaded = 1;
    bank->locked = lock;
    s_loaded._loadedBanks[s_loaded._nextIdx] = bank;
    s_loaded._nextIdx++;
}

/* 0x8009e294, size 0x4 */
void AudioUnloadAllBanks(void)
{

}

/* 0x8009e298, size 0x84 */
static void get_effect_volume(float vol, float pan, int *left, int *right)
{
    vol *= gSfxMasterVol;
    if (gMono) {
        pan = 0.5f;
    }
    *left = 16383.0f * vol * ((1.0f - pan) < 0.1f ? 0.1f : (1.0f - pan));
    *right = 16383.0f * vol * (pan < 0.1f ? 0.1f : pan);
}

/* 0x8009e31c, size 0x250 */
int _AudioPlayEffect(anon_used_started_4c *theSound, float volume, float pan)
{
    int soundId;
    unsigned int resid; /* r30 */
    _PS2Bank *bank; /* stack */
    _SND_RESOURCE_HEADER *resource; /* r0 */
    _SND_RESOURCE_COMMON *common; /* r0 */
    _SNDPLAYMONOEFFECT pme; /* stack */

    pan = pan / 2.0 + 0.5;
    resid = gEffectResourceID[theSound->soundId];
    bank = 0;
    soundId = theSound->soundId;
    if (resid == 0xffffffff) {
        resid = gEffectLoop[soundId] ? 9 : 10;
    }
    resource = sndFindEffectResource(resid, &bank);
    common = &resource->resource.common_res;
    theSound->start_time = appGetTime();
    theSound->duration = (float)common->total_time / 1000.0f;
    theSound->needLoop = gEffectLoop[theSound->soundId];
    theSound->floated = 0.0f;
    theSound->ps2_pitch = PS2_PITCH(common->sample_freq);
    theSound->sound = 1;
    theSound->ps2_pitch = PS2_PITCH(common->sample_freq);
    theSound->ngcPitch = theSound->ps2_pitch;
    theSound->loop = gEffectLoop[theSound->soundId];
    theSound->ngcEffect = 0;
    if (theSound->sound != 0xffffffff) {
        pme.nCoreID = theSound->sound >= 24;
        pme.nVoiceID = theSound->sound < 24 ? theSound->sound : theSound->sound - 24;
        pme.nPitch = theSound->ps2_pitch;
        get_effect_volume(volume, pan, &pme.nVolumeL, &pme.nVolumeR);
        pme.nSpuAddr = bank->uSpuStartAddr + common->start_addr;
        pme.bReverb = 1;
        ngcPlaySfx(theSound, bank->pSpTable, resid & 0xffff, volume);
    }
    theSound->playingPitch = 1.0f;
    theSound->targetPitch = 1.0f;
    theSound->lrpan = pan;
    theSound->fbpan = pan;
    theSound->vol = volume;
    theSound->dirty = 0;
    theSound->started = 1;
    return 1;
}

/* Inline parameters preserve duration-first operand order in CodeWarrior. */
static inline float _SfxTimeAdd(float duration, float grace)
{
    return duration + grace;
}

/* 0x8009e56c, size 0xac */
int _AudioEffectPlaying(anon_used_started_4c *theSound)
{
    int ret; /* r3 */
    float cur_time; /* f0 */

    if (gEffectLoop[theSound->soundId]) {
        ret = 1;
    } else {
        cur_time = appGetTime();
        if (cur_time - theSound->start_time > _SfxTimeAdd(theSound->duration, 0.02f)) {
            if (theSound->sound != 0xffffffff) {
                ret = 0;
            } else {
                ret = 0;
            }
        } else {
            ret = 1;
        }
    }
    if (!ret && theSound->sound != 0xffffffff) {
        theSound->sound = 0xffffffff;
    }
    return ret;
}

/* 0x8009e618, size 0x58 */
int _AudioStopEffect(anon_used_started_4c *theSound)
{
    if (theSound->sound != 0xffffffff) {
        theSound->sound = 0xffffffff;
        ngcStopSfx(theSound, 0, 0);
    }
    _FreeSfxInstance(theSound);
    return 1;
}

/* 0x8009e670, size 0x14 */
int _AudioSetEffectVolume(anon_used_started_4c *theSound, float vol)
{
    theSound->vol = vol;
    theSound->dirty = 1;
    return 1;
}

/* 0x8009e684, size 0x14 */
int _AudioSetEffectLRPan(anon_used_started_4c *theSound, float pan)
{
    theSound->lrpan = pan;
    theSound->dirty = 1;
    return 1;
}

/* 0x8009e698, size 0x14 */
int _AudioSetEffectFBPan(anon_used_started_4c *theSound, float pan)
{
    theSound->fbpan = pan;
    theSound->dirty = 1;
    return 1;
}

/* 0x8009e6ac, size 0x14 */
int _AudioSetEffectPitch(anon_used_started_4c *theSound, float pitch)
{
    theSound->targetPitch = pitch;
    theSound->dirty = 1;
    return 1;
}

/* 0x8009e6c0, size 0x1e4 */
void _AudioUpdateEffect(anon_used_started_4c *theSound)
{
    float fPitchInc; /* f1 */
    _SNDUPDATEMONOEFFECT ume; /* stack */
    unsigned int resid; /* r31 */
    _PS2Bank *bank; /* stack */
    _SND_RESOURCE_HEADER *resource; /* r0 */

    if (theSound->targetPitch != theSound->playingPitch) {
        fPitchInc = theSound->targetPitch - theSound->playingPitch;
        if (fPitchInc > 0.1f) {
            fPitchInc = 0.1f;
        }
        if (fPitchInc < -0.1f) {
            fPitchInc = -0.1f;
        }
        theSound->playingPitch += fPitchInc;
        theSound->dirty = 1;
    }
    if (gForceUpdateSfx) {
        theSound->dirty = 1;
    }
    if (theSound->dirty) {
        if (theSound->sound != 0xffffffff) {
            resid = gEffectResourceID[theSound->soundId];
            bank = 0;
            if (resid == 0xffffffff) {
                resid = gEffectLoop[theSound->soundId] ? 9 : 10;
            }
            resource = sndFindEffectResource(resid, &bank);
            if (!resource) {
                resid = 0;
            }
            ume.nCoreID = theSound->sound >= 24;
            ume.nVoiceID = theSound->sound < 24 ? theSound->sound : theSound->sound - 24;
            ume.nPitch = theSound->ps2_pitch * theSound->playingPitch;
            theSound->ngcPitch = ((ume.nPitch << 12) / 3763 * 44100) >> 12;
            get_effect_volume(theSound->vol, theSound->lrpan, &ume.nVolumeL, &ume.nVolumeR);
            ngcLoopSfx(theSound, bank->pSpTable, resid & 0xffff, theSound->vol);
        }
        theSound->dirty = 0;
    }
}

/* 0x8009e8a4, size 0x8 */
int _AudioSetMono(int mono)
{
    gMono = mono;
}

