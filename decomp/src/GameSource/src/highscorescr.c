/* highscorescr.c - high score screen: unlocked-stuff summary, circuit code, name entry, ghost save menu, score table.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\highscorescr.c; assembly: build/2049LE/asm/GameSource/src/highscorescr.s */
#include "rush/rush_protos.h"

/* same inline helper as videoscr.c: `160 - w - 8` folds to one subfic, the target has subfic 160; subi 8 */
static inline int centeredLeft(int halfWidth) { return 160 - halfWidth; }

static void _InitScreen(void);
static void _UninitScreen(void);
static void _SetHighScoreState(enum_kNumHighScoreStates newState);
static void _RankScores(void);
static void _PrintScores(void);
static signed short _HiScoreRank(float time, float *table);
static void _AddScore(void);
static int _GhostChanged(int pak);
static void _FindGhosts(int pak);
static void ControlUnlocked(void);
static void ControlCircuitCode(void);
static void ControlName(int p);
static void ControlGhost(void);
static void ControlScores(void);
static void DrawUnlocked(void);
static void DrawCircuitCode(void);
static void DrawName(int p);
static void DrawGhost(void);
static void DrawScores(void);
static int animArrowR(_tBlit_2 *blt);
static int animArrowL(_tBlit_2 *blt);
static int animText(void);
static int fixword(char *op, char *wp);

unsigned char *gDefaultNames[58] = {
    (unsigned char *)"MICROSERF", (unsigned char *)"YEAH BABY!", (unsigned char *)"EVAN J KELLY",
    (unsigned char *)"LITTLE BOO", (unsigned char *)"(-: TINA :-)", (unsigned char *)"EL",
    (unsigned char *)"ED LOGG", (unsigned char *)"GEL", (unsigned char *)"CAL",
    (unsigned char *)"RENE", (unsigned char *)"MERRILL", (unsigned char *)"REESER",
    (unsigned char *)"REESE", (unsigned char *)"DADDY-O", (unsigned char *)"MASON D",
    (unsigned char *)"SALTED SLUG", (unsigned char *)"GIO", (unsigned char *)"HANSON",
    (unsigned char *)"ROCKNROLL", (unsigned char *)"TODZILLA", (unsigned char *)"NED",
    (unsigned char *)"PEPSCHARMS@A", (unsigned char *)"HARVEY", (unsigned char *)"MAYA",
    (unsigned char *)"GREGMA", (unsigned char *)"CARA", (unsigned char *)"CHEPS",
    (unsigned char *)"JOSE E.", (unsigned char *)"BARRY L", (unsigned char *)"CHRIS L",
    (unsigned char *)"MICHAEL L", (unsigned char *)"JAMIE L", (unsigned char *)"THE JACKAL",
    (unsigned char *)"CPT AD'LSENT", (unsigned char *)"SCOTCHMO", (unsigned char *)"BIG UNIT",
    (unsigned char *)"ROTTWEILER", (unsigned char *)"SNIFF SNIFF", (unsigned char *)"KRUSER",
    (unsigned char *)"BIGTYMER JOF", (unsigned char *)"PIRU-DAMU-T", (unsigned char *)"SRAYER",
    (unsigned char *)"ISLND_KIDD", (unsigned char *)"RAVEN", (unsigned char *)"AXE_23",
    (unsigned char *)"DARTHRAIDER!", (unsigned char *)"STEELE", (unsigned char *)"MACHINE",
    (unsigned char *)"MEGABUSIVE", (unsigned char *)"CYRUS JACK", (unsigned char *)"MOUTH4WAR",
    (unsigned char *)"JAZWEL", (unsigned char *)"DSKWEEZ", (unsigned char *)"ISLANDXANGEL",
    (unsigned char *)"ORENGEJUSE", (unsigned char *)"SW3ET ROMEO", (unsigned char *)"GOOBLEMAN",
    (unsigned char *)"SHAWN P."
};
/* declared after gDefaultNames on purpose: that table's long string literals precede mblit in .rodata */
static const anon_texname_dulx_24 mblit[1] = { (void *)0xffffffff, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, (void *)&animText, 0 };
static anon_ll_fileh_4c **gLastPlayer[4] = { (void *)0xffffffff, (void *)0xffffffff, (void *)0xffffffff, (void *)0xffffffff };
static anon_x_y_c gPlayerRects[4][4] = {
    80, 55, 160, 110, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220, 160, 110, 0, 0, 320, 220,
    160, 110, 80, 0, 160, 110, 160, 55, 80, 110, 160, 110, 160, 165, 0, 0, 320, 220, 160, 110, 0,
    0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160, 110, 80,
    165, 0, 0, 320, 220, 160, 110, 0, 0, 160, 110, 80, 55, 160, 0, 160, 110, 240, 55, 0, 110, 160,
    110, 80, 165, 160, 110, 160, 110, 240, 165
};
/* explicit `= 0`: these three are the first .sbss symbols in forward order, so they are emitted at their declaration point; the rest stay uninitialised (deferred, reverse order) */
static signed char inited = 0;
static _tBlit_2 *blit = 0;
static signed char gTable = 0;
static enum_kNumHighScoreStates gHighScoreState;
static signed short gThisTrack;
static signed char gStuffUnlocked[5];
static signed char circuitCode;
static signed char gDrawName[4];
static signed short gNameSelect[4];
static signed short gNameLen[4];
static enum_kGhostMenuCount gGhostMenuSelect;
static enum_kGhostConfirmMenuCount gGhostConfirmMenuSelect;
static signed char gGhostPak;
static signed char gGhostMenuConflict;
static signed char gGhostMenuConfirm;
static signed char gYes;
static signed char gGhostMenuNotify;
static signed char gShostSaveFail;
static anon_ll_fileh_30 **gSavedGhost;
anon_ll_fileh_4c *gGhostPlayer;
static signed char gSaveGhostFile;
static anon_used_perm_20 *codePane;
static float gCameraMatrix[4][4];
static signed char gFlash[3][5];
static enum_kGhostNotSame gGhostStatus[4];
static anon_ll_fileh_30 **gPakGhosts[4];
static unsigned char gScoreTimes[5][10];
static unsigned char *gScoreNames[5];
static anon_ll_fileh_4c **gThisPlayer[4];
static anon_used_perm_20 *ghostPanes[3];
static anon_used_perm_20 *scorePanes[3];
static unsigned char gCircuitCode[32];
static signed char gPrevUnlockedFrames[4][6];
static signed char gPrevUnlockedTires[4][5];
static signed char gPrevUnlockedEngines[4][9];
static signed char gPrevUnlockedTrans[4][8];
static signed char gPrevUnlockedCircuits[4][4];
static signed char gPrevUnlockedCars[4][13];
static signed char gPrevUnlockedTracks[4][19];
anon_ll_fileh_4c gRealGhostPlayer;
static unsigned char gScoreName[4][13];
static anon_used_perm_20 *namePanes[4];

