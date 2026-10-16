/* audiosfx.c - sound effect command queue: effect instances, play/stop/change commands, per-frame server.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\audiosfx.c; assembly: build/2049LE/asm/GameSource/src/audiosfx.s */
#include "rush/rush_protos.h"

static int _GetSfxInstance(anon_used_started_4c **theSound);
static anon_dlle_cmd_20 *_NewCmd(void);
static anon_dlle_cmd_20 *_FindChangeCmd(anon_used_started_4c *sound);
static void _SendCmd(anon_dlle_cmd_20 *cmd);
static void _FreeCmd(anon_dlle_cmd_20 *cmd);

static unsigned int gNextIndex = 1;
float gFrontDistance;
float gSoundSpeed;
static anon_used_started_4c *gSfxInstances;
static int gNumChannels;
static int gStoppedChannels[1];
static int gNumInstances;
static anon_dlle_cmd_20 *gCmds;
static signed char gStopAllEffects;
static signed char gInited;
static anon_handle_dbl_10 gCmdQ;
static anon_handle_dbl_10 gFreeQ;

/* 0x8009d630, size 0xd8 */
static int _GetSfxInstance(anon_used_started_4c **theSound)
{
    unsigned int i; /* r9 */

    *theSound = 0;
    for (i = 0; i < gNumInstances; i++) {
        if (!gSfxInstances[i].used) {
            gSfxInstances[i].used = 1;
            gSfxInstances[i].started = 0;
            gSfxInstances[i].stop = 0;
            gSfxInstances[i].stopped = 0;
            gSfxInstances[i].stopAll = 0;
            gSfxInstances[i].fail = 0;
            gSfxInstances[i].index = (gNextIndex << 8) | i;
            gNextIndex = gNextIndex < 0xffffff ? gNextIndex + 1 : 1;
            *theSound = &gSfxInstances[i];
            return 1;
        }
    }
    return 0;
}

/* 0x8009d708, size 0x10 */
void _FreeSfxInstance(anon_used_started_4c *theSound)
{
    theSound->used = 0;
    theSound->index = 0;
}

/* 0x8009d718, size 0x44 */
static anon_dlle_cmd_20 *_NewCmd(void)
{
    anon_dlle_cmd_20 *cmd; /* r31 */

    cmd = gFreeQ.tail;
    cmd->failCounter = 0;
    RemoveLLE(&gFreeQ, cmd);
    return cmd;
}

/* 0x8009d75c, size 0x44 */
static anon_dlle_cmd_20 *_FindChangeCmd(anon_used_started_4c *sound)
{
    anon_dlle_cmd_20 *c; /* r4 */

    for (c = gCmdQ.tail; c; c = c->dlle.prev) {
        if (c->sound == sound && c->cmd == 3) {
            return c;
        }
    }
    return 0;
}

/* 0x8009d7a0, size 0xc0 */
static void _SendCmd(anon_dlle_cmd_20 *cmd)
{
    anon_dlle_cmd_20 *c; /* r3 */
    anon_dlle_cmd_20 *next; /* r31 */

    if (cmd->cmd != 4 && cmd->cmd == 2 && !cmd->sound->started) {
        for (c = gCmdQ.tail; c; c = next) {
            next = c->dlle.prev;
            if (c->sound == cmd->sound) {
                _FreeCmd(c);
            }
        }
        _FreeSfxInstance(cmd->sound);
        InsertLLE(&gFreeQ, cmd, gFreeQ.head);
    } else {
        InsertLLE(&gCmdQ, cmd, gCmdQ.head);
    }
}

/* 0x8009d860, size 0x4c */
static void _FreeCmd(anon_dlle_cmd_20 *cmd)
{
    RemoveLLE(&gCmdQ, cmd);
    InsertLLE(&gFreeQ, cmd, gFreeQ.head);
}

/* 0x8009d8ac, size 0x100 */
void AudioSfxInit(int maxSfx)
{
    int i; /* r29 */
    int numCmds; /* r28 */

    InitLL(&gFreeQ, 0, 1);
    InitLL(&gCmdQ, 0, 1);
    gNumInstances = maxSfx;
    gSfxInstances = AllocateMemoryHeap(0, maxSfx * sizeof(anon_used_started_4c));
    PurgeableMemory(gSfxInstances, 0);
    memset(gSfxInstances, 0, gNumInstances * sizeof(anon_used_started_4c));
    numCmds = maxSfx * 3;
    gCmds = AllocateMemoryHeap(0, numCmds * sizeof(anon_dlle_cmd_20));
    PurgeableMemory(gCmds, 0);
    memset(gCmds, 0, numCmds * sizeof(anon_dlle_cmd_20));
    for (i = 0; i < numCmds; i++) {
        InsertLLE(&gFreeQ, &gCmds[i], gFreeQ.head);
    }
    gNumChannels = maxSfx;
    gStoppedChannels[0] = 0;
    gStopAllEffects = 0;
    gInited = 1;
}

