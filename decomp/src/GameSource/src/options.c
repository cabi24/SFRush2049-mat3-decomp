/* options.c - game options: defaults, applying the option table to the game globals, and saving to player files.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\options.c; assembly: build/2049LE/asm/GameSource/src/options.s */
#include "rush/rush_protos.h"

static void _SetDefaultOptions(void);

signed char gCheatActive[24] = { 0 };
signed short gCheatState[23] = { 0 };
signed char gOptions[41];

/* 0x8002ce10, size 0x8c */
void InitOptions(void)
{
    gPlayers[0].controller = 5;
    gPlayers[0].trans = 0;
    gPlayers[0].listener = -1;
    gPlayers[0].slot = 0;
    gPlayers[1].controller = 5;
    gPlayers[1].trans = 0;
    gPlayers[1].listener = -1;
    gPlayers[1].slot = 1;
    gPlayers[2].controller = 5;
    gPlayers[2].trans = 0;
    gPlayers[2].listener = -1;
    gPlayers[2].slot = 2;
    gPlayers[3].controller = 5;
    gPlayers[3].trans = 0;
    gPlayers[3].listener = -1;
    gPlayers[3].slot = 3;
    _SetDefaultOptions();
    gNumPlayers = gNumWindows = 1;
}

/* 0x8002ce9c, size 0x2f4 */
static void _SetDefaultOptions(void)
{
    int i; /* r20 */

    SetControls(1);
    if (!gOptions[21]) {
        for (i = 0; i < 41; i++) {
            gOptions[i] = GetDefaultOption(i);
        }
    }
    gOptions[15] = ngcGetAxMode();
    AudSetFxVol(gOptions[13] / 10.0f, 0.05f);
    AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
    AudMono(gOptions[15]);
    VideoSetOffset(gOptions[19], gOptions[20]);
    gNumLaps = gOptions[21];
    gNumDrones = gOptions[22];
    gShowTrackMap = gOptions[0];
    gGravityScale = gOptions[23];
    gShowRadar = gOptions[1];
    gShowElapsed = gOptions[2];
    gShowRemaining = gOptions[3];
    gShowTach = gOptions[4];
    gShowSpeed = gOptions[5];
    gShowOdometer = gOptions[6];
    gShowPlace = gOptions[7];
    gShowShifter = gOptions[8];
    gDifficulty = gOptions[24];
    gGhostSelect = gOptions[25];
    gCatchup = gOptions[26];
    gBackwards = gOptions[27];
    gMirrorWaves = gOptions[28] > 0;
    gMirrorControls = gOptions[28] > 1;
    gFogIntensity = gOptions[29];
    gWind = gOptions[30];
    gShowMetric = gOptions[9];
    gDeathMode = gOptions[31];
    gShowPlayerArrows = gOptions[10];
    gShowCoins = gOptions[11];
    gLang = gOptions[16];
    gShowWrongWay = gOptions[18];
    gStuntMode = gOptions[32];
    gStuntFlow = gOptions[33];
    gStuntModeMins = gOptions[35];
    gStuntModePoints = gOptions[34];
    gStuntModeDeaths = gOptions[36];
    gBattleMode = gOptions[37];
    gBattleModeMins = gOptions[39];
    gBattleModePoints = gOptions[38];
    gNoCarCollisions = 0;
    gFogColor = 0;
    gMineCones = 0;
    gCarMines = 0;
    gRandomWeapons = 0;
    gDemolitionBattle = 0;
    gCheatState[3] = 0;
    gUpsideDownTrack = 0;
    gNoAutoAbort = 0;
    gInvincible = 0;
    gInvisibleCar = 0;
    gInvisibleTrack = 0;
    gNoBrakes = 0;
    gSpecialTires = 0;
    gMassiveMass = 0;
    gSuicideMode = 0;
    gCheatState[13] = 0;
    gCheatState[16] = 0;
    gCheatState[17] = 0;
    gCheatState[18] = 0;
    gSuperSpeed = 0;
    gResurrectInPlace = 0;
    gFrameScale = 0;
    gEnableTireScaling = 0;
    gDriveHulk[0] = 0;
    gDriveHulk[1] = 0;
    gDriveHulk[2] = 0;
    gDriveHulk[3] = 0;
}