/* 0x8006fccc, size 0x1bc */
void updateHighScoreScreen(void)
{
    int exit; /* r28 */
    int p; /* r27 */

    if (!inited) {
        _InitScreen();
    }
    if (gSoftReset) {
        _SetHighScoreState(0);
        return;
    }
    switch (gHighScoreState) {
    case 1:
        ControlUnlocked();
        break;
    case 2:
        ControlCircuitCode();
        break;
    case 3:
        exit = 1;
        for (p = 0; p < gNumPlayers; p++) {
            if (gDrawName[p] == 1) {
                ControlName(p);
                exit = 0;
            } else if (namePanes[p]) {
                RemovePane(namePanes[p]);
                namePanes[p] = 0;
            }
        }
        if (exit == 1) {
            if (gRace == 2 && gGhostToSave) {
                TB_Strcpy((*gGhostToSave).name, gScoreName[0]);
                UnloadWaveAll(gTrackNum + 0x10f);
                gSavedGhost = CompressGhost(gGhostToSave, (unsigned char *)&gBlocksNeeded);
                gGhostToSave = 0;
                if (gSaveGhostFile && gSaveGhostFiles) {
                    _SetHighScoreState(4);
                } else {
                    SaveGhost(gGhostPak, &gGhostPlayer, gSavedGhost, 1);
                    gSavedGhost = 0;
                    _SetHighScoreState(5);
                }
            } else {
                _SetHighScoreState(5);
            }
        }
        break;
    case 4:
        SaveServer(0, 0);
        ControlGhost();
        break;
    case 5:
        ControlScores();
        break;
    }
}

/* 0x8006fe88, size 0x4bc */
static void _InitScreen(void)
{
    int p; /* r15 */
    int stuffUnlocked; /* stack */
    int oldCircuitTrack; /* r15 */
    unsigned char ranks[6]; /* stack */

    gSavedGhost = 0;
    stuffUnlocked = 0;
    if (gRace == 2) {
        gSaveGhostFile = (*gPlayers[0].save)->fileh != 0; /* stored second in the target, but written first: the other order blocks hoisting the *save loads (possible alias) */
        gGhostPlayer = &gRealGhostPlayer;
        memcpy(gGhostPlayer, *gPlayers[0].save, sizeof(anon_ll_fileh_4c));
        gGhostPlayer->ll.prev = 0;
        gGhostPlayer->ll.next = 0;
        gGhostPlayer->fileh = 0;
        gGhostPlayer->datah = 0;
    }
    AudKillSong();
    if (gOptions[14] <= 18) {
        AudPlaySong(18, 1);
        AudSetMusicVol(gOptions[12] / 10.0f, 0.05f);
    }
    memcpy(gPrevUnlockedTracks, gUnlockedTracks, sizeof(gPrevUnlockedTracks));
    memcpy(gPrevUnlockedCars, gUnlockedCars, sizeof(gPrevUnlockedCars));
    memcpy(gPrevUnlockedCircuits, gUnlockedCircuits, sizeof(gPrevUnlockedCircuits));
    memcpy(gPrevUnlockedTrans, gUnlockedTrans, sizeof(gPrevUnlockedTrans));
    memcpy(gPrevUnlockedEngines, gUnlockedEngines, sizeof(gPrevUnlockedEngines));
    memcpy(gPrevUnlockedTires, gUnlockedTires, sizeof(gPrevUnlockedTires));
    memcpy(gPrevUnlockedFrames, gUnlockedFrames, sizeof(gPrevUnlockedFrames));
    RecordGameOver();
    gHighScoreState = 0;
    inited = 1;
    circuitCode = 0;
    if (gRace == 3 && gGameCar[gCircuitPlayer].place_locked) {
        for (p = 0; p < 6; p++) {
            ranks[p] = gGameCar[p].place;
        }
        oldCircuitTrack = gCircuitTrack;
        UpdateCircuit(ranks, gGameCar[gCircuitPlayer].score);
        gCircuitTrack = oldCircuitTrack;
        if (!(*gPlayers[gCircuitPlayer].save)->fileh && gCircuitTrack < gCircuitSize - 1) {
            circuitCode = 1;
            GetCircuitCode(gCircuitCode);
        }
    }
    CalcUnlockedStuff();
    for (p = 0; p < gNumPlayers; p++) {
        gStuffUnlocked[p] = memcmp(gPrevUnlockedTracks[p], gUnlockedTracks[p], 19) ||
                            memcmp(gPrevUnlockedCars[p], gUnlockedCars[p], 13) ||
                            memcmp(gPrevUnlockedCircuits[p], gUnlockedCircuits[p], 4) ||
                            memcmp(gPrevUnlockedTrans[p], gUnlockedTrans[p], 8) ||
                            memcmp(gPrevUnlockedEngines[p], gUnlockedEngines[p], 9) ||
                            memcmp(gPrevUnlockedTires[p], gUnlockedTires[p], 5) ||
                            memcmp(gPrevUnlockedFrames[p], gUnlockedFrames[p], 6);
        if (gStuffUnlocked[p]) {
            stuffUnlocked = 1;
        }
        gDrawName[p] = 0;
    }
    gThisTrack = gTrackNum + (gBackwards ? 6 : 0);
    if (gRace == 0 || gRace == 2 || gRace == 3) {
        _RankScores();
    }
    ClearRushControls();
    SaveServer(1, 0);
    SaveFlush(0);
    if (stuffUnlocked) {
        _SetHighScoreState(1);
    } else if (circuitCode) {
        _SetHighScoreState(2);
    } else if (gRace == 0 || gRace == 3 || gRace == 2) {
        _SetHighScoreState(3);
    } else {
        _UninitScreen();
    }
}

/* 0x80070344, size 0x144 */
static void _UninitScreen(void)
{
    int i; /* r28 */
    int p; /* r28 */

    if (gRace == 2 && gSavedGhost) {
        PS_LockFile((*gSavedGhost)->fileh, 0);
    }
    if (codePane) {
        RemovePane(codePane);
        codePane = 0;
    }
    for (p = 0; p < gNumPlayers; p++) {
        if (namePanes[p]) {
            RemovePane(namePanes[p]);
            namePanes[p] = 0;
        }
    }
    for (i = 0; i < 3; i++) {
        if (scorePanes[i]) {
            RemovePane(scorePanes[i]);
            scorePanes[i] = 0;
        }
    }
    for (i = 0; i < 3; i++) {
        if (ghostPanes[i]) {
            RemovePane(ghostPanes[i]);
            ghostPanes[i] = 0;
        }
    }
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    inited = 0;
    setGameState(0x800000);
}

