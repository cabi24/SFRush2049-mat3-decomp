/* save.c - player and ghost persistence. Status: Matching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\save.c; assembly: build/2049LE/asm/GameSource/src/save.s */
#define SaveGhost SaveGhost_DWARF_decl
#define PS_CreateFile PS_CreateFile_DWARF_decl
#define PS_WriteFile PS_WriteFile_DWARF_decl
#include "rush/rush_protos.h"
#undef SaveGhost
#undef PS_CreateFile
#undef PS_WriteFile
/* Palette/pixels are supplied in the GC ABI but absent from callee DWARF. */
extern tPS_File **PS_CreateFile(int pak, void *name, void *ext, unsigned int numBytes, char *gameName, char *bootStr, char *vmuStr, signed short *palette, char *pixels);
/* The GC callers pass an address and length omitted by the callee DWARF. */
extern void PS_WriteFile(tPS_File **fileh, void *data, unsigned int size);

static void _RemovePlayerFile(tPS_File **fileh);
static void _MovePlayerFile(tPS_File **from, tPS_File **to);
static signed char _GetRaceSetting(anon_ll_fileh_4c **player, unsigned char option, int track);
static signed char _GetBattleSetting(anon_ll_fileh_4c **player, unsigned char option, int track);
static signed char _GetStuntSetting(anon_ll_fileh_4c **player, unsigned char option, int track);
static signed char _GetObstacleSetting(anon_ll_fileh_4c **player, unsigned char option, int track);
static void _SaveRaceSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, unsigned char track);
static void _SaveBattleSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, int track);
static void _SaveStuntSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, int track);
static void _SaveObstacleSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, int track);
static unsigned int _CalcChecksum(void * start, unsigned int size);
static void _SetDefaultCarSettings(anon_ll_fileh_4c **p, int car);
static void _InitPlayer(anon_ll_fileh_4c **player);
static anon_ll_fileh_4c **_NewPlayer(void);
static void _FreePlayer(anon_ll_fileh_4c **player);
static int _AlreadyReadPlayer(tPS_File **fileh);
static anon_ll_fileh_4c **_ReadPlayer(tPS_File **fileh);
static int _CheckAndRepairPlayer(anon_ll_fileh_4c **player, int repair);
static int _AlreadyReadGhost(tPS_File **fileh);
static anon_ll_fileh_30 **_ReadGhostHeader(tPS_File **fileh);
static void _DecodeGhost(anon_ll_fileh_30 **ghosth);
static int _CheckAndRepairGhost(anon_ll_fileh_30 **ghosth, int data, int repair);
static int _GetUniqueExtension(int pak, char *name, char *extPrefix, char *ext);
static int _Pow(int num, int exp);
static int _VerifyPlayer(anon_ll_fileh_4c **playerh, int pak);
static int _VerifyGhost(anon_ll_fileh_30 **ghosth, int pak);
static int _VerifyFile(tPS_File **fileh, int pak);

const char *gPlayerNameChars = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:<>!@$^*(){}-_=|'" };
signed char gSaveGhostFiles = 1;
anon_ll_fileh_30 **gLimboGhosts;
static anon_ll_fileh_30 *gRealLimboGhosts;
static unsigned char gNumLimbo;
static signed char gGlobalsSet;
static signed char gInited;
anon_ll_fileh_30 **gGhosts[4];
static float gCameraMatrix[4][4];
anon_handle_dbl_10 gGhostFiles;
anon_handle_dbl_10 gPlayerFiles;
static anon_nameChecksum_id_814 gRealJustPlayerData[4];
static anon_nameChecksum_id_814 *gJustPlayerData[4];
anon_ll_fileh_4c *gJustPlayers[4];
static anon_ll_fileh_4c gRealJustPlayers[4];

/* inline narrowing to unsigned char (no DWARF entry): its return temporary reproduces the original code for the track argument of the race-setting calls */
static inline unsigned char track_arg(int t) { return t; }

/* 0x80037a18, size 0x1c0 */
void SaveInit(void)
{
    int i;
    for (i = 0; i < 4; i++) {
        gJustPlayers[i] = &gRealJustPlayers[i];
        gRealJustPlayers[i].datah = &gJustPlayerData[i];
        gJustPlayerData[i] = &gRealJustPlayerData[i];
        _InitPlayer(&gJustPlayers[i]);
        sprintf((char *)gJustPlayers[i]->name, "PLAYER %d", i + 1);
        strcpy((char *)(*gJustPlayers[i]->datah)->name, (char *)gJustPlayers[i]->name);
    }
    InitLL(&gPlayerFiles, 1, 1);
    InitLL(&gGhostFiles, 1, 1);
    gNumLimbo = 3;
    gRealLimboGhosts = AllocateMemoryHeap(0, 36 * sizeof(*gRealLimboGhosts));
    PurgeableMemory(gRealLimboGhosts, 0);
    memset(gRealLimboGhosts, 0, 36 * sizeof(*gRealLimboGhosts));
    gLimboGhosts = AllocateMemoryHeap(0, 36 * sizeof(*gLimboGhosts));
    PurgeableMemory(gLimboGhosts, 0);
    for (i = 0; i < 36; i++) gLimboGhosts[i] = &gRealLimboGhosts[i];
    gGlobalsSet = 1;
    gInited = 1;
}

/* 0x80037bd8, size 0x1d4 */
void SaveServer(int forceUpdate, int rumbleOnly)
{
    int pak;
    tPS_File **fileh;
    tPS_File **next;
    static signed char sInitCountdown = 3;
    static signed char sReadingPak[4] = {0, 0, 0, 0};
    static signed char sPakChanged[4] = {0, 0, 0, 0};
    PS_PakServer(forceUpdate, rumbleOnly);
    if (sInitCountdown > 1) {
        sInitCountdown--;
        return;
    }
    for (pak = 0; pak < 4; pak++) {
        sPakChanged[pak] = 1;
        if (sReadingPak[pak]) continue;
        sReadingPak[pak] = 1;
        for (fileh = gPS_Files[pak].head; fileh; fileh = next) {
            if (!PS_PakInserted(pak) || !_VerifyFile(fileh, pak)) break;
            next = (*fileh)->ll.next;
            if (!strncmp("RUSH2049", (*fileh)->name, 8) &&
                !strncmp("P", (*fileh)->ext, 1) && !_AlreadyReadPlayer(fileh)) {
                _ReadPlayer(fileh);
            }
        }
        if (!fileh) {
            for (fileh = gPS_Files[pak].head; fileh; fileh = next) {
                if (!PS_PakInserted(pak) || !_VerifyFile(fileh, pak)) break;
                next = (*fileh)->ll.next;
                if (!strncmp("RUSH2049", (*fileh)->name, 8) &&
                    strncmp("P", (*fileh)->ext, 1) && !_AlreadyReadGhost(fileh)) {
                    _ReadGhostHeader(fileh);
                }
            }
        }
        sReadingPak[pak] = 0;
        if (!fileh) sPakChanged[pak] = 0;
    }
    sInitCountdown = 0;
}

/* 0x80037dac, size 0x90 */
void SaveFlush(int ignoreErrors)
{
    int pak;
    tPS_File **fileh;
    for (pak = 0; pak < 4; pak++) {
        for (fileh = gPS_Files[pak].head; fileh; fileh = (*fileh)->ll.next) {
            if ((*fileh)->data) PS_FlushFile(fileh, ignoreErrors);
        }
    }
}

/* 0x80037e3c, size 0xc */
void SaveGlobals(void)
{
    gGlobalsSet = 1;
}

/* 0x80037e48, size 0xc4 */
void SaveSelectPlayer(anon_ll_fileh_4c **player)
{
    int i;
    if (!gGlobalsSet) {
        gGlobalsSet = 1;
        for (i = 0; i < 21; i++) gOptions[i] = GetPlayerOption(player, i);
        VideoSetOffset(gOptions[19], gOptions[20]);
    } else {
        for (i = 0; i < 21; i++) SavePlayerOption(player, i, gOptions[i]);
    }
}