/* 0x8002d190, size 0x174 */
signed char GetDefaultOption(int opt)
{
    switch (opt) {
    case 21:
        return 3;
    case 22:
        return 5;
    case 0:
        return 1;
    case 23:
        return 1;
    case 1:
        return 1;
    case 2:
        return 1;
    case 3:
        return 1;
    case 4:
        return 1;
    case 5:
        return 1;
    case 6:
        return 1;
    case 7:
        return 1;
    case 8:
        return 1;
    case 24:
        return 2;
    case 25:
        return 0;
    case 26:
        return 2;
    case 27:
        return 0;
    case 28:
        return 0;
    case 29:
        return 0;
    case 30:
        return 0;
    case 9:
        return 0;
    case 31:
        return 0;
    case 10:
        return 1;
    case 11:
        return 1;
    case 12:
        return 10;
    case 13:
        return 10;
    case 14:
        return 18;
    case 15:
        return 0;
    case 16:
        return gLang;
    case 17:
        return 1;
    case 18:
        return 1;
    case 19:
        return 0;
    case 20:
        return 0;
    case 32:
        return 0;
    case 33:
        return 0;
    case 35:
        return 5;
    case 34:
        return 69;
    case 36:
        return 30;
    case 37:
        return 0;
    case 39:
        return 8;
    case 38:
        return 10;
    case 40:
        return 0;
    }
    return 0;
}

