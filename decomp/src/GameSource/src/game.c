/* game.c - Main game flow and controller warnings. Status: NonMatching.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\game.c; assembly: build/2049LE/asm/GameSource/src/game.s */
#define gravityArray game_header_gravityArray
#include "rush/rush_protos.h"
#undef gravityArray

static void _CheckControls(void);
static void _UpdateGame(void);
static void _EndGame(void);
static void _CheckButtons(void);
static void _CalcPolePositions(void);
static int _animScrollText(void);
static void _DrawNoControllers(void);

const float gravityArray[4] = { 1.0f, 1.15f, 1.3f, 1.5f };
unsigned char gFogColors[27][3] = {
    150, 150, 190, 150, 150, 190, 140, 140, 140, 0, 0, 0, 140, 140, 140, 0, 0, 0, 50, 50, 50, 150,
    150, 190, 150, 150, 190, 0, 0, 0, 150, 150, 190, 150, 150, 190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 150, 150, 190, 50, 50, 50, 0, 0, 0, 0, 0, 0, 150, 0, 0, 0, 150, 0, 0, 0, 190, 150, 150,
    0, 255, 255, 255, 150, 100, 0, 180, 0, 100
};
int playerTextColor[4] = { 7, 5, 14, 12 };
anon_c_rgba_4 flashColors[2] = { 224, 224, 0, 255, 0, 0, 0, 255 };
enum_kNumRaceTypes_8 gRace;
enum_kNumStuntModes gStuntMode;
enum_kNumStuntFlows gStuntFlow;
int gStuntModeMins;
int gStuntModePoints;
int gStuntModeDeaths;
enum_kNumBattleModes gBattleMode;
int gBattleModeMins;
int gBattleModePoints;
signed char gCoastFlag;
signed char gOutOfTime;
signed char gEndGame;
signed char gRetireGame;
signed char gDifficulty;
signed char gGhostSelect;
signed short gNumDrones;
signed short gNumLaps;
signed char gDeathMode;
signed char gBackwards;
signed char gFogColor;
unsigned int gFrames;
int g_bIfReload;
signed char carsetup_flag[6];
int gAPlayerReallyFinished;
_tBlit_2 *inactiveBlit;
static int _PAD_Pause;
static _tBlit_2 *_PAD_ScrollBlit;
signed char gGamePointPlayed;
signed char gRestartGame;
anon_track_car_a gLink[13];

/* 0x8001407c, size 0x200 */
int GameLoop(void)
{
    int renderFlag;
    static signed char sInited;
    static signed char gPauseAllowed;
    renderFlag = 0;
    statsClock(6);
    if (!sInited) {
        gFrames = 0;
        sInited = 1;
    }
    OncePerFrame();
    getGameStateUpdate();
    updateGameState();
    if (g_bStateJustUpdated) NGCResetPadStatus();
    if (gGameState & 0x7c03fffe) {
        if (!gSelPauseState || g_bStateJustUpdated) {
            updateFrontEnd();
            if (!g_bStateJustUpdated) updateSelectPauseScreen();
        } else updateSelectPauseScreen();
    }
    if ((gGameState & 0x200000) || (gGameState & 0x400000)) {
        if (gPauseAllowed) {
            if (!gSelPauseState) {
                updatePauseScreen();
                updateSelectPauseScreen();
            } else updateSelectPauseScreen();
        }
    } else gPauseAllowed = 0;
    if (!(gGameState & 0x200000) || gPauseAllowed) _CheckControls();
    if ((gGameState & 0x200000) || (gGameState & 0x400000)) gPauseAllowed = 1;
    if (((gGameState & 0x200000) || (gGameState & 0x400000)) && (gPauseState || gSelPauseState)) {
        _DrawNoControllers();
        UpdateGraphics(1, 1);
        statsUnclock(6);
        return 1;
    }
    if (gGameState & 0x3fc0000) {
        if (gGameState & 0x7c0000) _UpdateGame();
        else if (!gSelPauseState) {
            _UpdateGame();
            getGameStateUpdate();
            if (!g_bStateJustUpdated) updateSelectPauseScreen();
        } else updateSelectPauseScreen();
    }
    if (gNewGameState == gGameState) {
        _DrawNoControllers();
        UpdateGraphics(1, 1);
        renderFlag = 1;
    } else Fake_EndFrame();
    gFrames++;
    statsUnclock(6);
    return renderFlag;
}