/* Keep the generic list accessor's pointer conversion in the inline body.
   Together with the explicit list cast below, this reproduces the original
   separate queue-address temporaries instead of reusing loop 2's r31. */
static inline void *_CmdQueueTail(void *queue)
{
    return ((anon_handle_dbl_10 *)queue)->tail;
}

/* 0x8009d9ac, size 0x388 */
void AudioSfxServer(void)
{
    anon_dlle_cmd_20 *cmd; /* r3 */
    anon_dlle_cmd_20 *next; /* r28 */
    int i; /* r5 */

    if (!gInited) {
        return;
    }
    gNumChannels += gStoppedChannels[0];
    for (i = sizeof(gStoppedChannels) / sizeof(gStoppedChannels[0]) - 1; i > 0; i--) {
        gStoppedChannels[i] = gStoppedChannels[i - 1];
    }
    gStoppedChannels[0] = 0;
    if (gStopAllEffects) {
        for (i = 0; i < gNumInstances; i++) {
            if (gSfxInstances[i].used && gSfxInstances[i].stopAll && !gSfxInstances[i].stop) {
                AudioStopEffect(gSfxInstances[i].index);
            }
        }
        if (i == gNumInstances) {
            gStopAllEffects = 0;
        }
    }
    for (i = 0; i < gNumInstances; i++) {
        if (gSfxInstances[i].used && gSfxInstances[i].started) {
            if (!_AudioEffectPlaying(&gSfxInstances[i])) {
                for (cmd = gCmdQ.tail; cmd; cmd = next) {
                    next = cmd->dlle.prev;
                    if (cmd->sound == &gSfxInstances[i]) {
                        _FreeCmd(cmd);
                    }
                }
                gStoppedChannels[0]++;
                _FreeSfxInstance(&gSfxInstances[i]);
            } else {
                _AudioUpdateEffect(&gSfxInstances[i]);
            }
        }
    }
    for (cmd = ((anon_handle_dbl_10 *)&gCmdQ)->tail; cmd; cmd = next) {
        next = cmd->dlle.prev;
        if (cmd->sound && cmd->sound->stop && cmd->cmd != 2) {
            _FreeCmd(cmd);
            continue;
        }
        if (cmd->sound && cmd->sound->fail) {
            continue;
        }
        switch (cmd->cmd) {
        case 0:
            if (gNumChannels <= 0 || !_AudioPlayEffect(cmd->sound, cmd->vol, cmd->lrpan)) {
                goto fail;
            }
            gNumChannels--;
            break;
        case 2:
            if (!_AudioStopEffect(cmd->sound)) {
                goto fail;
            }
            gStoppedChannels[0]++;
            break;
        case 3:
            if (cmd->vol != -2.0f && !_AudioSetEffectVolume(cmd->sound, cmd->vol)) {
                goto fail;
            }
            if (cmd->lrpan != -2.0f && !_AudioSetEffectLRPan(cmd->sound, cmd->lrpan)) {
                goto fail;
            }
            if (cmd->fbpan != -2.0f && !_AudioSetEffectFBPan(cmd->sound, cmd->fbpan)) {
                goto fail;
            }
            if (cmd->pitch != -2.0f && !_AudioSetEffectPitch(cmd->sound, cmd->pitch)) {
                goto fail;
            }
            break;
        case 4:
            if (!_AudioSetMono(cmd->vol)) {
                goto fail;
            }
            break;
        }
        _FreeCmd(cmd);
        continue;
    fail:
        cmd->failCounter++;
        if (cmd->sound) {
            cmd->sound->fail = 1;
        }
        if (cmd->failCounter >= 20) {
            _FreeCmd(cmd);
        }
    }
    for (cmd = _CmdQueueTail(&gCmdQ); cmd; cmd = cmd->dlle.prev) {
        if (cmd->sound) {
            cmd->sound->fail = 0;
        }
    }
}

/* 0x8009dd34, size 0xa0 */
unsigned int AudioPlayEffect(int soundId, float volume, float pan)
{
    anon_used_started_4c *theSound; /* stack */
    anon_dlle_cmd_20 *cmd; /* r0 */

    theSound = 0;
    if (soundId == -1) {
        return 0;
    }
    if (!_GetSfxInstance(&theSound)) {
        return 0;
    }
    cmd = _NewCmd();
    cmd->cmd = 0;
    cmd->sound = theSound;
    theSound->soundId = soundId;
    cmd->vol = volume;
    cmd->lrpan = pan;
    _SendCmd(cmd);
    return theSound->index;
}

