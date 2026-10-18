/* Audio command, listener, emitter and sequence management.
 * Recovered from the original GameCube ELF and DWARF.
 * All 58 complete bodies retain their verified original target bytes.
 * Song-name data restores the original object and literal layout.
 */
/* Generated DWARF declarations omit erased ABI parameters, varargs and const.
 * Correct them locally without changing the shared generated headers. */
#define AudPlaySequence AudPlaySequence_DWARF_decl
#define AudioInitSys AudioInitSys_DWARF_decl
#define AudEmitEffect AudEmitEffect_DWARF_decl
#define gSfxBanks gSfxBanks_DWARF_decl
#define gEffectLoop gEffectLoop_prototype
#include "rush/rush_protos.h"
#undef gEffectLoop
#undef gSfxBanks
#undef AudEmitEffect
#undef AudioInitSys
#undef AudPlaySequence
extern void AudioInitSys(int maxVoices, int maxSfx, int maxMusic);

/* Emit recovered static bodies even while their callers remain unrecovered. */
#pragma force_active on

static void update_reverb(void);
static void _PeriodicMaintenance(void);
static void _ProcessCommand(tAudioMsg *cmd);
static tAudioMsg *_GetCmd(void);
static void _FreeCmd(tAudioMsg *cmd);
static tFxSequence *_GetFxTag(void);
static void _FreeFxTag(tFxSequence *tag);
static void _PlayFxTag(tFxSequence *tag);
static void _InterruptFxTag(tFxSequence *tag);
static tFxSequence *_ConvertTag(int tag);
static void _AudChangeEffect(int tag, float volume, float lrpan, float fbpan, float pitch);
static void _InsertSorted(tFxSequence *tag);
static int _OrderFx(void * el1, void * el2);
static void _UpdateFx(void * element, int elNum);
static void _ProcessPriorities(void);
static void _AudSetMasterVol(float volume, float time, int music, int sfx);
static void _ActivateListener(anon_ll_free_3c *lis);
static void _FreeListener(anon_ll_free_3c *lis);
static void _ValidateListener(void);
static void _ActivateEmitter(tSfxEmitter *em);
static void _FreeEmitter(tSfxEmitter *em);
static void _ValidateEmitter();
static void _UpdateEmitter(tSfxEmitter *em, float *vol, float *lrpan, float *fbpan, float *pitch);
static void _ActivateSequence(anon_ll_id_3c *s);
static void _FreeSequence(anon_ll_id_3c *s);
static void _StopAllSequences(void);
static void _ProcessSequences(void);

const anon_name_first_24 gSfxBanks[9] = {
    {"common.kat", 0, 19, 0, 0, 0, 0, 0},
    {"battle.kat", 20, 32, 0, 0, 1, 0, 0},
    {"engines.kat", 33, 47, 0, 0, 2, 0, 0},
    {"front.kat", 48, 52, 0, 0, 3, 0, 0},
    {"gamecommon.kat", 53, 92, 0, 0, 4, 0, 0},
    {"race.kat", 93, 151, 0, 0, 5, 0, 0},
    {"track3.kat", 152, 153, 0, 0, 6, 0, 0},
    {"unused1.kat", 154, 243, 0, 0, 7, 0, 0},
    {"unused2.kat", 244, 310, 0, 0, 8, 0, 0},
};
char *gSongNames[20] = {
    "Bassy", "Flier", "Garage", "Hidden", "High", "Morning", "Night", "Noon",
    "Robo", "Speed", "Starsky", "Stunted", "Sunset", "TheRock", "Trancey", "Vice",
    "Warrior", "Wingey", "HighScore", "Select",
};

const char gEffectLoop[311] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
};

static int gSong = -1;
static unsigned int gNextFxId = 1;
static unsigned int gNextSeqId = 1;
static unsigned int gTagShift;
static unsigned int gTagMask;
static int gMaxSfx;
static int gMaxMusic;
static signed char gCalcDoppler;
int gNumFxInterrupted;
int gNumFxPending;
int gNumFxPlaying;
signed char gStarted;
static anon_ll_id_3c *gSequences;
static int gNumEmitters;
static int gNumListeners;
static int gNumFxTags;
static tSfxEmitter *gEmitters;
static anon_ll_free_3c *gListeners;
static int gNumStopCmds;
static int gNumStartCmds;
static anon_te_tag_c *gSortedFxTags;
static tFxSequence *gFxTags;
static signed char gInited;
static tDLList gActiveEmitters;
static tDLList gFreeEmitters;
static tDLList gActiveListeners;
static tDLList gFreeListeners;
static tAudioMsg *gStartCmds[128];
static tAudioMsg *gStopCmds[128];
static tDLList gActiveSequences;
static tDLList gFreeSequences;
static tOrderedList gSortedFxTagTree;
static tDLList gInterruptedFxTags;
static tDLList gPlayingFxTags;
static tDLList gFreeFxTags;
static tAudioMsg gCmds[128];