/* 0x8001427c, size 0x2c */
void OncePerFrame(void)
{
    UpdateFrameTimer();
    UpdateRushControls();
    PS2MCTick();
    HAL_CallOncePerFrameFuncs();
}

/* 0x800142a8, size 0x4 */
static void _CheckControls(void)
{
    /* The original body is empty. */
}

/* 0x800142ac, size 0xc */
void PauseControllerWarning(void)
{
    _PAD_Pause = 1;
}

/* 0x800142b8, size 0x34 */
void HideControllerWarning(void)
{
    if (_PAD_ScrollBlit) {
        RemoveBlit((_tBlit *)_PAD_ScrollBlit);
        _PAD_ScrollBlit = 0;
    }
}

/* 0x800142ec, size 0x54 */
void UpdateFrame(void)
{
    _DrawNoControllers();
    UpdateGraphics(0, 0);
    _CheckControls();
    EndGraphics();
    checkForExit(1);
    if (gExitGame) ExitSystem();
    StartGraphics();
    OncePerFrame();
}

/* 0x80014340, size 0x9e0 */
static void _UpdateGame(void)
{
    int i; /* r5 */
    signed char allSolid; /* r30 */
    int p; /* r31 */
    int numDeaths; /* r7 */
    /* TODO */
}

/* 0x80014d20, size 0x624 */
int PrePlay(int useHud, int cancel)
{
    int i; /* r28 */
    int slot; /* r4 */
    int rrr; /* r0 */
    /* TODO */
}

/* 0x80015344, size 0x2ac */
void PlayGame(void)
{
    int i; /* r28 */
    signed short slot; /* stack */
    int camera; /* r5 */
    /* TODO */
}

/* 0x800155f0, size 0xcc */
void AbortGame(void)
{
    signed short i;
    ShowCountdown(0);
    RemoveHUD();
    RemoveMultiplayerHUD();
    for (i = 0; i < gNumActiveCars; i++) End_MDrive(model[i].slot);
    modelstop();
    StopListeners();
    RemoveEnvirons();
    AudStopAllEffects();
    if (gRestartGame) setGameState(0x40000);
    else {
        GhostEnd();
        UnloadTrack(gTrackNum);
        gNumWindows = 1;
        ZOID_SplitScreen(1);
        setGameState(4);
        updateGameState();
        updateTitleScreen();
    }
    ClearRushControls();
}

/* 0x800156bc, size 0xa4 */
static void _EndGame(void)
{
    signed short i;
    for (i = 0; i < gNumActiveCars; i++) {
        End_MDrive(model[i].slot);
        gGameCar[i].appearance &= ~8;
        UpdateVisuals(gGameCar[i].slot);
    }
    modelstop();
    StopListeners();
    SetCountdownTimer(4.0f);
    setGameState(0x2000000);
}

/* 0x80015760, size 0x9c */
void CarSetup(void)
{
    signed short i;
    signed short slot;
    for (i = 0; i < gNumActiveCars; i++) {
        slot = model[i].slot;
        if (model[slot].data_valid) {
            if (!carsetup_flag[slot]) SetupCar(slot, model[slot].body_type, 1);
            carsetup_flag[slot] = 1;
            UpdateVisuals(slot);
        }
    }
}

/* 0x800157fc, size 0x20 */
static void _CheckButtons(void)
{
    signed short i;
    i = 0;
    goto test;
again:
    i++;
test:
    if (i < gNumPlayers) goto again;
}