/* 0x8009ddd4, size 0x64 */
int AudioEffectPlaying(unsigned int theSound)
{
    if (theSound && gSfxInstances[theSound & 0xff].index == theSound) {
        if (gSfxInstances[theSound & 0xff].started) {
            return _AudioEffectPlaying(&gSfxInstances[theSound & 0xff]);
        }
        return 1;
    }
    return 0;
}

/* 0x8009de38, size 0x80 */
void AudioStopEffect(unsigned int theSound)
{
    anon_dlle_cmd_20 *cmd; /* r0 */

    if (!theSound || gSfxInstances[theSound & 0xff].index != theSound) {
        return;
    }
    cmd = _NewCmd();
    cmd->cmd = 2;
    cmd->sound = &gSfxInstances[theSound & 0xff];
    cmd->sound->stop = 1;
    cmd->sound->index = 0;
    _SendCmd(cmd);
}

/* 0x8009deb8, size 0x48 */
void AudioStopAllEffects(void)
{
    int i; /* r6 */

    for (i = 0; i < gNumInstances; i++) {
        if (gSfxInstances[i].used) {
            gSfxInstances[i].stopAll = 1;
        }
    }
    gStopAllEffects = 1;
}

/* 0x8009df00, size 0xa8 */
void AudioSetEffectVolume(unsigned int theSound, float vol)
{
    anon_dlle_cmd_20 *cmd; /* r0 */

    if (!theSound || gSfxInstances[theSound & 0xff].index != theSound) {
        return;
    }
    cmd = _FindChangeCmd(&gSfxInstances[theSound & 0xff]);
    if (!cmd) {
        cmd = _NewCmd();
        cmd->cmd = 3;
        cmd->sound = &gSfxInstances[theSound & 0xff];
        cmd->lrpan = -2.0f;
        cmd->fbpan = -2.0f;
        cmd->pitch = -2.0f;
        cmd->vol = vol;
        _SendCmd(cmd);
    } else {
        cmd->vol = vol;
    }
}

/* 0x8009dfa8, size 0xa8 */
void AudioSetEffectLRPan(unsigned int theSound, float pan)
{
    anon_dlle_cmd_20 *cmd; /* r0 */

    if (!theSound || gSfxInstances[theSound & 0xff].index != theSound) {
        return;
    }
    cmd = _FindChangeCmd(&gSfxInstances[theSound & 0xff]);
    if (!cmd) {
        cmd = _NewCmd();
        cmd->cmd = 3;
        cmd->sound = &gSfxInstances[theSound & 0xff];
        cmd->vol = -2.0f;
        cmd->fbpan = -2.0f;
        cmd->pitch = -2.0f;
        cmd->lrpan = pan;
        _SendCmd(cmd);
    } else {
        cmd->lrpan = pan;
    }
}

/* 0x8009e050, size 0xa8 */
void AudioSetEffectFBPan(unsigned int theSound, float pan)
{
    anon_dlle_cmd_20 *cmd; /* r0 */

    if (!theSound || gSfxInstances[theSound & 0xff].index != theSound) {
        return;
    }
    cmd = _FindChangeCmd(&gSfxInstances[theSound & 0xff]);
    if (!cmd) {
        cmd = _NewCmd();
        cmd->cmd = 3;
        cmd->sound = &gSfxInstances[theSound & 0xff];
        cmd->vol = -2.0f;
        cmd->lrpan = -2.0f;
        cmd->pitch = -2.0f;
        cmd->fbpan = pan;
        _SendCmd(cmd);
    } else {
        cmd->fbpan = pan;
    }
}

/* 0x8009e0f8, size 0xa8 */
void AudioSetEffectPitch(unsigned int theSound, float pitch)
{
    anon_dlle_cmd_20 *cmd; /* r0 */

    if (!theSound || gSfxInstances[theSound & 0xff].index != theSound) {
        return;
    }
    cmd = _FindChangeCmd(&gSfxInstances[theSound & 0xff]);
    if (!cmd) {
        cmd = _NewCmd();
        cmd->cmd = 3;
        cmd->sound = &gSfxInstances[theSound & 0xff];
        cmd->vol = -2.0f;
        cmd->lrpan = -2.0f;
        cmd->fbpan = -2.0f;
        cmd->pitch = pitch;
        _SendCmd(cmd);
    } else {
        cmd->pitch = pitch;
    }
}

/* 0x8009e1a0, size 0x60 */
void AudioSetMono(int mono)
{
    anon_dlle_cmd_20 *cmd; /* r0 */

    cmd = _NewCmd();
    cmd->cmd = 4;
    cmd->vol = mono;
    cmd->sound = 0;
    _SendCmd(cmd);
}