/* 0x80037f0c, size 0xf0 */
static void _RemovePlayerFile(tPS_File **fileh)
{
    anon_ll_fileh_4c **player;
    anon_ll_fileh_30 **ghost;
    int i;
    for (player = gPlayerFiles.head; player; player = (*player)->ll.next) {
        if ((*player)->fileh == fileh) {
            for (i = 0; i < gNumPlayers; i++) {
                if (gPlayers[i].save == player) gPlayers[i].save = 0;
            }
            RemovePlayerScores(player);
            RemoveLLE(&gPlayerFiles, player);
            _FreePlayer(player);
            return;
        }
    }
    for (ghost = gGhostFiles.head; ghost; ghost = (*ghost)->ll.next) {
        if ((*ghost)->fileh == fileh) {
            RemoveLLE(&gGhostFiles, ghost);
            _FreeGhost(ghost);
            return;
        }
    }
}

/* 0x80037ffc, size 0x6c */
static void _MovePlayerFile(tPS_File **from, tPS_File **to)
{
    anon_ll_fileh_4c **player;
    anon_ll_fileh_30 **ghost;
    for (player = gPlayerFiles.head; player; player = (*player)->ll.next) {
        if ((*player)->fileh == from) {
            (*player)->fileh = to;
            return;
        }
    }
    for (ghost = gGhostFiles.head; ghost; ghost = (*ghost)->ll.next) {
        if ((*ghost)->fileh == from) {
            (*ghost)->fileh = to;
            return;
        }
    }
}

/* 0x80038068, size 0x14 */
unsigned char GetPlayerRaceType(anon_ll_fileh_4c **player)
{
    return (*(*player)->datah)->raceType;
}

/* 0x8003807c, size 0x54 */
void SavePlayerRaceType(anon_ll_fileh_4c **player, int raceType)
{
    if ((*(*player)->datah)->raceType != raceType) {
        (*(*player)->datah)->raceType = raceType;
        PS_WriteFile((*player)->fileh, &(*(*player)->datah)->raceType, 1);
    }
}

/* 0x800380d0, size 0x50 */
unsigned char GetPlayerCar(anon_ll_fileh_4c **player)
{
    switch ((*(*player)->datah)->raceType) {
    case 6: return (*(*player)->datah)->battleSettings.car;
    case 4: return (*(*player)->datah)->stuntSettings.car;
    case 5: return (*(*player)->datah)->obstacleSettings.car;
    default: return (*(*player)->datah)->raceSettings.car;
    }
}