/* 0x8001581c, size 0x208 */
static void _CalcPolePositions(void)
{
    int i;
    int j;
    int max_drones;
    unsigned char nextSlot;
    anon_track_car_a *lp;
    nextSlot = 0;
    max_drones = gNumDrones;
    if (max_drones > 6 - gNumPlayers) max_drones = 6 - gNumPlayers;
    if (gRace == kStunt || gRace == kObstacle || gRace == kBattle) max_drones = 0;
    gNumCars = gNumPlayers + max_drones;
    for (i = 0, lp = gLink; i < 6; i++, lp++) {
        if (gRace == kCircuit && i < gNumCars && gCircuitTrack > 0) {
            int slot;
            if (i == 0) slot = gCircuitPlayer;
            else slot = i == 1 ? gCircuitPlayer ^ 1 : i;
            lp->polepos = gCircuitCars[slot].totalPlace;
            lp->flags = 0xb0;
            if (i < gNumPlayers) lp->owner = 6;
            else lp->owner = 0;
        } else if (i < gNumPlayers) {
            lp->polepos = max_drones++;
            lp->owner = 6;
            lp->flags = 0xb0;
        } else if (i < gNumCars) {
            if (gRace == kCircuit) {
                for (;;) {
                    lp->polepos = Random(gNumCars);
                    for (j = 0; j < i; j++) if (lp->polepos == gLink[j].polepos) break;
                    if (i == j) break;
                }
            } else lp->polepos = nextSlot++;
            lp->owner = 0;
            lp->flags = 0xb0;
        } else {
            lp->owner = 7;
            lp->flags = 0;
        }
        lp->track = gTrackNum;
    }
}

/* 0x80015a24, size 0x16c */
static int _animScrollText(void)
{
    char str[256];
    static char *sContName1[4] = { "1", "2", "3", "4" };
    static char *sContName2[4] = { "1-A", "1-B", "1-C", "1-D" };
    static int x = 320;
    static int i;
    static unsigned long vsync;
    int numPlayers;
    numPlayers = gRace == kGhost ? 1 : gNumPlayers;
    for (i = 0; i < numPlayers; i++) if (!gControllerData[i].attached) break;
    if (i == numPlayers) return 1;
    sprintf(str, "THE CONTROLLER IN CONTROLLER PORT %s HAS BEEN REMOVED. REINSERT THE CONTROLLER TO CONTINUE WITH THE GAME",
            MtapConnected ? sContName2[i] : sContName1[i]);
    HAL_SetTextDepth(0.0f);
    HAL_SetFont(11);
    TB_SetTextColor(22);
    if (!(gGameState & 1) && !(gGameState & 2) && !(gGameState & 0x40000000) && !(gGameState & 4) && !(gGameState & 8))
        TB_DrawText(x, 194, str);
    if (vsync != GVSyncCount) {
        x--;
        vsync = GVSyncCount;
    }
    if (x < -(int)HAL_GetTextWidth(str, -1)) x = 320;
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80015b90, size 0xb8 */
static void _DrawNoControllers(void)
{
    static const anon_texname_dulx_24 scrollMultiBlit[1] = {
        (const char *)-1, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0,
        (int (*)(_tBlit *))_animScrollText, 0
    };
    static int i;
    int numPlayers;
    numPlayers = gRace == kGhost ? 1 : gNumPlayers;
    for (i = 0; i < numPlayers; i++) if (!gControllerData[i].attached) break;
    if (i >= numPlayers) {
        if (_PAD_ScrollBlit) {
            RemoveBlit((_tBlit *)_PAD_ScrollBlit);
            _PAD_ScrollBlit = 0;
        }
    } else if (!_PAD_ScrollBlit) {
        _PAD_ScrollBlit = (_tBlit_2 *)NewMultiBlit(0, 0, scrollMultiBlit, 1);
    }
}