/* 0x80070488, size 0x610 */
static void _SetHighScoreState(enum_kNumHighScoreStates newState)
{
    if (gHighScoreState == newState) {
        return;
    }
    if (blit) {
        RemoveBlit((_tBlit *)blit);
        blit = 0;
    }
    switch (gHighScoreState) {
    case 1:
        {
            int p; /* r27 */

            for (p = 0; p < gNumPlayers; p++) {
                if (namePanes[p]) {
                    RemovePane(namePanes[p]);
                    namePanes[p] = 0;
                }
            }
        }
        break;
    case 2:
        if (codePane) {
            RemovePane(codePane);
            codePane = 0;
        }
        break;
    case 3:
        {
            int p; /* r27 */

            for (p = 0; p < gNumPlayers; p++) {
                if (namePanes[p]) {
                    RemovePane(namePanes[p]);
                    namePanes[p] = 0;
                }
            }
        }
        break;
    case 4:
        {
            int i; /* r27 */

            for (i = 0; i < 3; i++) {
                if (ghostPanes[i]) {
                    RemovePane(ghostPanes[i]);
                    ghostPanes[i] = 0;
                }
            }
        }
        break;
    case 5:
        {
            int i; /* r27 */

            for (i = 0; i < 3; i++) {
                if (scorePanes[i]) {
                    RemovePane(scorePanes[i]);
                    scorePanes[i] = 0;
                }
            }
        }
        break;
    }
    gHighScoreState = newState;
    switch (newState) {
    case 0:
        _UninitScreen();
        break;
    case 1:
        {
            int p; /* r27 */

            for (p = 0; p < gNumPlayers; p++) {
                if (!namePanes[p]) {
                    namePanes[p] = InitPane(gPlayerRects[gNumWindows - 1][p].x, gPlayerRects[gNumWindows - 1][p].y,
                                            gPlayerRects[gNumWindows - 1][p].x + gPlayerRects[gNumWindows - 1][p].w,
                                            gPlayerRects[gNumWindows - 1][p].y + gPlayerRects[gNumWindows - 1][p].h,
                                            176, 0, 0, 0);
                }
            }
        }
        SetCountdownTimer(600.0f);
        break;
    case 2:
        codePane = InitPane(83, 72, 237, 138, 192, 0, 0, 0);
        SetCountdownTimer(600.0f);
        break;
    case 3:
        {
            int p; /* r27 */

            for (p = 0; p < gNumPlayers; p++) {
                if (!namePanes[p]) {
                    namePanes[p] = InitPane(gPlayerRects[gNumWindows - 1][p].x, gPlayerRects[gNumWindows - 1][p].y,
                                            gPlayerRects[gNumWindows - 1][p].x + gPlayerRects[gNumWindows - 1][p].w,
                                            gPlayerRects[gNumWindows - 1][p].y + gPlayerRects[gNumWindows - 1][p].h,
                                            176, 0, 0, 0);
                }
            }
        }
        SetCountdownTimer(600.0f);
        break;
    case 4:
        if (!ghostPanes[0]) {
            int width; /* r0 */

            HAL_SetFont(13);
            width = HAL_GetTextWidth(gText.single[230], -1);
            width /= 2;
            ghostPanes[0] = InitPane(centeredLeft(width) - 8, 6, 160 + width + 8, 14 + TB_GetFontLineHeight(),
                                     176, 0, 0, 0);
        }
        if (!ghostPanes[1]) {
            HAL_SetFont(10);
            ghostPanes[1] = InitPane(71, 52, 249, TB_GetFontLineHeight() * 3 + 60, 176, 0, 0, 0);
        }
        if (!ghostPanes[2]) {
            HAL_SetFont(10);
            ghostPanes[2] = InitPane(31, 108, 289, TB_GetFontLineHeight() * 3 + 116, 176, 0, 0, 0);
        }
        SetCountdownTimer(600.0f);
        gGhostMenuSelect = 2;
        gGhostPak = 0;
        {
            int p; /* r27 */

            for (p = 0; p < gNumPlayers; p++) {
                if (gPlayers[p].save) {
                    PS_LockFile((*gPlayers[p].save)->fileh, 0);
                }
            }
        }
        {
            int pak; /* r27 */

            for (pak = 0; pak < 4; pak++) {
                _FindGhosts(pak);
            }
        }
        break;
    case 5:
        gTable = gRace == 2 ? 2 : 0;
        _PrintScores();
        if (!scorePanes[0]) {
            int width; /* r0 */

            HAL_SetFont(13);
            width = HAL_GetTextWidth(gText.taba[gText.tab[28] + gTable], -1);
            width /= 2;
            scorePanes[0] = InitPane(centeredLeft(width) - 8, 6, 160 + width + 8, 14 + TB_GetFontLineHeight(),
                                     176, 0, 0, 0);
        }
        if (!scorePanes[1]) {
            scorePanes[1] = InitPane(8, 56, 312, 184, 176, 0, 0, 0);
        }
        if (!scorePanes[2]) {
            int width; /* r0 */

            HAL_SetFont(11);
            width = HAL_GetTextWidth(gText.single[112], -1);
            /* `width / 2` inline here (the other two pane blocks use `width /= 2;`) gives the original register order between width and the pane address */
            scorePanes[2] = InitPane(centeredLeft(width / 2) - 8, 190, 160 + width / 2 + 8, 198 + TB_GetFontLineHeight(),
                                     176, 0, 0, 0);
        }
        if (gRace == 2) {
            SetCountdownTimer(30.0f);
        } else {
            SetCountdownTimer(15.0f);
        }
        break;
    }
    if (gHighScoreState) {
        blit = (_tBlit_2 *)NewMultiBlit(0, 0, mblit, 1);
    }
}

/* 0x80070a98, size 0x1e8 */
static void _RankScores(void)
{
    int p; /* r24 */
    int t; /* r0 */
    int s; /* r0 */

    for (p = 0; p < gNumPlayers; p++) {
        if (!(*gPlayers[p].save)->fileh) {
            gThisPlayer[p] = gLastPlayer[gPlayers[p].controller];
        } else {
            gThisPlayer[p] = gPlayers[p].save;
        }
        for (t = 0; t < 3; t++) {
            for (s = 0; s < 5; s++) {
                if (p == gNewTotalScore[t][s]) {
                    gDrawName[p] = 1;
                }
            }
        }
        if (gRace == 2 && gGhostToSave) {
            gDrawName[p] = 1;
        }
        if (gDrawName[p]) {
            if (!(*gPlayers[p].save)->fileh) {
                gDrawName[p] = 1;
                gNameSelect[p] = -1;
                if (gThisPlayer[p] == (void *)0xFFFFFFFF) {
                    memset(gScoreName[p], 0, 13);
                } else {
                    strcpy((char *)gScoreName[p], (char *)gTempNames[(int)gThisPlayer[p]]);
                }
                gNameLen[p] = strlen((char *)gScoreName[p]);
            } else {
                gDrawName[p] = 0;
                strcpy((char *)gScoreName[p], (char *)(*gPlayers[p].save)->name);
            }
        }
    }
}

/* stripped by the linker; kept only because its 0.0f literal is the unit's `@526` in .sdata2 (between
   _SetHighScoreState's 15.0f and _PrintScores's 6969.0f; animText reuses it) */
static signed short _HiScoreRank(float time, float *table)
{
    signed short i; /* r4 */
    float *score; /* r3 */
    if (time <= 0.0f) {
        return -1;
    }
    for (i = 0, score = table; i < 5; i++, score++) {
        if (time < *score) {
            return i;
        }
    }
    return -1;
}