/* 0x800a1ab8, size 0x34c */
void AudioInit(unsigned char numVoices, unsigned char maxSfx, unsigned char maxMusic, int numListeners, int numEmitters, float footUnits, float frontDistance, int doppler)
{
    int i;
    if (gInited) {
        if (gMaxSfx == maxSfx && gMaxMusic == maxMusic) return;
        AudioShutdown();
    }
    gMaxSfx = maxSfx;
    gMaxMusic = maxMusic;
    for (i = 0; i < 128; i++) gCmds[i].hdr.used = 0;
    AudioInitSys(numVoices, maxSfx, maxMusic);
    i = gMaxSfx * 2 < 64 ? gMaxSfx * 2 : 64;
    gNumFxTags = i > gMaxSfx + numEmitters ? i : gMaxSfx + numEmitters;
    gFxTags = AllocateMemoryHeap(0, gNumFxTags * sizeof(*gFxTags));
    PurgeableMemory(gFxTags, 0);
    InitLL(&gFreeFxTags, 0, 1);
    InitLL(&gPlayingFxTags, 0, 1);
    InitLL(&gInterruptedFxTags, 0, 1);
    gSortedFxTags = AllocateMemoryHeap(0, gNumFxTags * sizeof(*gSortedFxTags));
    PurgeableMemory(gSortedFxTags, 0);
    InitBST(&gSortedFxTagTree, _OrderFx);
    gNumStopCmds = 0;
    gNumStartCmds = 0;
    for (i = 0; i < gNumFxTags; i++) {
        gFxTags[i].id = i;
        InsertLLE(&gFreeFxTags, &gFxTags[i], gFreeFxTags.head);
        gFxTags[i].list = &gFreeFxTags;
    }
    gTagShift = HAL_IntLog2(gNumFxTags);
    if (gNumFxTags > (1 << gTagShift)) gTagShift++;
    gTagMask = (1 << gTagShift) - 1;
    gSequences = AllocateMemoryHeap(0, 6 * sizeof(*gSequences));
    PurgeableMemory(gSequences, 0);
    InitLL(&gFreeSequences, 0, 1);
    InitLL(&gActiveSequences, 0, 1);
    for (i = 0; i < 6; i++) {
        gSequences[i].id = i;
        gSequences[i].free = 1;
        gSequences[i].active = 0;
        InsertLLE(&gFreeSequences, &gSequences[i], gFreeSequences.head);
    }
    AudInit3D(numListeners, numEmitters, footUnits, frontDistance, doppler);
    for (i = 0; i < 9; i++) _AudioLoadBank((anon_name_first_24 *)&gSfxBanks[i], 0);
    gInited = 1;
}


/* 0x800a1e04, size 0x58 */
void AudioShutdown(void)
{
    if (gNoAudio) return;
    if (gInited) {
    gInited = 0;
    AudioShutdownSys();
    DeallocateMemory(gFxTags);
    DeallocateMemory(gSortedFxTags);
    DeallocateMemory(gSequences);
    }
}


/* 0x800a1e5c, size 0x4 */
static void update_reverb(void)
{
    
}


/* 0x800a1e60, size 0x44 */
void AudioMain(void)
{
    statsUnclock(6);
    statsClock(4);
    _PeriodicMaintenance();
    update_reverb();
    statsUnclock(4);
    statsClock(6);
}


/* 0x800a1ea4, size 0x94 */
static void _PeriodicMaintenance(void)
{
    unsigned int timer;
    if (gInited) {
        timer = MyGetTime();
        _ProcessEmitters();
        _ProcessSequences();
        gAudioEmitTime += MyGetTime() - timer;
        timer = MyGetTime();
        _ProcessPriorities();
        gAudioPriTime += MyGetTime() - timer;
    }
    timer = MyGetTime();
    AudioPeriodicMaintenance();
    gAudioTime += MyGetTime() - timer;
}


