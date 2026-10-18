/* demo.c - attract-mode demo race.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\demo.c; assembly: build/2049LE/asm/GameSource/src/demo.s */
#include "rush/rush_protos.h"

static int animText(void);

static const anon_texname_dulx_24 multiblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static signed short gCurrentDemoTrack = -1;
/* explicit `= 0`: emitted at the declaration point, before the function-local statics (.sbss order) */
static signed char inited = 0;
static _tBlit *blit = 0;
static signed char preplayDone = 0;

static void initDemoGame(void);
static void uninitDemoGame(void);
static int animText(void);
static void playDemoGame(); /* no prototype: callers pass a second argument */
static void endDemoGame(void);
static void nextAvailDemoTrack(void);

/* 0x8000e814, size 0xb4 */
void updateDemoGame(void)
{
    if (!inited) {
        initDemoGame();
    }
    playDemoGame(1, 0);
    if (gSoftReset && preplayDone) {
        uninitDemoGame();
        setGameState(4);
        return;
    }
    if (((gActiveEdges & 3) && preplayDone) || CountdownTimeOut()) {
        uninitDemoGame();
        if (gCurrentDemoTrack == 5 && CountdownTimeOut()) {
            setGameState(2);
        } else {
            setGameState(4);
        }
    }
}

/* 0x8000e8c8, size 0x50 */
static void initDemoGame(void)
{
    blit = NewMultiBlit(0, 0, multiblit, 1);
    SetCountdownTimer(30.0f);
    nextAvailDemoTrack();
    inited = 1;
    ClearRushControls();
}

/* 0x8000e918, size 0x4c */
static void uninitDemoGame(void)
{
    if (blit) {
        RemoveBlit(blit);
        blit = 0;
    }
    playDemoGame(0, 0);
    HideControllerWarning();
    inited = 0;
}

/* 0x8000e964, size 0x84 */
static int animText(void)
{
    HAL_SetTextDepth(0.0f);
    TB_SetJust(1, 1);
    HAL_SetFont(11);
    TB_SetTextColor(22);
    TB_DrawTextScrollBox(160, 188, 320, 112, -1, 0, "PRESS START");
    TB_SetTextColor(1);
    TB_SetJust(0, 3);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x8000e9e8, size 0xd8 */
static void playDemoGame(int show)
{
    static signed char gDemoInited;

    if (show) {
        if (!gDemoInited) {
            gDemoInited = 1;
            gNumWindows = 1;
            if (!(gGameState & 0x2000000)) {
                gRace = 0;
                fake_init(gCurrentDemoTrack);
            }
        }
        if (!preplayDone && PrePlay(gGameState & 8, 0)) {
            CarSetup();
            Update_MDrive();
            InitCPTime();
            preplayDone = 1;
        } else if (preplayDone) {
            PlayGame();
        }
    } else {
        WaitTillDMAEnd();
        gDemoInited = 0;
        preplayDone = 0;
        endDemoGame();
        if (!(gGameState & 0x2000000)) {
            UnloadTrack(gTrackNum);
        }
        ClearAlphaList();
    }
}

/* 0x8000eac0, size 0xc0 */
static void endDemoGame(void)
{
    signed short i; /* r30 */
    for (i = 0; i < gNumActiveCars; i++) {
        End_MDrive(model[i].slot);
    }
    modelstop();
    StopListeners();
    RemoveHUD();
    RemoveEnvirons();
    for (i = 0; i < gNumActiveCars; i++) {
        RemoveCar(i);
    }
    gPlayers[0].slot = 0;
    gPlayers[1].slot = 1;
    gPlayers[2].slot = 2;
    gPlayers[3].slot = 3;
}

/* 0x8000eb80, size 0x30 */
static void nextAvailDemoTrack(void)
{
    gShowCredits = 0;
    gCurrentDemoTrack++;
    if (gCurrentDemoTrack > 5) {
        gCurrentDemoTrack = 0;
        gShowCredits = 1;
    }
}