/* 0x80038120, size 0x158 */
void SavePlayerCar(anon_ll_fileh_4c **player, unsigned char car)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (data->raceType) {
    case 0:
    case 1:
    case 2:
    case 3:
        if (data->raceSettings.car != car) {
            data->raceSettings.car = car;
            data->raceSettings.checksum = _CalcChecksum(&data->raceSettings.track, sizeof(data->raceSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->raceSettings, sizeof(data->raceSettings));
        }
        break;
    case 6:
        if (data->battleSettings.car != car) {
            data->battleSettings.car = car;
            data->battleSettings.checksum = _CalcChecksum(&data->battleSettings.track, sizeof(data->battleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->battleSettings, sizeof(data->battleSettings));
        }
        break;
    case 4:
        if (data->stuntSettings.car != car) {
            data->stuntSettings.car = car;
            data->stuntSettings.checksum = _CalcChecksum(&data->stuntSettings.track, sizeof(data->stuntSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->stuntSettings, sizeof(data->stuntSettings));
        }
        break;
    case 5:
        if (data->obstacleSettings.car != car) {
            data->obstacleSettings.car = car;
            data->obstacleSettings.checksum = _CalcChecksum(&data->obstacleSettings.track, sizeof(data->obstacleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->obstacleSettings, sizeof(data->obstacleSettings));
        }
        break;
    }
}

/* 0x80038278, size 0x50 */
unsigned char GetPlayerCarView(anon_ll_fileh_4c **player)
{
    switch ((*(*player)->datah)->raceType) {
    case 6: return (*(*player)->datah)->battleSettings.carView;
    case 4: return (*(*player)->datah)->stuntSettings.carView;
    case 5: return (*(*player)->datah)->obstacleSettings.carView;
    default: return (*(*player)->datah)->raceSettings.carView;
    }
}

/* 0x800382c8, size 0x158 */
void SavePlayerCarView(anon_ll_fileh_4c **player, unsigned char view)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (data->raceType) {
    case 0:
    case 1:
    case 2:
    case 3:
        if (data->raceSettings.carView != view) {
            data->raceSettings.carView = view;
            data->raceSettings.checksum = _CalcChecksum(&data->raceSettings.track, sizeof(data->raceSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->raceSettings, sizeof(data->raceSettings));
        }
        break;
    case 6:
        if (data->battleSettings.carView != view) {
            data->battleSettings.carView = view;
            data->battleSettings.checksum = _CalcChecksum(&data->battleSettings.track, sizeof(data->battleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->battleSettings, sizeof(data->battleSettings));
        }
        break;
    case 4:
        if (data->stuntSettings.carView != view) {
            data->stuntSettings.carView = view;
            data->stuntSettings.checksum = _CalcChecksum(&data->stuntSettings.track, sizeof(data->stuntSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->stuntSettings, sizeof(data->stuntSettings));
        }
        break;
    case 5:
        if (data->obstacleSettings.carView != view) {
            data->obstacleSettings.carView = view;
            data->obstacleSettings.checksum = _CalcChecksum(&data->obstacleSettings.track, sizeof(data->obstacleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->obstacleSettings, sizeof(data->obstacleSettings));
        }
        break;
    }
}

/* 0x80038420, size 0x50 */
unsigned char GetPlayerCarTrans(anon_ll_fileh_4c **player)
{
    switch ((*(*player)->datah)->raceType) {
    case 6: return (*(*player)->datah)->battleSettings.trans;
    case 4: return (*(*player)->datah)->stuntSettings.trans;
    case 5: return (*(*player)->datah)->obstacleSettings.trans;
    default: return (*(*player)->datah)->raceSettings.trans;
    }
}

/* 0x80038470, size 0x158 */
void SavePlayerCarTrans(anon_ll_fileh_4c **player, unsigned char trans)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (data->raceType) {
    case 0:
    case 1:
    case 2:
    case 3:
        if (data->raceSettings.trans != trans) {
            data->raceSettings.trans = trans;
            data->raceSettings.checksum = _CalcChecksum(&data->raceSettings.track, sizeof(data->raceSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->raceSettings, sizeof(data->raceSettings));
        }
        break;
    case 6:
        if (data->battleSettings.trans != trans) {
            data->battleSettings.trans = trans;
            data->battleSettings.checksum = _CalcChecksum(&data->battleSettings.track, sizeof(data->battleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->battleSettings, sizeof(data->battleSettings));
        }
        break;
    case 4:
        if (data->stuntSettings.trans != trans) {
            data->stuntSettings.trans = trans;
            data->stuntSettings.checksum = _CalcChecksum(&data->stuntSettings.track, sizeof(data->stuntSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->stuntSettings, sizeof(data->stuntSettings));
        }
        break;
    case 5:
        if (data->obstacleSettings.trans != trans) {
            data->obstacleSettings.trans = trans;
            data->obstacleSettings.checksum = _CalcChecksum(&data->obstacleSettings.track, sizeof(data->obstacleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->obstacleSettings, sizeof(data->obstacleSettings));
        }
        break;
    }
}

/* 0x800385c8, size 0x70 */
unsigned char GetPlayerTrack(anon_ll_fileh_4c **player, int raceType)
{
    switch (raceType) {
    case 6: return (*(*player)->datah)->battleSettings.track;
    case 4: return (*(*player)->datah)->stuntSettings.track;
    case 5: return (*(*player)->datah)->obstacleSettings.track;
    default: return (*(*player)->datah)->raceSettings.track;
    }
}

/* 0x80038638, size 0x178 */
void SavePlayerTrack(anon_ll_fileh_4c **player, int raceType, unsigned char track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (raceType) {
    case 0:
    case 1:
    case 2:
        if (data->raceSettings.track != track) {
            data->raceSettings.track = track;
            data->raceSettings.checksum = _CalcChecksum(&data->raceSettings.track, sizeof(data->raceSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->raceSettings, sizeof(data->raceSettings));
        }
        break;
    case 6:
        if (data->battleSettings.track != track) {
            data->battleSettings.track = track;
            data->battleSettings.checksum = _CalcChecksum(&data->battleSettings.track, sizeof(data->battleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->battleSettings, sizeof(data->battleSettings));
        }
        break;
    case 4:
        if (data->stuntSettings.track != track) {
            data->stuntSettings.track = track;
            data->stuntSettings.checksum = _CalcChecksum(&data->stuntSettings.track, sizeof(data->stuntSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->stuntSettings, sizeof(data->stuntSettings));
        }
        break;
    case 5:
        if (data->obstacleSettings.track != track) {
            data->obstacleSettings.track = track;
            data->obstacleSettings.checksum = _CalcChecksum(&data->obstacleSettings.track, sizeof(data->obstacleSettings) - 4);
            PS_WriteFile((*player)->fileh, &data->obstacleSettings, sizeof(data->obstacleSettings));
        }
        break;
    default: return;
    }
    SavePlayerRaceType(player, raceType);
}

/* 0x800387b0, size 0xd8 */
signed char GetPlayerOption(anon_ll_fileh_4c **player, unsigned char option)
{
    if (option < 21) return (*(*player)->datah)->options[option];
    if (gTrackNum >= 0 && gTrackNum < 6) return _GetRaceSetting(player, option, track_arg(gTrackNum));
    if (gTrackNum >= 19 && gTrackNum < 25) return _GetRaceSetting(player, option, track_arg(gTrackNum));
    if (gTrackNum >= 6 && gTrackNum < 14) return _GetBattleSetting(player, option, (unsigned char)(gTrackNum - 6));
    if (gTrackNum >= 14 && gTrackNum < 18) return _GetStuntSetting(player, option, (unsigned char)(gTrackNum - 14));
    return _GetObstacleSetting(player, option, (unsigned char)(gTrackNum - 18));
}

/* 0x80038888, size 0x8c */
static signed char _GetRaceSetting(anon_ll_fileh_4c **player, unsigned char option, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (option) {
    case 21: return data->raceSettings.laps;
    case 22: return data->raceSettings.drones;
    case 23: return data->raceSettings.gravity;
    case 24: return data->raceSettings.difficulty;
    case 26: return data->raceSettings.catchup;
    case 27: return data->raceSettings.backwards;
    case 28: return data->raceSettings.mirror;
    case 29: return data->raceSettings.fog;
    case 30: return data->raceSettings.wind;
    case 31: return data->raceSettings.deaths;
    default: return 0;
    }
}

/* 0x80038914, size 0x7c */
static signed char _GetBattleSetting(anon_ll_fileh_4c **player, unsigned char option, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (option) {
    case 23: return data->battleSettings.gravity;
    case 28: return data->battleSettings.mirror;
    case 29: return data->battleSettings.fog;
    case 30: return data->battleSettings.wind;
    case 37: return data->battleSettings.mode;
    case 38: return data->battleSettings.points;
    case 39: return data->battleSettings.minutes;
    case 40: return data->battleSettings.neg;
    default: return 0;
    }
}

/* 0x80038990, size 0x84 */
static signed char _GetStuntSetting(anon_ll_fileh_4c **player, unsigned char option, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (option) {
    case 23: return data->stuntSettings.gravity;
    case 28: return data->stuntSettings.mirror;
    case 29: return data->stuntSettings.fog;
    case 30: return data->stuntSettings.wind;
    case 32: return data->stuntSettings.mode;
    case 33: return data->stuntSettings.flow;
    case 34: return data->stuntSettings.points;
    case 35: return data->stuntSettings.minutes;
    case 36: return data->stuntSettings.deaths;
    default: return 0;
    }
}

/* 0x80038a14, size 0x60 */
static signed char _GetObstacleSetting(anon_ll_fileh_4c **player, unsigned char option, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    switch (option) {
    case 23: return data->obstacleSettings.gravity;
    case 28: return data->obstacleSettings.mirror;
    case 29: return data->obstacleSettings.fog;
    case 30: return data->obstacleSettings.wind;
    default: return 0;
    }
}

/* 0x80038a74, size 0x13c */
void SavePlayerOption(anon_ll_fileh_4c **player, unsigned char option, signed char value)
{
    anon_nameChecksum_id_814 *data;
    if ((*player)->datah == 0) return;
    data = *(*player)->datah;
    if (option < 21) {
        if (data->options[option] != value) {
            data->options[option] = value;
            data->optChecksum = _CalcChecksum(data->options, 21);
            PS_WriteFile((*player)->fileh, &data->optChecksum, 25);
        }
    } else if (gTrackNum >= 0 && gTrackNum < 6) _SaveRaceSetting(player, option, value, track_arg(gTrackNum));
    else if (gTrackNum >= 19 && gTrackNum < 25) _SaveRaceSetting(player, option, value, track_arg(gTrackNum));
    else if (gTrackNum >= 6 && gTrackNum < 14) _SaveBattleSetting(player, option, value, (unsigned char)(gTrackNum - 6));
    else if (gTrackNum >= 14 && gTrackNum < 18) _SaveStuntSetting(player, option, value, (unsigned char)(gTrackNum - 14));
    else _SaveObstacleSetting(player, option, value, (unsigned char)(gTrackNum - 18));
}

/* 0x80038bb0, size 0xf4 */
static void _SaveRaceSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, unsigned char track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    signed char *setting;
    switch (option) {
    case 21: setting = &data->raceSettings.laps; break;
    case 22: setting = &data->raceSettings.drones; break;
    case 23: setting = &data->raceSettings.gravity; break;
    case 24: setting = &data->raceSettings.difficulty; break;
    case 26: setting = &data->raceSettings.catchup; break;
    case 27: setting = &data->raceSettings.backwards; break;
    case 28: setting = &data->raceSettings.mirror; break;
    case 29: setting = &data->raceSettings.fog; break;
    case 30: setting = &data->raceSettings.wind; break;
    case 31: setting = &data->raceSettings.deaths; break;
    default: return;
    }
    if (*setting != value) {
        *setting = value;
        data->raceSettings.checksum = _CalcChecksum(&data->raceSettings.track, sizeof(data->raceSettings) - 4);
        PS_WriteFile((*player)->fileh, &data->raceSettings, sizeof(data->raceSettings));
    }
}

/* 0x80038ca4, size 0xe4 */
static void _SaveBattleSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    signed char *setting;
    switch (option) {
    case 23: setting = &data->battleSettings.gravity; break;
    case 28: setting = &data->battleSettings.mirror; break;
    case 29: setting = &data->battleSettings.fog; break;
    case 30: setting = &data->battleSettings.wind; break;
    case 37: setting = &data->battleSettings.mode; break;
    case 38: setting = &data->battleSettings.points; break;
    case 39: setting = &data->battleSettings.minutes; break;
    case 40: setting = &data->battleSettings.neg; break;
    default: return;
    }
    if (*setting != value) {
        *setting = value;
        data->battleSettings.checksum = _CalcChecksum(&data->battleSettings.track, sizeof(data->battleSettings) - 4);
        PS_WriteFile((*player)->fileh, &data->battleSettings, sizeof(data->battleSettings));
    }
}

/* 0x80038d88, size 0xec */
static void _SaveStuntSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    signed char *setting;
    switch (option) {
    case 23: setting = &data->stuntSettings.gravity; break;
    case 28: setting = &data->stuntSettings.mirror; break;
    case 29: setting = &data->stuntSettings.fog; break;
    case 30: setting = &data->stuntSettings.wind; break;
    case 32: setting = &data->stuntSettings.mode; break;
    case 33: setting = &data->stuntSettings.flow; break;
    case 34: setting = &data->stuntSettings.points; break;
    case 35: setting = &data->stuntSettings.minutes; break;
    case 36: setting = &data->stuntSettings.deaths; break;
    default: return;
    }
    if (*setting != value) {
        *setting = value;
        data->stuntSettings.checksum = _CalcChecksum(&data->stuntSettings.track, sizeof(data->stuntSettings) - 4);
        PS_WriteFile((*player)->fileh, &data->stuntSettings, sizeof(data->stuntSettings));
    }
}

/* 0x80038e74, size 0xc8 */
static void _SaveObstacleSetting(anon_ll_fileh_4c **player, unsigned char option, signed char value, int track)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    signed char *setting;
    switch (option) {
    case 23: setting = &data->obstacleSettings.gravity; break;
    case 28: setting = &data->obstacleSettings.mirror; break;
    case 29: setting = &data->obstacleSettings.fog; break;
    case 30: setting = &data->obstacleSettings.wind; break;
    default: return;
    }
    if (*setting != value) {
        *setting = value;
        data->obstacleSettings.checksum = _CalcChecksum(&data->obstacleSettings.track, sizeof(data->obstacleSettings) - 4);
        PS_WriteFile((*player)->fileh, &data->obstacleSettings, sizeof(data->obstacleSettings));
    }
}

/* 0x80038f3c, size 0x24 */
signed char GetPlayerCarSetting(anon_ll_fileh_4c **player, unsigned char car, unsigned char setting)
{
    return (*(*player)->datah)->carSettings[car].settings[setting];
}

/* 0x80038f60, size 0x98 */
void SavePlayerCarSetting(anon_ll_fileh_4c **player, unsigned char car, unsigned char setting, signed char value)
{
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    if (data->carSettings[car].settings[setting] != value) {
        data->carSettings[car].settings[setting] = value;
        data->carSettings[car].checksum = _CalcChecksum(data->carSettings[car].settings, 12);
        PS_WriteFile((*player)->fileh, &data->carSettings[car], 16);
    }
}

/* 0x80038ff8, size 0xac */
void SavePlayerControls(anon_ll_fileh_4c **player, signed char *controls)
{
    anon_nameChecksum_id_814 *data;
    if ((*player)->datah == 0) return;
    data = *(*player)->datah;
    if (memcmp(data->controls, controls, 12)) {
        memcpy((*player)->controls, controls, 12);
        memcpy(data->controls, controls, 12);
        data->contChecksum = _CalcChecksum(data->controls, 12);
        PS_WriteFile((*player)->fileh, &data->contChecksum, 16);
    }
}

/* 0x800390a4, size 0x17c */
void SavePlayerStats(anon_ll_fileh_4c **player, unsigned char track)
{
    anon_checksum_scores_60 *raceStats;
    anon_checksum_totalMatches_c *battleStats;
    anon_checksum_bestStunt_40 *stuntStats;
    anon_checksum_bestTime_18 *obstacleStats;
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    if (track < 6 || (track >= 19 && track < 25)) {
        if (track >= 19) raceStats = &data->raceStats[track - 13];
        else raceStats = &data->raceStats[track];
        raceStats->checksum = _CalcChecksum((char *)raceStats + 4, sizeof(*raceStats) - 4);
        PS_WriteFile((*player)->fileh, raceStats, sizeof(*raceStats));
    } else if (track >= 6 && track < 14) {
        battleStats = &data->battleStats[track - 6];
        battleStats->checksum = _CalcChecksum((char *)battleStats + 4, sizeof(*battleStats) - 4);
        PS_WriteFile((*player)->fileh, battleStats, sizeof(*battleStats));
    } else if (track >= 14 && track < 18) {
        stuntStats = &data->stuntStats[track - 14];
        stuntStats->checksum = _CalcChecksum((char *)stuntStats + 4, sizeof(*stuntStats) - 4);
        PS_WriteFile((*player)->fileh, stuntStats, sizeof(*stuntStats));
    } else {
        obstacleStats = &data->obstacleStats[track - 18];
        obstacleStats->checksum = _CalcChecksum((char *)obstacleStats + 4, sizeof(*obstacleStats) - 4);
        PS_WriteFile((*player)->fileh, obstacleStats, sizeof(*obstacleStats));
    }
}

/* 0x80039220, size 0x15c */
void SavePlayerCoins(anon_ll_fileh_4c **player, unsigned char track)
{
    int raceTrack;
    anon_checksum_scores_60 *raceStats;
    anon_checksum_bestStunt_40 *stuntStats;
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    if (track < 6 || (track >= 19 && track < 25)) {
        if (track >= 19) track = track_arg(track - 19);
        raceTrack = track;
        raceStats = &data->raceStats[raceTrack];
        raceStats->checksum = _CalcChecksum((char *)raceStats + 4, sizeof(*raceStats) - 4);
        PS_WriteFile((*player)->fileh, raceStats, 1);
        PS_WriteFile((*player)->fileh, &raceStats->coinsFound, 2);
        raceStats = &data->raceStats[raceTrack + 6];
        PS_WriteFile((*player)->fileh, raceStats, 1);
        PS_WriteFile((*player)->fileh, &raceStats->coinsFound, 2);
    } else if (track >= 14 && track < 18) {
        stuntStats = &data->stuntStats[track - 14];
        stuntStats->checksum = _CalcChecksum((char *)stuntStats + 4, sizeof(*stuntStats) - 4);
        PS_WriteFile((*player)->fileh, stuntStats, 1);
        PS_WriteFile((*player)->fileh, &stuntStats->coinsFound, 2);
    }
}

/* 0x8003937c, size 0x64 */
void SavePlayerCircuit(anon_ll_fileh_4c **player)
{
    anon_checksum_active_4c *data = &(*(*player)->datah)->circuit;
    data->checksum = _CalcChecksum((char *)data + 4, sizeof(*data) - 4);
    PS_WriteFile((*player)->fileh, data, sizeof(*data));
}

/* 0x800393e0, size 0x70 */
void SavePlayerCircuitStats(anon_ll_fileh_4c **player, unsigned char circuit)
{
    anon_checksum_numCircuits_1c *data = &(*(*player)->datah)->circuitStats[circuit];
    data->checksum = _CalcChecksum((char *)data + 4, sizeof(*data) - 4);
    PS_WriteFile((*player)->fileh, data, sizeof(*data));
}

/* 0x80039450, size 0xb4 */
static unsigned int _CalcChecksum(void * start, unsigned int size)
{
    signed char *ptr = start;
    unsigned int sum = 0;
    while (size--) {
        switch (*ptr & 7) {
        case 0: sum -= *ptr++; break;
        case 1: sum |= *ptr++; break;
        case 2: sum &= *ptr++; break;
        case 3: sum ^= *ptr++; break;
        case 4: sum *= *ptr++; break;
        case 5: sum /= *ptr++; break;
        default: sum += *ptr++; break;
        }
    }
    return sum;
}

/* 0x80039504, size 0x184 */
static void _SetDefaultCarSettings(anon_ll_fileh_4c **p, int car)
{
    SavePlayerCarSetting(p, car, 0, ((signed char *)gCarHandling)[car]);
    SavePlayerCarSetting(p, car, 1, ((signed char *)gCarEngines)[car]);
    SavePlayerCarSetting(p, car, 2, ((signed char *)gCarTires)[car]);
    SavePlayerCarSetting(p, car, 3, ((signed char *)gCarFrame)[car]);
    SavePlayerCarSetting(p, car, 4, ((signed char *)gCarWings)[car]);
    SavePlayerCarSetting(p, car, 5, ((signed char *)gCarPaintJob)[car]);
    SavePlayerCarSetting(p, car, 6, ((signed char *)gTireRims)[car]);
    SavePlayerCarSetting(p, car, 7, ((signed char *)gCarHorn)[car]);
    SavePlayerCarSetting(p, car, 8, (signed char)(100.0f * ((float *)gFrontTireScale)[car] - 75.0f));
    SavePlayerCarSetting(p, car, 9, (signed char)(100.0f * ((float *)gRearTireScale)[car] - 75.0f));
}

/* 0x80039688, size 0x18c */
static void _InitPlayer(anon_ll_fileh_4c **player)
{
    anon_nameChecksum_id_814 *data;
    int i;
    int track;
    memset(*player, 0, sizeof(**player) - sizeof((*player)->datah));
    data = *(*player)->datah;
    memset(data, 0, sizeof(*data));
    data->revision = 19;
    SetDefaultControls(data->controls);
    data->rumble = 1;
    data->contChecksum = _CalcChecksum(data->controls, 12);
    memcpy((*player)->controls, data->controls, 12);
    for (i = 0; i < 13; i++) _SetDefaultCarSettings(player, i);
    for (i = 0; i < 21; i++) SavePlayerOption(player, i, GetDefaultOption(i));
    track = gTrackNum;
    for (gTrackNum = 0; gTrackNum < 19; gTrackNum++) {
        for (i = 21; i < 41; i++) SavePlayerOption(player, i, GetDefaultOption(i));
    }
    gTrackNum = track;
    for (data->raceType = 0; data->raceType < 7; data->raceType++) {
        SavePlayerCar(player, 2);
        SavePlayerCarView(player, 2);
    }
    data->raceType = 0;
}

/* 0x80039814, size 0x174 */
int ClearPlayerRecords(anon_ll_fileh_4c **player)
{
    anon_nameChecksum_id_814 *data;
    if (!ReadPlayerData(player)) return 0;
    data = *(*player)->datah;
    memset(data->raceStats, 0, sizeof(data->raceStats));
    PS_WriteFile((*player)->fileh, data->raceStats, sizeof(data->raceStats));
    memset(data->stuntStats, 0, sizeof(data->stuntStats));
    PS_WriteFile((*player)->fileh, data->stuntStats, sizeof(data->stuntStats));
    memset(data->battleStats, 0, sizeof(data->battleStats));
    PS_WriteFile((*player)->fileh, data->battleStats, sizeof(data->battleStats));
    memset(data->obstacleStats, 0, sizeof(data->obstacleStats));
    PS_WriteFile((*player)->fileh, data->obstacleStats, sizeof(data->obstacleStats));
    memset(data->circuitStats, 0, sizeof(data->circuitStats));
    PS_WriteFile((*player)->fileh, data->circuitStats, sizeof(data->circuitStats));
    memset(&data->circuit, 0, sizeof(data->circuit));
    PS_WriteFile((*player)->fileh, &data->circuit, sizeof(data->circuit));
    if (!PS_FlushFile((*player)->fileh, 0)) {
        PurgePlayerData(player);
        return 0;
    }
    RemovePlayerScores(player);
    PS_FlushPak((*(*player)->fileh)->pak, 1);
    return 1;
}

/* 0x80039988, size 0x28 */
static anon_ll_fileh_4c **_NewPlayer(void)
{
    return (anon_ll_fileh_4c **)AllocateHandleHeap(0, sizeof(anon_ll_fileh_4c));
}

/* 0x800399b0, size 0x20 */
static void _FreePlayer(anon_ll_fileh_4c **player)
{
    DeallocateMemory(player);
}

/* 0x800399d0, size 0x3c */
static int _AlreadyReadPlayer(tPS_File **fileh)
{
    anon_ll_fileh_4c **player;
    for (player = (anon_ll_fileh_4c **)gPlayerFiles.head; player; player = (anon_ll_fileh_4c **)(*player)->ll.next) {
        if ((*player)->fileh == fileh) return 1;
    }
    return 0;
}

/* 0x80039a0c, size 0x2a0 */
static anon_ll_fileh_4c **_ReadPlayer(tPS_File **fileh)
{
    anon_ll_fileh_4c **player;
    anon_ll_fileh_4c *p;
    anon_ll_fileh_4c **before;
    int i;
    int pak = (*fileh)->pak;
    signed char choice1;
    signed char choice2;
    PS_ReadFile(fileh);
    if (!_VerifyFile(fileh, pak)) return 0;
    if (!(*fileh)->data) return 0;
    /* The original caller supplies pak/id although allocator DWARF omits them. */
    player = ((anon_ll_fileh_4c **(*)(int, unsigned int))_NewPlayer)((*fileh)->pak, (*fileh)->id);
    p = *player;
    p->fileh = fileh;
    (*p->fileh)->RemoveFile = _RemovePlayerFile;
    (*p->fileh)->MoveFile = _MovePlayerFile;
    strcpy(p->filename, (*p->fileh)->filename);
    p->datah = (anon_nameChecksum_id_814 **)(*p->fileh)->data;
    if (_CheckAndRepairPlayer(player, 0)) {
        if (!_CheckAndRepairPlayer(player, 1)) {
            PS_ErrorFunc(pak, 12, p->filename, 0, &choice1, &choice2, 0);
            _FreePlayer(player);
            return 0;
        }
    }
    memcpy(p->name, (*p->datah)->name, sizeof(p->name));
    memcpy(p->id, (*p->datah)->id, sizeof(p->id));
    memcpy(p->controls, (*p->datah)->controls, sizeof(p->controls) + 1);
    if (!gGlobalsSet && !gPlayerFiles.head) {
        for (i = 0; i < 21; i++) gOptions[i] = GetPlayerOption(player, i);
        gOptions[17] = 1;
        gLang = gOptions[16] = GetPlayerOption(player, 16);
        LoadTextWave();
        VideoSetOffset(gOptions[19], gOptions[20]);
    }
    for (before = gPlayerFiles.head; before; before = (*before)->ll.next) {
        if (strcmp((char *)(*before)->name, (char *)p->name) > 0) break;
    }
    InsertLLE(&gPlayerFiles, player, before);
    AddPlayerScores(player);
    PurgePlayerData(player);
    if (!strcmp((char *)p->name, "000F1CC5 EJK")) gSaveGhostFiles = 1;
    return player;
}

/* 0x80039cac, size 0x104 */
int ReadPlayerData(anon_ll_fileh_4c **playerh)
{
    anon_ll_fileh_4c *player = *playerh;
    int pak;
    signed char choice1;
    signed char choice2;
    if (player->datah) return 1;
    pak = (*player->fileh)->pak;
    PS_ReadFile(player->fileh);
    if (!_VerifyPlayer(playerh, pak)) return 0;
    if (!(*player->fileh)->data) return 0;
    player->datah = (anon_nameChecksum_id_814 **)(*player->fileh)->data;
    if (_CheckAndRepairPlayer(playerh, 0)) {
        if (!_CheckAndRepairPlayer(playerh, 1)) {
            PS_ErrorFunc((*player->fileh)->pak, 12, player->filename, 0, &choice1, &choice2, 0);
            return 0;
        }
    }
    SavePlayerControls(playerh, player->controls);
    return 1;
}

/* 0x80039db0, size 0x4 */
void PurgePlayerData(void)
{
    /* GC implementation is empty; no data is purged here. */
}

/* 0x80039db4, size 0x618 */
static int _CheckAndRepairPlayer(anon_ll_fileh_4c **player, int repair)
{
    int result = 0;
    int i;
    int j;
    int oldTracknum;
    anon_nameChecksum_id_814 *data = *(*player)->datah;
    if ((data->revision & 63) != 19) {
        if (repair) return 0;
        result = 1;
    }
    if (data->nameChecksum != _CalcChecksum(data->id, (char *)&data->optChecksum - (char *)data - 4)) {
        if (repair) {
            data->nameChecksum = _CalcChecksum(data->id, (char *)&data->optChecksum - (char *)data - 4);
            PS_WriteFile((*player)->fileh, data, 4);
        }
        result = 1;
    }
    if (data->optChecksum != _CalcChecksum(data->options, sizeof(data->options))) {
        if (repair) {
            for (i = 0; i < 21; i++) SavePlayerOption(player, i, gOptions[i]);
        }
        result = 1;
    }
    if (data->contChecksum != _CalcChecksum(data->controls, sizeof(data->controls) + 1)) {
        if (repair) {
            SetDefaultControls((*player)->controls);
            (*player)->rumble = 1;
            memcpy(data->controls, (*player)->controls, sizeof(data->controls) + 1);
            data->contChecksum = _CalcChecksum(data->controls, sizeof(data->controls) + 1);
            PS_WriteFile((*player)->fileh, &data->contChecksum, 16);
        }
        result = 1;
    }
    if (data->raceSettings.checksum != _CalcChecksum(&data->raceSettings.track, sizeof(data->raceSettings) - 4)) {
        if (repair) {
            oldTracknum = gTrackNum;
            gTrackNum = 0;
            for (i = 21; i < 41; i++) SavePlayerOption(player, i, gOptions[i]);
            gTrackNum = oldTracknum;
            memset(&data->raceSettings, 0, sizeof(data->raceSettings));
            PS_WriteFile((*player)->fileh, &data->raceSettings, sizeof(data->raceSettings));
        }
        result = 1;
    }
    if (data->battleSettings.checksum != _CalcChecksum(&data->battleSettings.track, sizeof(data->battleSettings) - 4)) {
        if (repair) {
            oldTracknum = gTrackNum;
            gTrackNum = 6;
            for (j = 21; j < 41; j++) SavePlayerOption(player, j, gOptions[j]);
            gTrackNum = oldTracknum;
            memset(&data->battleSettings, 0, sizeof(data->battleSettings));
            PS_WriteFile((*player)->fileh, &data->battleSettings, sizeof(data->battleSettings));
        }
        result = 1;
    }
    if (data->stuntSettings.checksum != _CalcChecksum(&data->stuntSettings.track, sizeof(data->stuntSettings) - 4)) {
        if (repair) {
            oldTracknum = gTrackNum;
            gTrackNum = 14;
            for (j = 21; j < 41; j++) SavePlayerOption(player, j, gOptions[j]);
            gTrackNum = oldTracknum;
            memset(&data->stuntSettings, 0, sizeof(data->stuntSettings));
            PS_WriteFile((*player)->fileh, &data->stuntSettings, sizeof(data->stuntSettings));
        }
        result = 1;
    }
    if (data->obstacleSettings.checksum != _CalcChecksum(&data->obstacleSettings.track, sizeof(data->obstacleSettings) - 4)) {
        if (repair) {
            oldTracknum = gTrackNum;
            gTrackNum = 18;
            for (j = 21; j < 41; j++) SavePlayerOption(player, j, gOptions[j]);
            gTrackNum = oldTracknum;
            memset(&data->obstacleSettings, 0, sizeof(data->obstacleSettings));
            PS_WriteFile((*player)->fileh, &data->obstacleSettings, sizeof(data->obstacleSettings));
        }
        result = 1;
    }
    for (i = 0; i < 6; i++) {
        if (data->raceStats[i].checksum != _CalcChecksum(&data->raceStats[i].scores, sizeof(data->raceStats[i]) - 4)) {
            if (repair) {
                memset(&data->raceStats[i], 0, sizeof(data->raceStats[i]));
                PS_WriteFile((*player)->fileh, &data->raceStats[i], sizeof(data->raceStats[i]));
            }
            result = 1;
        }
    }
    for (i = 0; i < 4; i++) {
        if (data->stuntStats[i].checksum != _CalcChecksum(&data->stuntStats[i].bestStunt, sizeof(data->stuntStats[i]) - 4)) {
            if (repair) {
                memset(&data->stuntStats[i], 0, sizeof(data->stuntStats[i]));
                PS_WriteFile((*player)->fileh, &data->stuntStats[i], sizeof(data->stuntStats[i]));
            }
            result = 1;
        }
    }
    for (i = 0; i < 8; i++) {
        if (data->battleStats[i].checksum != _CalcChecksum(&data->battleStats[i].totalMatches, sizeof(data->battleStats[i]) - 4)) {
            if (repair) {
                memset(&data->battleStats[i], 0, sizeof(data->battleStats[i]));
                PS_WriteFile((*player)->fileh, &data->battleStats[i], sizeof(data->battleStats[i]));
            }
            result = 1;
        }
    }
    if (data->obstacleStats[0].checksum != _CalcChecksum(&data->obstacleStats[0].bestTime, sizeof(data->obstacleStats) - 4)) {
        if (repair) {
            memset(&data->obstacleStats[0], 0, sizeof(data->obstacleStats));
            PS_WriteFile((*player)->fileh, &data->obstacleStats[0], sizeof(data->obstacleStats));
        }
        result = 1;
    }
    for (i = 0; i < 4; i++) {
        if (data->circuitStats[i].checksum != _CalcChecksum(&data->circuitStats[i].numCircuits, sizeof(data->circuitStats[i]) - 4)) {
            if (repair) {
                memset(&data->circuitStats[i], 0, sizeof(data->circuitStats[i]));
                PS_WriteFile((*player)->fileh, &data->circuitStats[i], sizeof(data->circuitStats[i]));
            }
            result = 1;
        }
    }
    if (data->circuit.checksum != _CalcChecksum(&data->circuit.active, sizeof(data->circuit) - 4)) {
        if (repair) {
            memset(&data->circuit, 0, sizeof(data->circuit));
            PS_WriteFile((*player)->fileh, &data->circuit, sizeof(data->circuit));
        }
        result = 1;
    }
    for (i = 0; i < 13; i++) {
        if (data->carSettings[i].checksum != _CalcChecksum(&data->carSettings[i].settings, sizeof(data->carSettings[i]) - 4)) {
            if (repair) {
                _SetDefaultCarSettings(player, i);
                PS_WriteFile((*player)->fileh, &data->carSettings[i], sizeof(data->carSettings[i]));
            }
            result = 1;
        }
    }
    return result;
}

/* 0x8003a3cc, size 0x2ec */
anon_ll_fileh_4c **CreatePlayer(int pak, unsigned char *name)
{
    signed char gDreamcastId[32] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    signed short playerPalette[16] = {
        -4096, -31, -4095, -863, -1440, -2495, -4093, -4091, -3549, -4071, -3819, -3823, -3256, -3003, -1915, -4084,
    };
    char playerPixels[512] = {
#include "generated/playerPixels.inc"
};
    anon_ll_fileh_4c **player;
    anon_ll_fileh_4c **before;
    anon_ll_fileh_4c *p;
    anon_nameChecksum_id_814 *data;
    tPS_File **fileh;
    char ext[4];
    _GetUniqueExtension(pak, "RUSH2049", "P", ext);
    fileh = PS_CreateFile(pak, "RUSH2049", ext, sizeof(*data), "RUSH 2049", "RUSH 2049 PLAYER DATA", (char *)name, playerPalette, playerPixels);
    if (!fileh) return 0;
    player = ((anon_ll_fileh_4c **(*)(int, unsigned int))_NewPlayer)((*fileh)->pak, (*fileh)->id);
    p = *player;
    p->datah = (anon_nameChecksum_id_814 **)(*fileh)->data;
    data = *p->datah;
    _InitPlayer(player);
    p->fileh = fileh;
    (*fileh)->RemoveFile = _RemovePlayerFile;
    (*fileh)->MoveFile = _MovePlayerFile;
    strcpy(p->filename, (*fileh)->filename);
    data->id[0] = sc.frameCount;
    data->id[0] &= 0xffff;
    data->id[0] |= ((gDreamcastId[1] & 0xff) << 24) | ((gDreamcastId[5] & 0xff) << 16);
    data->id[1] = ((gDreamcastId[4] & 0xff) << 24) | ((gDreamcastId[2] & 0xff) << 16) | ((gDreamcastId[0] & 0xff) << 8) | (gDreamcastId[3] & 0xff);
    strcpy((char *)data->name, (char *)name);
    data->nameChecksum = _CalcChecksum(data->id, (char *)&data->optChecksum - (char *)data - 4);
    memcpy(p->name, data->name, sizeof(p->name));
    memcpy(p->id, data->id, sizeof(p->id));
    PS_WriteFile(p->fileh, data, sizeof(*data));
    if (!PS_FlushFile(p->fileh, 0)) {
        if (_VerifyFile(p->fileh, pak)) PS_DeleteFile(p->fileh);
        _FreePlayer(player);
        return 0;
    }
    PS_FlushPak(pak, 1);
    for (before = gPlayerFiles.head; before; before = (*before)->ll.next) {
        if (strcmp((char *)(*before)->name, (char *)p->name) > 0) break;
    }
    InsertLLE(&gPlayerFiles, player, before);
    return player;
}

/* 0x8003a6b8, size 0x58 */
void DeletePlayer(anon_ll_fileh_4c **player)
{
    if ((*player)->fileh) {
        if (ReadPlayerData(player)) PS_DeleteFile((*player)->fileh);
    } else {
        _InitPlayer(player);
    }
}

/* 0x8003a710, size 0x28 */
anon_ll_fileh_30 **_NewGhost(void)
{
    return (anon_ll_fileh_30 **)AllocateHandleHeap(0, sizeof(anon_ll_fileh_30));
}

/* 0x8003a738, size 0x20 */
void _FreeGhost(anon_ll_fileh_30 **ghost)
{
    DeallocateMemory(ghost);
}

/* 0x8003a758, size 0x3c */
static int _AlreadyReadGhost(tPS_File **fileh)
{
    anon_ll_fileh_30 **ghost;
    for (ghost = (anon_ll_fileh_30 **)gGhostFiles.head; ghost; ghost = (anon_ll_fileh_30 **)(*ghost)->ll.next) {
        if ((*ghost)->fileh == fileh) return 1;
    }
    return 0;
}

/* 0x8003a794, size 0x1d0 */
static anon_ll_fileh_30 **_ReadGhostHeader(tPS_File **fileh)
{
    anon_ll_fileh_30 **ghosth;
    anon_ll_fileh_30 *ghost;
    anon_ll_fileh_30 **before;
    anon_headerChecksum_revision_54 *header;
    int pak = (*fileh)->pak;
    signed char choice1;
    signed char choice2;
    PS_ReadFile(fileh);
    if (!_VerifyFile(fileh, pak)) return 0;
    if (!(*fileh)->data) return 0;
    ghosth = _NewGhost();
    ghost = *ghosth;
    memset(ghost, 0, sizeof(*ghost));
    ghost->fileh = fileh;
    (*ghost->fileh)->RemoveFile = _RemovePlayerFile;
    (*ghost->fileh)->MoveFile = _MovePlayerFile;
    ghost->datah = 0;
    ghost->header = (anon_headerChecksum_revision_54 **)(*fileh)->data;
    header = *ghost->header;
    header->start = 0;
    if (_CheckAndRepairGhost(ghosth, 1, 0)) {
        if (!_CheckAndRepairGhost(ghosth, 1, 1)) {
            PS_ErrorFunc((*fileh)->pak, 12, (*fileh)->filename, 0, &choice1, &choice2, 0);
            PS_PurgeFile(fileh);
            _FreeGhost(ghosth);
            return 0;
        }
    }
    ghost->track = header->track;
    ghost->backward = header->backward;
    memcpy(ghost->name, header->name, sizeof(ghost->name));
    ghost->playerId[0] = header->playerId[0];
    ghost->playerId[1] = header->playerId[1];
    ghost->finish = header->finish;
    for (before = gGhostFiles.head; before; before = (*before)->ll.next) {
        if ((*before)->finish > ghost->finish) break;
    }
    InsertLLE(&gGhostFiles, ghosth, before);
    return ghosth;
}

/* 0x8003a964, size 0x158 */
int _ReadGhostData(anon_ll_fileh_30 **ghosth, int decode)
{
    anon_ll_fileh_30 *ghost = *ghosth;
    int pak;
    signed char choice1;
    signed char choice2;
    if (ghost->header) {
        if (decode) _DecodeGhost(ghosth);
        return 1;
    }
    pak = (*ghost->fileh)->pak;
    PS_ReadFile(ghost->fileh);
    if (!_VerifyGhost(ghosth, pak)) return 0;
    if (!(*ghost->fileh)->data) return 0;
    ghost->header = (anon_headerChecksum_revision_54 **)(*ghost->fileh)->data;
    (*ghost->header)->start = 0;
    if (_CheckAndRepairGhost(ghosth, 1, 0)) {
        if (!_CheckAndRepairGhost(ghosth, 1, 1)) {
            PS_ErrorFunc((*ghost->fileh)->pak, 12, (*ghost->fileh)->filename, 0, &choice1, &choice2, 0);
            RemoveLLE(&gGhostFiles, ghosth);
            PS_PurgeFile(ghost->fileh);
            _FreeGhost(ghosth);
            return 0;
        }
    }
    if (decode) _DecodeGhost(ghosth);
    return 1;
}

/* 0x8003aabc, size 0x28 */
int ReadGhostData(anon_ll_fileh_30 **ghosth)
{
    /* The original caller passes a third argument absent from the callee DWARF. */
    return ((int (*)())_ReadGhostData)(ghosth, 1, 1);
}

/* 0x8003aae4, size 0x4c */
void PurgeGhostData(anon_ll_fileh_30 **ghosth)
{
    anon_ll_fileh_30 *ghost = *ghosth;
    if (ghost->datah) {
        DeallocateMemory(ghost->datah);
        ghost->datah = 0;
        (*ghost->header)->start = 0;
    }
}

/* 0x8003ab30, size 0xa8 */
static void _DecodeGhost(anon_ll_fileh_30 **ghosth)
{
    int size;
    anon_headerChecksum_revision_54 *header = *(*ghosth)->header;
    if (!header->start) {
        (*ghosth)->datah = AllocateHandleHeap(0, header->unzippedSize);
        header->start = *(*ghosth)->datah;
        size = unzipdata((unsigned char *)(header + 1), header->zippedSize, header->start);
        header->size = size / 3;
        _CalcChecksum(header->start, size);
        header->flag = 1;
    }
}

/* 0x8003abd8, size 0xec */
static int _CheckAndRepairGhost(anon_ll_fileh_30 **ghosth, int data, int repair)
{
    int result = 0;
    anon_headerChecksum_revision_54 *header = *(*ghosth)->header;
    if ((header->revision & 63) != 20) {
        if (repair) return 0;
        result = 1;
    }
    if (header->headerChecksum != _CalcChecksum((char *)header + 4,
            (unsigned int)&header->zippedChecksum - (unsigned int)header - 4)) {
        if (repair) return 0;
        result = 1;
    }
    if (!data) return result;
    if (header->zippedChecksum != _CalcChecksum((unsigned char *)(header + 1), header->zippedSize)) {
        if (repair) return 0;
        result = 1;
    }
    return result;
}

/* 0x8003acc4, size 0x27c */
anon_ll_fileh_30 **CompressGhost(anon_headerChecksum_revision_54 *header, unsigned char *blocks)
{
    anon_ll_fileh_30 *ghost;
    anon_ll_fileh_30 **ghosth;
    unsigned int maxsize;
    unsigned char *sorted;
    unsigned char *gdata;
    unsigned int num;
    unsigned int dataSize;
    unsigned int unzippedChecksum;
    sorted = header->start;
    maxsize = (int)(gRetraceRate * (float)(ghostRaceTimes[gTrackNum] + 10.0));
    memmove(header->start + header->size, header->start + maxsize, header->size);
    memmove(header->start + header->size * 2, header->start + maxsize * 2, header->size);
    dataSize = header->size * 3;
    unzippedChecksum = _CalcChecksum(sorted, dataSize);
    header->tournamentKey = 0;
    gdata = AllocateMemoryHeapHigh(0, dataSize);
    num = zip(sorted, dataSize, gdata, dataSize, 9, 10, 4);
    DeallocateMemory(header->start);
    header->start = 0;
    ghosth = _NewGhost();
    ghost = *ghosth;
    memset(ghost, 0, sizeof(*ghost));
    ghost->header = (anon_headerChecksum_revision_54 **)AllocateHandleHeap(0, num + sizeof(*header));
    memcpy(*ghost->header, header, sizeof(*header));
    (*ghost->header)->revision = 20;
    (*ghost->header)->start = 0;
    (*ghost->header)->offset = 0;
    (*ghost->header)->size = 0;
    (*ghost->header)->zippedSize = num;
    (*ghost->header)->unzippedSize = dataSize;
    (*ghost->header)->headerChecksum = _CalcChecksum(&(*ghost->header)->revision, (char *)&(*ghost->header)->zippedChecksum - (char *)*ghost->header - 4);
    (*ghost->header)->zippedChecksum = _CalcChecksum(gdata, num);
    (*ghost->header)->unzippedChecksum = unzippedChecksum;
    memcpy(*ghost->header + 1, gdata, num);
    DeallocateMemory(gdata);
    ghost->track = (*ghost->header)->track;
    ghost->backward = (*ghost->header)->backward;
    memcpy(ghost->name, (*ghost->header)->name, sizeof(ghost->name));
    ghost->playerId[0] = (*ghost->header)->playerId[0];
    ghost->playerId[1] = (*ghost->header)->playerId[1];
    ghost->finish = (*ghost->header)->finish;
    *(unsigned int *)blocks = PS_GetBlocks(GetPtrSize(ghost->header));
    return ghosth;
}

/* 0x8003af40, size 0x2dc */
/* GC code consumes r5/r6; r4 is an unused parameter omitted from DWARF. */
int SaveGhost(int pak, void *unused, anon_ll_fileh_30 **saveGhosth, int limbo)
{
    signed short ghostPalette[16] = {
        -4096, -2495, -879, -47, -1696, -3823, -4095, -4093, -4091, -3293, -4071, -3820, -3256, -3003, -1915, -4084,
    };
    char ghostPixels[512] = {
#include "generated/ghostPixels.inc"
};
    anon_ll_fileh_30 *ghost;
    anon_ll_fileh_30 **before;
    anon_headerChecksum_revision_54 *header;
    anon_headerChecksum_revision_54 **oldHeader;
    tPS_File **fileh;
    unsigned int size;
    char str[32];
    ghost = *saveGhosth;
    header = *ghost->header;
    if (limbo) {
        SaveGhostInLimbo(header, header + 1);
        DeallocateMemory((*saveGhosth)->header);
        _FreeGhost(saveGhosth);
        return 1;
    }
    size = GetPtrSize(ghost->header);
    GetGhostExtension(pak, header->track, header->backward, "TRN");
    sprintf(str, "GHOST RACE: TRK %d %s: %01d:%02d.%03d", header->track + 1, header->backward ? "BCK" : "FWD",
        (unsigned int)(header->finish / 60.0f), (unsigned int)header->finish % 60, (unsigned int)(header->finish * 1000.0f) % 1000);
    fileh = PS_CreateFile(pak, "RUSH2049", "TRN", size, "RUSH 2049", str, (char *)ghost->name, ghostPalette, ghostPixels);
    if (!fileh) return 0;
    memcpy(*(void **)(*fileh)->data, *ghost->header, size);
    oldHeader = ghost->header;
    ghost->header = (anon_headerChecksum_revision_54 **)(*fileh)->data;
    PS_WriteFile(fileh, *ghost->header, size);
    if (!PS_FlushFile(fileh, 0)) {
        if (_VerifyFile(fileh, pak)) PS_DeleteFile(fileh);
        ghost->fileh = 0;
        ghost->header = oldHeader;
        return 0;
    }
    ghost->fileh = fileh;
    (*ghost->fileh)->RemoveFile = _RemovePlayerFile;
    (*ghost->fileh)->MoveFile = _MovePlayerFile;
    DeallocateMemory(oldHeader);
    PS_FlushPak(pak, 1);
    for (before = gGhostFiles.head; before; before = (*before)->ll.next) {
        if ((*before)->finish > ghost->finish) break;
    }
    InsertLLE(&gGhostFiles, saveGhosth, before);
    return 1;
}

/* 0x8003b21c, size 0x30 */
void DeleteGhost(anon_ll_fileh_30 **ghosth)
{
    if ((*ghosth)->fileh) PS_DeleteFile((*ghosth)->fileh);
}

/* 0x8003b24c, size 0x1e4 */
void SaveGhostInLimbo(anon_headerChecksum_revision_54 *header, void * data)
{
int i;
    anon_headerChecksum_revision_54 *limbo;
    anon_ll_fileh_30 **limboGhosts;
    limboGhosts = gLimboGhosts + (header->track + (header->backward ? 6 : 0)) * 3;
    for (i = 0; i < gNumLimbo; i++) {
        if (limboGhosts[i]->header) {
            limbo = *limboGhosts[i]->header;
            if (limbo->track == header->track && limbo->backward == header->backward) {
                if (!(header->finish < limbo->finish)) break;
            }
        }
    }
    if (i - 1 >= 0) {
        if (limboGhosts[0]->header) DeallocateMemory(limboGhosts[0]->header);
        if (i - 1 > 0) memmove(limboGhosts[0], limboGhosts[1], (i - 1) * sizeof(*limboGhosts[0]));
        i--;
        limboGhosts[i]->track = header->track;
        limboGhosts[i]->backward = header->backward;
        memcpy(limboGhosts[i]->name, header->name, sizeof(limboGhosts[i]->name));
        memcpy(limboGhosts[i]->playerId, header->playerId, sizeof(limboGhosts[i]->playerId));
        limboGhosts[i]->finish = header->finish;
        limboGhosts[i]->header = (anon_headerChecksum_revision_54 **)AllocateHandleHeap(0, sizeof(*header) + header->zippedSize);
        memcpy(*limboGhosts[i]->header, header, sizeof(*header));
        memcpy(*limboGhosts[i]->header + 1, data, header->zippedSize);
        (*limboGhosts[i]->header)->start = 0;
        (*limboGhosts[i]->header)->size = 0;
        (*limboGhosts[i]->header)->offset = 0;
    }
}

/* 0x8003b430, size 0xa4 */
int GhostFileAvailable(anon_ll_fileh_4c **player, int pak, int track, int backward)
{
    char filename[64];
    tPS_File **psfileh;
    sprintf(filename, "%s%d%s", (*player)->filename, track + 1, backward ? "B" : "F");
    for (psfileh = gPS_Files[pak].head; psfileh; psfileh = (*psfileh)->ll.next) {
        if (!strcmp((*psfileh)->name, filename)) return 0;
    }
    return 1;
}

/* 0x8003b4d4, size 0x70 */
int GetGhostExtension(int pak, int track, int backward, char *ext)
{
    char str[3];
    sprintf(str, "%d%s", track + 1, backward ? "B" : "F");
    return _GetUniqueExtension(pak, "RUSH2049", str, ext);
}

/* 0x8003b544, size 0x100 */
static int _GetUniqueExtension(int pak, char *name, char *extPrefix, char *ext)
{
    int i;
    tPS_File **fileh;
    for (i = 0; ; i++) {
        _Pow(10, 3 - strlen(extPrefix));
        if (i >= _Pow(10, 3 - strlen(extPrefix))) return 0;
        sprintf(ext, "%s%0*d", extPrefix, 3 - strlen(extPrefix), i);
        for (fileh = gPS_Files[pak].head; fileh; fileh = (*fileh)->ll.next) {
            if (strstr((*fileh)->name, name) && !strcmp((*fileh)->ext, ext)) break;
        }
        if (!fileh) break;
    }
    return 1;
}

/* 0x8003b644, size 0x58 */
static int _Pow(int num, int exp)
{
    int result = 1;
    while (exp--) result *= num;
    return result;
}

/* 0x8003b69c, size 0x58 */
static int _VerifyPlayer(anon_ll_fileh_4c **playerh, int pak)
{
    anon_ll_fileh_4c **h;
    for (h = gPlayerFiles.head; h; h = (*h)->ll.next) {
        if (h == playerh) return _VerifyFile((*h)->fileh, pak);
    }
    return 0;
}

/* 0x8003b6f4, size 0x58 */
static int _VerifyGhost(anon_ll_fileh_30 **ghosth, int pak)
{
    anon_ll_fileh_30 **h;
    for (h = gGhostFiles.head; h; h = (*h)->ll.next) {
        if (h == ghosth) return _VerifyFile((*h)->fileh, pak);
    }
    return 0;
}

/* 0x8003b74c, size 0x40 */
static int _VerifyFile(tPS_File **fileh, int pak)
{
    tPS_File **file;
    for (file = gPS_Files[pak].head; file; file = (*file)->ll.next) {
        if (file == fileh) return 1;
    }
    return 0;
}