/* 0x800a1f38, size 0x630 */
static void _ProcessCommand(tAudioMsg *cmd)
{
    unsigned int timer;
    tFxSequence *tag;
    tFxSequence *next;
    timer = MyGetTime();
    switch (cmd->hdr.cmd) {
    case 0:
        if (gSong != cmd->playSong.song && gSong < 0) {
            gSong = cmd->playSong.song;
            AudioSongPlay(gSong, cmd->playSong.loop);
        }
        break;
    case 1:
        if (gSong >= 0) {
            AudioSongStop(gSong);
            gSong = -1;
        }
        break;
    case 2:
        cmd->playEffect.tag->numCmds--;
        if (cmd->playEffect.tag->state == 0) {
            _FreeFxTag(cmd->playEffect.tag);
            break;
        }
        cmd->playEffect.tag->vol = cmd->playEffect.vol;
        cmd->playEffect.tag->lrpan = cmd->playEffect.lrpan;
        cmd->playEffect.tag->fbpan = 0.0f;
        cmd->playEffect.tag->pitch = 1.0f;
        if (cmd->playEffect.replace != 0) {
            for (tag = gPlayingFxTags.head; tag; tag = tag->ll.next) {
                if (tag != cmd->playEffect.tag && tag->state != 0
                    && tag->fxid == cmd->playEffect.tag->fxid
                    && tag->subId == cmd->playEffect.tag->subId
                    && tag->type == cmd->playEffect.tag->type) break;
            }
            if (!tag) {
                for (tag = gInterruptedFxTags.head; tag; tag = tag->ll.next) {
                    if (tag != cmd->playEffect.tag && tag->state == 1
                        && tag->fxid == cmd->playEffect.tag->fxid
                        && tag->subId == cmd->playEffect.tag->subId
                        && tag->type == cmd->playEffect.tag->type) break;
                }
            }
            if (tag) {
                if (tag->state == 2 || tag->state == 1) {
                    if (cmd->playEffect.replace == 2) {
                        _FreeFxTag(cmd->playEffect.tag);
                        if (tag->state == 1) tag->startTime = -1.0f;
                        break;
                    } else if (tag->state == 2) AudioStopEffect(tag->sysId);
                }
                _FreeFxTag(tag);
            }
        }
        if (cmd->playEffect.fbpan != -2.0f)
            cmd->playEffect.tag->fbpan = cmd->playEffect.fbpan;
        else cmd->playEffect.tag->fbpan = 0.0f;
        cmd->playEffect.tag->state = 3;
        break;
    case 3:
        cmd->startEffect.tag->numCmds--;
        if (cmd->startEffect.tag->state == 0) {
            _FreeFxTag(cmd->startEffect.tag);
            break;
        }
        cmd->startEffect.tag->sysId = AudioPlayEffect(cmd->startEffect.tag->fxid,
            cmd->startEffect.tag->vol, cmd->startEffect.tag->lrpan);
        if (cmd->startEffect.tag->sysId) {
            _PlayFxTag(cmd->startEffect.tag);
            if (cmd->startEffect.tag->fbpan)
                AudioSetEffectFBPan(cmd->startEffect.tag->sysId, cmd->startEffect.tag->fbpan);
            if (cmd->startEffect.tag->pitch != 1.0f)
                AudioSetEffectPitch(cmd->startEffect.tag->sysId, cmd->startEffect.tag->pitch);
            cmd->startEffect.tag->state = 2;
            cmd->startEffect.tag->startTime = GetAudioTime();
        }
        break;
    case 4:
        cmd->changeEffect.tag->numCmds--;
        if (cmd->changeEffect.tag->state == 0) {
            _FreeFxTag(cmd->changeEffect.tag);
            break;
        }
        if (cmd->changeEffect.vol != -2.0f) {
            if (cmd->changeEffect.tag->state == 2)
                AudioSetEffectVolume(cmd->changeEffect.tag->sysId, cmd->changeEffect.vol);
            cmd->changeEffect.tag->vol = cmd->changeEffect.vol;
        }
        if (cmd->changeEffect.lrpan != -2.0f) {
            if (cmd->changeEffect.tag->state == 2)
                AudioSetEffectLRPan(cmd->changeEffect.tag->sysId, cmd->changeEffect.lrpan);
            cmd->changeEffect.tag->lrpan = cmd->changeEffect.lrpan;
        }
        if (cmd->changeEffect.fbpan != -2.0f) {
            if (cmd->changeEffect.tag->state == 2)
                AudioSetEffectFBPan(cmd->changeEffect.tag->sysId, cmd->changeEffect.fbpan);
            cmd->changeEffect.tag->fbpan = cmd->changeEffect.fbpan;
        }
        if (cmd->changeEffect.pitch != -2.0f) {
            if (cmd->changeEffect.tag->state == 2 && !cmd->changeEffect.tag->paused)
                AudioSetEffectPitch(cmd->changeEffect.tag->sysId, cmd->changeEffect.pitch);
            cmd->changeEffect.tag->pitch = cmd->changeEffect.pitch;
        }
        break;
    case 5:
        cmd->intEffect.tag->numCmds--;
        if (cmd->intEffect.tag->state == 0) {
            _FreeFxTag(cmd->intEffect.tag);
            break;
        }
        AudioStopEffect(cmd->intEffect.tag->sysId);
        if (cmd->intEffect.tag->loop) _InterruptFxTag(cmd->intEffect.tag);
        else _FreeFxTag(cmd->intEffect.tag);
        break;
    case 6:
        cmd->stopEffect.tag->numCmds--;
        if (cmd->stopEffect.tag->state == 0) {
            _FreeFxTag(cmd->stopEffect.tag);
            break;
        }
        if (cmd->stopEffect.tag->state == 2) AudioStopEffect(cmd->stopEffect.tag->sysId);
        _FreeFxTag(cmd->stopEffect.tag);
        break;
    case 7:
        _StopAllSequences();
        AudioStopAllEffects();
        for (tag = gPlayingFxTags.head; tag; tag = next) {
            next = tag->ll.next;
            _FreeFxTag(tag);
        }
        for (tag = gInterruptedFxTags.head; tag; tag = next) {
            next = tag->ll.next;
            _FreeFxTag(tag);
        }
        break;
    case 8:
        for (tag = gPlayingFxTags.head; tag; tag = next) {
            next = tag->ll.next;
            if (tag->state == 2) AudioSetEffectPitch(tag->sysId, 0.0f);
            tag->paused = 1;
        }
        for (tag = gInterruptedFxTags.head; tag; tag = next) {
            next = tag->ll.next;
            tag->paused = 1;
        }
        break;
    case 9:
        for (tag = gPlayingFxTags.head; tag; tag = next) {
            next = tag->ll.next;
            if (tag->paused && tag->state == 2) {
                tag->paused = 0;
                AudioSetEffectPitch(tag->sysId, tag->pitch);
            }
        }
        for (tag = gInterruptedFxTags.head; tag; tag = next) {
            next = tag->ll.next;
            tag->paused = 0;
        }
        break;
    case 10:
        AudioSetMasterVolume(cmd->setMasterVol.vol, (unsigned int)(1000.0f * cmd->setMasterVol.time),
            cmd->setMasterVol.music, cmd->setMasterVol.sfx);
        break;
    case 11:
        AudioSetMono(cmd->mono.mono);
        break;
    }
    _FreeCmd(cmd);
    gAudioTime += MyGetTime() - timer;
}


/* 0x800a2568, size 0x58 */
static tAudioMsg *_GetCmd(void)
{
int i; for (i=0;i<128;i++) { if (!gCmds[i].hdr.used) { gCmds[i].hdr.used=1; *(signed short *)((char *)gCmds+i*sizeof(gCmds[0]))=-1; return &gCmds[i]; } } return 0;
}