/* stripped by the linker and leaves no data; an empty placeholder kept only at its DWARF position */
static void _AddScore(void)
{
    /* TODO */
}

/* 0x80070c80, size 0x34c */
static void _PrintScores(void)
{
    int i; /* r27 */
    float *s; /* r26 */
    anon_ll_fileh_4c ***p; /* r25 */
    float defTime; /* f31 */
    unsigned int seed; /* stack */
    int width; /* r0 */

    gThisTrack = gTrackNum + (gBackwards ? 6 : 0);
    p = gTotalNames[gThisTrack][gTable];
    s = gTotalRaceStats[gThisTrack].scores[gTable];
    defTime = gDefStartTimes[gTrackNum + (gBackwards ? 19 : 0)];
    seed = defTime * 6969.0f;
    if (gTable == 1) {
        defTime += 1.0f + (rand_r(&seed) % 2000) / 1000.0f;
    }
    for (i = 0; i < 5; i++, s++, p++) {
        if (*p == (void *)0xFFFFFFFF) {
            gScoreNames[i] = gDefaultNames[rand_r(&seed) % 58];
            defTime += (rand_r(&seed) % 1000) / 1000.0f;
            sprintf((char *)gScoreTimes[i], "%01d:%02d.%03d", (unsigned int)defTime / 60, (unsigned int)defTime % 60,
                    (unsigned int)(defTime * 1000.0f) % 1000);
        } else {
            if (*p >= (anon_ll_fileh_4c **)0 && *p <= (anon_ll_fileh_4c **)20) {
                gScoreNames[i] = gTempNames[(int)*p];
            } else {
                gScoreNames[i] = (**p)->name;
            }
            defTime = *s;
            seed = defTime;
            sprintf((char *)gScoreTimes[i], "%01d:%02d.%03d", (unsigned int)defTime / 60, (unsigned int)defTime % 60,
                    (unsigned int)(defTime * 1000.0f) % 1000);
        }
    }
    if (scorePanes[0]) {
        HAL_SetFont(13);
        width = HAL_GetTextWidth(gText.taba[gText.tab[28] + gTable], -1);
        width /= 2;
        ChangePaneSize(scorePanes[0], centeredLeft(width) - 8, 6, 160 + width + 8, 14 + TB_GetFontLineHeight());
    }
}

/* 0x80070fcc, size 0x20 */
static int _GhostChanged(int pak)
{
    return PS_PakNotTheSame();
}

/* 0x80070fec, size 0x21c */
static void _FindGhosts(int pak)
{
    anon_ll_fileh_4c *player = gGhostPlayer; /* r28 */
    anon_ll_fileh_30 **ghosth; /* r27 */
    anon_ll_fileh_30 *ghost; /* r26 */
    unsigned int existingGhostSize = 0; /* r25 */

    gPakGhosts[pak] = 0;
    if (!PS_PakInserted(pak)) {
        gGhostStatus[pak] = -1;
        return;
    }
    if (PS_PakDamaged(pak)) {
        gGhostStatus[pak] = -2;
        return;
    }
    if (!PS_PakFormatted(pak)) {
        gGhostStatus[pak] = -6;
        return;
    }
    gGhostStatus[pak] = 0;
    if (_GhostChanged(pak)) {
        gGhostStatus[pak] = 3;
        return;
    }
    if (gRace == 2) {
        if (!GhostFileAvailable(&gGhostPlayer, pak, gTrackNum, gBackwards)) {
            gGhostStatus[pak] = -5;
        }
        ghosth = gGhostFiles.head;
        while (ghosth) {
            ghost = *ghosth;
            if (ghost->fileh && (*ghost->fileh)->pak == pak && ghost->track == gTrackNum &&
                ghost->backward == gBackwards && !strcmp((char *)ghost->name, (char *)player->name) &&
                ghost->playerId[0] == player->id[0] && ghost->playerId[1] == player->id[1] &&
                (!gPakGhosts[pak] || ghost->finish > (*gPakGhosts[pak])->finish)) {
                gPakGhosts[pak] = ghosth;
                gGhostStatus[pak] = 1;
            }
            ghosth = ghost->ll.next;
        }
        if (gPakGhosts[pak]) {
            existingGhostSize = (*(*gPakGhosts[pak])->fileh)->blocks;
        }
        if (gBlocksNeeded > existingGhostSize + PS_GetFreeBlocks(pak)) {
            gPakGhosts[pak] = 0;
            gGhostStatus[pak] = -5;
        }
    }
}

/* 0x80071208, size 0x128 */
static void ControlUnlocked(void)
{
    int nextState; /* r27 */
    int p; /* r26 */

    nextState = 1;
    for (p = 0; p < gNumPlayers; p++) {
        if (gStuffUnlocked[p] == 1) {
            if ((gPlayers[p].edges & 3) || CountdownTimeOut()) {
                PlayStartABSound(gPlayers[p].edges);
                gStuffUnlocked[p] = 0;
            }
        } else if (namePanes[p]) {
            RemovePane(namePanes[p]);
            namePanes[p] = 0;
        }
    }
    for (p = 0; p < gNumPlayers; p++) {
        if (gStuffUnlocked[p] == 1) {
            nextState = 0;
        }
    }
    if (nextState == 1) {
        if (circuitCode) {
            _SetHighScoreState(2);
        } else if (gRace == 0 || gRace == 3 || gRace == 2) {
            _SetHighScoreState(3);
        } else {
            _SetHighScoreState(0);
        }
    }
}

/* 0x80071330, size 0x78 */
static void ControlCircuitCode(void)
{
    int p; /* r30 */

    for (p = 0; p < gNumPlayers; p++) {
        if ((gPlayers[p].edges & 3) || CountdownTimeOut()) {
            PlayStartABSound(gPlayers[p].edges);
            _SetHighScoreState(3);
        }
    }
}