/* 0x8002d304, size 0x5fc */
void SetOptions(void)
{
    int i; /* r26 */
    int j; /* r27 */

    if (!(gGameState & 8)) {
        for (i = 0; i < gNumPlayers; i++) {
            if (gPlayers[i].save) {
                for (j = 0; j < 21; j++) {
                    SavePlayerOption(gPlayers[i].save, j, gOptions[j]);
                }
            }
        }
    }
    gShowTrackMap = gOptions[0];
    gShowRadar = gOptions[1];
    gShowElapsed = gOptions[2];
    gShowRemaining = gOptions[3];
    gShowTach = gOptions[4];
    gShowSpeed = gOptions[5];
    gShowPlace = gOptions[7];
    gShowShifter = gOptions[8];
    gShowMetric = gOptions[9];
    gShowPlayerArrows = gOptions[10];
    gShowWrongWay = gOptions[18];
    gNoAutoAbort = gCheatState[4];
    gInvincible = gCheatState[6];
    gNoBrakes = gCheatState[9];
    gSpecialTires = gCheatState[10];
    gMassiveMass = gCheatState[11];
    gSuicideMode = gCheatState[12];
    gSuperSpeed = gCheatState[5];
    gResurrectInPlace = gCheatState[19];
    if (gGameState & 8) {
        gNumLaps = 3;
        gNumDrones = 6;
        gBackwards = 0;
        gMirrorWaves = 0;
        gMirrorControls = 0;
        gFogIntensity = 0;
        gWind = 0;
        gGravityScale = 1;
        gDifficulty = 2;
        gCatchup = 2;
        gDeathMode = 0;
    } else {
        switch (gRace) {
        case 0:
            gNumLaps = gOptions[21];
            gNumDrones = gOptions[22];
            if (gNumPlayers > 2) {
                gNumDrones = 0;
            }
            gBackwards = gOptions[27];
            gMirrorWaves = gOptions[28] > 0;
            gMirrorControls = gOptions[28] > 1;
            gFogIntensity = gOptions[29];
            gWind = gOptions[30];
            gGravityScale = gOptions[23];
            gDifficulty = gOptions[24];
            gCatchup = gOptions[26];
            gDeathMode = gOptions[31];
            gShowOdometer = gOptions[6];
            gShowCoins = gOptions[11];
            break;
        case 1:
            gNumLaps = 1;
            gNumDrones = 0;
            gBackwards = gOptions[27];
            gMirrorWaves = gOptions[28] > 0;
            gMirrorControls = gOptions[28] > 1;
            gFogIntensity = gOptions[29];
            gWind = gOptions[30];
            gGravityScale = gOptions[23];
            gDifficulty = gOptions[24];
            gCatchup = gOptions[26];
            gDeathMode = 0;
            gShowOdometer = gOptions[6];
            gShowCoins = gOptions[11];
            break;
        case 2:
            gNumLaps = 3;
            gNumDrones = 0;
            gBackwards = gOptions[27];
            gMirrorWaves = 0;
            gMirrorControls = 0;
            gFogIntensity = gOptions[29];
            gWind = 0;
            gGravityScale = 1;
            gDifficulty = 2;
            gGhostSelect = gOptions[25];
            gCatchup = 0;
            gDeathMode = 0;
            gShowOdometer = gOptions[6];
            gShowCoins = gOptions[11];
            gNoAutoAbort = 0;
            gInvincible = 0;
            gNoBrakes = 0;
            gSpecialTires = 0;
            gMassiveMass = 0;
            gSuicideMode = 0;
            gSuperSpeed = 0;
            gResurrectInPlace = 0;
            break;
        case 3:
            gNumLaps = 3;
            gNumDrones = 5;
            gBackwards = gCircuitTracks[gCircuitTrack].backward;
            gMirrorWaves = gCircuitTracks[gCircuitTrack].mirror;
            gMirrorControls = 0;
            gFogIntensity = gCircuitTracks[gCircuitTrack].fog;
            gWind = gCircuitTracks[gCircuitTrack].wind;
            gGravityScale = 1;
            gDifficulty = (*(*gPlayers[gCircuitPlayer].save)->datah)->circuit.difficulty;
            gCatchup = 2;
            gDeathMode = (*(*gPlayers[gCircuitPlayer].save)->datah)->circuit.death;
            gShowOdometer = gOptions[6];
            gShowCoins = gOptions[11];
            break;
        case 4:
            gNumLaps = 1;
            gNumDrones = 0;
            gBackwards = 0;
            gMirrorWaves = gOptions[28] > 0;
            gMirrorControls = gOptions[28] > 1;
            gFogIntensity = gOptions[29];
            gWind = gOptions[30];
            gGravityScale = gOptions[23];
            gDifficulty = 0;
            gCatchup = 0;
            gDeathMode = 0;
            gStuntMode = gOptions[32];
            gStuntFlow = gOptions[33];
            gStuntModePoints = gOptions[34];
            gStuntModeMins = gOptions[35];
            gStuntModeDeaths = gOptions[36];
            gShowTrackMap = 0;
            gShowOdometer = 0;
            gShowCoins = gOptions[11];
            break;
        case 5:
            gNumLaps = 1;
            gNumDrones = 0;
            gBackwards = 0;
            gMirrorWaves = gOptions[28] > 0;
            gMirrorControls = gOptions[28] > 1;
            gFogIntensity = 0;
            gWind = 0;
            gGravityScale = gOptions[23];
            gDifficulty = 0;
            gCatchup = 0;
            gDeathMode = 0;
            gShowTrackMap = 0;
            gShowOdometer = 0;
            gShowCoins = 0;
            break;
        case 6:
            gNumLaps = 1;
            gNumDrones = 0;
            gBackwards = 0;
            gMirrorWaves = gOptions[28] > 0;
            gMirrorControls = gOptions[28] > 1;
            gFogIntensity = gOptions[29];
            gWind = gOptions[30];
            gGravityScale = gOptions[23];
            gDifficulty = 0;
            gCatchup = 0;
            gDeathMode = 0;
            gShowSpeed = 0;
            gBattleMode = gOptions[37];
            gBattleModePoints = gOptions[38];
            gBattleModeMins = gOptions[39];
            gShowTrackMap = 0;
            gShowOdometer = 0;
            gShowCoins = 0;
            break;
        }
    }
    if (gGameState & 0x7c03fffe) {
        gMirrorWaves = 0;
        gMirrorControls = 0;
    }
}