/* 0x800a25c0, size 0xc */
static void _FreeCmd(tAudioMsg *cmd)
{
    cmd->hdr.used = 0;
}


/* 0x800a25cc, size 0xd8 */
static tFxSequence *_GetFxTag(void)
{
    tFxSequence *tag = gFreeFxTags.head;
    tag->id = (gNextFxId << gTagShift) | (tag->id & gTagMask);
    if (gNextFxId < (~gTagMask >> (gTagShift + 1))) gNextFxId = gNextFxId + 1;
    else gNextFxId = 1;
    tag->startTime = -1.0f;
    tag->data = 0;
    tag->vol = 0.0f;
    tag->lrpan = 0.0f;
    tag->fbpan = 0.0f;
    tag->pitch = 1.0f;
    RemoveLLE(&gFreeFxTags, tag);
    InsertLLE(&gInterruptedFxTags, tag, gInterruptedFxTags.head);
    tag->list = &gInterruptedFxTags;
    return tag;
}


/* 0x800a26a4, size 0x9c */
static void _FreeFxTag(tFxSequence *tag)
{
    tag->id &= gTagMask;
    tag->state = 0;
    if (tag->type == 1 && tag->data) {
        _FreeEmitter(tag->data);
        tag->data = 0;
    }
    if (!tag->numCmds) {
        RemoveLLE(tag->list, tag);
        InsertLLE(&gFreeFxTags, tag, gFreeFxTags.head);
        tag->list = &gFreeFxTags;
    }
}


/* 0x800a2740, size 0x54 */
static void _PlayFxTag(tFxSequence *tag)
{
    RemoveLLE(tag->list, tag);
    InsertLLE(&gPlayingFxTags, tag, gPlayingFxTags.head);
    tag->list = &gPlayingFxTags;
}


/* 0x800a2794, size 0x5c */
static void _InterruptFxTag(tFxSequence *tag)
{
    tag->state = 3;
    RemoveLLE(tag->list, tag);
    InsertLLE(&gInterruptedFxTags, tag, gInterruptedFxTags.head);
    tag->list = &gInterruptedFxTags;
}


/* 0x800a27f0, size 0x40 */
static tFxSequence *_ConvertTag(int tag)
{
    if (tag == -1) return 0;
    if (gFxTags[tag & gTagMask].id != tag) return 0;
    return &gFxTags[tag & gTagMask];
}


/* 0x800a2830, size 0x58 */
void AudPlaySong(int song, int loop)
{
    tAudioMsg *cmd;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 0;
    cmd->playSong.song = song;
    cmd->playSong.loop = loop;
    _ProcessCommand(cmd);
}


/* 0x800a2888, size 0x38 */
void AudKillSong(void)
{
    tAudioMsg *cmd;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 1;
    _ProcessCommand(cmd);
}


/* 0x800a28c0, size 0xdc */
int AudPlayEffect(int fxid, int subId, enum_kFxNoReplace_4 replace, unsigned char priority)
{
    tAudioMsg *cmd;
    int tag;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 2;
    cmd->playEffect.replace = replace;
    cmd->playEffect.tag = _GetFxTag();
    cmd->playEffect.tag->type = 0;
    cmd->playEffect.tag->fxid = fxid;
    cmd->playEffect.tag->subId = subId;
    cmd->playEffect.tag->priority = priority;
    cmd->playEffect.tag->state = 1;
    cmd->playEffect.tag->paused = 0;
    cmd->playEffect.tag->numCmds = 1;
    cmd->playEffect.tag->loop = gEffectLoop[fxid];
    cmd->playEffect.vol = 1.0f;
    cmd->playEffect.lrpan = 0.0f;
    cmd->playEffect.fbpan = -2.0f;
    tag = cmd->playEffect.tag->id;
    _ProcessCommand(cmd);
    return tag;
}


/* 0x800a299c, size 0x180 */
int AudPlayEffectVP(int fxid, int subId, enum_kFxNoReplace_4 replace, unsigned char priority, float volume, float lrpan, float fbpan)
{
    tAudioMsg *cmd;
    int tag;
    if (gNoAudio) return 1;
    cmd = _GetCmd();
    cmd->hdr.cmd = 2;
    cmd->playEffect.replace = replace;
    cmd->playEffect.tag = _GetFxTag();
    cmd->playEffect.tag->type = 0;
    cmd->playEffect.tag->fxid = fxid;
    cmd->playEffect.tag->subId = subId;
    cmd->playEffect.tag->priority = priority;
    cmd->playEffect.tag->state = 1;
    cmd->playEffect.tag->paused = 0;
    cmd->playEffect.tag->numCmds = 1;
    cmd->playEffect.tag->loop = gEffectLoop[fxid];
    cmd->playEffect.vol = volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume;
    cmd->playEffect.lrpan = lrpan < -1.0f ? -1.0f : lrpan > 1.0f ? 1.0f : lrpan;
    cmd->playEffect.fbpan = fbpan < -1.0f ? -1.0f : fbpan > 1.0f ? 1.0f : fbpan;
    tag = cmd->playEffect.tag->id;
    _ProcessCommand(cmd);
    return tag;
}


/* 0x800a2b1c, size 0x78 */
int AudEffectPlaying(int tag)
{
    int playing;
    tFxSequence *t;
    if (gNoAudio) return 1;
    t = _ConvertTag(tag);
    if (t) {
        if (t->state == 1 || t->state == 3) playing = 1;
        else if (t->state != 2) playing = 0;
        else playing = AudioEffectPlaying(t->sysId);
    } else playing = 0;
    return playing;
}