/* 0x800713a8, size 0x588 */
static void ControlName(int p)
{
    int t; /* r0 */
    int s; /* r0 */
    if (gPlayers[p].autoEdges & 0x400) {
        PlayUDSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) {
            gNameSelect[p] = 52;
        } else if (gNameSelect[p] == -2) {
            gNameSelect[p] = 49;
        } else if (gNameSelect[p] == -3) {
            gNameSelect[p] = 46;
        } else if (gNameSelect[p] < 3) {
            gNameSelect[p] = -3;
        } else if (gNameSelect[p] < 6) {
            gNameSelect[p] = -2;
        } else if (gNameSelect[p] < 9) {
            gNameSelect[p] = -1;
        } else {
            gNameSelect[p] -= 9;
        }
    } else if (gPlayers[p].autoEdges & 0x800) {
        PlayUDSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) {
            gNameSelect[p] = 7;
        } else if (gNameSelect[p] == -2) {
            gNameSelect[p] = 4;
        } else if (gNameSelect[p] == -3) {
            gNameSelect[p] = 1;
        } else if (gNameSelect[p] < 45) {
            gNameSelect[p] += 9;
        } else if (gNameSelect[p] < 48) {
            gNameSelect[p] = -3;
        } else if (gNameSelect[p] < 51) {
            gNameSelect[p] = -2;
        } else {
            gNameSelect[p] = -1;
        }
    } else if (gPlayers[p].autoEdges & 0x1000) {
        PlayLRSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) {
            gNameSelect[p] = -2;
        } else if (gNameSelect[p] == -2) {
            gNameSelect[p] = -3;
        } else if (gNameSelect[p] == -3) {
            gNameSelect[p] = -1;
        } else if (gNameSelect[p] % 9 == 0) {
            gNameSelect[p] += 8;
        } else {
            gNameSelect[p] -= 1;
        }
    } else if (gPlayers[p].autoEdges & 0x2000) {
        PlayLRSound(gPlayers[p].autoEdges);
        if (gNameSelect[p] == -1) {
            gNameSelect[p] = -3;
        } else if (gNameSelect[p] == -2) {
            gNameSelect[p] = -1;
        } else if (gNameSelect[p] == -3) {
            gNameSelect[p] = -2;
        } else if (gNameSelect[p] % 9 == 8) {
            gNameSelect[p] -= 8;
        } else {
            gNameSelect[p] += 1;
        }
    }
    if ((gPlayers[p].edges & 3) || CountdownTimeOut()) {
        PlayStartABSound(gPlayers[p].edges);
        if ((gPlayers[p].edges & 1) || CountdownTimeOut()) {
            gNameSelect[p] = -1;
        }
        if (gNameSelect[p] == -2) {
            if (gNameLen[p] > 0) {
                gNameLen[p]--;
                gScoreName[p][gNameLen[p]] = 0;
            }
        } else if (gNameSelect[p] == -1) {
            while (gScoreName[p][gNameLen[p] - 1] == ' ') {
                gNameLen[p]--;
                gScoreName[p][gNameLen[p]] = 0;
            }
            censor((char *)gScoreName[p]);
            gDrawName[p] = 0;
            gThisPlayer[p] = gLastPlayer[gPlayers[p].controller] = (anon_ll_fileh_4c **)FindTempNameIdx((char *)gScoreName[p]);
            for (t = 0; t < 3; t++) {
                for (s = 0; s < 5; s++) {
                    if (gNewTotalScore[t][s] == p) {
                        gTotalNames[gThisTrack][t][s] = gThisPlayer[p];
                    }
                }
            }
        } else if (gNameLen[p] < 12 && (gNameSelect[p] != -3 || gNameLen[p] > 0)) {
            if (gNameSelect[p] == -3) {
                gScoreName[p][gNameLen[p]] = ' ';
            } else {
                gScoreName[p][gNameLen[p]] = gPlayerNameChars[gNameSelect[p]];
            }
            gNameLen[p]++;
            gScoreName[p][gNameLen[p]] = 0;
            if (gNameLen[p] == 12) {
                gNameSelect[p] = -1;
            }
        }
    } else if (gPlayers[p].edges & 4) {
        PlayStartABSound(gPlayers[p].edges);
        if (gNameLen[p] > 0) {
            gNameLen[p]--;
            gScoreName[p][gNameLen[p]] = 0;
        }
    }
}

/* 0x80071930, size 0x4f4 */
static void ControlGhost(void)
{
    int exitGhost; /* r31 */
    signed char delete; /* r30 */

    exitGhost = 0;
    if (gGhostMenuConflict == 1) {
        if (gActiveEdges & 7) {
            PlayStartABSound(gActiveEdges);
            gGhostMenuConflict = 0;
        }
    } else if (gGhostMenuConfirm == 1) {
        if (!PS_PakInserted(gGhostPak)) {
            PS_LockFile((*gPakGhosts[gGhostPak])->fileh, 0);
            gGhostMenuConfirm = 0;
        } else if (gActiveEdges & 0xc00) {
            PlayUDSound(gActiveEdges);
            do {
                gGhostConfirmMenuSelect += (gActiveEdges & 0x800) ? 1 : -1;
                if (gGhostConfirmMenuSelect < 0) {
                    gGhostConfirmMenuSelect = 2;
                }
                if (gGhostConfirmMenuSelect > 2) {
                    gGhostConfirmMenuSelect = 0;
                }
            } while (gGhostConfirmMenuSelect == 1 &&
                     (gBlocksNeeded > PS_GetFreeBlocks(gGhostPak) ||
                      !GhostFileAvailable(&gGhostPlayer, gGhostPak, gTrackNum, gBackwards)));
        } else if (gActiveEdges & 7) {
            delete = 0;
            PlayStartABSound(gActiveEdges);
            if ((gActiveEdges & 3) && gGhostConfirmMenuSelect != 2) {
                if (gGhostConfirmMenuSelect == 0) {
                    delete = 1;
                    if (ReadGhostData(gPakGhosts[gGhostPak])) {
                        SaveGhostInLimbo(*(*gPakGhosts[gGhostPak])->header, *(*gPakGhosts[gGhostPak])->header + 1);
                        DeleteGhost(gPakGhosts[gGhostPak]);
                    }
                }
                if (GhostFileAvailable(&gGhostPlayer, gGhostPak, gTrackNum, gBackwards)) {
                    if (SaveGhost(gGhostPak, &gGhostPlayer, gSavedGhost, 0)) {
                        PS_LockFile((*gSavedGhost)->fileh, 1);
                        gGhostMenuNotify = 1;
                        gShostSaveFail = 0;
                    } else {
                        gGhostMenuNotify = 1;
                        gShostSaveFail = 1;
                    }
                }
            }
            if (!delete) {
                PS_LockFile((*gPakGhosts[gGhostPak])->fileh, 0);
            }
            gGhostMenuConfirm = 0;
        }
    } else if (gGhostMenuNotify == 1) {
        if (gActiveEdges & 7) {
            PlayStartABSound(gActiveEdges);
            gGhostMenuNotify = 0;
            exitGhost = 1;
        }
    } else if ((gActiveLevels & 0x3000) && gGhostMenuSelect == 0) {
        if (gActiveEdges & 0x3000) {
            PlayLRSound(gActiveEdges);
            gGhostPak += (gActiveEdges & 0x2000) ? 1 : -1;
            if (gGhostPak < 0) {
                if (MtapConnected) {
                    gGhostPak = 3;
                } else {
                    gGhostPak = 1;
                }
            }
            if (MtapConnected) {
                if (gGhostPak > 3) {
                    gGhostPak = 0;
                }
            } else if (gGhostPak > 1) {
                gGhostPak = 0;
            }
        }
    } else if (gActiveEdges & 0xc00) {
        PlayUDSound(gActiveEdges);
        gGhostMenuSelect = (gActiveEdges & 0x800) ? kGhostMenuNoSave : kGhostMenuSave; /* enum names, not 3/2: with int literals the select becomes branch-free, the original branches */
    } else if (gActiveEdges & 3) {
        PlayStartABSound(gActiveEdges);
        if (gGhostMenuSelect == 2) {
            gGhostStatus[gGhostPak] = 0;
            if (gGhostStatus[gGhostPak] == 0) {
                if (SaveGhost(gGhostPak, &gGhostPlayer, gSavedGhost, 0)) {
                    PS_LockFile((*gSavedGhost)->fileh, 1);
                    gGhostMenuNotify = 1;
                } else {
                    gGhostMenuNotify = 1;
                    gShostSaveFail = 1;
                }
            } else if (gGhostStatus[gGhostPak] == 1) {
                PS_LockFile((*gPakGhosts[gGhostPak])->fileh, 1);
                gGhostMenuConfirm = 1;
                gGhostConfirmMenuSelect = 2;
                gYes = 0;
                gGhostMenuNotify = 0;
            } else if (gGhostStatus[gGhostPak] == 2) {
                gGhostMenuConflict = 1;
            } else if (gGhostStatus[gGhostPak] == 3) {
                gGhostMenuConflict = 1;
            }
        }
        if (gGhostMenuSelect == 3) {
            SaveGhost(gGhostPak, &gGhostPlayer, gSavedGhost, 1);
            gSavedGhost = 0;
            exitGhost = 1;
        }
    }
    if (exitGhost == 1) {
        _SetHighScoreState(5);
    } else {
        _FindGhosts(gGhostPak);
    }
}

