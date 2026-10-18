/* game.c - Main game flow and controller warnings. Status: Matching.
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
/* explicitly zero-initialised, in this order (.sbss layout; see writeup ngcBeginMenu/ngcVu1pack) */
signed char gRestartGame = 0;
signed char gGamePointPlayed = 0;
static _tBlit_2 *_PAD_ScrollBlit = 0;
static int _PAD_Pause = 0;
_tBlit_2 *inactiveBlit = 0;
anon_track_car_a gLink[13];

/* 0x8001407c, size 0x200 */
int GameLoop(void)
{
    int renderFlag;
    static signed char sInited = 0; /* explicit `= 0`: emitted at this point, in the target .sbss order */
    static signed char gPauseAllowed = 0; /* explicit `= 0`: emitted at this point, in the target .sbss order */
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

/* The original has seven unreferenced static functions here (controller-warning code, DWARF
 * `// Erased`): _animControllerDim, _animControllerText, _ShowControllerWarning,
 * ResumeControllerWarning, ShowControllerWarning, UpdateFrameForPad, UpdateFrameForMC.
 * CodeWarrior parses them (numbering their float literals) but emits no code, data or strings
 * for them, so the .sdata2 pool starts with the literals they mention: 0.0f, -1.0f, the
 * int->float constant, 3.0f, 4.0f. These placeholders reproduce only that. */
/* placeholder for a linker-stripped function (no literals); kept only in its original position */
static int _animControllerDim(_tBlit *blt)
{
    return 1;
}

/* placeholder for a linker-stripped function: exists only so its literals (0.0f, -1.0f and the int->float constant) get the first .sdata2 counters */
static int _animControllerText(void)
{
    unsigned char str[40];
    HAL_SetTextDepth(0.0f);
    HAL_SetTextTranslucency((float)gNumPlayers);
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* placeholder for a linker-stripped function: exists only to number 3.0f and 4.0f before the live code */
static void _ShowControllerWarning(void)
{
    float time;
    /* only the two literals matter: 3.0f must be numbered before 4.0f */
    time = 3.0f;
    HAL_SetTextTranslucency(time);
    time = 4.0f;
    HAL_SetTextTranslucency(time);
}

/* placeholder for a linker-stripped function (no literals); kept only in its original position */
static void ResumeControllerWarning(void)
{
}

/* placeholder for a linker-stripped function (no literals); kept only in its original position */
static void ShowControllerWarning(void)
{
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

/* placeholder for a linker-stripped function (no literals); kept only in its original position */
static void UpdateFrameForPad(void)
{
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

/* placeholder for a linker-stripped function (no literals); kept only in its original position */
static void UpdateFrameForMC(void)
{
}

/* 0x80014340, size 0x9e0 */
static void _UpdateGame(void)
{
    static signed char startCountdown = 0; /* explicit `= 0`: emitted at this point, in the target .sbss order */
    int i;
    signed char allSolid;
    int p;
    int numDeaths;
    if (gGameState & 0x40000) {
        for (i = 0; i < gNumPlayers; i++) gPakInserted[i] = (*gPlayers[i].save)->fileh != 0;
        gSkipToGameState = 1;
        SetOptions();
        _CalcPolePositions();
        gUseCatchup = gCatchup;
        setGameState(0x80000);
        goto preplay;
    } else if (gGameState & 0x80000) {
    preplay:
        if (PrePlay(1, 0)) {
            startCountdown = 1;
            setGameState(0x200000);
            ClearRushControls();
            goto countdown;
        } else {
            SaveServer(0, 1);
            goto done;
        }
    } else if (gGameState & 0x200000) {
    countdown:
        SwitchFrameMode(1);
        if (gRestartGame) goto done;
        CarSetup();
        Update_MDrive();
        TextureAnimation();
        CheckTargetCollision(model[0].slot);
        if (startCountdown) {
            SetCountdownTimer(3.5f);
            init_model_task(3.5f * gRetraceRate);
            startCountdown = 0;
        }
        CountDown();
        _CheckButtons();
    } else if (gGameState & 0x100000) {
        if (gRace == kPractice) SetCountdownTimer(1800.0f);
        else if (gRace == kStunt) {
            if (gStuntMode == 0) SetCountdownTimer(gStuntModeMins * 60);
            else SetCountdownTimer(600.0f);
        } else if (gRace == kObstacle) SetCountdownTimer(300.0f);
        else if (gRace == kBattle) {
            if (gBattleMode == 1) SetCountdownTimer(gBattleModeMins * 60);
            else SetCountdownTimer(1200.0f);
        } else SetCountdownTimer(CalcRaceTime());
        PlayGameSound(84, 0, 1, 0);
        StartGameMusic();
        InitCPTime();
        setGameState(0x400000);
    } else if (gGameState & 0x400000) {
        DisableDrawFace();
        SwitchFrameMode(1);
        PlayGame();
        _CheckButtons();
        UpdateStationaryEmitters();
        if (gRetireGame) {
            FindWinner();
            setGameState(0x1000000);
            GhostEnd();
            RemoveHUD();
            goto done;
        }
        if (gCoastFlag) {
            allSolid = 1;
            for (p = 0; p < gNumPlayers; p++) {
                if (!gDeathMode && !AreWeSolid(p)) allSolid = 0;
            }
            if (are_all_cars_stopped() && allSolid) {
                if (gRace == kStunt) FindWinner();
                setGameState(0x1000000);
                GhostEnd();
                RemoveHUD();
            }
            if (!CountdownTimeOut() && gRace != kStunt && gRace != kObstacle && gRace != kBattle) {
                gCoastFlag = 0; gOutOfTime = 0;
            }
        } else if (gRace == kStunt) {
            switch (gStuntMode) {
            case 0:
                if (CountdownTimeOut()) { gCoastFlag = 1; gOutOfTime = 1; }
                break;
            case 1:
                for (i = 0; i < gNumPlayers; i++) {
                    if (gStuntPlayer[i].totalPoints >= gStuntModePoints) { gCoastFlag = 1; gOutOfTime = 1; }
                }
                break;
            case 2:
                numDeaths = 0;
                for (i = 0; i < gNumPlayers; i++) {
                    if (gGameCar[i].player->numCrashes >= gStuntModeDeaths) { gGameCar[i].dead = 1; numDeaths++; }
                }
                if (numDeaths == gNumPlayers) { gCoastFlag = 1; gOutOfTime = 1; }
                break;
            }
        } else if (gRace == kBattle) {
            switch (gBattleMode) {
            case 1:
                if (CountdownTimeOut()) { FindWinner(); gCoastFlag = 1; gOutOfTime = 1; }
                break;
            case 0:
                memset(gTeamPoints, 0, 4);
                for (i = 0; i < gNumPlayers; i++) gTeamPoints[gCarTeam[i]] += gGameCar[i].battle.Points;
                for (p = 0; p < 4; p++) {
                    if (!gGamePointPlayed && gTeamPoints[p] == gBattleModePoints - 1) {
                        gGamePointPlayed = 1;
                        PlayGameSound(54, 0, 2, 0);
                    }
                    if (gTeamPoints[p] >= gBattleModePoints) {
                        FindWinner(); gCoastFlag = 1; gOutOfTime = 1;
                    }
                }
                break;
            }
        } else if (gRace == kOneRace || gRace == kPractice) {
            for (i = 0; i < gNumPlayers; i++) if (!gGameCar[gPlayers[i].slot].place_locked) goto done;
            gAPlayerReallyFinished = 0;
            for (i = 0; i < gNumPlayers; i++) if (gNumLaps == model[i].laps) gAPlayerReallyFinished = 1;
            if (!gCoastFlag) {
                if (gDeathMode) gDeathWaitEnd = 0.0f;
                gCoastFlag = 1;
                FindWinner();
                AudSetMusicVol(0.0f, 3.0f);
            }
        } else {
            if (CountdownTimeOut()) {
                if (!gCoastFlag) {
                    gCoastFlag = 1;
                    FindWinner();
                    AudSetMusicVol(0.0f, 3.0f);
                }
                for (i = 0; i < gNumPlayers; i++) {
                    if (gNumLaps != model[gPlayers[i].slot].laps) {
                        if (gRace != kGhost || !gGhosts[gPlayers[i].slot] || (*(*gGhosts[gPlayers[i].slot])->header)->flag < 0)
                            PlayGameSound(87, 0, 1, 0);
                        gOutOfTime = 1;
                        break;
                    }
                }
            }
        }
    } else if (gGameState & 0x1000000) {
        EnableDrawFace();
        _EndGame();
        SwitchFrameMode(0);
    } else if (gGameState & 0x2000000) updateHighScoreScreen();
    else if (gGameState & 0x800000) updateGameOverScreen();
    done:
    if (gGameState & 0x600000) {
        for (i = 0; i < gNumWindows; i++) WorldFogDensity(i, gCamera[i].mat.uvs[3]);
    }
}

/* 0x80014d20, size 0x624 */
int PrePlay(int useHud, int cancel)
{
    static signed char inited = 0; /* explicit `= 0`: emitted at this point, in the target .sbss order */
    int i, slot, rrr;
    if (cancel) { inited = 0; return 0; }
    if (!inited) {
        inited = 1;
        SetOptions();
        EnableGameSounds(!(gGameState & 8) && !(gGameState & 0x2000000));
        if (gGameState & 0x2000000) {
            /* volatile stores keep the three clears in the original order (header keeps the counts plain) */
            *(volatile short *)&num_our_drones = 0;
            *(volatile short *)&num_drones = 0;
            *(volatile short *)&num_humans = 0;
            for (i = 0; i < gNumPlayers; i++) {
                slot = gPlayers[i].slot;
                gLink[slot].owner -= 6;
                gGameCar[gPlayers[i].slot].dead = 0;
                model[gPlayers[i].slot].drone_type = 1;
                model[gPlayers[i].slot].we_control = 1;
                model[gPlayers[i].slot].autotrans = 1;
                gGameCar[gPlayers[i].slot].place_locked = 0;
                gGameCar[gPlayers[i].slot].player = 0;
                model[gPlayers[i].slot].appearance = 0;
                gGameCar[gPlayers[i].slot].appearance = 0;
                SetupCar(gPlayers[i].slot, model[gPlayers[i].slot].body_type, 1);
                drones[num_drones++] = i;
                our_drones[num_our_drones++] = i;
            }
            for (; i < gNumActiveCars; i++) {
                gGameCar[i].dead = 0;
                gGameCar[i].place_locked = 0;
                model[i].appearance = 0;
                gGameCar[i].appearance = 0;
                SetupCar(i, model[i].body_type, 1);
                drones[num_drones++] = i;
                our_drones[num_our_drones++] = i;
            }
            sched_index = -1;
            sched_to_use = num_our_drones;
            new_sched_to_use = num_our_drones;
            cur_drone = 0;
            _CalcPolePositions();
            InitCPS();
            InitDrones();
            gCoastFlag = 0;
            gRetireGame = 0;
            gGravity = 32.2f;
            for (i = 0; i < gNumActiveCars; i++) Init_MDrive(0, model[i].slot);
            ShowHUD();
            inited = 0;
            return 1;
        }
        HFOV = 90.0f;
        if (!(gGameState & 8)) AudKillSong();
        if (gRestartGame) { ZOID_Reset(); InitBlits(); }
        else if (!(gGameState & 8)) InitWaves();
        munge_gLink_data();
        GhostStart();
        ResetHighScores();
    }
    if (gFogColor < 19) gFogColor = gTrackNum;
    if (!(gGameState & 8)) {
        HFOV = 90.0f;
        ZOID_ResetWindows();
        gNumWindows = gRace == kGhost ? (short)1 : gNumPlayers; ZOID_SplitScreen(gNumWindows);
    }
    g_bIfReload = 0;
    if (gRestartGame) { g_bIfReload = 1; ReloadTrack(); }
    else {
        static int bPadInited = 0; /* explicit `= 0`: emitted at this point, in the target .sbss order */
        if (!bPadInited) { ChooseNewControl(0); bPadInited = 1; }
        leStartLoadingEffect();
        lePrepareLoading();
        HAL_SetFont(13); HAL_SetFont(10); HAL_SetFont(11); HAL_SetFont(12);
        HAL_SetFont(4); HAL_SetFont(1); HAL_SetFont(2); HAL_SetFont(3);
        HAL_SetFont(9); HAL_SetFont(8); HAL_SetFont(5); HAL_SetFont(6); HAL_SetFont(7);
        rrr = LoadTrack(gTrackNum, 1, 0);
        leEndLoadingEffect();
        if (!rrr) return 0;
    }
    g_bIfReload = 0;
    InitTextureAnimation();
    gRestartGame = 0;
    for (i = 0; i < gNumWindows; i++) {
        ZOID_SetFog(0, 0, gFogColors[gFogColor][0], gFogColors[gFogColor][1], gFogColors[gFogColor][2], 255, i);
        WorldFogDensity(i, gCamera[i].mat.uvs[3]);
    }
    if (!(gGameState & 8)) AudStopAllEffects();
    InitCPS();
    init_cars();
    for (i = 0; i < 6; i++) carsetup_flag[i] = 0;
    InitDrones();
    gCoastFlag = 0; gRetireGame = 0; gOutOfTime = 0;
    gGravity = 32.2f * gravityArray[gGravityScale];
    for (i = 0; i < gNumActiveCars; i++) {
        slot = model[i].slot;
        if (gLink[slot].owner == 0 || gLink[slot].owner == 6) Init_MDrive(0, slot);
    }
    StartEnvironment();
    init_views();
    if (useHud) ShowHUD();
    for (i = 0; i < gNumPlayers; i++) {
        gPlayers[i].numCrashes = 0;
        gGameCar[i].battle.health = 800;
        gGameCar[i].battle.Points = 0;
        gGameCar[i].battle.reloadingTime = 0.0f;
        if (gRace == kBattle) InitBattlePlayer((signed short)i);
        gGamePointPlayed = 0;
    }
    if (!(gGameState & 8)) ClearRushControls();
    StartListeners();
    if (gRace == kBattle) { InitBattlePoints(); ShotInit(); }
    if (gRace == kStunt) InitStunt();
    if (!(gGameState & 8)) PS_RumbleGo();
    inited = 0;
    gUpsideDownTrack = gCheatState[3];
    return 1;
}

/* 0x80015344, size 0x2ac */
void PlayGame(void)
{
    int i;
    signed short slot;
    int camera;
    Update_MDrive();
    if (gRace == kStunt) CheckForStunt();
    else if (gRace == kBattle) { ShotUpdate(); UpdateBattlePowerUps(); }
    else {
        if (gRace == kObstacle) GetResPt();
        else DoDrones();
        CheckCPs();
    }
    UpdateSmoke();
    UpdateArrowVis();
    for (i = 0; i < gNumActiveCars; i++) {
        /* volatile reads: the target loads restart here and again in the else-if below instead of reusing the first load */
        signed char restart = *(volatile signed char *)&gGameCar[i].restart;
        slot = model[i].slot;
        if (restart < 0) {
            camera = gGameCar[i].camera;
            if (!(gGameState & 8) && slot < gNumPlayers) gPlayers[slot].numCrashes++;
            if (camera >= 0) {
                gGameCar[i].saveview = gGameCar[i].view;
                gGameCar[i].view = 4;
                gCamera[camera].fcam.mat.uvs[3][0] = gCamera[camera].mat.uvs[3][0];
                gCamera[camera].fcam.mat.uvs[3][1] = gCamera[camera].mat.uvs[3][1];
                gCamera[camera].fcam.mat.uvs[3][2] = gCamera[camera].mat.uvs[3][2];
            }
            StartBlast(&slot, 1, 1.0f, 0);
            StopCarSounds(&model[i]);
            if (model[i].drone_type == 2) {
                if (gRace != kGhost || !gGhosts[model[i].slot] || (*(*gGhosts[model[i].slot])->header)->flag < 0)
                    PlayGameSound(9, model[i].slot, 1, 1);
            } else EmitGameSound(model[i].rwr, unitmatrix[3], 400.0f, 0.0f, 1.0f, 0.0f, 9, model[i].slot, 0, 128);
            if (gRace == kStunt) CrashedStunt(slot);
            gGameCar[i].restart = 0;
        } else if (*(volatile signed char *)&gGameCar[i].restart > 0) {
            InitSmokeBlast(slot);
            if (gGameCar[i].camera >= 0) {
                gGameCar[i].view = gGameCar[i].saveview;
                SetCameraView(gGameCar[i].camera);
            }
            gGameCar[i].restart = 0;
        }
        UpdateVisuals(slot);
        if (gLink[slot].owner == 0 || gLink[slot].owner == 6) {
            if (gRace != kGhost || (gRace == kGhost && slot == 0)) CheckTargetCollision(slot);
        }
    }
    TextureAnimation();
    DetectDroneAbort();
    if (!gRestartGame) ShowHUD();
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
    static int i; /* left uninitialised on purpose: deferred to the end of .sbss in reverse declaration order, after the explicitly zeroed vsync */
    static unsigned long vsync = 0; /* explicit `= 0`: emitted at this point, before both deferred i statics */
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
    static int i; /* left uninitialised on purpose: as the later declaration it is emitted before _animScrollText's i */
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