/* 0x800a2b94, size 0x200 */
static void _AudChangeEffect(int tag, float volume, float lrpan, float fbpan, float pitch)
{
    tAudioMsg *cmd = 0;
    tFxSequence *t = _ConvertTag(tag);
    if (t && ((-2.0f != volume && volume != t->vol) ||
              (-2.0f != lrpan && lrpan != t->lrpan) ||
              (-2.0f != fbpan && fbpan != t->fbpan) ||
              (-2.0f != pitch && pitch != t->pitch))) {
        cmd = _GetCmd();
        cmd->hdr.cmd = 4;
        cmd->changeEffect.tag = t;
        cmd->changeEffect.tag->numCmds++;
        cmd->changeEffect.vol = (-2.0f != volume && volume != t->vol) ? volume : -2.0f;
        cmd->changeEffect.lrpan = (-2.0f != lrpan && lrpan != t->lrpan) ? lrpan : -2.0f;
        cmd->changeEffect.fbpan = (-2.0f != fbpan && fbpan != t->fbpan) ? fbpan : -2.0f;
        cmd->changeEffect.pitch = (-2.0f != pitch && pitch != t->pitch) ? pitch : -2.0f;
    }
    if (cmd) _ProcessCommand(cmd);
}


/* 0x800a2d94, size 0x5c */
void AudChangeEffectVol(int tag, float volume)
{
    if (gNoAudio) return;
    if (volume < 0.0f) volume = 0.0f;
    else if (volume > 1.0f) volume = 1.0f;
    _AudChangeEffect(tag, volume, -2.0f, -2.0f, -2.0f);
}


/* 0x800a2df0, size 0x3c */
void AudChangeEffectPitch(int tag, float pitch)
{
    if (gNoAudio) return;
    _AudChangeEffect(tag, -2.0f, -2.0f, -2.0f, pitch);
}


/* 0x800a2e2c, size 0x7c */
void AudStopEffect(int tag)
{
    tAudioMsg *cmd = 0;
    tFxSequence *t;
    if (gNoAudio) return;
    t = _ConvertTag(tag);
    if (t) {
        cmd = _GetCmd();
        cmd->hdr.cmd = 6;
        cmd->stopEffect.tag = t;
        cmd->stopEffect.tag->numCmds++;
    }
    if (cmd) _ProcessCommand(cmd);
}


/* 0x800a2ea8, size 0x38 */
void AudStopAllEffects(void)
{
    tAudioMsg *cmd;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 7;
    _ProcessCommand(cmd);
}


/* 0x800a2ee0, size 0x38 */
void AudPauseAllEffects(void)
{
    tAudioMsg *cmd;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 7;
    _ProcessCommand(cmd);
}


/* 0x800a2f18, size 0x38 */
void AudResumeAllEffects(void)
{
    tAudioMsg *cmd;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 9;
    _ProcessCommand(cmd);
}


/* 0x800a2f50, size 0xd8 */
int _WeightedPriority(tFxSequence *tag)
{
    int priority;
    priority = (int)tag->priority + 80.0f * (1.0f - tag->vol);
    if (tag->loop) priority += 8;
    else if (tag->priority && tag->startTime >= 0.0f && tag->state == 2)
        priority += AudioTimeoutPerc(tag->startTime, 0.25f);
    return priority;
}


/* 0x800a3028, size 0x40 */
static void _InsertSorted(tFxSequence *tag)
{
    anon_te_tag_c *stag;
    stag = &gSortedFxTags[gSortedFxTagTree.numElements];
    stag->tag = tag;
    InsertBSTE(&gSortedFxTagTree, stag);
}


/* 0x800a3068, size 0x18 */
static int _OrderFx(void * el1, void * el2)
{
    return ((anon_te_tag_c *)el1)->tag->wpriority - ((anon_te_tag_c *)el2)->tag->wpriority;
}


/* 0x800a3080, size 0x188 */
static void _UpdateFx(void * element, int elNum)
{

    anon_te_tag_c *stag = element;
    tAudioMsg *cmd;
    tFxSequence *tag = stag->tag;
    if (tag->state != 1) {
        if (elNum >= 0.75f * gMaxSfx ||
            (tag->loop && tag->state == 2 && tag->vol <= 0.0f &&
             AudioTimeout(tag->startTime, 0.2f))) {
            if (stag->tag->state == 2) {
                cmd = _GetCmd();
                gStopCmds[gNumStopCmds++] = cmd;
                cmd->hdr.cmd = 5;
                cmd->intEffect.tag = stag->tag;
                cmd->intEffect.tag->numCmds++;
            }
        } else if (stag->tag->state != 2 &&
                   (stag->tag->vol > 0.1f || (!stag->tag->loop && stag->tag->type == 1))) {
            cmd = _GetCmd();
            gStartCmds[gNumStartCmds++] = cmd;
            cmd->hdr.cmd = 3;
            cmd->startEffect.tag = stag->tag;
            cmd->startEffect.tag->numCmds++;
        }
    }
}