/* 0x80071e24, size 0xc0 */
static void ControlScores(void)
{
    if (gRace != 2 && (gActiveEdges & 0x3000)) {
        PlayLRSound(gActiveEdges);
        gTable = !gTable;
        _PrintScores();
        SetCountdownTimer(15.0f);
    } else if ((gActiveEdges & 3) || CountdownTimeOut()) {
        if (gRace != 2 && CountdownTimeOut() && !gTable) {
            gTable = 1;
            _PrintScores();
            SetCountdownTimer(15.0f);
        } else {
            PlayStartABSound(gActiveEdges);
            _SetHighScoreState(0);
        }
    }
}

/* 0x80071ee4, size 0x970 */
static void DrawUnlocked(void)
{
    int x; /* r22 */
    int y; /* r21 */
    int i; /* r0 */
    int p; /* stack */
    unsigned char str[64]; /* stack */
    int stuffCount; /* r17 */
    HAL_SetFont(11);
    for (p = 0; p < gNumPlayers; p++) {
        if (gStuffUnlocked[p]) {
            stuffCount = 3;
            for (i = 0; i < 19; i++) {
                if (gUnlockedTracks[p][i] && !gPrevUnlockedTracks[p][i]) {
                    stuffCount++;
                }
            }
            for (i = 0; i < 13; i++) {
                if (gUnlockedCars[p][i] && !gPrevUnlockedCars[p][i]) {
                    stuffCount++;
                }
            }
            for (i = 0; i < 4; i++) {
                if (gUnlockedCircuits[p][i] && !gPrevUnlockedCircuits[p][i]) {
                    stuffCount++;
                }
            }
            for (i = 0; i < 4; i++) {
                if (gUnlockedTrans[p][i * 2] && !gPrevUnlockedTrans[p][i * 2]) {
                    stuffCount++;
                }
            }
            for (i = 0; i < 9; i++) {
                if (gUnlockedEngines[p][i] && !gPrevUnlockedEngines[p][i]) {
                    stuffCount++;
                }
            }
            for (i = 0; i < 5; i++) {
                if (gUnlockedTires[p][i] && !gPrevUnlockedTires[p][i]) {
                    stuffCount++;
                }
            }
            for (i = 0; i < 6; i++) {
                if (gUnlockedFrames[p][i] && !gPrevUnlockedFrames[p][i]) {
                    stuffCount++;
                }
            }
            x = gPlayerRects[gNumWindows - 1][p].cx;
            y = gPlayerRects[gNumWindows - 1][p].cy - stuffCount * TB_GetFontLineHeight() / 2;
            if (namePanes[p]) {
                ChangePaneSize(namePanes[p], gPlayerRects[gNumWindows - 1][p].x, y - 4,
                               gPlayerRects[gNumWindows - 1][p].x + gPlayerRects[gNumWindows - 1][p].w,
                               y + stuffCount * TB_GetFontLineHeight() + 4);
            }
            TB_SetTextColor(1);
            TB_DrawText(TB_CenterText(gText.single[59], x), y, gText.single[59]);
            y += TB_GetFontLineHeight();
            TB_DrawText(TB_CenterText(gText.single[60], x), y, gText.single[60]);
            y += TB_GetFontLineHeight() * 2;
            TB_SetTextColor(22);
            for (i = 0; i < 19; i++) {
                if (gUnlockedTracks[p][i] && !gPrevUnlockedTracks[p][i]) {
                    TB_DrawText(TB_CenterText(gTrackTitles[i], x), y, gTrackTitles[i]);
                    y += TB_GetFontLineHeight();
                }
            }
            for (i = 0; i < 13; i++) {
                if (gUnlockedCars[p][i] && !gPrevUnlockedCars[p][i]) {
                    TB_DrawText(TB_CenterText(CarTitles[i], x), y, CarTitles[i]);
                    y += TB_GetFontLineHeight();
                }
            }
            for (i = 0; i < 4; i++) {
                if (gUnlockedCircuits[p][i] && !gPrevUnlockedCircuits[p][i]) {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[45] + i], gText.single[42]);
                    TB_DrawText(TB_CenterText(str, x), y, str);
                    y += TB_GetFontLineHeight();
                }
            }
            for (i = 0; i < 4; i++) {
                if (gUnlockedTrans[p][i * 2] && !gPrevUnlockedTrans[p][i * 2]) {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[5] + i], gText.taba[gText.tab[2] + 1]);
                    TB_DrawText(TB_CenterText(str, x), y, str);
                    y += TB_GetFontLineHeight();
                }
            }
            for (i = 0; i < 9; i++) {
                if (gUnlockedEngines[p][i] && !gPrevUnlockedEngines[p][i]) {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[7] + i], gText.taba[gText.tab[2] + 3]);
                    TB_DrawText(TB_CenterText(str, x), y, str);
                    y += TB_GetFontLineHeight();
                }
            }
            for (i = 0; i < 5; i++) {
                if (gUnlockedTires[p][i] && !gPrevUnlockedTires[p][i]) {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[8] + i], gText.taba[gText.tab[2] + 4]);
                    TB_DrawText(TB_CenterText(str, x), y, str);
                    y += TB_GetFontLineHeight();
                }
            }
            for (i = 0; i < 6; i++) {
                if (gUnlockedFrames[p][i] && !gPrevUnlockedFrames[p][i]) {
                    TB_Sprintf(str, "%s %s", gText.taba[gText.tab[9] + i], gText.taba[gText.tab[2] + 5]);
                    TB_DrawText(TB_CenterText(str, x), y, str);
                    y += TB_GetFontLineHeight();
                }
            }
        }
    }
}