/* 0x800a3208, size 0x1c4 */
static void _ProcessPriorities(void)
{
    tFxSequence *tag, *next;
    int i;
    ClearBST(&gSortedFxTagTree);
    gNumStopCmds = 0;
    gNumStartCmds = 0;
    for (tag = gPlayingFxTags.head; tag; tag = next) {
        next = tag->ll.next;
        if (!AudioEffectPlaying(tag->sysId)) {
            if (tag->loop) _InterruptFxTag(tag);
            else _FreeFxTag(tag);
        } else {
            tag->wpriority = _WeightedPriority(tag);
            _InsertSorted(tag);
        }
    }
    gNumFxPlaying = gPlayingFxTags.numElements;
    gNumFxInterrupted = 0;
    gNumFxPending = 0;
    for (tag = gInterruptedFxTags.head; tag; tag = next) {
        next = tag->ll.next;
        if (!tag->loop) {
            tag->startTime -= 1.0f;
            if (tag->startTime < -10.0f) {
                _FreeFxTag(tag);
                continue;
            }
        }
        if (tag->state == 1) gNumFxPending++;
        if (tag->state != 1) gNumFxInterrupted++;
        tag->wpriority = _WeightedPriority(tag);
        _InsertSorted(tag);
    }
    WalkBST(&gSortedFxTagTree, _UpdateFx);
    for (i = 0; i < gNumStopCmds; i++) _ProcessCommand(gStopCmds[i]);
    for (i = 0; i < gNumStartCmds; i++) _ProcessCommand(gStartCmds[i]);
}


/* 0x800a33cc, size 0xb4 */
static void _AudSetMasterVol(float volume, float time, int music, int sfx)
{
    tAudioMsg *cmd = _GetCmd();
    cmd->hdr.cmd = 10;
    cmd->setMasterVol.vol = volume < 0.0f ? 0.0f : volume > 1.0f ? 1.0f : volume;
    cmd->setMasterVol.time = time < 0.0f ? 0.0f : time;
    cmd->setMasterVol.music = music;
    cmd->setMasterVol.sfx = sfx;
    _ProcessCommand(cmd);
}


/* 0x800a3480, size 0x34 */
void AudSetFxVol(float volume, float time)
{
    if (gNoAudio) return;
    _AudSetMasterVol(volume, time, 0, 1);
}


/* 0x800a34b4, size 0x34 */
void AudSetMusicVol(float volume, float time)
{
    if (gNoAudio) return;
    _AudSetMasterVol(volume, time, 1, 0);
}


/* 0x800a34e8, size 0x50 */
void AudMono(int mono)
{
    tAudioMsg *cmd;
    if (gNoAudio) return;
    cmd = _GetCmd();
    cmd->hdr.cmd = 11;
    cmd->mono.mono = mono;
    _ProcessCommand(cmd);
    ngcSetAxMode(mono);
}


/* 0x800a3538, size 0x44 */
anon_ll_free_3c *_NewListener(void)
{
    anon_ll_free_3c *lis = gFreeListeners.head;
    RemoveLLE(&gFreeListeners, lis);
    lis->free = 0;
    return lis;
}


/* 0x800a357c, size 0x44 */
static void _ActivateListener(anon_ll_free_3c *lis)
{
    InsertLLE(&gActiveListeners, lis, gActiveListeners.head);
    lis->active = 1;
}


/* 0x800a35c0, size 0x68 */
static void _FreeListener(anon_ll_free_3c *lis)
{
    if (lis->active) {
        RemoveLLE(&gActiveListeners, lis);
        lis->active = 0;
    }
    InsertLLE(&gFreeListeners, lis, gFreeListeners.head);
    lis->free = 1;
}


/* 0x800a3628, size 0x4 */
static void _ValidateListener(void)
{
    
}


/* 0x800a362c, size 0x54 */
tSfxEmitter *_NewEmitter(void)
{
    tSfxEmitter *em = gFreeEmitters.head;
    if (!em) return 0;
    RemoveLLE(&gFreeEmitters, em);
    em->free = 0;
    return em;
}


/* 0x800a3680, size 0x44 */
static void _ActivateEmitter(tSfxEmitter *em)
{
    InsertLLE(&gActiveEmitters, em, gActiveEmitters.head);
    em->active = 1;
}


/* 0x800a36c4, size 0x68 */
static void _FreeEmitter(tSfxEmitter *em)
{
    if (em->active) {
        RemoveLLE(&gActiveEmitters, em);
        em->active = 0;
    }
    InsertLLE(&gFreeEmitters, em, gFreeEmitters.head);
    em->free = 1;
}


/* 0x800a372c, size 0x4 */
static void _ValidateEmitter()
{
    
}


/* 0x800a3730, size 0x1bc */
void AudInit3D(int numListeners, int numEmitters, float footUnits, float frontDistance, int doppler)
{
    int i;
    if (gNoAudio) return;
    InitLL(&gFreeListeners, 0, 1);
    InitLL(&gActiveListeners, 0, 1);
    InitLL(&gFreeEmitters, 0, 1);
    InitLL(&gActiveEmitters, 0, 1);
    if (numListeners <= 0 || numEmitters <= 0) return;
    gNumListeners = numListeners;
    gListeners = AllocateMemoryHeap(0, gNumListeners * sizeof(*gListeners));
    PurgeableMemory(gListeners, 0);
    for (i = 0; i < gNumListeners; i++) {
        gListeners[i].free = 1;
        gListeners[i].active = 0;
        InsertLLE(&gFreeListeners, &gListeners[i], gFreeListeners.head);
    }
    gNumEmitters = numEmitters;
    gEmitters = AllocateMemoryHeap(0, gNumEmitters * sizeof(*gEmitters));
    PurgeableMemory(gEmitters, 0);
    for (i = 0; i < gNumEmitters; i++) {
        gEmitters[i].free = 1;
        gEmitters[i].active = 0;
        InsertLLE(&gFreeEmitters, &gEmitters[i], gFreeEmitters.head);
    }
    gFrontDistance = frontDistance;
    gSoundSpeed = 1100.0f * footUnits;
    gCalcDoppler = doppler;
}


/* 0x800a38ec, size 0xcc */
int AudAddListener(float *pos, float *dir, float *look, float *up)
{
    anon_ll_free_3c *lis;
    if (gNoAudio) return 1;
    if (!gListeners) return -1;
    lis = _NewListener();
    lis->pos[0] = pos[0]; lis->pos[1] = pos[1]; lis->pos[2] = pos[2];
    lis->dir[0] = dir[0]; lis->dir[1] = dir[1]; lis->dir[2] = dir[2];
    lis->look[0] = look[0]; lis->look[1] = look[1]; lis->look[2] = look[2];
    lis->up[0] = up[0]; lis->up[1] = up[1]; lis->up[2] = up[2];
    _ActivateListener(lis);
    return (int)lis;
}


/* 0x800a39b8, size 0x48 */
void AudRemoveListener(int lis)
{
    if (gNoAudio) return;
    if (lis == -1) return;
    _ValidateListener();
    _FreeListener((anon_ll_free_3c *)lis);
}


/* 0x800a3a00, size 0xb0 */
void AudUpdateListener(int lis, float *pos, float *dir, float *look, float *up)
{
    anon_ll_free_3c *l;
    if (gNoAudio) return;
    if (lis == -1) return;
    _ValidateListener();
    l = (anon_ll_free_3c *)lis;
    l->pos[0] = pos[0]; l->pos[1] = pos[1]; l->pos[2] = pos[2];
    l->dir[0] = dir[0]; l->dir[1] = dir[1]; l->dir[2] = dir[2];
    l->look[0] = look[0]; l->look[1] = look[1]; l->look[2] = look[2];
    l->up[0] = up[0]; l->up[1] = up[1]; l->up[2] = up[2];
}


/* 0x800a3ab0, size 0x1e0 */
int AudEmitEffect(float *pos, float *dir, float fadeDistance, float fadeCompression, float maxVol, float minVol, int fxid, int subId, enum_kFxNoReplace_4 replace, unsigned char priority)
{
    tSfxEmitter *em;
    tAudioMsg *cmd;
    int tag;
    if (gNoAudio) return 1;
    if (!gEmitters) return -1;
    em = _NewEmitter();
    if (!em) return -1;
    em->pos[0] = pos[0]; em->pos[1] = pos[1]; em->pos[2] = pos[2];
    em->fadeDistance = fadeDistance;
    em->maxVol = maxVol < 0.0f ? 0.0f : maxVol > 1.0f ? 1.0f : maxVol;
    em->minVol = minVol < 0.0f ? 0.0f : minVol > 1.0f ? 1.0f : minVol;
    em->basePitch = 1.0f;
    cmd = _GetCmd();
    cmd->hdr.cmd = 2;
    cmd->playEffect.replace = replace;
    cmd->playEffect.tag = _GetFxTag();
    cmd->playEffect.tag->type = 1;
    cmd->playEffect.tag->fxid = fxid;
    cmd->playEffect.tag->subId = subId;
    cmd->playEffect.tag->priority = priority;
    cmd->playEffect.tag->state = 1;
    cmd->playEffect.tag->paused = 0;
    cmd->playEffect.tag->numCmds = 1;
    cmd->playEffect.tag->loop = gEffectLoop[fxid];
    cmd->playEffect.vol = 0.0f;
    cmd->playEffect.lrpan = 0.0f;
    cmd->playEffect.fbpan = -2.0f;
    cmd->playEffect.tag->data = em;
    tag = em->sndTag = cmd->playEffect.tag->id;
    _ProcessCommand(cmd);
    if (!em->free) _ActivateEmitter(em);
    return tag;
}


/* 0x800a3c90, size 0x50 */
void AudStopEmitter(int tag)
{
    tFxSequence *t;
    if (gNoAudio) return;
    t = _ConvertTag(tag);
    if (t) {
        _ValidateEmitter(t->data);
        AudStopEffect(tag);
    }
}


/* 0x800a3ce0, size 0x60 */
int AudEmitterPlaying(int tag)
{
    tFxSequence *t;
    if (gNoAudio) return 1;
    t = _ConvertTag(tag);
    if (!t) return 0;
    _ValidateEmitter(t->data);
    return AudEffectPlaying(tag);
}


/* 0x800a3d40, size 0xc0 */
void AudUpdateEmitter(int tag, float *pos, float maxVol, float pitch)
{
    tFxSequence *t;
    tSfxEmitter *em;
    if (gNoAudio) return;
    t = _ConvertTag(tag);
    if (t) {
        em = t->data;
        _ValidateEmitter(em);
        em->pos[0] = pos[0]; em->pos[1] = pos[1]; em->pos[2] = pos[2];
        em->maxVol = maxVol < 0.0f ? 0.0f : maxVol > 1.0f ? 1.0f : maxVol;
        em->basePitch = pitch;
    }
}


/* 0x800a3e00, size 0xdc */
void _ProcessEmitters(void)
{
    tSfxEmitter *em, *next;
    tFxSequence *tag;
    float vol, lrpan, fbpan, pitch;
    for (em = gActiveEmitters.head; em; em = next) {
        next = em->ll.next;
        tag = _ConvertTag(em->sndTag);
        if (tag->state == 2 && !tag->numCmds && !tag->loop && !AudioEffectPlaying(tag->sysId))
            _FreeFxTag(tag);
        else if (!tag->paused) {
            _UpdateEmitter(em, &vol, &lrpan, &fbpan, &pitch);
            _AudChangeEffect(em->sndTag, vol, lrpan, fbpan, pitch);
        }
    }
}