/* 0x80072854, size 0x144 */
static void DrawCircuitCode(void)
{
    int x; /* r25 */
    int y; /* r29 */
    int i; /* r28 */
    int length; /* r0 */
    unsigned char str[64]; /* stack */
    HAL_SetFont(11);
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(gText.single[68], 160), 82, gText.single[68]);
    TB_DrawText(TB_CenterText(gText.single[69], 160), 122, gText.single[69]);
    length = TB_Strlen(gCircuitCode) > 22 ? 22 : TB_Strlen(gCircuitCode);
    str[1] = 0;
    x = 160 - (length + 1) / 2 * 10 / 2;
    y = 97;
    for (i = 0; i < length; i++) {
        str[0] = gCircuitCode[i];
        TB_DrawText(TB_CenterText(str, x + 5), y, str);
        if ((i + 1) % ((length + 1) / 2) == 0) {
            x = 160 - (length + 1) / 2 * 10 / 2;
            y += 10;
        } else {
            x += 10;
        }
    }
}

/* 0x80072998, size 0x24c */
static void DrawName(int p)
{
    static unsigned char del[4] = "DEL"; /* .sdata del$1225 */
    static unsigned char end[4] = "END"; /* .sdata end$1226 */
    static unsigned char spc[4] = "SPC"; /* .sdata spc$1227 */
    unsigned char ltr[2]; /* stack */
    int x; /* r24 */
    int y; /* r25 */
    int i; /* r23 */
    HAL_SetFont(12);
    ltr[1] = 0;
    x = gPlayerRects[gNumWindows - 1][p].cx;
    y = gPlayerRects[gNumWindows - 1][p].y + 4;
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(gText.single[160], x), y, gText.single[160]);
    x = gPlayerRects[gNumWindows - 1][p].cx;
    y = gPlayerRects[gNumWindows - 1][p].y + 15;
    TB_SetTextColor(22);
    TB_DrawText(TB_CenterText(gScoreName[p], x), y, gScoreName[p]);
    x = gPlayerRects[gNumWindows - 1][p].cx - 48;
    y = gPlayerRects[gNumWindows - 1][p].y + 30;
    for (i = 0; i < 54; i++) {
        ltr[0] = gPlayerNameChars[i];
        TB_SetTextColor(gNameSelect[p] == i ? 22 : 1);
        TB_DrawText(TB_CenterText(ltr, x), y, ltr);
        if ((i + 1) % 9 == 0) {
            y += 10;
            x = gPlayerRects[gNumWindows - 1][p].cx - 48;
        } else {
            x += 12;
        }
    }
    x = gPlayerRects[gNumWindows - 1][p].cx - 36;
    TB_SetTextColor(gNameSelect[p] == -3 ? 22 : 16);
    TB_DrawText(TB_CenterText(spc, x), y, spc);
    TB_SetTextColor(gNameSelect[p] == -2 ? 22 : 16);
    TB_DrawText(TB_CenterText(del, x + 36), y, del);
    TB_SetTextColor(gNameSelect[p] == -1 ? 22 : 16);
    TB_DrawText(TB_CenterText(end, x + 72), y, end);
}

/* mid-file data: the target symbol is plain `memcard` (no $ suffix), i.e. file scope, emitted in .data after gPlayerRects */
static char memcard[12] = "MEMORY CARD";

/* 0x80072be4, size 0x740 */
static void DrawGhost(void)
{
    int i; /* r27 */
    int y; /* r26 */
    unsigned char *str; /* r25 */
    unsigned char str1[128]; /* stack */
    unsigned char time[16]; /* stack */
    signed short gFreePages; /* r5 */
    HAL_SetFont(13);
    TB_SetTextColor(1);
    y = 10; /* propagated away; makes y the first local referenced, which puts the final loop's str in r27 and y in r28 */
    str = gText.single[230];
    TB_DrawText(TB_CenterText(str, 160), y, str);
    HAL_SetFont(10);
    str = (*gSavedGhost)->name;
    TB_DrawText(TB_CenterText(str, 160), 56, str);
    y = TB_GetFontLineHeight() + 56;
    HAL_SetFont(2);
    TB_Sprintf(time, "%01d:%02d.%03d", (int)((*gSavedGhost)->finish / 60.0f), (int)(*gSavedGhost)->finish % 60,
               (int)((*gSavedGhost)->finish * 1000.0f) % 1000);
    TB_DrawText(TB_CenterText(time, 160), y, time);
    y += TB_GetFontLineHeight();
    HAL_SetFont(11);
    if (gGhostMenuConflict == 1) {
        HAL_SetFont(10);
        TB_Sprintf(str1, gText.single[235], (*gPakGhosts[gGhostPak])->name);
        TB_SetJust(kTB_JustCenter, kTB_JustTop);
        TB_DrawTextScrollBox(160, 112, 130, TB_GetFontLineHeight() * 6, -1, 0, str1);
        TB_SetJust(kTB_JustLeft, kTB_JustTop);
    } else if (gGhostMenuConfirm == 1) {
        HAL_SetFont(11);
        TB_SetJust(kTB_JustCenter, kTB_JustTop);
        TB_DrawText(160, 112, gText.single[232]);
        y = TB_GetFontLineHeight() + 112;
        HAL_SetFont(2);
        TB_Printf(160, y, "%01d:%02d.%03d", (int)((*gPakGhosts[gGhostPak])->finish / 60.0f),
                  (int)(*gPakGhosts[gGhostPak])->finish % 60, (int)((*gPakGhosts[gGhostPak])->finish * 1000.0f) % 1000);
        HAL_SetFont(10);
        y = TB_GetFontLineHeight() * 2 + 112;
        for (i = 0; i < 3; i++) {
            TB_SetTextColor(i == gGhostConfirmMenuSelect ? 22 : 1);
            if (i == 1 && (gBlocksNeeded > PS_GetFreeBlocks(gGhostPak) ||
                           !GhostFileAvailable(&gGhostPlayer, gGhostPak, gTrackNum, gBackwards))) {
                TB_SetTextColor(4);
            }
            TB_DrawText(160, y, gText.taba[gText.tab[48] + i]);
            y += TB_GetFontLineHeight();
        }
        TB_SetJust(kTB_JustLeft, kTB_JustTop);
    } else if (gGhostMenuNotify == 1) {
        HAL_SetFont(10);
        if (gShostSaveFail) {
            TB_Sprintf(str1, "SUCCESSFUL");
        } else {
            TB_Sprintf(str1, "SUCCESSFUL");
        }
        TB_SetJust(kTB_JustCenter, kTB_JustTop);
        TB_DrawTextScrollBox(160, 112, 130, TB_GetFontLineHeight() * 6, -1, 0, str1);
        TB_SetJust(kTB_JustLeft, kTB_JustTop);
    } else {
        HAL_SetFont(10);
        y = 112;
        for (i = 0; i < 4; i++) {
            if (i == kGhostMenuPak || i == kGhostMenuStatus) {
                continue;
            }
            TB_SetTextColor(i == gGhostMenuSelect ? 22 : 1);
            time[0] = 0;
            if (i == kGhostMenuStatus) {
                switch (gGhostStatus[gGhostPak]) {
                case -4:
                    str = gText.single[175];
                    break;
                case -3:
                    str = gText.single[179];
                    break;
                case -2:
                    str = gText.single[176];
                    break;
                case -1:
                    str = gText.single[178];
                    break;
                case -5:
                    str = gText.single[170];
                    break;
                case -6:
                    str = (unsigned char *)"UNFORMATTED";
                    break;
                case kGhostNone:
                case kGhostMine:
                case kGhostOther:
                    gFreePages = PS_GetFreeBlocks(gGhostPak);
                    if (gPakGhosts[gGhostPak]) {
                        gFreePages += (*(*gPakGhosts[gGhostPak])->fileh)->blocks;
                    }
                    str = str1;
                    TB_Sprintf(str, "%d%s%s", gFreePages, gFreePages == 1 ? gText.single[93] : gText.single[94],
                               gFreePages == 1 ? gText.single[99] : gText.single[100]);
                    break;
                case kGhostNotSame:
                    str = (unsigned char *)"PROFILE NOT FOUND.";
                    break;
                }
                if (gGhostStatus[gGhostPak] == kGhostOther) {
                    TB_SetTextColor(5);
                }
            } else {
                str = gText.taba[gText.tab[47] + i];
                if (i == kGhostMenuPak) {
                    str = (unsigned char *)memcard;
                }
            }
            if (i == kGhostMenuStatus) {
                HAL_SetFont(11);
                TB_DrawText(TB_CenterText(str, 160), y, str);
                HAL_SetFont(10);
                y += TB_GetFontLineHeight();
                if (time[0]) {
                    HAL_SetFont(2);
                    TB_DrawText(TB_CenterText(time, 160), y, time);
                    HAL_SetFont(10);
                }
            } else if (i == kGhostMenuPak) {
                TB_DrawText(85, y, str);
                if (MtapConnected) {
                    TB_DrawText(TB_RightText(gPS_PakName[gGhostPak], 250), y, gPS_PakName[gGhostPak]);
                } else {
                    if (gGhostPak < 2) {
                        TB_DrawText(TB_RightText(gPS_PakName[gGhostPak], 250), y, gPS_PakNameNoMTP[gGhostPak]);
                    } else {
                        return;
                    }
                }
            } else {
                TB_DrawText(TB_CenterText(str, 160), y, str);
            }
            TB_SetTextColor(1);
            y += TB_GetFontLineHeight();
        }
    }
}

/* 0x80073324, size 0x288 */
static void DrawScores(void)
{
    int i; /* r27 */
    int numWid; /* r0 */
    unsigned char str[16]; /* stack */
    HAL_SetFont(13);
    TB_SetTextColor(1);
    TB_DrawText(TB_CenterText(gText.taba[gText.tab[28] + gTable], 160), 10, gText.taba[gText.tab[28] + gTable]);
    for (i = 0; i < 5; i++) {
        sprintf((char *)str, "%d", i + 1);
        HAL_SetFont(5);
        TB_SetTextColor(gNewTotalScore[gTable][i] == -1 ? 1 : playerTextColor[gNewTotalScore[gTable][i]]);
        numWid = HAL_GetTextWidth(str, -1);
        TB_DrawText(16, i * 24 + 56, str);
        HAL_SetFont(11);
        TB_DrawText(numWid + 16, i * 24 + 58, gText.taba[gText.tab[57] + i]);
    }
    HAL_SetFont(10);
    for (i = 0; i < 5; i++) {
        TB_SetTextColor(gNewTotalScore[gTable][i] == -1 ? 1 : playerTextColor[gNewTotalScore[gTable][i]]);
        TB_DrawEllipseText(69, i * 24 + 60, 160, gScoreNames[i]);
    }
    HAL_SetFont(2);
    for (i = 0; i < 5; i++) {
        TB_SetTextColor(gNewTotalScore[gTable][i] == -1 ? 1 : playerTextColor[gNewTotalScore[gTable][i]]);
        TB_DrawText(TB_RightText(gScoreTimes[i], 304), i * 24 + 60, gScoreTimes[i]);
    }
    HAL_SetFont(11);
    TB_SetTextColor(22);
    TB_DrawText(TB_CenterText(gText.single[112], 160), 194, gText.single[112]);
}

/* stripped by the linker and leaves no data; an empty placeholder kept only at its DWARF position */
static int animArrowR(_tBlit_2 *blt)
{
    int conflict; /* r30 */
    int uninited; /* r29 */
    /* TODO */
}

/* stripped by the linker and leaves no data; an empty placeholder kept only at its DWARF position */
static int animArrowL(_tBlit_2 *blt)
{
    int uninited; /* r30 */
    /* TODO */
}

/* 0x800735ac, size 0xc4 */
static int animText(void)
{
    int p; /* r30 */
    HAL_SetTextDepth(0.0f);
    switch (gHighScoreState) {
    case kHighScoreUnlocked:
        DrawUnlocked();
        break;
    case kHighScoreCircuitCode:
        DrawCircuitCode();
        break;
    case kHighScoreNames:
        for (p = 0; p < gNumPlayers; p++) {
            if (gDrawName[p] == 1) {
                DrawName(p);
            }
        }
        break;
    case kHighScoreGhost:
        DrawGhost();
        break;
    case kHighScoreScores:
        DrawScores();
        break;
    }
    HAL_SetTextDepth(-1.0f);
    return 1;
}

/* 0x80073670, size 0x98 */
int censor(char *wp)
{
    char **op; /* r31 */
    char retval; /* r30 */
    char *const obscenities[22] = {
        "ASSHOLE", "ASSWHOLE", "FUCK", "PHUK", "PHUCK", "FUK", "SHIT", "CUM", "PISS", "CUNT", "PUSSY",
        "TWAT", "COCK", "NIGGER", "NIGGA", "FAGGOT", "FAG", "CHINK", "GOOK", "BEANER", "WETBACK", ""
    };

    retval = 0;
    for (op = (char **)obscenities; **op; op++) {
        retval |= fixword(*op, wp);
    }
    return retval;
}

/* 0x80073708, size 0x17c */
static int fixword(char *op, char *wp)
{
    static char *op1; /* .sbss op1$1504 */
    static char *sp; /* .sbss sp$1505 */
    static signed short cnt; /* .sbss cnt$1506 */

    op1 = op;
    for (;;) {
        if (!*wp) {
            return 0;
        }
        if (*wp == *op) {
            break;
        }
        wp++;
    }
    sp = wp;
    while (*wp == *op) {
        wp++;
    }
    while (*op && *op == op[1]) {
        op++;
    }
    for (;;) {
        while (*++op == ' ') {
        }
        if (!*op) {
            break;
        }
        cnt = 0;
        for (;;) {
            while (*wp && *wp <= ' ') {
                wp++;
            }
            if (*wp == *op) {
                break;
            }
            if (!*wp || !cnt) {
                return fixword(op1, sp + 1);
            }
            cnt--;
            wp++;
        }
        while (*++wp == *op && *op != op[1]) {
        }
    }
    while (sp < wp) {
        *sp++ = '!';
    }
    fixword(op1, wp);
    return 1;
}