/* 0x800a3edc, size 0x304 */
static void _UpdateEmitter(tSfxEmitter *em, float *vol, float *lrpan, float *fbpan, float *pitch)
{
    anon_ll_free_3c *lis;
    float dir[3];
    float dist;
    float lrpans[5];
    float fbpans[5];
    float vols[5];
    float lrp, fbp;
    float totalVol = 0.0f;
    int l;
    *pitch = em->basePitch;
    for (lis = gActiveListeners.head, l = 0; lis; lis = lis->ll.next, l++) {
        dir[0] = em->pos[0] - lis->pos[0];
        dir[1] = em->pos[1] - lis->pos[1];
        dir[2] = em->pos[2] - lis->pos[2];
        dist = TB_Vector3Normalize(dir);
        {
            float lookX = lis->look[0];
            float lookY = lis->look[1];
            float lookZ = lis->look[2];
            float upX = lis->up[0];
            float upY = lis->up[1];
            float fadeDistance = em->fadeDistance;
            float upZ = lis->up[2];
            {
                float rightX, rightY, rightZ;
                /* Listener right axis is up cross look. */
                rightX = upY * lookZ - upZ * lookY;
                rightY = upZ * lookX - upX * lookZ;
                rightZ = upX * lookY - upY * lookX;
                lrpans[l] = rightX * dir[0] + rightY * dir[1] + rightZ * dir[2];
                fbpans[l] = lookX * dir[0] + lookY * dir[1] + lookZ * dir[2];
            }
            if (fadeDistance > 0.0f) {
                vols[l] = em->maxVol - (em->maxVol - em->minVol) * (dist / fadeDistance);
                if (vols[l] < 0.0f) vols[l] = 0.0f;
            } else vols[l] = em->maxVol;
        }
        totalVol += vols[l];
        lrpans[l] -= lrpans[l] * vols[l];
    }
    *vol = totalVol < 0.0f ? 0.0f : totalVol > 1.0f ? 1.0f : totalVol;
    lrp = 0.0f;
    fbp = 0.0f;
    for (l = 0; l < gActiveListeners.numElements; l++) {
        if (totalVol) {
            lrp += lrpans[l] * (vols[l] / totalVol);
            fbp += fbpans[l] * (vols[l] / totalVol);
        }
    }
    *lrpan = lrp < -1.0f ? -1.0f : lrp > 1.0f ? 1.0f : lrp;
    *fbpan = fbp < -1.0f ? -1.0f : fbp > 1.0f ? 1.0f : fbp;
}


/* 0x800a41e0, size 0x90 */
anon_ll_id_3c *_NewSequence(void)
{
    anon_ll_id_3c *s = gFreeSequences.head;
    RemoveLLE(&gFreeSequences, s);
    s->free = 0;
    s->id = (gNextSeqId << gTagShift) | (s->id & gTagMask);
    if (gNextSeqId < (~gTagMask >> (gTagShift + 1))) gNextSeqId = gNextSeqId + 1;
    else gNextSeqId = 1;
    return s;
}


/* 0x800a4270, size 0x44 */
static void _ActivateSequence(anon_ll_id_3c *s)
{
    InsertLLE(&gActiveSequences, s, gActiveSequences.head);
    s->active = 1;
}


/* 0x800a42b4, size 0x78 */
static void _FreeSequence(anon_ll_id_3c *s)
{
    s->id &= gTagMask;
    if (s->active) {
        RemoveLLE(&gActiveSequences, s);
        s->active = 0;
    }
    InsertLLE(&gFreeSequences, s, gFreeSequences.head);
    s->free = 1;
}


/* 0x800a432c, size 0x184 */
int AudPlaySequence(int priority, float timeout, enum_kFxNoReplace_4 replace, int subId, int num, ...)
{
    anon_ll_id_3c *s;
    va_list args;
    int i;
    if (gNoAudio) return 1;
    s = _NewSequence();
    s->priority = priority;
    s->startTime = GetAudioTime();
    s->timeout = timeout;
    s->replace = replace;
    s->subId = subId;
    s->stop = 0;
    s->next = 0;
    va_start(args, num);
    for (i = 0; i < num; i++) s->ids[i] = va_arg_int(args, int);
    va_end(args);
    for (; i < 4; i++) s->ids[i] = -1;
    s->sndTag = -1;
    _ActivateSequence(s);
    return s->id;
}


/* 0x800a44b0, size 0x28 */
static void _StopAllSequences(void)
{
    anon_ll_id_3c *s;
    for (s = gActiveSequences.head; s; s = s->ll.next) s->stop = 1;
}


/* 0x800a44d8, size 0x154 */
static void _ProcessSequences(void)
{
    anon_ll_id_3c *s, *next;
    tFxSequence *tag;
    for (s = gActiveSequences.head; s; s = next) {
        next = s->ll.next;
        if (s->stop || (s->sndTag == -1 && AudioTimeout(s->startTime, s->timeout))) {
            AudStopEffect(s->sndTag);
            _FreeSequence(s);
        } else {
            tag = _ConvertTag(s->sndTag);
            if (tag) {
                if (tag->state == 2 && !tag->numCmds && !AudioEffectPlaying(tag->sysId))
                    _FreeFxTag(tag);
                else continue;
            }
            while (s->ids[s->next] == -1) {
                s->next++;
                if (s->next >= 4) break;
            }
            if (s->next >= 4) _FreeSequence(s);
            else {
                s->sndTag = AudPlayEffect(s->ids[s->next++], s->subId, s->replace, s->priority);
                _ConvertTag(s->sndTag)->data = s;
            }
        }
    }
}


